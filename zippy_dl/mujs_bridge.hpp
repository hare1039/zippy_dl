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
extern "C"
{
    #include "mujs.h"
}

namespace mujs_bridge
{
    std::string js_get_url(std::string js_script, std::string var_name);
}

#endif /* tinyjs_bridge_hpp */
