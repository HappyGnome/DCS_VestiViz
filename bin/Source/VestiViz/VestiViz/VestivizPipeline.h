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

	std::vector<std::shared_ptr<DatumInputPostboxWrapper>> mLuaInputs;

	std::vector < std::shared_ptr<DatumOutputPostboxWrapper>> mLuaOutputs;

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
				auto input = PIB_Wrapper::Unwrap<TimedDatum<S, Tin>>(getLastInput());
				p->mLuaInputs.push_back(std::make_shared<DatumInputPostboxWrapper>(WrappedTPostbox(input)));
				lua_pushnumber(L, p->mLuaInputs.size() - 1);
				return 2;
			}
			return 1;
		}
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
				auto input = PIB_Wrapper::Unwrap<TimedDatum<S, Tin>>(getLastInput());
				p->mLuaInputs.push_back(std::make_shared<DatumInputPostboxWrapper>(WrappedTPostbox(input)));
				lua_pushnumber(L, p->mLuaInputs.size() - 1);
				return 2;
			}
			return 1;
		}
	}

	template<typename Tin1, typename Tin2, typename Tout, typename WrappedTPostbox>
	static int addSimpleDIF_lua(lua_State* L, PDFAB<S, Tin1, Tin2, Tout>&& action, int stackOffset) {

		VestivizPipeline* p = GetPipelineUpVal(L);
		if (p == nullptr) return 0;

		int newInputOffset = -1;
		std::size_t leaf1 = NEW_INPUT;
		if (lua_isnumber(L, stackOffset + 1)) leaf1 = lua_tointeger(L, stackOffset + 1);
		if (leaf1 == NEW_INPUT) newInputOffset++;

		std::size_t leaf2 = NEW_INPUT;
		if (lua_isnumber(L, stackOffset + 2)) leaf2 = lua_tointeger(L, stackOffset + 2);
		if (leaf2 == NEW_INPUT) newInputOffset++;

		std::size_t leafOut = -1;

		if (p->addSimpleDIF(leaf1, leaf2,
			PDFAB<S, Tin1, Tin2, Tout>(std::move(action)), leafOut)) {
			lua_pushnumber(L, leafOut);
			int pushed = 1;
			for (int i = newInputOffset; i >= 0; i--) {
				auto input = PIB_Wrapper::Unwrap<TimedDatum<S, Tin1>>(getLastInput(i));
				p->mLuaInputs.push_back(std::make_shared<DatumInputPostboxWrapper>(WrappedTPostbox(input)));
				lua_pushnumber(L, p->mLuaInputs.size() - 1);
				pushed++;
			}
			return pushed;
		}
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
				p->mLuaInputs.push_back(std::make_shared<DatumInputPostboxWrapper>(WrappedTPostbox(input)));
				lua_pushnumber(L, p->mLuaInputs.size() - 1);
				return 2;
			}
			return 1;
		}
	}
