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

//#include <jsapi.h>
//#include <js/Initialization.h>

#include <curl/curl.h>

#define SUCCESS 0
#define FAILED -1
#define UNTIL(x) while(!(x))

// spidermonkey
/*
JSRuntime * runtime;
bool spidermonkey_init()
{
    atexit([]{JS_DestroyRuntime(runtime);});
    atexit(JS_ShutDown);
    return JS_Init() && [&]{runtime = JS_NewRuntime(8192 * 16); return runtime != NULL ;}();
}*/


// curl
size_t writer(char *data, size_t size, size_t nmemb, std::string *writer_data)
{
    if (writer_data == NULL)
        return 0;
    
    writer_data->append(data, size*nmemb);
    return size * nmemb;
}

std::string exec(const char* cmd)
{
    char buffer[150];
    std::string result = "";
    std::shared_ptr<FILE> pipe(popen(cmd, "r"), pclose);
    if (!pipe) throw std::runtime_error("popen() failed!");
    while (!feof(pipe.get()))
    {
        if (fgets(buffer, 150, pipe.get()) != NULL)
            result += buffer;
    }
    return result;
}



int dl_zippy(std::string zippy_page_url, std::string jsapp)
{
    std::string buffer;
    CURL *handle = curl_easy_init();
    
    if(!handle)
        return FAILED;
    
    //auto itos = [](int i){std::stringstream ss; ss << i; std::string s; ss >> s; return s;};
    
    curl_easy_setopt(handle, CURLOPT_URL, zippy_page_url.c_str());
    curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, writer);
    curl_easy_setopt(handle, CURLOPT_WRITEDATA, &buffer);
    curl_easy_setopt(handle, CURLOPT_COOKIEFILE, "");
    curl_easy_setopt(handle, CURLOPT_VERBOSE, 1);
    curl_easy_setopt(handle, CURLOPT_USERAGENT, "Mozilla/5.0 Gecko/20100101 Firefox/46.0");
    
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
        std::cout << "no JSESSIONID found, strange\n";
    }
    
    curl_easy_cleanup(handle);
    
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
            line = "console.log(" + line.substr(eq, line.length() - eq - 1) + "); \n  quit();\n";
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
    }UNTIL(line.find("console") != std::string::npos);
    
    std::ofstream ofs("zippy_dl_url.js");
    ofs << script << std::endl;
    ofs.close();
    zippy_file_url.append(zippy_page_url.substr(0, zippy_page_url.find('/', 8)))// 8 is to aovid 'http://' <<-- this
                  .append(exec([&]{ return jsapp + " zippy_dl_url.js";}().c_str()));
    std::remove("zippy_dl_url.js"); // delete file
    
    
    
    //JSContext * context = JS_NewContext(runtime, 8192);
    //JS::CompileOptions options(context);
    
    //char16_t* script_sender = static_cast<char16_t *>(js_malloc(sizeof(char16_t) * script.length())); <<< cause problem
    //for(int i=0; i<script.length(); i++)
        //script_sender[i] = script[i];
    //JS::SourceBufferHolder buf(script_sender, script.length(), JS::SourceBufferHolder::GiveOwnership);
    
    //JS::Evaluate(context, options, buf, val);
    
    //JS_free(context, script_sender);
    //JS_DestroyContext(context);
    
    
    system([&](){return
        "wget " + zippy_file_url + " --referer='" + zippy_page_url + "' --cookies=off --header \"" + zippy_cookie +
        "\" --user-agent='Mozilla/5.0 Gecko/20100101 Firefox/47.0'";}().c_str());
    
    return SUCCESS;
}


int main(int argc, char *argv[])
{
    //if(! spidermonkey_init()) exit(1);
    std::vector<std::string> zippy_url;
    for(int i=1; i<argc; i++)
        zippy_url.push_back(std::string(argv[i]));
    
    std::vector<std::thread *> sync_dl;
    for(auto i : zippy_url)
        sync_dl.push_back(new std::thread(dl_zippy, i, "js"));
    
    for(auto &i : sync_dl)
    {
        i->join();
        delete i;
    }
    
    return 0;
}

