#include <iostream>
#include <sstream>
#include <vector>
#include <thread>
#include <algorithm>
#include <map>
#include <cstdlib>
#include <cstdio>
#include <curl/curl.h>
#include "calc.hpp"

#define SUCCESS 0
#define FAILED -1

size_t writer(char *data, size_t size, size_t nmemb, std::string *writer_data)
{
    if (writer_data == NULL)
        return 0;
    
    writer_data->append(data, size*nmemb);
    return size * nmemb;
}

int dl_zippy(std::string zippy_url)
{
    std::string buffer;
    CURL *handle = curl_easy_init();
    
    if(!handle)
        return FAILED;
    
    auto itos = [](int i){std::stringstream ss; ss << i; std::string s; ss >> s; return s;};
    
    curl_easy_setopt(handle, CURLOPT_URL, zippy_url.c_str());
    curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, writer);
    curl_easy_setopt(handle, CURLOPT_WRITEDATA, &buffer);
    curl_easy_setopt(handle, CURLOPT_COOKIEFILE, "");
    curl_easy_setopt(handle, CURLOPT_VERBOSE, 1);
    curl_easy_setopt(handle, CURLOPT_USERAGENT, "Mozilla/5.0 Gecko/20100101 Firefox/47.0");
    
    CURLcode res = curl_easy_perform(handle);
    if(res != CURLE_OK)
        std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
    
    curl_slist *cookies, *cur;
    curl_easy_getinfo(handle, CURLINFO_COOKIELIST, &cookies);
    cur = cookies;
    std::string zippy_cookie = "Cookie: JSESSIONID=";
    while(cur)
    {
        std::stringstream ss(cur->data);
        cur = cur->next;
        
        std::string s;
        while(ss >> s)
            if(s == "JSESSIONID")
            {
                ss >> s;
                zippy_cookie += s;
                cur = NULL;
                break;
            }
    }
    
    curl_easy_cleanup(handle);
    
    unsigned long script_pos_start = buffer.find("<script type=\"text/javascript\">", buffer.find("dlbutton")) +
    [](){return std::string("<script type=\"text/javascript\">").length();}() + 1;
    unsigned long script_pos_end   = buffer.find("</script>", script_pos_start);
    
    std::string dl_url, zippy_file_url;
    std::map<char, int> var_list;
    std::stringstream script(buffer.substr(script_pos_start, script_pos_end - script_pos_start + 9));
    do
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
                /*
                 document.getElementById('dlbutton').href
                 =
                 "/d/lurPAz1p/"+(n + n * 2 + b)+"3/XYZ%20-%20Solution%20%28Kenton%20Slash%20Demon%27s%204-4%20Thera%29_CMP3.eu.mp3"
                 */
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
    }UNTIL(dl_url.find("document.getElementById('dlbutton').href") != std::string::npos);
    
    system([&](){return
        "wget " + zippy_file_url + " --referer='" + zippy_url + "' --cookies=off --header \"" + zippy_cookie +
        "\" --user-agent='Mozilla/5.0 Gecko/20100101 Firefox/47.0'";}().c_str());
    
    return SUCCESS;
}

int main(int argc, char *argv[])
{
    std::vector<std::string> zippy_url;
    for(int i=1; i<argc; i++)
        zippy_url.push_back(std::string(argv[i]));
    
    std::vector<std::thread *> sync_dl;
    for(auto i : zippy_url)
        sync_dl.push_back(new std::thread(dl_zippy, i));
    
    for(auto &i : sync_dl)
    {
        i->join();
        delete i;
    }
    
    return 0;
}

