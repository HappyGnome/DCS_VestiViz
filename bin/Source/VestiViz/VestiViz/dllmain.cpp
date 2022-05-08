// dllmain.cpp : Defines the entry point for the DLL application.
#include "VestivizPipeline.h"

extern "C" {
    #include "lua.h"
    #include "lauxlib.h"
    #include "lualib.h"
}



extern "C"  int luaopen_vestiviz(lua_State* L) {
    static const luaL_Reg VestiViz_Index[] = {
          {"newPipeline", VestivizPipeline<double>::l_Pipeline_New},
          {nullptr, nullptr}  /* end */
    };
    luaL_register(L, "vestiviz", VestiViz_Index);

    return 1;
}

