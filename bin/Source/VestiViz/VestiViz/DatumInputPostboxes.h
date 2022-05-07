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

static bool PopTopVec3(lua_State* L, std::array<double, 3>& output) {
    int toPop = 1;
    bool success = false;
    if (lua_istable(L, -1)) {
        lua_getfield(L, -1, "x");
        lua_getfield(L, -2, "y");
        lua_getfield(L, -3, "z");
        toPop += 3;

        if (lua_isnumber(L, -1) && lua_isnumber(L, -2) && lua_isnumber(L, -3)) {
            output[0] = lua_tonumber(L, -3);
            output[1] = lua_tonumber(L, -2);
            output[2] = lua_tonumber(L, -1);
            success = true;
        }
    }
    lua_pop(L, toPop);
    return success;
}

template<std::size_t N>
static bool PopArray(lua_State* L, std::array<double, N>& output) {
    int toPop = 1;
    bool success = true;
    if (lua_istable(L, -1)) {

        for (int std::size_t = 0; success && i < N; i++) {
            lua_pushnumber(L, i+1);
            lua_gettable(L, -2);
            if (lua_isnumber(L, -1)) {
                output[i] = lua_tonumber(L, -1);
            }
            else success = false;
            lua_pop(L,1);
        }
    }
    lua_pop(L, toPop);
    return success;
}

template<std::size_t N>
static bool PopTupleArray(lua_State* L, std::array<std::tuple<std::size_t, std::size_t>, N>& output) {
    int toPop = 1;
    bool success = true;
    if (lua_istable(L, -1)) {

        for (int std::size_t = 0; success && i < N; i++) {
            lua_pushnumber(L, i + 1);
            lua_gettable(L, -2);
            std::size_t first, second;

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
    lua_pop(L, toPop);
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
        TimedDatum<S, DatumArr<S, S, 3>> value();
        value.t = lua_tonumber(L, stackOffset + 1);

        std::array<S, 3> p;

        lua_getfield(L, stackOffset + 2, "p");
        if (!PopTopVec3(L, p)) return 0;

        
        value.datum = DatumArr<S, S, 3>(std::move(p));

         mWrapped->addDatum(value);
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
        TimedDatum<S, DatumArr<S, S, 6>> value();
        value.t = lua_tonumber(L, stackOffset + 1);

        std::array<S, 3> x,y;

        lua_getfield(L, stackOffset + 2, "x");
        if (!PopTopVec3(L, x)) return 0;
        lua_getfield(L, stackOffset + 2, "y");
        if (!PopTopVec3(L, y)) return 0;

        value.datum = DatumArr<S, S, 6>(x[0], x[1], x[2],y[0], y[1], y[2]);

        mWrapped->addDatum(value);

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
        TimedDatum<S, DatumMatrix<S, 3, 3>> value();
        value.t = lua_tonumber(L, stackOffset + 1);

        std::array<S, 3> x, y, z;

        lua_getfield(L, stackOffset + 2, "x");
        if (!PopTopVec3(L, x)) return 0;
        lua_getfield(L, stackOffset + 2, "y");
        if (!PopTopVec3(L, y)) return 0;
        lua_getfield(L, stackOffset + 2, "z");
        if (!PopTopVec3(L, z)) return 0;

        value.datum = DatumMatrix<S, 3, 3>(
            x[0], x[1], x[2],
            y[0], y[1], y[2],
            z[0], z[1], z[2]);

        mWrapped->addDatum(value);

        return 0;
    }
};

template<typename S>
class DIPW_woff : public DatumInputPostboxWrapper {
    std::shared_ptr<SimplePostbox<TimedDatum<S, DatumArr<S, S, 8>>>> mWrapped;
public:
    explicit DIPW_woff(std::shared_ptr<SimplePostbox<TimedDatum<S, DatumArr<S, S, 8>>>> toWrap) :mWrapped(toWrap) {};

    int TryReadFromLua(lua_State* L, int stackOffset) final {
        if (mWrapped == nullptr)return 0;

        TimedDatum<S, DatumArr<S, S, 8>> value();

        if (!lua_isnumber(L, stackOffset + 1)) return 0;
        if (!lua_istable(L, stackOffset + 2)) return 0;

        value.t = lua_tonumber(L, stackOffset + 1);
        value.datum = DatumArr<S, S, 8>();

        lua_getfield(L, stackOffset + 2,"w");
        lua_getfield(L, stackOffset + 2, "off");

        bool success = true;
        if (lua_istable(L, -2) && lua_istable(L, -1)) {
            for (int i = -2; i < 0; i++) {
                lua_getfield(L, i, "top");
                if (lua_isnumber(L, -1)) value.datum[8 + 4 * i] = lua_tonumber(L, -1);
                else success = false;
                lua_pop(L, 1);

                lua_getfield(L, i, "right");
                if (lua_isnumber(L, -1)) value.datum[9 + 4 * i] = lua_tonumber(L, -1);
                else success = false;
                lua_pop(L, 1);

                lua_getfield(L, i, "bottom");
                if (lua_isnumber(L, -1)) value.datum[10 + 4 * i] = lua_tonumber(L, -1);
                else success = false;
                lua_pop(L, 1);

                lua_getfield(L, i, "left");
                if (lua_isnumber(L, -1)) value.datum[11 + 4 * i] = lua_tonumber(L, -1);
                else success = false;
                lua_pop(L, 1);
            }
        }
        else success = false;
        lua_pop(L, 2);

        if (success) {
            mWrapped->addDatum(value);
        }

        return 0;
    }
};

#endif