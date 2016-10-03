//
//  tinyjs_bridge.cpp
//  zippy_dl
//
//  Created by JetHare on 2016/10/2.
//  2016 JetHare. 
//

#include "mujs_bridge.hpp"


namespace mujs_bridge
{
    std::string val_return;
    void get_val(js_State *J)
    {
        const char *name = js_tostring(J, 1);
        val_return = name;
        js_pushundefined(J);
    }
    
    
    std::string js_get_url(std::string js_script, std::string var_name)
    {
        js_State *J;
        J = js_newstate(NULL, NULL, JS_STRICT);
        
        js_dostring(J, js_script.c_str());
        js_newcfunction(J, get_val, "getval", 1);
        js_setglobal(J, "getval");
        
        js_dostring(J, [&]{ return "getval(" + var_name + ");";}().c_str());
        js_pushundefined(J);
        js_freestate(J);
        
        return val_return;
    }
}
    
