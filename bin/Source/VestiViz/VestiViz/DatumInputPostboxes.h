#pragma once
#ifndef _DATUMINPUTPOSTBOXES_H_
#define _DATUMINPUTPOSTBOXES_H_

extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

#include<memory>
#include "DatumInputPostboxWrapper.h"
#include"TimedDatum.h"
#include"DatumArr.h"
#include"DatumMatrix.h"

static bool ReadVec3(lua_State* L, int tblIndex, std::array<double, 3>& output, bool pop1 = false) {
    bool success = true;
    if (lua_istable(L, tblIndex)) {
        lua_getfield(L, tblIndex, "x");
        if (lua_isnumber(L, -1)) {
            output[0] = lua_tonumber(L, -1);
        }
        else success = false;
        lua_pop(L, 1);

        lua_getfield(L, tblIndex, "y");
        if (lua_isnumber(L, -1)) {
            output[1] = lua_tonumber(L, -1);
        }
        else success = false;
        lua_pop(L, 1);

        lua_getfield(L, tblIndex, "z");
        if (lua_isnumber(L, -1)) {
            output[2] = lua_tonumber(L, -1);
        }
        else success = false;
        lua_pop(L, 1);
    }
    else success = false;

    if (pop1) lua_pop(L, 1);

    return success;
}

template<std::size_t N>
static bool ReadArray(lua_State* L, int tblIndex, std::array<double, N>& output, bool pop1 = false) {
    bool success = true;
    int getTableIndex = tblIndex;
    if (tblIndex < 0) getTableIndex--;
    if (lua_istable(L, tblIndex)) {

        for (std::size_t i = 0; success && i < N; i++) {
            lua_pushnumber(L, i+1);
            lua_gettable(L, getTableIndex);
            if (lua_isnumber(L, -1)) {
                output[i] = lua_tonumber(L, -1);
            }
            else success = false;
            lua_pop(L,1);
        }
    }
    else success = false;
    if (pop1) lua_pop(L, 1);
    return success;
}

static bool ReadVector(lua_State* L, int tblIndex, std::vector<double>& output, bool pop1 = false) {
    bool success = true;

    int getTableIndex = tblIndex;
    if (tblIndex < 0) getTableIndex--;
    if (lua_istable(L, tblIndex)) {
        int i = 0;
        output.clear();
        bool loop = true;
        while (loop) {
            i++;
            lua_pushnumber(L, i);
            lua_gettable(L, getTableIndex);
            if (lua_isnumber(L, -1)) {
                output.push_back(lua_tonumber(L, -1));
            }
            else loop = false;
            lua_pop(L, 1);
        }
    }
    else success = false;
    if (pop1) lua_pop(L, 1);
    return success;
}

template<std::size_t N>
static bool ReadTupleArray(lua_State* L, int tblIndex, std::array<std::tuple<std::size_t, std::size_t>, N>& output, bool pop1 = false) {
    bool success = true;

    int getTableIndex = tblIndex;
    if (tblIndex < 0) getTableIndex--;
    if (lua_istable(L, tblIndex)) {

        for (std::size_t i = 0; success && i < N; i++) {
            lua_pushnumber(L, lua_Number(i + 1));
            lua_gettable(L, getTableIndex);

            if (lua_istable(L, -1) ) {
                lua_pushnumber(L, 1);
                lua_gettable(L, -2);
                lua_pushnumber(L, 2);
                lua_gettable(L, -3);

                if (lua_isnumber(L, -1) && lua_isnumber(L, -2)) {
                    output[i] =  std::tuple<std::size_t, std::size_t>(lua_tonumber(L, -2),lua_tonumber(L, -1));
                }
                lua_pop(L, 2);
            }
            else success = false;
            lua_pop(L, 1);
        }
    }
    else success = false;
    if (pop1) lua_pop(L, 1);
    return success;
}

static bool PopTopTRBL(lua_State* L, std::array<double, 4>& output) {
    int toPop = 1;
    bool success = false;
    if (lua_istable(L, -1)) {
        lua_getfield(L, -1, "top");
        lua_getfield(L, -2, "right");
        lua_getfield(L, -3, "bottom");
        lua_getfield(L, -4, "left");
        toPop += 4;

        if (lua_isnumber(L, -1) && lua_isnumber(L, -2) && lua_isnumber(L, -3) && lua_isnumber(L, -4)) {
            output[0] = lua_tonumber(L, -4);
            output[1] = lua_tonumber(L, -3);
            output[2] = lua_tonumber(L, -2);
            output[3] = lua_tonumber(L, -1);
            success = true;
        }
    }
    lua_pop(L, toPop);
    return success;
}

template<typename S>
static bool ReadWOff(lua_State* L, std::size_t stackOffset, DatumArr<S, S, 8>& output, bool pop1 = false) {
    std::array<double, 4> w;
    std::array<double, 4> off;
    lua_getfield(L, stackOffset, "w");
    bool success = PopTopTRBL(L, w);
    if (success) {
        lua_getfield(L, stackOffset, "off");
        success = PopTopTRBL(L, off);
    }

    for (int i = 0; i < 4; i++) {
        output[i] = w[i];
        output[i+4] = off[i];
    }  
    if (pop1) lua_pop(L, 1);
    return success;
}

