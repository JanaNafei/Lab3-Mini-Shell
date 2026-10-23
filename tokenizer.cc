#include "tokenizer.h"
#include <sstream>
#include <iostream>

std::vector<Token> tokenize(const std::string &input)
{
    std::vector<Token> tokens;
    std::istringstream stream(input);
    std::string word;

    bool getcommand =true;

    while (stream >> word)
    {
        Token token;

        if (word == "|")
        {   
        getcommand = true;    
        token.type = TOKEN_PIPE;
        }
        else if (word == ">")
            token.type = TOKEN_REDIRECT;
        else if (word == ">>")
            token.type = TOKEN_APPEND;
        else if (word == "<")
            token.type = TOKEN_INPUT;
        else if (word == "&")
            token.type = TOKEN_BACKGROUND;
        else if (word == "2>")
            token.type = TOKEN_ERROR;
        else if (word == ">>&")
            token.type = TOKEN_REDIRECT_AND_ERROR;
        else
            {
                bool redirection = false;
                if (!tokens.empty())
                {
                    
                    Token &prev = tokens.back();
                    if (prev.type == TOKEN_REDIRECT ||
                        prev.type == TOKEN_APPEND ||
                        prev.type == TOKEN_INPUT ||
                        prev.type == TOKEN_ERROR || 
                        prev.type == TOKEN_REDIRECT_AND_ERROR )
                    
                    {
                        redirection = true;
                    }
                }
                    if(redirection == true)
                    {
                        token.type = TOKEN_ARGUMENT;
                    }
                    else if(getcommand)
                    {
                        token.type = TOKEN_COMMAND;
                        getcommand = false;
                    }
                    else
                    {
                        token.type = TOKEN_ARGUMENT;
                    }
                
            }

        token.value = word;
        tokens.push_back(token);
    }

    tokens.push_back({TOKEN_EOF, ""});
    return tokens;
}