public:
	
	using V3 = DatumArr<S, S, 3>;
	using V6 = DatumArr<S, S, 6>;
	using V8 = DatumArr<S, S, 8>;
	using M3 = DatumMatrix<S, 3, 3>;
	using M8x3 = DatumMatrix<S, 8, 3>;

	void init() {
		/*std::size_t gLeaf;
		std::size_t rotLeaf;
		std::size_t outLeaf;

		addBufferedSIF(NEW_INPUT, 8, PFAB<S,V3, V3>
			(new AccelByRegressionFilterAction<S, V3, CircBufL>()), gLeaf);
		addSimpleSIF(
			gLeaf,
			PFAB<S, V3, V3>(new StatAddFilterAction<S, V3, CircBufL>(V3(0.0f, 9.81f, 0.0f))),
			gLeaf);
		addSimpleDIF(
			gLeaf, 
			NEW_INPUT, 
			PDFAB<S, V3, M3, V3>(new DynMatMultFilterAction<S,V3,V3,M3,CircBufL,CircBufL> ()),
			gLeaf);
		addSimpleSIF(
			gLeaf,
			PFAB<S, V3, V3>(new StatAddFilterAction<S, V3, CircBufL>(V3(0.0f, -9.81f, 0.0f))),
			gLeaf);
#ifdef _DEBUG_LOG_PIPE
		addSimpleSIF(
			gLeaf,
			PFAB<S, V3, V3>(new LogFilterAction<S, V3>("LocalG ")),
			gLeaf);
#endif //_DEBUG_LOG_PIPE

		addSimpleSIF(
			gLeaf,
			PFAB<S, V3, V3>(new QuickCompressFilterAction<S, V3, CircBufL>(V3(1.0f, 1.0f, 1.0f))),
			gLeaf);
		addSimpleSIF(
			gLeaf,
			PFAB<S, V3, V3>
			(new ExpDecayFilterAction<S, V3, CircBufL>(1.0f)),
			gLeaf);

		addSimpleSIF(
			gLeaf,
			PFAB<S, V3,V8>
			(new StatMatMultFilterAction<S,V3,M8x3,V8,CircBufL> (
				M8x3(0.5f, -0.5f, 0.0f,//T width
					0.5f, 0.0f, -0.5f,//R width
					0.5f, 0.5f, 0.0f,//B width
					0.5f, 0.0f, 0.5f,//L width
					0.0f, 0.0f, 1.0f, //T somatograv
					1.0f, 0.0f, 1.0f,//R
					0.0f, 0.0f, -1.0f,//B
					1.0f, 0.0f, -1.0f))),//L))),
			gLeaf);
		addBufferedSIF(NEW_INPUT, 2, PFAB<S, V6, V6>
			(new SimpleDiffFilterAction<S, V6, CircBufL>()), rotLeaf);

		addSimpleDIF(
			rotLeaf,
			NEW_INPUT,
			PDFAB<S, V6, M3, V3>(new DynMatMultPickFilterAction<S,S,3,3,6,3,CircBufL,CircBufL> (
					std::array<std::tuple<std::size_t, std::size_t>, 3>{
						std::tuple<std::size_t, std::size_t>(2, 1), //x-axis rot
						std::tuple<std::size_t, std::size_t>(2, 0), // negative y-axis rot
						std::tuple<std::size_t, std::size_t>(1, 0) //z-axis rot
					}
				)),
			rotLeaf);

		addSimpleSIF(
			rotLeaf,
			PFAB<S, V3, V3>
			(new QuickCompressFilterAction<S, V3, CircBufL> (V3(1.0f, 1.0f, 1.0f))),
			rotLeaf);

		addSimpleSIF(
			rotLeaf,
			PFAB<S, V3, V3>
			(new ExpDecayFilterAction<S, V3, CircBufL>(1.0f)),
			rotLeaf);
		
		addSimpleSIF(
			rotLeaf,
			PFAB<S, V3,V8>
				(new StatMatMultFilterAction<S, V3, M8x3, V8, CircBufL>(
					M8x3(0.0f, 0.0f, 0.0f,//T width
						0.0f, 0.0f, 0.0f,//R
						0.0f, 0.0f, 0.0f,//B
						0.0f, 0.0f, 0.0f,//L 
						-1.0f, 1.0f, 0.0f, //T displacement
						-1.0f, 0.0f, 1.0f,//R
						1.0f, 1.0f, 0.0f,//B
						1.0f, 0.0f, 1.0f))),//L))),
			rotLeaf);

		addSimpleDIF(
			rotLeaf,
			gLeaf,
			PDFAB<S, V8, V8, V8>(new LinCombFilterAction<S, V8, CircBufL>(1.0f, 1.0f)),
			outLeaf);

		addSimpleSIF(
			outLeaf,
			PFAB<S, V8, V8>
				(new QuickCompressFilterAction<S, V8, CircBufL>(V8(1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f))),
			outLeaf);

		addBufferedOutF(
			outLeaf,
			3,
			PFAB<S, V8, V8>
			(new ConvolveFilterAction<S, V8, CircBufL>(std::vector<S>{0.25f, 0.5f, 0.25f})),
			outLeaf);

		mInputCamP = PIB_Wrapper::Unwrap<TimedDatum<S, V3>>(getInput(0));
		mInputCamFrame = PIB_Wrapper::Unwrap<TimedDatum<S, M3>>(getInput(1));
		mInputCamFrame2 = PIB_Wrapper::Unwrap<TimedDatum<S, M3>>(getInput(3));
		mInputCamXY = PIB_Wrapper::Unwrap<TimedDatum<S, V6>>(getInput(2));
		setOutput(outLeaf,PIB_Wrapper::Wrap((std::shared_ptr<PostboxInputBase<TimedDatum<S, V8>>>)mOutput));*/
	}

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

	//args: leaf index
	// return leafIndex, inputIndex = nil
	static int AccelByRegressionFilterPoint(lua_State* L) {
		return VestivizPipeline::AddBufferedSIF_lua<V3,V3, DIPW_point<S>>(L, PFAB<S, V3, V3>
			(new AccelByRegressionFilterAction<S, V3, CircBufL>()), 0);
	}

	//args: {x,y,z}, leaf index
	// return leafIndex, inputIndex = nil
	static int StaticAddFilterPoint(lua_State* L) {
		if (!lua_istable(L, 1)) return 0;
		lua_gettable(L, 1);
		std::array<S, 3> x;
		if (!PopTopVec3(L, x)) return 0;

		return VestivizPipeline::AddSimpleSIF_lua<V3, V3, DIPW_point<S>>(
			L, 
			PFAB<S, V3, V3>(new StatAddFilterAction<S, V3, CircBufL>(V3(x[0], x[1], x[2]))), 1);
	}

	//args: leaf index1, leaf index2
	// return leafIndex, inputIndex1 = nil, inputIndex2 = nil
	static int DynMatMultFilterPoint(lua_State* L) {
		return VestivizPipeline::AddSimpleDIF_lua<V3, M3, V3, DIPW_point<S>>(
			L,
			PDFAB<S, V3, M3, V3>(new DynMatMultFilterAction<S, V3, V3, M3, CircBufL, CircBufL>()), 0);
	}

	//args: {x,y,z}, leaf index
	// return leafIndex, inputIndex = nil
	static int QuickCompressFilterPoint(lua_State* L) {
		if (!lua_istable(L, 1)) return 0;
		lua_gettable(L, 1);
		std::array<S, 3> x;
		if (!PopTopVec3(L, x)) return 0;

		return VestivizPipeline::AddSimpleSIF_lua<V3, V3, DIPW_point<S>>(
			L,
			PFAB<S, V3, V3>(new QuickCompressFilterAction<S, V3, CircBufL>(V3(x[0], x[1], x[2]))), 1);
	}

	//args: halflife, leaf index
	// return leafIndex, inputIndex = nil
	static int ExpDecayFilterPoint(lua_State* L) {
		if (!lua_isnumber(L, 1)) return 0;
		double hl = lua_tonumber(L, 1);

		return VestivizPipeline::AddSimpleSIF_lua<V3, V3, DIPW_point<S>>(
			L,
			PFAB<S, V3, V3>(new ExpDecayFilterAction<S, V3, CircBufL>(hl)), 1);
	}

	//args: {1=,2=,...,24 = }, leaf index
	// return leafIndex, inputIndex = nil
	static int M8x3Mult(lua_State* L) {
		if (!lua_istable(L, 1)) return 0;
		lua_gettable(L, 1);
		std::array<S, 24> m;
		if (!PopArray<24>(L, x)) return 0;

		return VestivizPipeline::AddSimpleSIF_lua<V3, V8, DIPW_point<S>>(
			L,
			PFAB<S, V3, V3>(new StatMatMultFilterAction<S, V3, M8x3, V8, CircBufL>(m)), 1);
	}

	//args:  leaf index
	// return leafIndex, inputIndex = nil
	static int SimpleDiffFilterXY(lua_State* L) {
		return VestivizPipeline::AddSimpleSIF_lua<V6, V6, DIPW_xy<S>>(
			L,
			PFAB<S, V6, V6>(new SimpleDiffFilterAction<S, V6, CircBufL>())), 0);
	}

	//args:  leaf index
	// return leafIndex, inputIndex = nil
	static int SimpleDiffFilterPoint(lua_State* L) {
		return VestivizPipeline::AddSimpleSIF_lua<V3, V3, DIPW_point<S>>(
			L,
			PFAB<S, V3, V3>(new SimpleDiffFilterAction<S, V3, CircBufL>())), 0);
	}

	//args: leaf index1, leaf index2
	// return leafIndex, inputIndex1 = nil, inputIndex2 = nil
	static int DynMatMultPickFilterXYtoPoint(lua_State* L) {
		if (!lua_istable(L, 1)) return 0;
		lua_gettable(L, 1);
		std::array<std::tuple<std::size_t, std::size_t>, 3> m;
		if (!PopTupleArray<3>(L, m)) return 0;

		return VestivizPipeline::AddSimpleDIF_lua<V6, M3, V3, DIPW_xy<S>>(
			L,
			PDFAB<S, V6, M3, V3>(new DynMatMultPickFilterAction<S, S, 3, 3, 6, 3, CircBufL, CircBufL>(m), 1);
	}

	//args: coeff1,coeff2, leaf index1, leaf index2
	// return leafIndex, inputIndex1 = nil, inputIndex2 = nil
	static int LinCombV8Filter(lua_State* L) {
		if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2)) return 0;
		double coeff1 = lua_tonumber(L, 1);
		double coeff2 = lua_tonumber(L, 2);

		return VestivizPipeline::AddSimpleDIF_lua<V8, V8, V8, DIPW_woff<S>>(
			L,
			PDFAB<S, V6, M3, V3>(new LinCombFilterAction<S, V8, CircBufL>(coeff1, coeff2), 2);
	}
