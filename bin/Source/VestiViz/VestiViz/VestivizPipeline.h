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

template<typename S>
class VestivizPipeline : public PipelineBase<PIB_Wrapper> {

	std::shared_ptr<PostboxInputBase<TimedDatum<S, DatumArr<S, S, 3>>>> mInputCamP;
	std::shared_ptr<PostboxInputBase<TimedDatum<S, DatumMatrix<S, 3, 3>>>> mInputCamFrame;
	std::shared_ptr<PostboxInputBase<TimedDatum<S, DatumMatrix<S, 3, 3>>>> mInputCamFrame2;
	std::shared_ptr<PostboxInputBase<TimedDatum<S, DatumArr<S, S, 6>>>> mInputCamXY;

	std::shared_ptr<SimplePostbox<TimedDatum<S, DatumArr<S, S, 8>>>> mOutput;
public:

	VestivizPipeline() :mOutput(new SimplePostbox<TimedDatum<S, DatumArr<S, S, 8>>>()) {};
	
	void init() {
		std::size_t gLeaf;
		std::size_t rotLeaf;
		std::size_t outLeaf;

		using V3 = DatumArr<S, S, 3>;
		using V6 = DatumArr<S, S, 6>;
		using V8 = DatumArr<S, S, 8>;
		using M3 = DatumMatrix<S, 3, 3>;
		using M8x3 = DatumMatrix<S, 8, 3>;

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
		/*
		* Output: TRBL widths,
		* TRBL displacement (Top Left origin)
		*/
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
		setOutput(outLeaf,PIB_Wrapper::Wrap((std::shared_ptr<PostboxInputBase<TimedDatum<S, V8>>>)mOutput));
	}

	void addDatum(S t, DatumMatrix<S, 3, 3> && frame, DatumArr<S, S, 3>&& point) {
		if (mInputCamP == nullptr || mInputCamFrame == nullptr) return;

		mInputCamP->addDatum(TimedDatum<S, DatumArr<S, S, 3>> (t, std::move(point)));

		std::array<S, 6> xyVec;
		for (std::size_t i = 0; i < 3; i++) {
			xyVec[i] = frame[i];
			xyVec[i+3] = frame[i+3];
		}
		
		mInputCamXY->addDatum(TimedDatum<S, DatumArr<S, S, 6>>(t, DatumArr<S, S, 6>(xyVec)));
		mInputCamFrame->addDatum(TimedDatum<S, DatumMatrix<S, 3, 3>> (t, DatumMatrix<S, 3, 3>(frame)));
		mInputCamFrame2->addDatum(TimedDatum<S, DatumMatrix<S, 3, 3>>(t, std::move(frame)));
		
	}

	//Get Datum
	TimedDatum<S, DatumArr<S, S, 8>> getDatum() {
		if (mOutput == nullptr) return TimedDatum<S, DatumArr<S, S, 8>>::zero();
		mOutput->flushPost();
		if(mOutput->empty()) return TimedDatum<S, DatumArr<S, S, 8>>::zero();
		return *(mOutput->output().cbegin());
	}
};


#endif