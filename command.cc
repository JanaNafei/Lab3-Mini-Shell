
/*
 * 
 *
 * Template file.
 * You will need to add more code here to execute the command table.
 *
 * NOTE: You are responsible for fixing any bugs this code may have!
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <time.h>
#include <glob.h>
#include <wordexp.h>
#include <vector>
#include <string>
#include <cctype>
#include <iostream>
#include <sstream>
#include "command.h"
#include "tokenizer.h"
#include <cerrno>

bool validate(std::vector<Token> &tokens)
{
    if(tokens.empty() )
    {
        return false;
    }
    if(tokens.back().type != TOKEN_EOF)
    {
        std::cerr << "Error: Missing EOF token at the end of the input." << std::endl;
        return false;
    }
    if(tokens.size()==1)
    {
        return true;
    }
    bool flag=false;
    
    for(size_t i=0;i<tokens.size()-1;i++)
    {
        if (tokens[i].type == TOKEN_REDIRECT ||
        tokens[i].type == TOKEN_APPEND ||
        tokens[i].type == TOKEN_INPUT ||
        tokens[i].type == TOKEN_ERROR ||
        tokens[i].type == TOKEN_REDIRECT_AND_ERROR)
        {
        if (!flag)
                {
            std::cerr << "Error: Command must come before redirection."
                      << std::endl;
            return false ;
                }
        }


        if(tokens[i].type ==TOKEN_COMMAND)
        {
            flag=true;
        }
        else if(tokens[i].type ==TOKEN_PIPE)
        {
            if(!flag)
            {
                std::cerr << "Error: Pipe token must be preceded by a command and can not be followed by another pipe." << std::endl;
                return false;
            }
            if(i+1>=tokens.size() || tokens[i+1].type != TOKEN_COMMAND)
            {
                std::cerr << "Error: Pipe token must be followed by a command." << std::endl;
                return false;
            }
            flag=false;
        }
        if(tokens[i].type == TOKEN_REDIRECT || tokens[i].type == TOKEN_APPEND || tokens[i].type == TOKEN_INPUT || tokens[i].type == TOKEN_ERROR || tokens[i].type == TOKEN_REDIRECT_AND_ERROR)
        {
            if(i+1>=tokens.size() || tokens[i+1].type != TOKEN_ARGUMENT)
            {
                std::cerr << "Error: Redirect token must be followed by an argument." << std::endl;
                return false;
            }
        }
        if (tokens[i].type == TOKEN_BACKGROUND)
        {
            if (i != tokens.size() - 2)
            {
                std::cerr << "Error: '&' must appear at the end of the command."
                        << std::endl;
                return false;
            }
            for (size_t j = 0; j < i; j++)
            {
                if (tokens[j].type == TOKEN_BACKGROUND)
                {
                    std::cerr << "Error: Only one & is allowed." << std::endl;
                    return false;
                }
            }
        }

    }
    if(!flag)
    {
        std::cerr << "Error: Missing command." << std::endl;
        return false;
    }
    return true;
}


void parse(std::vector<Token> &tokens)
{

    if(tokens.empty() || tokens[0].type == TOKEN_EOF)
    {
        return;
    }
    if(!validate(tokens))
    {
        return;
    }

    Command currentCommand;
    SimpleCommand *currentSimpleCommand = new SimpleCommand();

    
    /*
      Implement this function 
      you must parse for all possible tokens scenario
      right now command.execute only print the format , you will inject it in your logic.

    */
        for (size_t i=0; i<tokens.size();i++)
        {    
            if(tokens[i].type == TOKEN_COMMAND || tokens[i].type == TOKEN_ARGUMENT)
            {
                char *currentargument = strdup(tokens[i].value.c_str());
                currentSimpleCommand -> insertArgument(currentargument);
            }
        
            else if(tokens[i].type == TOKEN_REDIRECT)
            {
                if(i+1 < tokens.size() && tokens[i+1].type == TOKEN_ARGUMENT)
                {
                    char *currentargument = strdup(tokens[i+1].value.c_str());
                    currentCommand._outFile = currentargument;
                    i++;
                }
                else 
                {
                    std::cerr << "Error: Expected argument after redirect token." << std::endl;
                }
            }
            else if(tokens[i].type ==TOKEN_APPEND)
            {
                if(i+1 < tokens.size() && tokens[i+1].type == TOKEN_ARGUMENT)
                {
                    char *currentargument = strdup(tokens[i+1].value.c_str());
                    currentCommand._append = 1;
                    currentCommand._outFile = currentargument;
                    i++;
                }
                else 
                {
                    std::cerr << "Error: Expected argument after redirect token." << std::endl;
                }
            }
            else if(tokens[i].type ==TOKEN_INPUT)
            {
                if(i+1 < tokens.size() && tokens[i+1].type == TOKEN_ARGUMENT)
                {
                    char *currentargument = strdup(tokens[i+1].value.c_str());
                    
                    currentCommand._inputFile = currentargument;
                    i++;
                }
                else 
                {
                    std::cerr << "Error: Expected argument after redirect token." << std::endl;
                }
            }
            else if(tokens[i].type ==TOKEN_ERROR)
            {
                if(i+1 < tokens.size() && tokens[i+1].type == TOKEN_ARGUMENT)
                {
                    char *currentargument = strdup(tokens[i+1].value.c_str());
                    
                    currentCommand._errFile = currentargument;
                    i++;
                }
                else 
                {
                    std::cerr << "Error: Expected argument after redirect token." << std::endl;
                }
            }
            else if(tokens[i].type ==TOKEN_REDIRECT_AND_ERROR)
            {
                if(i+1 < tokens.size() && tokens[i+1].type == TOKEN_ARGUMENT)
                {
                    char *currentargument = strdup(tokens[i+1].value.c_str());
                    currentCommand._out_error = 1;
                    currentCommand._errFile = currentargument;
                    currentCommand._append = 1;
                    i++;
                }
                else 
                {
                    std::cerr << "Error: Expected argument after redirect token." << std::endl;
                }
            }
            else if(tokens[i].type ==TOKEN_BACKGROUND)
            {
                currentCommand._background =1 ;
            }
            else if(tokens[i].type == TOKEN_PIPE)
            {
                currentCommand.insertSimpleCommand(currentSimpleCommand);
                currentSimpleCommand = new SimpleCommand();
            }
            else if(tokens[i].type == TOKEN_EOF)
            {
                currentCommand.insertSimpleCommand(currentSimpleCommand);
                break;
            }
        }

        //currentCommand.print();
