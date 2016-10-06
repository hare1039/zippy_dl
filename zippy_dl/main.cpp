#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <thread>
#include <algorithm>
#include <stdexcept>
#include <cstdlib>
#include <cstdio>
#include <cstring>

#include <unistd.h>
#include <curl/curl.h>

#include "mujs_bridge.hpp"
#define SUCCESS 0
#define FAILED -1
#define UNTIL(x) while(!(x))


// curl get page
size_t writer_get_page(char *data, size_t size, size_t nmemb, std::string *writer_data)
{
    if (writer_data == NULL)
        return 0;
    
    writer_data->append(data, size*nmemb);
    return size * nmemb;
}

// curl get file
size_t writer_get_file(char *data, size_t size, size_t nmemb, std::string *writer_data)
{
    if (writer_data == NULL)
        return 0;
    
    writer_data->append(data, size*nmemb);
    return size * nmemb;
}

int dl_zippy(std::string zippy_page_url)
{
    std::string buffer;
    CURL *handle = curl_easy_init();
    
    if(!handle)
        return FAILED;
    
    
    curl_easy_setopt(handle, CURLOPT_URL, zippy_page_url.c_str());
    curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, writer_get_page);
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
        {
            if(s == "JSESSIONID")
            {
                ss >> s;
                zippy_cookie += s;
                cur = NULL;
                break;
            }
        }
        
        std::cerr << "no JSESSIONID found, strange\n";
    }
    
    curl_easy_reset(handle);
    
    unsigned long script_pos_start = buffer.find("<script type=\"text/javascript\">", buffer.find("dlbutton")) +
                                                  strlen("<script type=\"text/javascript\">") + 1;
    unsigned long script_pos_end   = buffer.find("</script>", script_pos_start);
    
    std::string dl_url, zippy_file_url;
    std::stringstream script_parser(buffer.substr(script_pos_start, script_pos_end - script_pos_start));
    
    std::string line, script;
    do
    {
        std::getline(script_parser, line);
        if(line.find("    document") != std::string::npos && line.find("dlbutton") == std::string::npos)
        {
            std::string line_tmp = "var ";
            for(char &i : line){
                if(i != '(' && i != ')' && i != '\'' && i != '.' && i != '-') line_tmp += i;
            }
            line = line_tmp;
        }
        else if(line.find("dlbutton") != std::string::npos)
        {
            int eq = 0;
            try{
                UNTIL(line.at(eq++) == '=');
            } catch (const std::out_of_range& oor){
                std::cerr << "something strange occured: " << oor.what() << '\n';
            }
            line = "var result_ = " + line.substr(eq, line.length() - eq - 1) + "; \n ";
        }
        else if(line.find("document") != std::string::npos)
        {
            std::string line_tmp;
            for(char &i : line){
                if(i != '(' && i != ')' && i != '\'' && i != '.' && i != '-') line_tmp += i;
            }
            line = line_tmp;
        }
        script.append(line + "\n");
    }UNTIL(line.find("result_") != std::string::npos);
    
    
    zippy_file_url.append(zippy_page_url.substr(0, zippy_page_url.find('/', 8)))// 8 is to aovid 'http://' <<-- this
                  .append(mujs_bridge::js_get_url(script, "result_"));
    
    curl_easy_setopt(handle, CURLOPT_URL, zippy_file_url.c_str());
    curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, writer_get_page);
    curl_easy_setopt(handle, CURLOPT_WRITEDATA, &buffer);
    curl_easy_setopt(handle, CURLOPT_COOKIEFILE, "");
    curl_easy_setopt(handle, CURLOPT_VERBOSE, 1);
    curl_easy_setopt(handle, CURLOPT_USERAGENT, "Mozilla/5.0 Gecko/20100101 Firefox/47.0");
    
    
    
    curl_easy_cleanup(handle);
    system([&](){return
        "wget " + zippy_file_url + " --referer='" + zippy_page_url + "' --cookies=off --header \"" + zippy_cookie +
        "\" --user-agent='Mozilla/5.0 Gecko/20100101 Firefox/47.0'";}().c_str());
    
    
    
    return SUCCESS;
}


int main(int argc, char *argv[])
{
    std::srand(static_cast<unsigned int> (std::time(0)));
    extern char *optarg;
    extern int optind, opterr, optopt;
    int option_code;
    
    std::vector<std::string> zippy_url;
    while ((option_code = getopt(argc, argv, "hl:")) != -1)
    {
        switch(option_code)
        {
            case 'h':
                std::cout << "usage:\n"
                          << "  zippy_dl [options] URL [URLs...]\n"
                          << "options:\n"
                          << "  -l string    -- read from list\n";
                break;
                
            case 'l':
            {
                std::ifstream ifs(optarg);
                if(!ifs.is_open())
                {
                    std::cerr << optarg <<" can't open\n";
                    exit(-1);
                }
                std::string s;
                while(ifs >> s)
                    zippy_url.push_back(s);
            }
            case ':':
                std::cout << optopt << " without filename\n";
                break;
                
            case '?':
                std::cerr << "unknown arg '" << optopt << "', please use -h to display help\n";
                break;
        }
    }
    for(;optind<argc; optind++)
        zippy_url.push_back(std::string(argv[optind]));
    
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

