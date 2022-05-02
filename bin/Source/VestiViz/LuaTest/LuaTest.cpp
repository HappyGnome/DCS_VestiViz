#include <iostream>
#include <filesystem>

extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

int main()
{
	lua_State* L = lua_open();
	luaL_openlibs(L);

	std::filesystem::path path = std::filesystem::current_path().parent_path();
	path+="\\LuaTest\\run_pipeline.lua";

	std::filesystem::path binpath = std::filesystem::current_path().parent_path().parent_path().parent_path();

	lua_pushfstring(L, binpath.string().c_str());
	lua_setglobal(L, "BinDir");

	if (luaL_dofile(L,  path.string().c_str()) == 0) {
		std::cout << "Lua file run" << std::endl;
	}
	else { std::cout << lua_tostring(L, -1) << std::endl; }

	lua_close(L);
	return 0;
}
