//
//  tinyjs_bridge.hpp
//  zippy_dl
//
//  Created by JetHare on 2016/10/2.
//  2016 JetHare.
//

#ifndef tinyjs_bridge_hpp
#define tinyjs_bridge_hpp

#include <iostream>
namespace mujs_bridge
{
    std::string js_get_url(std::string js_script, std::string var_name);
}
#ifdef  __cplusplus
extern  "C" {
#endif
#include "mujs.h"
#include <string.h>
    void mujs(const char* js_script, const char* var_name);
    void get_val(js_State *J);

#ifdef  __cplusplus
}
#endif
#endif /* tinyjs_bridge_hpp */
