// dllmain.cpp : Defines the entry point for the DLL application.
#include "VestivizPipeline.h"

extern "C" {
    #include "lua.h"
    #include "lauxlib.h"
    #include "lualib.h"
}

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

static VestivizPipeline<double>* GetPipelineUpVal(lua_State* L) {
    if (!lua_islightuserdata(L, lua_upvalueindex(1))) return nullptr;
    return (VestivizPipeline<double>*)lua_touserdata(L, lua_upvalueindex(1));
}

// void F(handle, t, {p,x,y,z})
static int l_Pipeline_AddDatum(lua_State* L) {
    auto pipeline = GetPipelineUpVal(L);
    if (pipeline != nullptr) {
        if (!lua_isnumber(L, 1)) return 0;
        if (!lua_istable(L, 2)) return 0;

        double t = lua_tonumber(L, 1);

        std::array<double, 3> p, x, y, z;

        lua_getfield(L, 2, "p");
        if (!PopTopVec3(L, p)) return 0;
        lua_getfield(L, 2, "x");
        if (!PopTopVec3(L, x)) return 0;
        lua_getfield(L, 2, "y");
        if (!PopTopVec3(L, y)) return 0;
        lua_getfield(L, 2, "z");
        if (!PopTopVec3(L, z)) return 0;

        pipeline->addDatum(t,
            DatumMatrix<double, 3, 3>(x[0], x[1], x[2],
                y[0], y[1], y[2],
                z[0], z[1], z[2]),
            DatumArr<double, double, 3>(p[0], p[1], p[2]));
    }
    return 0;
}
//{t,w = {top,right,bottom,left}, off = {top,right,bottom,left}} F(handle)
static int l_Pipeline_GetDatum(lua_State* L) {
    auto pipeline = GetPipelineUpVal(L);

    if (pipeline != nullptr) {

        auto val = pipeline->getDatum();
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
    return 0;
}

static int l_Pipeline_Delete(lua_State* L) {

    VestivizPipeline<double> *p = GetPipelineUpVal(L);
    if (p != nullptr) {
        p->stopPipeline();
        delete p;
    }

    return 0;
}

static int l_Pipeline_Start(lua_State* L) {

    VestivizPipeline<double>* p = GetPipelineUpVal(L);
    if (p != nullptr) {
        p->startPipeline();
    }

    return 0;
}

static int l_Pipeline_Stop(lua_State* L) {

    VestivizPipeline<double>* p = GetPipelineUpVal(L);
    if (p != nullptr) {
        p->stopPipeline();
    }
    return 0;
}

static int l_Pipeline_New(lua_State* L) {

    auto pNew = new VestivizPipeline<double>();
    pNew->init();

    lua_createtable(L, 0, 5);
    lua_newuserdata(L, 1);
    lua_createtable(L, 0, 1);
    lua_pushlightuserdata(L, pNew);
    lua_pushcclosure(L, l_Pipeline_Delete, 1);
    lua_setfield(L, -2, "__gc");
    lua_setmetatable(L, -2);
    lua_setfield(L, -2, "destructor");
    lua_pushlightuserdata(L, pNew);
    lua_pushcclosure(L, l_Pipeline_AddDatum, 1);
    lua_setfield(L, -2, "addDatum");
    lua_pushlightuserdata(L, pNew);
    lua_pushcclosure(L, l_Pipeline_GetDatum, 1);
    lua_setfield(L, -2, "getDatum");
    lua_pushlightuserdata(L, pNew);
    lua_pushcclosure(L, l_Pipeline_Start, 1);
    lua_setfield(L, -2, "start");
    lua_pushlightuserdata(L, pNew);
    lua_pushcclosure(L, l_Pipeline_Stop, 1);
    lua_setfield(L, -2, "stop");

    return 1;
}

extern "C"  int luaopen_vestiviz(lua_State* L) {
    static const luaL_Reg VestiViz_Index[] = {
          {"newPipeline", l_Pipeline_New},
          {nullptr, nullptr}  /* end */
    };
    luaL_register(L, "vestiviz", VestiViz_Index);

    return 1;
}

