#pragma once
#ifndef _DATUMOUTPUTPOSTBOXWRAPPER_H_
#define _DATUMOUTPUTPOSTBOXWRAPPER_H_

extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

#include<memory>


class DatumOutputPostboxWrapper {
public:
	virtual ~DatumOutputPostboxWrapper() = default;

	virtual int WriteToLua(lua_State* L) = 0;
};

#endif