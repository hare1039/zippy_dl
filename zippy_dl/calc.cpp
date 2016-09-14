#include "calc.hpp"

namespace calc
{
    enum calc_operator{ PLUS, MINUS, MUTLIPLY, DEVIDE, END };
    struct token
    {
        int num;
        calc_operator OPERATOR;
        token(){num = 0; OPERATOR = END;}
        token(int n, calc_operator op): num(n), OPERATOR(op) {}
        
    };
    
    /*
     UNTIL(tokens.front().OPERATOR == END)
     {
     std::cout << tokens.front().num << ((tokens.front().OPERATOR == PLUS)    ?  " +":
     (tokens.front().OPERATOR == MINUS)   ?  " -":
     (tokens.front().OPERATOR == MUTLIPLY)?  " *":
     (tokens.front().OPERATOR == DEVIDE)  ?  " /": " END") << std::endl;
     tokens.pop();
     }
     */
    
    int calculate(std::string raw_data)
    {
        std::queue<char> data;
        for(auto i : raw_data)
            if(i != ' ')
                data.push(i);
        
        std::queue<token> tokens;
        std::string int_part;
        while(!data.empty())
        {
            try
            {
                switch(data.front())
                {
                    case '+':
                        tokens.push(token(stoi(int_part), PLUS));
                        int_part = "";
                        break;
                        
                    case '-':
                        tokens.push(token(stoi(int_part), MINUS));
                        int_part = "";
                        break;
                        
                    case '*':
                        tokens.push(token(stoi(int_part), MUTLIPLY));
                        int_part = "";
                        break;
                        
                    case '/':
                        tokens.push(token(stoi(int_part), DEVIDE));
                        int_part = "";
                        break;
                        
                    case '(':
                    {
                        std::string next_raw_data;
                        data.pop();
                        do
                        {
                            next_raw_data += data.front();
                            data.pop();
                        }UNTIL(data.front() == ')');
                        
                        std::stringstream ss;
                        ss << calculate(next_raw_data);
                        ss >> int_part;
                        break;
                    }
                    default:
                        int_part += data.front();
                        break;
                }
            }
            catch (std::invalid_argument)
            {
                std::cerr << "'" << int_part << "' stoi invalid argument caught at string '" << raw_data << "'\n";
            }
            
            data.pop();
        }
        
        if(!int_part.empty())
            tokens.push(token(stoi(int_part), END));
        
        
        std::queue<token> next_tokens;
        do
        {
            switch(tokens.front().OPERATOR)
            {
                case MUTLIPLY:
                {
                    int num = tokens.front().num;
                    tokens.pop();
                    tokens.front().num *= num;
                    break;
                }
                case DEVIDE:
                {
                    int num = tokens.front().num;
                    tokens.pop();
                    tokens.front().num = num / tokens.front().num;
                    break;
                }
                case PLUS :
                case MINUS:
                case END  :
                    next_tokens.push(tokens.front());
                    tokens.pop();
                    break;
            }
        }UNTIL(tokens.empty());
        
        next_tokens.swap(tokens);
        do
        {
            switch(tokens.front().OPERATOR)
            {
                case MUTLIPLY:
                case DEVIDE  :
                    std::cerr << "queue left * and / undo.\n";
                    break;
                    
                case PLUS :
                {
                    int num = tokens.front().num;
                    tokens.pop();
                    tokens.front().num += num;
                    break;
                }
                case MINUS:
                {
                    int num = tokens.front().num;
                    tokens.pop();
                    tokens.front().num = num - tokens.front().num;
                    break;
                }
                case END :
                    break;
            }
            
        }UNTIL(tokens.front().OPERATOR == END || tokens.empty());
        
        return tokens.front().num;
    }
}