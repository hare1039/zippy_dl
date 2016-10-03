
project(mujs)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -pedantic -Wall -Wextra -Wno-unused-parameter")

add_library(mujs STATIC jserror.c jsfunction.c jsgc.c jsintern.c jslex.c jsmath.c jsnumber.c jsobject.c json.c jsparse.c jsproperty.c  jsregexp.c jsrun.c jsstate.c jsstring.c jsvalue.c utf.c utftype.c regexp.c)