//currentCommand.clear();
//free(currentCommand._simpleCommands);

   currentCommand.execute();
}

SimpleCommand::SimpleCommand()
{
    _numberOfAvailableArguments = 5;
    _numberOfArguments = 0;
    _arguments = (char **)malloc(_numberOfAvailableArguments * sizeof(char *));
}

void SimpleCommand::insertArgument(char *argument)
{
    if (_numberOfAvailableArguments == _numberOfArguments + 1)
    {
        _numberOfAvailableArguments *= 2;
        _arguments = (char **)realloc(_arguments,
                                      _numberOfAvailableArguments * sizeof(char *));
    }

    _arguments[_numberOfArguments] = argument;
    _arguments[_numberOfArguments + 1] = NULL;

    _numberOfArguments++;
}

Command::Command()
{
    _numberOfAvailableSimpleCommands = 1;
    _numberOfSimpleCommands = 0;
    _simpleCommands = (SimpleCommand **)
        malloc(_numberOfAvailableSimpleCommands * sizeof(SimpleCommand *));

    
    _outFile = NULL;
    _inputFile = NULL;
    _errFile = NULL;
    _background = 0;
    _append =0;
    _out_error =0;
}

void Command::insertSimpleCommand(SimpleCommand *simpleCommand)
{
    if (_numberOfAvailableSimpleCommands == _numberOfSimpleCommands)
    {
        _numberOfAvailableSimpleCommands *= 2;
        _simpleCommands = (SimpleCommand **)realloc(_simpleCommands,
                                                    _numberOfAvailableSimpleCommands * sizeof(SimpleCommand *));
    }

    _simpleCommands[_numberOfSimpleCommands] = simpleCommand;
    _numberOfSimpleCommands++;
}

void Command::clear()
{
    for (int i = 0; i < _numberOfSimpleCommands; i++)
    {
        for (int j = 0; j < _simpleCommands[i]->_numberOfArguments; j++)
        {
            free(_simpleCommands[i]->_arguments[j]);
        }

        free(_simpleCommands[i]->_arguments);
        delete _simpleCommands[i];
    }

    if (_outFile)
    {
        free(_outFile);
    }

    if (_inputFile)
    {
        free(_inputFile);
    }

    if (_errFile)
    {
        free(_errFile);
    }

    _numberOfSimpleCommands = 0;
    _outFile = NULL;
    _inputFile = NULL;
    _errFile = NULL;
    _background = 0;
    _append= 0;
    _out_error= 0;
}

