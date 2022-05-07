#pragma once
#ifndef _DATUMOUTPUTPOSTBOXES_H_
#define _DATUMOUTPUTPOSTBOXES_H_

extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

#include<memory>
#include "DatumOutputPostboxWrapper.h"
#include "PostboxBase.h"
#include "SimplePostbox.h"
#include"TimedDatum.h"
#include"DatumArr.h"

template<typename S>
class DOPW_woff : public DatumOutputPostboxWrapper {
	std::shared_ptr<SimplePostbox<TimedDatum<S, DatumArr<S, S, 8>>>> mWrapped;
public:
	explicit DOPW_woff(std::shared_ptr<SimplePostbox<TimedDatum<S, DatumArr<S, S, 8>>>> toWrap) :mWrapped(toWrap) {};

	int WriteToLua(lua_State* L) final{
        auto val = TimedDatum<S, DatumArr<S, S, 8>>::zero();

        if (mWrapped != nullptr) {
            mWrapped->flushPost();
            if (!mWrapped->empty()) val = *(mWrapped->output().cbegin());
        }

        lua_newtable(L);
        lua_pushnumber(L, val.t);
        lua_setfield(L, -2, "t");

        lua_newtable(L);
        lua_pushnumber(L, val.datum[0]);
        lua_setfield(L, -2, "top");
        lua_pushnumber(L, val.datum[1]);
        lua_setfield(L, -2, "right");
        lua_pushnumber(L, val.datum[2]);
        lua_setfield(L, -2, "bottom");
        lua_pushnumber(L, val.datum[3]);
        lua_setfield(L, -2, "left");
        lua_setfield(L, -2, "w");

        lua_newtable(L);
        lua_pushnumber(L, val.datum[4]);
        lua_setfield(L, -2, "top");
        lua_pushnumber(L, val.datum[5]);
        lua_setfield(L, -2, "right");
        lua_pushnumber(L, val.datum[6]);
        lua_setfield(L, -2, "bottom");
        lua_pushnumber(L, val.datum[7]);
        lua_setfield(L, -2, "left");
        lua_setfield(L, -2, "off");

        return 1;
	}
};

#endif