template<typename S>
class DIPW_point : public DatumInputPostboxWrapper {
	std::shared_ptr<PostboxInputBase<TimedDatum<S, DatumArr<S, S, 3>>>> mWrapped;
public:
	explicit DIPW_point(std::shared_ptr<PostboxInputBase<TimedDatum<S, DatumArr<S, S, 3>>>> toWrap) :mWrapped(toWrap) {};
    
	int TryReadFromLua(lua_State* L, int stackOffset) final {
		if (mWrapped == nullptr)return 0;

        if (!lua_isnumber(L, stackOffset + 1)) return 0;
        if (!lua_istable(L, stackOffset + 2)) return 0;
        TimedDatum<S, DatumArr<S, S, 3>> value;
        value.t = lua_tonumber(L, stackOffset + 1);

        std::array<S, 3> p;

        lua_getfield(L, stackOffset + 2, "p");
        if (!ReadVec3(L,-1, p,true)) return 0;

        
        value.datum = DatumArr<S, S, 3>(std::move(p));

        mWrapped->addDatum(std::move(value));
         return 0;
	}
};

template<typename S>
class DIPW_xy : public DatumInputPostboxWrapper {
    std::shared_ptr<PostboxInputBase<TimedDatum<S, DatumArr<S, S, 6>>>> mWrapped;
public:
    explicit DIPW_xy(std::shared_ptr<PostboxInputBase<TimedDatum<S, DatumArr<S, S, 6>>>> toWrap) :mWrapped(toWrap) {};

    int TryReadFromLua(lua_State* L, int stackOffset) final {
        if (mWrapped == nullptr)return 0;

        if (!lua_isnumber(L, stackOffset + 1)) return 0;
        if (!lua_istable(L, stackOffset + 2)) return 0;
        TimedDatum<S, DatumArr<S, S, 6>> value;
        value.t = lua_tonumber(L, stackOffset + 1);

        std::array<S, 3> x,y;

        lua_getfield(L, stackOffset + 2, "x");
        if (!ReadVec3(L, -1,x,true)) return 0;
        lua_getfield(L, stackOffset + 2, "y");
        if (!ReadVec3(L, -1,y,true)) return 0;

        value.datum = DatumArr<S, S, 6>(x[0], x[1], x[2],y[0], y[1], y[2]);

        
        mWrapped->addDatum(std::move(value));
        return 0;
    }
};

template<typename S>
class DIPW_frame : public DatumInputPostboxWrapper {
    std::shared_ptr<PostboxInputBase<TimedDatum<S, DatumMatrix<S, 3, 3>>>> mWrapped;
public:
    explicit DIPW_frame(std::shared_ptr<PostboxInputBase<TimedDatum<S, DatumMatrix<S, 3, 3>>>> toWrap) :mWrapped(toWrap) {};

    int TryReadFromLua(lua_State* L, int stackOffset) final {
        if (mWrapped == nullptr)return 0;
        if (!lua_isnumber(L, stackOffset + 1)) return 0;
        if (!lua_istable(L, stackOffset + 2)) return 0;
        TimedDatum<S, DatumMatrix<S, 3, 3>> value;
        value.t = lua_tonumber(L, stackOffset + 1);

        std::array<S, 3> x, y, z;

        lua_getfield(L, stackOffset + 2, "x");
        if (!ReadVec3(L, -1,x, true)) return 0;
        lua_getfield(L, stackOffset + 2, "y");
        if (!ReadVec3(L, -1, y, true)) return 0;
        lua_getfield(L, stackOffset + 2, "z");
        if (!ReadVec3(L, -1, z, true)) return 0;

        value.datum = DatumMatrix<S, 3, 3>(
            x[0], x[1], x[2],
            y[0], y[1], y[2],
            z[0], z[1], z[2]);

        mWrapped->addDatum(std::move(value));
        return 0;
    }
};

template<typename S>
class DIPW_woff : public DatumInputPostboxWrapper {
    std::shared_ptr<PostboxInputBase<TimedDatum<S, DatumArr<S, S, 8>>>> mWrapped;
public:
    explicit DIPW_woff(std::shared_ptr<PostboxInputBase<TimedDatum<S, DatumArr<S, S, 8>>>> toWrap) :mWrapped(toWrap) {};

    int TryReadFromLua(lua_State* L, int stackOffset) final {
        if (mWrapped == nullptr)return 0;
        TimedDatum<S, DatumArr<S, S, 8>> value;

        if (!lua_isnumber(L, stackOffset + 1)) return 0;
        if (!lua_istable(L, stackOffset + 2)) return 0;

        value.t = lua_tonumber(L, stackOffset + 1);
        value.datum = DatumArr<S, S, 8>();

        if (ReadWOff(L, stackOffset+2, value.datum)) {
            mWrapped->addDatum(value);
        }
        return 0;
    }
};

#endif