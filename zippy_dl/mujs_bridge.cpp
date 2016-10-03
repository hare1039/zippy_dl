//
//  tinyjs_bridge.cpp
//  zippy_dl
//
//  Created by JetHare on 2016/10/2.
//  2016 JetHare. 
//

#include "mujs_bridge.hpp"
const char * val_return;
namespace mujs_bridge
{
    std::string js_get_url(std::string js_script, std::string var_name)
    {
        mujs(js_script.c_str(), var_name.c_str());
        return std::string(val_return);
    }
}

void get_val(js_State *J)
{
    const char *name = js_tostring(J, 1);
    val_return = name;
    js_pushundefined(J);
}

void mujs(const char* js_script, const char* var_name)
{
    js_State *J = js_newstate(NULL, NULL, JS_STRICT);
    
    js_dostring(J, js_script);
    js_newcfunction(J, get_val, "getval", 1);
    js_setglobal(J, "getval");
    char sender[300];
    memset(sender, '\0', 300);
    strcat(sender, "getval(");
    strcat(sender, var_name);
    strcat(sender, ");");
    js_dostring(J, sender);
    js_pushundefined(J);
    js_freestate(J);
}


    
