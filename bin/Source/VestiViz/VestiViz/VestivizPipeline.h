#pragma once

#ifndef _VESTIVIZPIPELINE_H_
#define _VESTIVIZPIPELINE_H_

#include "ConvOutF.h"
#include "ExpDecaySIF.h"
#include "RegDiffSIF.h"
#include "SimpleDiffSIF.h"
#include "QCompSIF.h"
#include "DynMatMultDIF.h"
#include "StatAddFilterAction.h"
#include "DynMatMultPickDIF.h"
#include "LinCombFilterAction.h"//"LinCombDIF.h"
#include "StatMatMultSIF.h"
#include "LogFilterAction.h"
#include "PipelineBase.h"
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
		return (VestivizPipeline*)lua_touserdata(L, lua_upvalueindex(1));
	}


	template<typename Tin, typename Tout, typename WrappedTPostbox>
	static int addBufferedSIF_lua(lua_State* L, PFAB<S, Tin, Tout>&& action, int stackOffset) {

		VestivizPipeline* p = GetPipelineUpVal(L);
		if (p == nullptr) return 0;

		std::size_t leaf = NEW_INPUT;
		if (lua_isnumber(L, stackOffset + 1)) leaf = lua_tointeger(L, stackOffset + 1);
		bool newInput = leaf == NEW_INPUT;

		std::size_t windowSize = 8;
		if (lua_isnumber(L, stackOffset + 2)) windowSize = lua_tointeger(L, stackOffset + 2);

		if (p->addBufferedSIF(leaf, windowSize,
			PFAB<S, Tin, Tout>(std::move(action)), leaf)) {

			lua_pushnumber(L, leaf);
			if (newInput) {
				auto input = PIB_Wrapper::Unwrap<TimedDatum<S, Tin>>(p->getLastInput());
				p->mLuaInputs.push_back(std::shared_ptr<DatumInputPostboxWrapper>(new WrappedTPostbox(input)));
				lua_pushnumber(L, p->mLuaInputs.size() - 1);
				return 2;
			}
			return 1;
		}
		VestivizPipeline::log(p,"Failed to add buffered SIF.");
		return 0;
	}

	template<typename Tin, typename Tout, typename WrappedTPostbox>
	static int addSimpleSIF_lua(lua_State* L, PFAB<S, Tin, Tout>&& action, int stackOffset) {

		VestivizPipeline* p = GetPipelineUpVal(L);
		if (p == nullptr) return 0;

		std::size_t leaf = NEW_INPUT;
		if (lua_isnumber(L, stackOffset + 1)) leaf = lua_tointeger(L, stackOffset + 1);
		bool newInput = leaf == NEW_INPUT;

		if (p->addSimpleSIF(leaf,
			PFAB<S, Tin, Tout>(std::move(action)), leaf)) {
			lua_pushnumber(L, leaf);
			if (newInput) {
				auto input = PIB_Wrapper::Unwrap<TimedDatum<S, Tin>>(p -> getLastInput());
				p->mLuaInputs.push_back(std::shared_ptr<DatumInputPostboxWrapper>(new WrappedTPostbox(input)));
				lua_pushnumber(L, p->mLuaInputs.size() - 1);
				return 2;
			}
			return 1;
		}

		VestivizPipeline::log(p, "Failed to add simple SIF.");
		return 0;
	}

	template<typename Tin1, typename Tin2, typename Tout, typename WrappedT1Postbox, typename WrappedT2Postbox>
	static int addSimpleDIF_lua(lua_State* L, PDFAB<S, Tin1, Tin2, Tout>&& action, int stackOffset) {

		VestivizPipeline* p = GetPipelineUpVal(L);
		if (p == nullptr) return 0;

		std::size_t leaf1 = NEW_INPUT;
		if (lua_isnumber(L, stackOffset + 1)) leaf1 = lua_tointeger(L, stackOffset + 1);
		bool newInput1 = leaf1 == NEW_INPUT;

		std::size_t leaf2 = NEW_INPUT;
		if (lua_isnumber(L, stackOffset + 2)) leaf2 = lua_tointeger(L, stackOffset + 2);
		bool newInput2 = leaf2 == NEW_INPUT;

		std::size_t leafOut = -1;

		if (p->addSimpleDIF(leaf1, leaf2,
			PDFAB<S, Tin1, Tin2, Tout>(std::move(action)), leafOut)) {
			lua_pushnumber(L, leafOut);
			int pushed = 1;

			if (newInput1) {
				p->mLuaInputs.push_back(std::shared_ptr<DatumInputPostboxWrapper>(
					new WrappedT1Postbox(PIB_Wrapper::Unwrap<TimedDatum<S, Tin1>>(p->getLastInput()))));
				lua_pushnumber(L, p->mLuaInputs.size() - 1);
				pushed++;
			}
			if (newInput2) {
				p->mLuaInputs.push_back(std::shared_ptr<DatumInputPostboxWrapper>(
					new WrappedT2Postbox(PIB_Wrapper::Unwrap<TimedDatum<S, Tin2>>(p->getLastInput()))));
				lua_pushnumber(L, p->mLuaInputs.size() - 1);
				pushed++;
			}
			return pushed;
		}
		VestivizPipeline::log(p, "Failed to add simple DIF.");
		return 0;
	}

	template<typename Tin, typename Tout, typename WrappedTPostbox >
	static int addBufferedOutF_lua(lua_State* L, PFAB<S, Tin, Tout>&& action, int stackOffset) {

		VestivizPipeline* p = GetPipelineUpVal(L);
		if (p == nullptr) return 0;

		std::size_t leaf = NEW_INPUT;
		if (lua_isnumber(L, stackOffset + 1)) leaf = lua_tointeger(L, stackOffset + 1);
		bool newInput = leaf == NEW_INPUT;

		std::size_t windowSize = 8;
		if (lua_isnumber(L, stackOffset + 2)) windowSize = lua_tointeger(L, stackOffset + 2);

		if (p->addBufferedOutF(leaf, windowSize,
			PFAB<S, Tin, Tout>(std::move(action)), leaf)) {
			lua_pushnumber(L, leaf);
			if (newInput) {
				auto input = PIB_Wrapper::Unwrap<TimedDatum<S, Tin>>(getLastInput());
				p->mLuaInputs.push_back(std::shared_ptr<DatumInputPostboxWrapper>(new WrappedTPostbox(input)));
				lua_pushnumber(L, p->mLuaInputs.size() - 1);
				return 2;
			}
			return 1;
		}
		VestivizPipeline::log(p, "Failed to add buffered OutF.");
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
public:
	
	using V3 = DatumArr<S, S, 3>;
	using V6 = DatumArr<S, S, 6>;
	using V8 = DatumArr<S, S, 8>;
	using M3 = DatumMatrix<S, 3, 3>;
	using M8x3 = DatumMatrix<S, 8, 3>;

	
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

	//args: leaf index, buffer size
	// return leafIndex, inputIndex = nil
	static int l_AccelByRegressionFilterPoint(lua_State* L) {
		try{
			return VestivizPipeline::addBufferedSIF_lua<V3,V3, DIPW_point<S>>(L, PFAB<S, V3, V3>
				(new AccelByRegressionFilterAction<S, V3, CircBufL>()), 
				0);
		}
		catch (...) {/*TODO logging*/ }
		return 0;
	}

	//args: {x,y,z}, leaf index
	// return leafIndex, inputIndex = nil
	static int l_StaticAddFilterPoint(lua_State* L) {
		try{
			std::array<S, 3> x;
			if (!ReadVec3(L, 1, x)) return 0;

			return VestivizPipeline::addSimpleSIF_lua<V3, V3, DIPW_point<S>>(
				L, 
				PFAB<S, V3, V3>(new StatAddFilterAction<S, V3, CircBufL>(V3(x[0], x[1], x[2]))), 
				1);
		}
		catch (...) {/*TODO logging*/ }
		return 0;
	}

	//args: leaf index1, leaf index2
	// return leafIndex, inputIndex1 = nil, inputIndex2 = nil
	static int l_DynMatMultFilterPoint(lua_State* L) {
		try{
			return VestivizPipeline::addSimpleDIF_lua<V3, M3, V3, DIPW_point<S>, DIPW_frame<S>>(
				L,
				PDFAB<S, V3, M3, V3>(new DynMatMultFilterAction<S, V3, V3, M3, CircBufL, CircBufL>()), 
				0);
		}
		catch (...) {/*TODO logging*/ }
		return 0;
	}

	//args: {x,y,z}, leaf index
	// return leafIndex, inputIndex = nil
	static int l_QuickCompressFilterPoint(lua_State* L) {
		try {
			std::array<S, 3> x;
			if (!ReadVec3(L, 1, x)) return 0;

			return VestivizPipeline::addSimpleSIF_lua<V3, V3, DIPW_point<S>>(
				L,
				PFAB<S, V3, V3>(new QuickCompressFilterAction<S, V3, CircBufL>(V3(x[0], x[1], x[2]))),
				1);
		}
		catch (...) {/*TODO logging*/ }
		return 0;
	}

	//args: halflife, leaf index
	// return leafIndex, inputIndex = nil
	static int l_ExpDecayFilterPoint(lua_State* L) {
		try{
			if (!lua_isnumber(L, 1)) return 0;
			double hl = lua_tonumber(L, 1);

			return VestivizPipeline::addSimpleSIF_lua<V3, V3, DIPW_point<S>>(
				L,
				PFAB<S, V3, V3>(new ExpDecayFilterAction<S, V3, CircBufL>(hl)), 
				1);
		}
		catch (...) {/*TODO logging*/ }
		return 0;
	}

	//args: {1=,2=,...,24 = }, leaf index
	// return leafIndex, inputIndex = nil
	static int l_MatMultFilterPointToWOff(lua_State* L) {
		try{
			std::array<S, 24> arr;
			if (!ReadArray<24>(L,1, arr)) return 0;

			M8x3 m = M8x3(arr);

			return VestivizPipeline::addSimpleSIF_lua<V3, V8, DIPW_point<S>>(
				L,
				PFAB<S, V3, V8>(new StatMatMultFilterAction<S, V3, M8x3, V8, CircBufL>(std::move(m))), 
				1);
		}
		catch (...) {/*TODO logging*/ }
		return 0;
	}

	//args:  leaf index, buffer size
	// return leafIndex, inputIndex = nil
	static int l_SimpleDiffFilterXY(lua_State* L) {
		try{
			return VestivizPipeline::addBufferedSIF_lua<V6, V6, DIPW_xy<S>>(
				L,
				PFAB<S, V6, V6>(new SimpleDiffFilterAction<S, V6, CircBufL>()), 
				0);
		}
		catch (...) {/*TODO logging*/ }
		return 0;
	}

	//args:  leaf index, buffer size
	// return leafIndex, inputIndex = nil
	static int l_SimpleDiffFilterPoint(lua_State* L) {
		try{
			return VestivizPipeline::addBufferedSIF_lua<V3, V3, DIPW_point<S>>(
				L,
				PFAB<S, V3, V3>(new SimpleDiffFilterAction<S, V3, CircBufL>()), 
				0);
		}
		catch (...) {/*TODO logging*/ }
		return 0;
	}

	//args:{{<xtuple>},{<ytuple>},{<ztuple>} } ,leaf index1, leaf index2
	// return leafIndex, inputIndex1 = nil, inputIndex2 = nil
	static int l_DynMatMultPickFilterXYtoPoint(lua_State* L) {
		try{
			std::array<std::tuple<std::size_t, std::size_t>, 3> m;
			if (!ReadTupleArray<3>(L, 1, m)) return 0;

			return VestivizPipeline::addSimpleDIF_lua<V6, M3, V3, DIPW_xy<S>, DIPW_frame<S>>(
				L,
				PDFAB<S, V6, M3, V3>(new DynMatMultPickFilterAction<S, S, 3, 3, 6, 3, CircBufL, CircBufL>(std::move(m))),
				1);
		}
		catch (...) {/*TODO logging*/ }
		return 0;
	}

	//args: coeff1,coeff2, leaf index1, leaf index2
	// return leafIndex, inputIndex1 = nil, inputIndex2 = nil
	static int l_LinCombFilterWOff(lua_State* L) {
		try{
			if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2)) return 0;
			double coeff1 = lua_tonumber(L, 1);
			double coeff2 = lua_tonumber(L, 2);

			return VestivizPipeline::addSimpleDIF_lua<V8, V8, V8, DIPW_woff<S>, DIPW_woff<S>>(
				L,
				PDFAB<S, V8, V8, V8>(new LinCombFilterAction<S, V8, CircBufL>(coeff1, coeff2)),
				2);
		}
		catch (...) {/*TODO logging*/ }
		return 0;
	}

	//args: {w = {top= ...,,,}, off ={top=...,,,}}, leaf index
	// return leafIndex, inputIndex = nil
	static int l_QuickCompressFilterWOff(lua_State* L) {
		try {
			V8 x;
			if (!ReadWOff(L, 1, x)) return 0;

			return VestivizPipeline::addSimpleSIF_lua<V8, V8, DIPW_woff<S>>(
				L,
				PFAB<S, V8, V8>(new QuickCompressFilterAction<S, V8, CircBufL>(x)),
				1);
		}
		catch (...) {/*TODO logging*/ }
		return 0;
	}

	//args: {1 = .., 2= ...,..}, leaf index, buffer size
	// return leafIndex, inputIndex = nil
	static int l_ConvolveOutputFilterWOff(lua_State* L) {
		try{
			std::vector<S> x;
			if (!ReadVector(L, 1, x)) return 0;

			return  VestivizPipeline::addBufferedSIF_lua<V8, V8, DIPW_woff<S>>(
				L,
				PFAB<S, V8, V8>(new ConvolveFilterAction<S, V8, CircBufL>(std::move(x))),
				1);
		}
		catch (...) {/*TODO logging*/ }
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


	static int l_Pipeline_New(lua_State* L) {

		std::shared_ptr<ErrorStack> log(new ErrorStack());

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
		lua_pushcclosure(L, l_MakeWOffOutput, 1);
		lua_setfield(L, -2, "makeWOffOutput");
		lua_pushlightuserdata(L, pNew);
		lua_pushcclosure(L, l_MakePointOutput, 1);
		lua_setfield(L, -2, "makePointOutput");
		lua_pushlightuserdata(L, pNew);
		lua_pushcclosure(L, l_MakeXYOutput, 1);
		lua_setfield(L, -2, "makeXYOutput");
		return 1;
	}
};


#endif