/*

	addSimpleSIF(
		gLeaf,
		PFAB<S, V3, V8>
		(new StatMatMultFilterAction<S, V3, M8x3, V8, CircBufL>(
			M8x3(0.5f, -0.5f, 0.0f,//T width
				0.5f, 0.0f, -0.5f,//R width
				0.5f, 0.5f, 0.0f,//B width
				0.5f, 0.0f, 0.5f,//L width
				0.0f, 0.0f, 1.0f, //T somatograv
				1.0f, 0.0f, 1.0f,//R
				0.0f, 0.0f, -1.0f,//B
				1.0f, 0.0f, -1.0f))),//L))),
		gLeaf);*/
	/*
	* Output: TRBL widths,
	* TRBL displacement (Top Left origin)
	*/
	/*addBufferedSIF(NEW_INPUT, 2, PFAB<S, V6, V6>
		(new SimpleDiffFilterAction<S, V6, CircBufL>()), rotLeaf);

	addSimpleDIF(
		rotLeaf,
		NEW_INPUT,
		PDFAB<S, V6, M3, V3>(new DynMatMultPickFilterAction<S, S, 3, 3, 6, 3, CircBufL, CircBufL>(
			std::array<std::tuple<std::size_t, std::size_t>, 3>{
		std::tuple<std::size_t, std::size_t>(2, 1), //x-axis rot
			std::tuple<std::size_t, std::size_t>(2, 0), // negative y-axis rot
			std::tuple<std::size_t, std::size_t>(1, 0) //z-axis rot
	}
	)),
		rotLeaf);

	addSimpleDIF(
		rotLeaf,
		gLeaf,
		PDFAB<S, V8, V8, V8>(new LinCombFilterAction<S, V8, CircBufL>(1.0f, 1.0f)),
		outLeaf);

	addSimpleSIF(
		outLeaf,
		PFAB<S, V8, V8>
		(new QuickCompressFilterAction<S, V8, CircBufL>(V8(1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f))),
		outLeaf);

	addBufferedOutF(
		outLeaf,
		3,
		PFAB<S, V8, V8>
		(new ConvolveFilterAction<S, V8, CircBufL>(std::vector<S>{0.25f, 0.5f, 0.25f})),
		outLeaf);*/

	static int luaNew(lua_State* L) {
		//Create new lua table encapsulating an instance and push it to the lua stack
	}
};


#endif