// Students   : Zachary Wilson, Richard McDaniel, Alex White
// Assignment : CS390 Spring 2015 Project
// Date       : 3/28/15


#include <iostream>
#include <fstream>
#include <cstdlib>
#include <vector>
#include <unistd.h>

using namespace std;

bool checkSize(string command);
void error();

/* Global Variables */

string command = "";
string prompt = "mysh> ";
string version  = "mysh version 1.0 Copyright 2014 by Zachary Wilson, Richard McDaniel, Alex White";
string filename = "";
bool append = false;

//COMMANDS AVAILABLE
//chpr = Change Prompt : allow user to change the prompt output
//ver  = Show Shell Version
//exit = Exit the program

//I am also checking the size of the entered command and holding it under 255 chars


/*
This function outputs a line of text to a file or standard output.
@param string line - a line of text to output
@returns nothing
*/
void output(string line)
{
    if (filename == "")
    {
        cout << line << endl;
    }
    else
    {
        ofstream fout;
        if (append)
        {
            fout.open(filename.c_str(), std::ofstream::app);
        }
        else
        {
            fout.open(filename.c_str());
        }
        fout << line << endl;
        fout.close();
    }
}

/*
This function executes a command and outputs the result to a file or STDOUT.
@returns nothing
*/
int execute(string command)
{
    int pipes[3] = {0};
    
    char* argv[256];    //TODO
    // argv[0] = "ls";
    // argv[1] = NULL;
    
    int pid;
    
    int in[2];
    int out[2];
    int err[2];
    
    
    pipe(in);
    pipe(out);
    pipe(err);
    
    pid = fork();
    
    if (pid > 0)
    {
        close(in[0]);
        close(out[1]);
        close(err[1]);
        
        pipes[0] = in[1];
        pipes[1] = out[0];
        pipes[2] = err[0];
    }
    else
    {
        close(in[1]);
        close(out[0]);
        close(err[0]);
        
        if (pipes[1] != 0)
        {
            in[0] = pipes[1];
        }
        
        dup2(in[0], STDIN_FILENO);
        dup2(out[1], STDOUT_FILENO);
        dup2(err[1], STDERR_FILENO);
        
        execvp(argv[0], argv);
        exit(1);
    }
    
    string result = "";
    char nChar;
    
    while (read(pipes[1], &nChar, 1) == 1) {
        result += nChar;
    }
    
    output(result);
    
    close(pipes[0]);
    close(pipes[1]);
    close(pipes[2]);
    
    return pid;
}

/*
This function parses a command line and created an argv array.
It also sets the global string filename and gobal bools redirect and append.
@returns nothing
*/
void parse(string line)
{
    bool quoting = false;
    bool redirecting = false;
    bool appending = false;
    
    vector<string> argv;
    string word = "";
    
    filename = "";
    append = false;
    line += " ";
    
    for (string::size_type i = 0; i < line.size(); ++i) {
        switch (line[i])
        {
            case ' ':
                if (quoting)
                {
                    word += line[i];
                }
                else
                {
                    if ((word != "") && (word != " ")) {
                        if (redirecting || appending)
                        {
                            append = appending;
                            filename = word;
                            redirecting = false;
                            appending = false;
                        }
                        else
                        {
                            argv.push_back(word);
                        }
                    }
                    word = "";
                }
                break;
            
            case '>':
                if (redirecting)
                {
                    redirecting = false;
                    appending = true;
                    word = "";
                }
                else
                {
                    redirecting = true;
                    if ((word != "") && (word != " ")) {
                        argv.push_back(word);
                    }
                    word = "";
                }
                break;
            
            case '"':
                quoting = !quoting;
                break;
            
            default:
                word += line[i];
        }
    }
    
    if (argv[0] == "chpr")
    {
        prompt = argv[1];
        output("Prompt changed to : [" + prompt + "] sucessfully");
    }
    else if (argv[0] == "ver")
    {
        output("Shell Version : " + version);
    }
    else if (argv[0] == "pwd")
    {
        char cwd[255];
        getcwd(cwd, 255);
        string s(cwd);
        output(s);
    }
    else if (argv[0] == "cd")
    {
        if (argv.size() > 1)
        {
            chdir(argv[1].c_str());            
        }
        else
        {
            chdir(getenv("HOME"));
        }
    }
    else
    {
        if(argv[0] != "exit")
        {
            error();
        }
    }
}

int main()
{
    cout << "Welcome to the mysh shell !!! " << endl;
    
    command = "ver";
    
    while (command != "exit")
    {
        cout << prompt;
        getline(cin, command);
        bool in_bounds = checkSize(command);
        
        if (in_bounds)
        {
            parse(command);
        }
        else
        {
            error();
        }
    }
    
    output("Exiting Shell...");
    
    return 0;
}

bool checkSize(string command)
{
    int std_size = 255;
    
    if (command.size() <= std_size)
    {
        return true;
    }
    else
    {
        return false;
    }
}

void error()
{
    output("mysh: Error detected");
}