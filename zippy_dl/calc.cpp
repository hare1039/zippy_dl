#include "calc.hpp"

namespace calc
{
    /*do
     {
     getline(script, dl_url);
     std::stringstream parse(dl_url);
     std::string token;
     while(parse >> token)
     {
     if(token == "var")
     {
     char name;
     std::string eq, val;
     parse >> name >> eq >> val;
     var_list[name] = stoi(val.substr(0 , val.length() - 1));
     }
     else if (token == "document.getElementById('dlbutton').href")
     {
     
     document.getElementById('dlbutton').href
     =
     "/d/lurPAz1p/"+(n + n * 2 + b)+"3/XYZ%20-%20Solution%20%28Kenton%20Slash%20Demon%27s%204-4%20Thera%29_CMP3.eu.mp3"
     
     std::string dl_url;
     getline(parse, dl_url);
     dl_url = dl_url.substr(3);
     
     std::string first_pt, calc_pt, last_pt;
     
     int pos = 0;
     for(int occur_times = 0;occur_times != 2; pos++)
     if(dl_url[pos] == '"')
     occur_times++;
     else
     first_pt += dl_url[pos];
     
     bool semi = true;
     for(int occur_times = 0 ;occur_times != 2 && semi; pos++)
     if(dl_url[pos] == ')')
     semi = false;
     else if(dl_url[pos] == '+' && !semi)
     occur_times++;
     else if(var_list.find(dl_url[pos]) != var_list.end())
     calc_pt += itos(var_list[dl_url[pos]]);
     else
     calc_pt += dl_url[pos];
     
     calc_pt = calc_pt.substr(1);
     calc_pt += ')';
     
     pos++;
     for(int occur_times = 0;occur_times != 2; pos++)
     if(dl_url[pos] == '"')
     occur_times++;
     else
     last_pt += dl_url[pos];
     
     
     zippy_file_url.append(zippy_url.substr(0,zippy_url.find('/', 8)))// 8 is to aovid 'http://' <<-- this
     .append(first_pt)
     .append(itos(calc::calculate(calc_pt)))
     .append(last_pt);
     
     break;
     }
     }
     }UNTIL(dl_url.find("document.getElementById('dlbutton').href") != std::string::npos);*/
    
    
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
