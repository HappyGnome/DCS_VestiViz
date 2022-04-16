// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"

extern "C" {
    #include "lua.h"
    #include "lauxlib.h"
    #include "lualib.h"
}

static int l_foo(lua_State* L) {
    lua_pushstring(L, "Hi from C++");
    return 1;
}

extern "C"  int luaopen_vestiviz(lua_State* L) {
    static const luaL_Reg VestiViz_Index[] = {
          {"Foo", l_foo},
          {nullptr, nullptr}  /* end */
    };
    luaL_register(L, "vestiviz", VestiViz_Index);
    return 1;
}

