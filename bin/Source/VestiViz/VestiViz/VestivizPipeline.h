#pragma once

#ifndef _VESTIVIZPIPELINE_H_
#define _VESTIVIZPIPELINE_H_

#include "AccelByRegressionFilterAction.h"
#include "DynMatMultFilterAction.h"
#include "StatAddFilterAction.h"
#include "QuickCompressFilterAction.h"
#include "ExpDecayFilterAction.h"
#include "StatMatMultFilterAction.h"
#include "SimpleDiffFilterAction.h"
#include "DynMatMultPickFilterAction.h"
#include "ConvolveFilterAction.h"
#include "LinCombFilterAction.h"
#include "LogFilterAction.h"
#include "PipelineBase.h"
#include "SignScaleFilterAction.h"
#include "DatumMatrix.h"
#include "PIB_Wrapper.h"
#include "DatumInputPostboxes.h"
#include "DatumOutputPostboxes.h"

template<typename S>
class VestivizPipeline : public PipelineBase<PIB_Wrapper> {

	std::shared_ptr<ErrorStack> mErrors;

	std::vector<std::shared_ptr<DatumInputPostboxWrapper>> mLuaInputs;

	std::vector <std::shared_ptr<DatumOutputPostboxWrapper>> mLuaOutputs;

	static void log(VestivizPipeline* p, const std::exception& e) {
		if (p!=nullptr && p->mErrors != nullptr) {
			p->mErrors->push_exception(e);
		}
	}
	static void log(VestivizPipeline* p, const std::string& msg) {
		if (p != nullptr && p->mErrors != nullptr) {
			p->mErrors->push_message(msg);
		}
	}
	//Static Lua helper methods

	static VestivizPipeline* GetPipelineUpVal(lua_State* L) {
		if (!lua_islightuserdata(L, lua_upvalueindex(1))) return nullptr;
		return (VestivizPipeline*)lua_topointer(L, lua_upvalueindex(1));
	}

	// Args: filterHandle (or nil for new), internal leaf handles to connect new action to .. ( x N & nil for new input)
	//return: filterhandle, new internal leaf handle, input handles added to the filter...
	template<std::size_t N>
	static int addAction_lua(lua_State* L, std::shared_ptr<FilterActionBase<PIB_Wrapper>> action, int stackOffset) {

		VestivizPipeline* p = GetPipelineUpVal(L);
		if (p == nullptr) return 0;

		std::size_t leaf = NEW_LEAF_FILTER;
		if (lua_isnumber(L, stackOffset + 1)) leaf
			= lua_tointeger(L, stackOffset + 1);
		bool newInput = leaf == NEW_LEAF_FILTER;

		std::vector<std::size_t> internalLeaves;
		for (int i = 0; i < N; i++) {
			if (lua_isnumber(L, stackOffset + 2 + i)) {
				int n = lua_tonumber(L, stackOffset + 2 + i);
				internalLeaves.push_back( n < 0 ? MultiPartAsyncFilter<PIB_Wrapper>::NEW_INPUT : n);
			}
			else internalLeaves.push_back(MultiPartAsyncFilter<PIB_Wrapper>::NEW_INPUT);
		}
		
		
		std::size_t filterHandle, newInternalLeafHandle;
		std::vector<std::size_t> newFilterInputHandles;

		if (p->TryAddFilterAction(action,leaf,internalLeaves,filterHandle, newInternalLeafHandle,newFilterInputHandles)) {

			lua_pushnumber(L, filterHandle);
			lua_pushnumber(L, newInternalLeafHandle);
			lua_createtable(L, newFilterInputHandles.size(), 0);

			for (int j = 0; j < newFilterInputHandles.size(); j++) {
				lua_pushnumber(L, newFilterInputHandles[j]);
				lua_rawseti(L, -2, j);
			}
		
			return 3;
		}
		VestivizPipeline::log(p,"Failed to add buffered SIF.");
		return 0;
	}

