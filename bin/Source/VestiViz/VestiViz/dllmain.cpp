// dllmain.cpp : Defines the entry point for the DLL application.
#include "VestivizPipeline.h"

extern "C" {
    #include "lua.h"
    #include "lauxlib.h"
    #include "lualib.h"
}

class PipelineManager{
    std::vector<std::shared_ptr<VestivizPipeline<double>>> mPipelines;
    std::mutex mPipelinesMutex;
public:
    std::size_t AddPipeline(std::shared_ptr<VestivizPipeline<double>>value) {
        std::lock_guard<std::mutex> lock(mPipelinesMutex);
        mPipelines.push_back(value);
        return mPipelines.size() - 1;
    }

    std::shared_ptr<VestivizPipeline<double>> GetPipeline(std::size_t index) {
        std::lock_guard<std::mutex> lock(mPipelinesMutex);
        if (index >= mPipelines.size()) return nullptr;
        return mPipelines[index];
    }

    std::shared_ptr<VestivizPipeline<double>> GetPipelineArg(lua_State* L) {
        if (lua_isnumber(L, 1)) {
            auto index = (std::size_t)lua_tonumber(L, 1);
            return PipelineManager::GetPipeline(index);
        }
        return nullptr;
    }
};

PipelineManager gPipelineManager;

static bool PopTopVec3(lua_State* L, std::array<double, 3>& output) {
    int toPop = 1;
    bool success = false;
    if (lua_istable(L, -1)) {
        lua_getfield(L, -1, "x");
        lua_getfield(L, -2, "y");
        lua_getfield(L, -3, "z");
        toPop+=3;

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

static int l_StartPipeline(lua_State* L) {
    auto pNew = std::make_shared<VestivizPipeline<double>>();    
    lua_pushinteger(L, gPipelineManager.AddPipeline(pNew));
    pNew->init();
    pNew->startPipeline();
    return 1;
}

static int l_StopPipeline(lua_State* L) {
    auto pipeline = gPipelineManager.GetPipelineArg(L);
    if (pipeline != nullptr) pipeline->stopPipeline();
    return 0;
}

// void F(handle, t, {p,x,y,z})
static int l_AddDatum(lua_State* L) {
    auto pipeline = gPipelineManager.GetPipelineArg(L);
    if (pipeline != nullptr) {
        if (!lua_isnumber(L, 2)) return 0;
        if (!lua_istable(L, 3)) return 0;

        double t = lua_tonumber(L, 2);

        std::array<double, 3> p, x, y, z;

        lua_getfield(L, 3, "p");
        if (!PopTopVec3(L,p)) return 0;
        lua_getfield(L, 3, "x");
        if (!PopTopVec3(L, x)) return 0;
        lua_getfield(L, 3, "y");
        if (!PopTopVec3(L, y)) return 0;
        lua_getfield(L, 3, "z");
        if (!PopTopVec3(L, z)) return 0;

        pipeline->addDatum(t,
            DatumMatrix<double,3,3>(x[0], x[1], x[2],
                                    y[0], y[1], y[2], 
                                    z[0], z[1], z[2]), 
            DatumArr<double, double, 3> (p[0],p[1],p[2]));
    }
    return 0;
}
//{t,w = {top,right,bottom,left}, off = {top,right,bottom,left}} F(handle)
static int l_GetDatum(lua_State* L) {
    auto pipeline = gPipelineManager.GetPipelineArg(L);
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

extern "C"  int luaopen_vestiviz(lua_State* L) {
    static const luaL_Reg VestiViz_Index[] = {
          {"Start", l_StartPipeline},
          {"Stop", l_StopPipeline},
          {"AddDatum", l_AddDatum},
          {"GetDatum", l_GetDatum},
          {nullptr, nullptr}  /* end */
    };
    luaL_register(L, "vestiviz", VestiViz_Index);
    return 1;
}

