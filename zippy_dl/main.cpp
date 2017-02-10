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
#include <sys/types.h>
#include <pwd.h>


#include <curl/curl.h>

#define SUCCESS 0
#define FAILED -1
#define UNTIL(x) while(!(x))


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
        if (fgets(buffer, 150, pipe.get()) != NULL)
            result += buffer;

    return result;
}

class wget_options
{
    public:
    std::string dir;
}opt;

int dl_zippy(std::string zippy_page_url)
{

    std::string buffer;
    CURL *handle = curl_easy_init();

    if(!handle)
        throw std::runtime_error("curl_easy_init() failed!");


    curl_easy_setopt(handle, CURLOPT_URL, zippy_page_url.c_str());
    curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, writer);
    curl_easy_setopt(handle, CURLOPT_WRITEDATA, &buffer);
    curl_easy_setopt(handle, CURLOPT_COOKIEFILE, "");
    curl_easy_setopt(handle, CURLOPT_VERBOSE, 0);
    curl_easy_setopt(handle, CURLOPT_USERAGENT, "Mozilla/5.0 Gecko/20100101 Firefox/49.0");

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
        //std::cerr << "no JSESSIONID found, strange\n";
    }

    curl_easy_cleanup(handle);

    unsigned long script_pos_start = buffer.find("<script type=\"text/javascript\">", buffer.find("dlbutton")) +
                                                  strlen("<script type=\"text/javascript\">") + 1;
    unsigned long script_pos_end   = buffer.find("</script>", script_pos_start);

    std::string dl_url, zippy_file_url;

    std::string name = "/tmp/zippy_dl_url_" + std::to_string(std::hash<std::thread::id>()(std::this_thread::get_id()));
    std::ofstream ofs(name + ".html");
    ofs << "<html><head><a id='dlbutton' href='#'><div class='download'></div></a><script>" << buffer.substr(script_pos_start, script_pos_end - script_pos_start) << "</script></head></html>" << std::endl;
    ofs.close();
    ofs.clear();
    ofs.open(name + ".js");
    ofs << "var page = require('webpage').create(); page.open('" << name << ".html', function(status) {"
        << "var ua = page.evaluate(function() { return document.getElementById('dlbutton').href;});"
        << "console.log(ua);phantom.exit();});"
        << std::endl;
    ofs.close();

    zippy_file_url.append(zippy_page_url.substr(0, zippy_page_url.find('/', 8)))// 8 is to aovid 'http://' <<-- this
                  .append(exec(("phantomjs " + name + ".js; " + "exit 0;").c_str()).substr(7) );
    std::remove((name + ".js").c_str()); // delete file
    std::remove((name + ".html").c_str());
    exec( ("wget " + zippy_file_url + " --directory-prefix='" + opt.dir + "'" + " --referer='" + zippy_page_url + "' --cookies=off --header \"" + zippy_cookie + "\" --user-agent='Mozilla/5.0 Gecko/20100101 Firefox/50.0' 2>/dev/null && exit 0;").c_str() );

    return SUCCESS;
}


int main(int argc, char *argv[])
{
    
    extern char *optarg;
    extern int optind, opterr, optopt;
    int option_code;
    std::vector<std::string> zippy_url;
    opt.dir = "./";
    while ((option_code = getopt(argc, argv, "hl:")) != -1)
    {
        switch(option_code)
        {
            case 'h':
                std::cout << "usage:\n"
                          << "  zippy_dl [options] zippy-urls\n"
                          << "options:\n"
                          << "  -l string    -- read from list\n";
                break;
            case 'p':
                opt.dir = optarg;
                break;

            case 'l':
            {
                std::ifstream ifs(optarg);
                if(!ifs.is_open())
                {
                    std::cerr << optarg <<" can't open\n";
                    exit(-1);
                }
                for(std::string s; ifs >> s;)
                    zippy_url.push_back(s);
                ifs.close();
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