	// Args: leaf index
	// Return: output index
	template<typename S, typename Tout, typename WrappedTPostbox >
	static int MakeOutput_lua(lua_State* L) {

		VestivizPipeline* p = GetPipelineUpVal(L);
		if (p == nullptr) return 0;

		try {
			if (!lua_isnumber(L, 1)) {
				VestivizPipeline::log(p, "Missing leaf index for output.");
				return 0;
			}
			int leafIndex = lua_tonumber(L, 1);

			auto sharedOutputPBox = std::shared_ptr<SimplePostbox<TimedDatum<S, Tout>>>(new SimplePostbox<TimedDatum<S, Tout>>());
			if (!p->setOutput((std::size_t)leafIndex, PIB_Wrapper::Wrap<TimedDatum<S, Tout>>(sharedOutputPBox))) {
				VestivizPipeline::log(p, "Failed to connect output.");
				return 0;
			}

			p->mLuaOutputs.push_back(std::shared_ptr<DatumOutputPostboxWrapper>(new WrappedTPostbox(sharedOutputPBox)));
			lua_pushnumber(L, p->mLuaOutputs.size() - 1);
			return 1;
		}
		catch (const std::exception& e) { VestivizPipeline::log(p, e); }
		return 0;
	}


	// Args: input handle for pipeline
	// Return: input index for add Datum
	template<typename S, typename Tin, typename WrappedTPostbox >
	static int MakeInput_lua(lua_State* L) {

		VestivizPipeline* p = GetPipelineUpVal(L);
		if (p == nullptr) return 0;

		try {
			if (!lua_isnumber(L, 1)) {
				VestivizPipeline::log(p, "Missing leaf index for input.");
				return 0;
			}
			int inputHandle = lua_tonumber(L, 1);			

			auto input = PIB_Wrapper::Unwrap<TimedDatum<S, Tin>>(p->getInput((std::size_t)inputHandle));
			
			if (input == nullptr) {
				VestivizPipeline::log(p, "Unable to connect input.");
				return 0;
			}
			p->mLuaInputs.push_back(std::shared_ptr<DatumInputPostboxWrapper>(new WrappedTPostbox(input)));

			lua_pushnumber(L, p->mLuaInputs.size() - 1);
			return 1;
		}
		catch (const std::exception& e) { VestivizPipeline::log(p, e); }
		return 0;
	}
public:
	
	using V3 = DatumArr<S, S, 3>;
	using V6 = DatumArr<S, S, 6>;
	using V8 = DatumArr<S, S, 8>;
	using M3 = DatumMatrix<S, 3, 3>;
	using M8x3 = DatumMatrix<S, 8, 3>;
	using M6x3 = DatumMatrix<S, 6, 3>;

	using PFAB = std::shared_ptr<FilterActionBase<PIB_Wrapper>>;
	
	explicit VestivizPipeline(std::shared_ptr<ErrorStack> errors) :
		PipelineBase<PIB_Wrapper>(errors),
		mErrors(errors) {};

	//Upvalues: instance ptr
    //Argument: input index
	static int addDatum(lua_State *L) {

		VestivizPipeline* p = GetPipelineUpVal(L);
		if (p == nullptr) return 0;

		if (!lua_isnumber(L, 1)) return 0;
		int inputIndex = lua_tointeger(L, 1);

		if (p->mLuaInputs.size() <= inputIndex) return 0;

		return p->mLuaInputs[inputIndex]->TryReadFromLua(L, 1);		
	}

	//Get Datum
	//Upvalues: instance ptr
	//Argument: output index
	static int getDatum(lua_State* L) {

		VestivizPipeline* p = GetPipelineUpVal(L);
		if (p == nullptr) return 0;

		if (!lua_isnumber(L, 1)) return 0;
		int outputIndex = lua_tointeger(L, 1);

		if (p->mLuaOutputs.size() <= outputIndex) return 0;

		return p->mLuaOutputs[outputIndex]->WriteToLua(L, 1);
	}