void Command::print()
{
    printf("\n\n");
    printf("              COMMAND TABLE                \n");
    printf("\n");
    printf("  #   Simple Commands\n");
    printf("  --- ----------------------------------------------------------\n");

    for (int i = 0; i < _numberOfSimpleCommands; i++)
    {
        printf("  %-3d ", i);
        for (int j = 0; j < _simpleCommands[i]->_numberOfArguments; j++)
        {
            printf("\"%s\" \t", _simpleCommands[i]->_arguments[j]);
        }
    }

    printf("\n\n");
    printf("  Output       Input        Error        Err&Out       Background\n");
    printf("  ------------ ------------ ------------ ------------ ------------\n");
    printf("  %-12s %-12s %-12s %-12s %-12s\n", _outFile ? _outFile : "default",
           _inputFile ? _inputFile : "default", _errFile ? _errFile : "default", _out_error == 1 ? _errFile : "default"
           ,_background ? "YES" : "NO");
    printf("\n\n");
}

void Command::execute()
{
    print();
    /*
        Write your code here
    */
   
   if(_numberOfSimpleCommands == 0 || _simpleCommands == NULL || _simpleCommands[0] == NULL)
   {
   std::cerr << "Error: Empty command." << std::endl;
   clear();
   free(_simpleCommands);
   _simpleCommands=NULL;
   return;
   }
   SimpleCommand *current = _simpleCommands[0];
   if (current->_numberOfArguments == 0 ||
        current->_arguments[0] == NULL)
    {
        std::cerr << "Error: Empty command." << std::endl;
        clear();
        free(_simpleCommands);
        _simpleCommands = NULL;
        return;
    }

    pid_t pid = fork();

    //error 
     if (pid < 0)
    {
        perror("fork");

        clear();
        free(_simpleCommands);
        _simpleCommands = NULL;
        return;
    }

    //child
    if(pid == 0)
    { 
        if(_outFile != NULL)
        {
            int fd;
            if(_append)
            {
                 fd = open(_outFile,O_WRONLY | O_CREAT | O_APPEND, 0644);
            }
            else{
                fd = open(_outFile,O_WRONLY | O_CREAT | O_TRUNC, 0644);
            }
            if (fd == -1)
            {
                perror("open");
                _exit(1);
            }
            if(dup2(fd,1)==-1)
            {
                perror("dup2");
                _exit(1);
            }
            close(fd);
            
        }
        if(_inputFile != NULL)
        {
            int fd = open(_inputFile,O_RDONLY);
            if (fd == -1)
            {
                perror("open");
                _exit(1);
            }
            if(dup2(fd,0)==-1)
            {
                perror("dup2");
                _exit(1);
            }
            close(fd);
        }
        
        if (_out_error == 1)
        {
            int fd = open(_errFile,O_WRONLY | O_CREAT | O_APPEND, 0644);
            if (fd == -1)
            {
                perror("open");
                _exit(1);
            }
            if(dup2(fd,1)==-1)
            {
                perror("dup2");
                _exit(1);
            }
            if(dup2(fd,2)==-1)
            {
                perror("dup2");
                _exit(1);
            }
            close(fd);

        }
        else if(_errFile != NULL) //must be after ou_error because it is a special case of error redirection and i placed else if so that it doesnot do both
        {
            int fd = open(_errFile,O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd == -1)
            {
                perror("open");
                _exit(1);
            }
            if(dup2(fd,2)==-1)
            {
                perror("dup2");
                _exit(1);
            }
            close(fd);
        }
        execvp(current->_arguments[0], current->_arguments);
        perror(current->_arguments[0]);
        exit(127);
    }

    //parent
    int status_child;
    while(waitpid(pid, &status_child,0) == -1)
    {
            if(errno == EINTR)
            {
                continue;
            }

            perror("waitpid");
            break;
    }

    clear();
    free(_simpleCommands);
    _simpleCommands = NULL;
    prompt();
}

void Command::prompt()
{
    printf("myshell>");
    fflush(stdout);
    std::string input;
    while (true)
    {
        std::getline(std::cin, input); 
        std::vector<Token> tokens = tokenize(input);
        parse(tokens);
    }
}

Command Command::_currentCommand;
SimpleCommand *Command::_currentSimpleCommand;

int main()
{
    Command::_currentCommand.prompt();
    return 0;
}
