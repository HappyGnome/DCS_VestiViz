#pragma once
#ifndef _DATUMINPUTPOSTBOXWRAPPER_H_
#define _DATUMINPUTPOSTBOXWRAPPER_H_

extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

#include<memory>

class DatumInputPostboxWrapper {
public:
	virtual ~DatumInputPostboxWrapper() = default;

	virtual int TryReadFromLua(lua_State* L, int stackOffset) = 0;
};

#endif