	// Args: buffer size, filterHandle (or nil for new), internal leaf handles to connect new action to .. ( x N & nil for new input)
	//return: filterhandle, new internal leaf handle
	static int l_AccelByRegressionFilterPoint(lua_State* L) {
		try{
			if (!lua_isnumber(L, 1)) return 0;
			int window = lua_tointeger(L, 1);

			return VestivizPipeline::addAction_lua<1>(L, 
				PFAB(new AccelByRegressionFilterAction<PIB_Wrapper,S, V3>(window)), 
				1);
		}
		catch (...) {/*TODO logging*/ }
		return 0;
	}

	// Args: {x,y,z}, filterHandle (or nil for new), internal leaf handles to connect new action to .. ( x N & nil for new input)
	//return: filterhandle, new internal leaf handle, input handles added to the filter...
	static int l_StaticAddFilterPoint(lua_State* L) {
		try{
			std::array<S, 3> x;
			if (!ReadVec3(L, 1, x)) return 0;
			return VestivizPipeline::addAction_lua<1>(
				L, 
				PFAB(new StatAddFilterAction<PIB_Wrapper, S, V3>(V3(x[0], x[1], x[2]))),
				1);
		}
		catch (...) {/*TODO logging*/ }
		return 0;
	}

	// Args: filterHandle (or nil for new), internal leaf handles to connect new action to .. ( x N & nil for new input)
	//return: filterhandle, new internal leaf handle, input handles added to the filter...
	static int l_DynMatMultFilterPoint(lua_State* L) {
		try{
			return VestivizPipeline::addAction_lua<2>(
				L,
				PFAB(new DynMatMultFilterAction<PIB_Wrapper, S, V3, V3, M3>()),
				0);
		}
		catch (...) {/*TODO logging*/ }
		return 0;
	}

	// Args: {x,y,z}, filterHandle (or nil for new), internal leaf handles to connect new action to .. ( x N & nil for new input)
	//return: filterhandle, new internal leaf handle, input handles added to the filter...
	static int l_QuickCompressFilterPoint(lua_State* L) {
		try {
			std::array<S, 3> x;
			if (!ReadVec3(L, 1, x)) return 0;

			return VestivizPipeline::addAction_lua<1>(
				L,
				PFAB(new QuickCompressFilterAction<PIB_Wrapper, S, V3>(V3(x[0], x[1], x[2]))),
				1);
		}
		catch (...) {/*TODO logging*/ }
		return 0;
	}

	// Args: {x,y,z}, filterHandle (or nil for new), internal leaf handles to connect new action to .. ( x N & nil for new input)
	//return: filterhandle, new internal leaf handle, input handles added to the filter...
	static int l_SignScaleFilterPoint(lua_State* L) {
		try {
			std::array<S, 3> x;
			if (!ReadVec3(L, 1, x)) return 0;

			return VestivizPipeline::addAction_lua<1>(
				L,
				PFAB(new SignScaleFilterAction<PIB_Wrapper, S, V3>(V3(x[0], x[1], x[2]))),
				1);
		}
		catch (...) {/*TODO logging*/ }
		return 0;
	}

	// Args: halflife, filterHandle (or nil for new), internal leaf handles to connect new action to .. ( x N & nil for new input)
	//return: filterhandle, new internal leaf handle, input handles added to the filter...
	static int l_ExpDecayFilterPoint(lua_State* L) {
		try{
			if (!lua_isnumber(L, 1)) return 0;
			double hl = lua_tonumber(L, 1);

			return VestivizPipeline::addAction_lua<1>(
				L,
				PFAB(new ExpDecayFilterAction<PIB_Wrapper, S, V3>(hl)),
				1);
		}
		catch (...) {/*TODO logging*/ }
		return 0;
	}

	// Args: {1=,2=,...,24 = }, filterHandle (or nil for new), internal leaf handles to connect new action to .. ( x N & nil for new input)
	//return: filterhandle, new internal leaf handle, input handles added to the filter...
	static int l_MatMultFilterPointToWOff(lua_State* L) {
		try{
			std::array<S, 24> arr;
			if (!ReadArray<24>(L,1, arr)) return 0;
			M8x3 m = M8x3(arr);

			return VestivizPipeline::addAction_lua<1>(
				L,
				PFAB(new StatMatMultFilterAction<PIB_Wrapper, S, V3, M8x3, V8>(std::move(m))),
				1);
		}
		catch (...) {/*TODO logging*/ }
		return 0;
	}

	// Args: filterHandle (or nil for new), internal leaf handles to connect new action to .. ( x N & nil for new input)
	//return: filterhandle, new internal leaf handle, input handles added to the filter...
	static int l_SimpleDiffFilterXY(lua_State* L) {
		try{
			return VestivizPipeline::addAction_lua<1>(
				L,
				PFAB(new SimpleDiffFilterAction<PIB_Wrapper, S, V6>()),
				0);
		}
		catch (...) {/*TODO logging*/ }
		return 0;
	}

	// Args: filterHandle (or nil for new), internal leaf handles to connect new action to .. ( x N & nil for new input)
	//return: filterhandle, new internal leaf handle, input handles added to the filter...
	static int l_SimpleDiffFilterPoint(lua_State* L) {
		try{
			return VestivizPipeline::addAction_lua<1>(
				L,
				PFAB(new SimpleDiffFilterAction<PIB_Wrapper, S, V3>()),
				0);
		}
		catch (...) {/*TODO logging*/ }
		return 0;
	}

	// Args: {{<xtuple>},{<ytuple>},{<ztuple>} } , filterHandle (or nil for new), internal leaf handles to connect new action to .. ( x N & nil for new input)
	//return: filterhandle, new internal leaf handle, input handles added to the filter...
	static int l_DynMatMultPickFilterXYtoPoint(lua_State* L) {
		try{
			std::array<std::tuple<std::size_t, std::size_t>, 3> m;
			if (!ReadTupleArray<3>(L, 1, m)) return 0;
			return VestivizPipeline::addAction_lua<2>(
				L,
				PFAB(new DynMatMultPickFilterAction<PIB_Wrapper, S, S, 3, 3, 6, 3>(std::move(m))),
				1);
		}
		catch (...) {/*TODO logging*/ }
		return 0;
	}

	// Args: filterHandle (or nil for new), internal leaf handles to connect new action to .. ( x N & nil for new input)
	//return: filterhandle, new internal leaf handle, input handles added to the filter...
	static int l_DynMatMultPickFilterPointToXY(lua_State* L) {
		try {
			return VestivizPipeline::addAction_lua<2>(
				L,
				PFAB(new DynMatMultFilterAction<PIB_Wrapper, S, V6, V3, M6x3>()),
				0);
		}
		catch (...) {/*TODO logging*/ }
		return 0;
	}

	// Args: coeff1,coeff2,  filterHandle (or nil for new), internal leaf handles to connect new action to .. ( x N & nil for new input)
	//return: filterhandle, new internal leaf handle, input handles added to the filter...
	static int l_LinCombFilterWOff(lua_State* L) {
		try{
			if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2)) return 0;
			double coeff1 = lua_tonumber(L, 1);
			double coeff2 = lua_tonumber(L, 2);

			return VestivizPipeline::addAction_lua<2>(
				L,
				PFAB(new LinCombFilterAction<PIB_Wrapper, S, V8>(coeff1, coeff2)),
				2);
		}
		catch (...) {/*TODO logging*/ }
		return 0;
	}

	// Args: {w = {top= ...,,,}, off ={top=...,,,}}, filterHandle (or nil for new), internal leaf handles to connect new action to .. ( x N & nil for new input)
	//return: filterhandle, new internal leaf handle, input handles added to the filter...
	static int l_QuickCompressFilterWOff(lua_State* L) {
		try {
			V8 x;
			if (!ReadWOff(L, 1, x)) return 0;
			return VestivizPipeline::addAction_lua<1>(
				L,
				PFAB(new QuickCompressFilterAction<PIB_Wrapper, S, V8>(x)),
				1);
		}
		catch (...) {/*TODO logging*/ }
		return 0;
	}

	// Args: {1 = .., 2= ...,..}, filterHandle (or nil for new), internal leaf handles to connect new action to .. ( x N & nil for new input)
	//return: filterhandle, new internal leaf handle, input handles added to the filter...
	static int l_ConvolveOutputFilterWOff(lua_State* L) {
		try{
			std::vector<S> x;
			if (!ReadVector(L, 1, x)) return 0;
			return  VestivizPipeline::addAction_lua<1>(
				L,
				PFAB(new ConvolveFilterAction<PIB_Wrapper, S, V8>(std::move(x))),
				1);
		}
		catch (...) {/*TODO logging*/ }
		return 0;
	}

	// Args: filterHandle, leaf handles to connect to inputs...
	//return: new output leaf handle, new input handles...
	static int l_ConnectFilter(lua_State* L) {

		VestivizPipeline* p = GetPipelineUpVal(L);
		if (p == nullptr) return 0;

		int inputsToConnect = 0;

		if (!lua_isnumber(L, 1)) return 0;

		int filterConstructionHandle = lua_tointeger(L, 1);

		if (!p->TryGetFilterInputCount(filterConstructionHandle, inputsToConnect)) return 0;

		std::vector<std::size_t> leafHandlesForInputs(inputsToConnect, NEW_INPUT);

		for (int i = 0; i < inputsToConnect; i++) {
			if (lua_isnumber(L, i + 2)) {
				leafHandlesForInputs[i] = lua_tonumber(L, i + 2);
			}
		}

		std::size_t newLeafHandle = 0;
		std::vector<std::size_t> newInputHandles;

		if (!p->TryConnectFilter(leafHandlesForInputs, filterConstructionHandle, newLeafHandle, newInputHandles)) return 0;


		int pushCount = 0;

		lua_pushnumber(L, newLeafHandle);
		pushCount++;

		for (auto it = newInputHandles.cbegin(); it != newInputHandles.cend(); it++) {
			lua_pushnumber(L, *it);
			pushCount++;
		}

		return pushCount;
	}

	// Args: filterUnderConstructionHandle, internal leaf handle to add output to
	//return: nil
	static int l_AddInputToFilter(lua_State* L) {

		VestivizPipeline* p = GetPipelineUpVal(L);
		if (p == nullptr) return 0;

		int inputsToConnect = 0;

		if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2)) return 0;

		std::size_t filterConstructionHandle = lua_tointeger(L, 1);
		std::size_t internalLeafHandle = lua_tointeger(L, 2);

		p->TryAddFilterActionOutput(filterConstructionHandle, internalLeafHandle);

		return 0;
	}

	// args: inputIndex, timed datum
	static int l_Pipeline_AddDatum(lua_State* L) {
		try{
			auto pipeline = GetPipelineUpVal(L);
			if (pipeline != nullptr) {
				if (!lua_isnumber(L, 1)) return 0;
				int inputIndex = (int)lua_tointeger(L, 1);

				if (pipeline->mLuaInputs.size() > inputIndex && inputIndex >= 0) {
					auto pInput = pipeline->mLuaInputs[inputIndex];
					if (pInput != nullptr) {
						return pInput->TryReadFromLua(L, 1);
					}
				}
			}
			return 0;
		}
		catch (...) {/*TODO logging*/ }
		return 0;
	}
	// args: outputIndex
	//return: timedDatum
	static int l_Pipeline_GetDatum(lua_State* L) {
		try {
			auto pipeline = GetPipelineUpVal(L);

			if (pipeline != nullptr) {

				if (!lua_isnumber(L, 1)) return 0;
				int outputIndex = (int)lua_tointeger(L, 1);

				if (pipeline->mLuaOutputs.size() > outputIndex && outputIndex >= 0) {
					auto pOutput = pipeline->mLuaOutputs[outputIndex];
					if (pOutput != nullptr) {
						return pOutput->WriteToLua(L);
					}
				}
			}
			return 0;
		}
		catch (...) {/*TODO logging*/ }
		return 0;
	}
	// Args: leaf index
	// Return: output index
	static int l_MakeWOffOutput(lua_State* L) {
		return MakeOutput_lua<S, DatumArr<S, S, 8>, DOPW_woff<S>>(L);
	}

	// Args: leaf index
	// Return: output index
	static int l_MakePointOutput(lua_State* L) {
		return MakeOutput_lua<S, DatumArr<S, S, 3>, DOPW_point<S>>(L);
	}

	// Args: leaf index
	// Return: output index
	static int l_MakeXYOutput(lua_State* L) {
		return MakeOutput_lua<S, DatumArr<S, S, 6>, DOPW_xy<S>>(L);
	}

	//

	// Args: input handle for pipeline
	// Return: input index for add Datum
	static int l_MakePointInput(lua_State* L) {
		return MakeInput_lua<S, DatumArr<S, S, 3>, DIPW_point<S>>(L);
	}

	// Args: input handle for pipeline
	// Return: input index for add Datum
	static int l_MakeXYInput(lua_State* L) {
		return MakeInput_lua<S, DatumArr<S, S, 6>, DIPW_xy<S>>(L);
	}

	// Args: input handle for pipeline
	// Return: input index for add Datum
	static int l_MakeFrameInput(lua_State* L) {
		return MakeInput_lua<S, DatumMatrix<S, 3, 3>, DIPW_frame<S>>(L);
	}

	static int l_Pipeline_Delete(lua_State* L) {

		VestivizPipeline<double>* p = GetPipelineUpVal(L);
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

	static int l_Pipeline_PopError(lua_State* L) {

		VestivizPipeline<double>* p = GetPipelineUpVal(L);
		if (p != nullptr && p->mErrors != nullptr) {
			std::string msg;
			if (p->mErrors->pop_message(msg))
			{
				lua_pushstring(L, msg.c_str());
				return 1;
			}
		}
		return 0;
	}

	static int l_Validate(lua_State* L) {

		VestivizPipeline<double>* p = GetPipelineUpVal(L);
		if (p != nullptr && p->mErrors != nullptr) {
			if (!p->validate()) lua_pushboolean(L, 0);
			else lua_pushboolean(L, 1);
			return 1;
		}
		return 0;
	}
	static int l_Pipeline_New(lua_State* L) {

		std::shared_ptr<ErrorStack> log(new ErrorStack(5));

		auto pNew = new VestivizPipeline<double>(log);

		lua_createtable(L, 0, 21);
		lua_newuserdata(L, 1);
		lua_createtable(L, 0, 1);
		lua_pushlightuserdata(L, pNew);
		lua_pushcclosure(L, l_Pipeline_Delete, 1);
		lua_setfield(L, -2, "__gc");
		lua_setmetatable(L, -2);
		lua_setfield(L, -2, "destructor");
		lua_pushlightuserdata(L, pNew);
		lua_pushcclosure(L, l_Pipeline_GetDatum, 1);
		lua_setfield(L, -2, "getDatum");
		lua_pushlightuserdata(L, pNew);
		lua_pushcclosure(L, l_Pipeline_AddDatum, 1);
		lua_setfield(L, -2, "addDatum");
		lua_pushlightuserdata(L, pNew);
		lua_pushcclosure(L, l_Pipeline_PopError, 1);
		lua_setfield(L, -2, "popError");
		lua_pushlightuserdata(L, pNew);
		lua_pushcclosure(L, l_Pipeline_Start, 1);
		lua_setfield(L, -2, "start");
		lua_pushlightuserdata(L, pNew);
		lua_pushcclosure(L, l_Pipeline_Stop, 1);
		lua_setfield(L, -2, "stop");
		lua_pushlightuserdata(L, pNew);
		lua_pushcclosure(L, l_AccelByRegressionFilterPoint, 1);
		lua_setfield(L, -2, "accelByRegressionFilterPoint");
		lua_pushlightuserdata(L, pNew);
		lua_pushcclosure(L, l_StaticAddFilterPoint, 1);
		lua_setfield(L, -2, "staticAddFilterPoint");
		lua_pushlightuserdata(L, pNew);
		lua_pushcclosure(L, l_DynMatMultFilterPoint, 1);
		lua_setfield(L, -2, "dynMatMultFilterPoint");
		lua_pushlightuserdata(L, pNew);
		lua_pushcclosure(L, l_QuickCompressFilterPoint, 1);
		lua_setfield(L, -2, "quickCompressFilterPoint");
		lua_pushlightuserdata(L, pNew);
		lua_pushcclosure(L, l_ExpDecayFilterPoint, 1);
		lua_setfield(L, -2, "expDecayFilterPoint");
		lua_pushlightuserdata(L, pNew);
		lua_pushcclosure(L, l_MatMultFilterPointToWOff, 1);
		lua_setfield(L, -2, "matMultFilterPointToWOff");
		lua_pushlightuserdata(L, pNew);
		lua_pushcclosure(L, l_SimpleDiffFilterXY, 1);
		lua_setfield(L, -2, "simpleDiffFilterXY");
		lua_pushlightuserdata(L, pNew);
		lua_pushcclosure(L, l_SimpleDiffFilterPoint, 1);
		lua_setfield(L, -2, "simpleDiffFilterPoint");
		lua_pushlightuserdata(L, pNew);
		lua_pushcclosure(L, l_DynMatMultPickFilterXYtoPoint, 1);
		lua_setfield(L, -2, "dynMatMultPickFilterXYtoPoint");
		lua_pushlightuserdata(L, pNew);
		lua_pushcclosure(L, l_LinCombFilterWOff, 1);
		lua_setfield(L, -2, "linCombFilterWOff");
		lua_pushlightuserdata(L, pNew);
		lua_pushcclosure(L, l_QuickCompressFilterWOff, 1);
		lua_setfield(L, -2, "quickCompressFilterWOff");
		lua_pushlightuserdata(L, pNew);
		lua_pushcclosure(L, l_ConvolveOutputFilterWOff, 1);
		lua_setfield(L, -2, "convolveOutputFilterWOff");
		lua_pushlightuserdata(L, pNew);
		lua_pushcclosure(L, l_SignScaleFilterPoint, 1);
		lua_setfield(L, -2, "signScaleFilterPoint");
		lua_pushlightuserdata(L, pNew);
		lua_pushcclosure(L, l_MakeWOffOutput, 1);
		lua_setfield(L, -2, "makeWOffOutput");
		lua_pushlightuserdata(L, pNew);
		lua_pushcclosure(L, l_MakePointOutput, 1);
		lua_setfield(L, -2, "makePointOutput");
		lua_pushlightuserdata(L, pNew);
		lua_pushcclosure(L, l_MakeXYOutput, 1);
		lua_setfield(L, -2, "makeXYOutput");
		lua_pushlightuserdata(L, pNew);
		lua_pushcclosure(L, l_MakeFrameInput, 1);
		lua_setfield(L, -2, "makeFrameInput");
		lua_pushlightuserdata(L, pNew);
		lua_pushcclosure(L, l_MakePointInput, 1);
		lua_setfield(L, -2, "makePointInput");
		lua_pushlightuserdata(L, pNew);
		lua_pushcclosure(L, l_MakeXYInput, 1);
		lua_setfield(L, -2, "makeXYInput");
		lua_pushlightuserdata(L, pNew);
		lua_pushcclosure(L, l_ConnectFilter, 1);
		lua_setfield(L, -2, "connectFilter");
		lua_pushlightuserdata(L, pNew);
		lua_pushcclosure(L, l_AddInputToFilter, 1);
		lua_setfield(L, -2, "addInputToFilter");		
		lua_pushlightuserdata(L, pNew);
		lua_pushcclosure(L, l_Validate, 1);
		lua_setfield(L, -2, "validate");
		return 1;
	}
};


#endif