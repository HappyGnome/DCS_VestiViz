#pragma once

#ifndef _VESTIVIZPIPELINE_H_
#define _VESTIVIZPIPELINE_H_

#include "ConvOutF.h"
#include "ExpDecaySIF.h"
#include "RegDiffSIF.h"
#include "SimpleDiffSIF.h"
#include "QCompSIF.h"
#include "DynMatMultDIF.h"
#include "DynMatMultPickDIF.h"
#include "LinCombDIF.h"
#include "StatMatMultSIF.h"
#include"LogSIF.h"

#include "DatumMatrix.h"
#include "PIB_Wrapper.h"

template<typename S>
class VestivizPipeline {

	std::vector< std::shared_ptr<AsyncFilter<PIB_Wrapper>>> mFilters;

	std::shared_ptr<PostboxInputBase<TimedDatum<S, DatumArr<S, S, 3>>>> mInputCamP;
	std::shared_ptr<PostboxInputBase<TimedDatum<S, DatumMatrix<S, 3, 3>>>> mInputCamFrame;
	std::shared_ptr<PostboxInputBase<TimedDatum<S, DatumMatrix<S, 3, 3>>>> mInputCamFrame2;
	std::shared_ptr<PostboxInputBase<TimedDatum<S, DatumArr<S, S, 6>>>> mInputCamXY;

	std::shared_ptr<SimplePostbox<TimedDatum<S, DatumArr<S, S, 8>>>> mOutput;
public:

	VestivizPipeline() :mOutput(new SimplePostbox<TimedDatum<S, DatumArr<S, S, 8>>>()) {};

	void startPipeline() {
		for (auto it = mFilters.begin(); it!=mFilters.end();it++)
		{
			(*it)->startProcessing();
		}
	}
	void stopPipeline() {
		for (auto it = mFilters.begin(); it != mFilters.end(); it++)
		{
			(*it)->stopProcessing();
		}
	}
	
	void init() {
	    auto posDiff = std::shared_ptr<AsyncFilter<PIB_Wrapper>>(new RegDiffSIF<S, DatumArr<S, S, 3>, PIB_Wrapper>(8));
		auto xyDiff = std::shared_ptr<AsyncFilter<PIB_Wrapper>>(new SimpleDiffSIF<S, DatumArr<S, S, 6>, PIB_Wrapper>());
		auto toLocalFrame = std::shared_ptr<AsyncFilter<PIB_Wrapper>>(new DynMatMultDIF<S, S, PIB_Wrapper, 3, 3> ());
		auto  xyToLocalRot = std::shared_ptr<AsyncFilter<PIB_Wrapper>>(new DynMatMultPickDIF<S, S, PIB_Wrapper, 3, 3, 6, 3>(
			std::array<std::tuple<std::size_t, std::size_t>, 3>{
			std::tuple<std::size_t, std::size_t>(2,1), //x-axis rot
			std::tuple<std::size_t, std::size_t>(2, 0), // negative y-axis rot
			std::tuple<std::size_t, std::size_t>(1, 0) //z-axis rot
			}));

		auto gCompress = std::shared_ptr<AsyncFilter<PIB_Wrapper>>(new QCompSIF<S, DatumArr<S, S, 3>, PIB_Wrapper> (DatumArr<S, S, 3>(1.0f,1.0f,1.0f)));
		auto  rotCompress = std::shared_ptr<AsyncFilter<PIB_Wrapper>>(new QCompSIF<S, DatumArr<S, S, 3>, PIB_Wrapper>(DatumArr<S, S, 3>(1.0f, 1.0f, 1.0f)));

		auto gDecay = std::shared_ptr<AsyncFilter<PIB_Wrapper>>(new ExpDecaySIF<S, DatumArr<S, S, 3>, PIB_Wrapper> (1.0f));
		auto rotDecay = std::shared_ptr<AsyncFilter<PIB_Wrapper>>(new ExpDecaySIF<S, DatumArr<S, S, 3>, PIB_Wrapper> (1.0f));
		
		/*
		* Output: TRBL widths,
		*		  TRBL displacement (Top Left origin)
		*/
		auto gToScreen = std::shared_ptr<AsyncFilter<PIB_Wrapper>>(new StatMatMultSIF<S, S, PIB_Wrapper, 8, 3> (DatumMatrix<S, 8, 3>
			(0.5f, -0.5f, 0.0f,//T width
			0.5f, 0.0f, -0.5f,//R width
			0.5f, 0.5f, 0.0f,//B width
			0.5f, 0.0f, 0.5f,//L width
			0.0f, 0.0f, 1.0f, //T somatograv
			1.0f, 0.0f, 1.0f,//R
			0.0f, 0.0f, -1.0f,//B
			1.0f, 0.0f, -1.0f)));//L

		auto rotToScreen = std::shared_ptr<AsyncFilter<PIB_Wrapper>>(new StatMatMultSIF<S, S, PIB_Wrapper, 8, 3> (DatumMatrix<S, 8, 3>
			(0.0f, 0.0f, 0.0f,//T width
			0.0f, 0.0f, 0.0f,//R
			0.0f, 0.0f, 0.0f,//B
			0.0f, 0.0f, 0.0f,//L 
			-1.0f, 1.0f, 0.0f, //T displacement
			-1.0f, 0.0f, 1.0f,//R
			1.0f, 1.0f, 0.0f,//B
			1.0f, 0.0f, 1.0f)));//L

		auto gRotCombiner = std::shared_ptr<AsyncFilter<PIB_Wrapper>>(new LinCombDIF<S, DatumArr<S, S, 8>, PIB_Wrapper> (1.0f,1.0f));

		auto finalCompress = std::shared_ptr<AsyncFilter<PIB_Wrapper>>(new QCompSIF<S, DatumArr<S, S, 8>, PIB_Wrapper> 
			(DatumArr<S, S, 8>(1.0f,1.0f,1.0f,1.0f, 1.0f, 1.0f, 1.0f, 1.0f)));

		auto finalConvolve = std::shared_ptr<AsyncFilter<PIB_Wrapper>>(new ConvOutF<S, DatumArr<S, S, 8>, PIB_Wrapper> 
			(std::vector<S>{0.25f,0.5f,0.25f}));

#ifdef _DEBUG_LOG_PIPE
		auto lg1 = std::shared_ptr<AsyncFilter<PIB_Wrapper>>(new LogSIF<S, DatumArr<S, S, 3>, PIB_Wrapper>("g1 "));
		auto lg2 = std::shared_ptr<AsyncFilter<PIB_Wrapper>>(new LogSIF<S, DatumArr<S, S, 3>, PIB_Wrapper>("g2 "));
		auto lg3 = std::shared_ptr<AsyncFilter<PIB_Wrapper>>(new LogSIF<S, DatumArr<S, S, 3>, PIB_Wrapper>("g3 "));
		auto lg4 = std::shared_ptr<AsyncFilter<PIB_Wrapper>>(new LogSIF<S, DatumArr<S, S, 3>, PIB_Wrapper>("g4 "));
		auto lg5 = std::shared_ptr<AsyncFilter<PIB_Wrapper>>(new LogSIF<S, DatumArr<S, S, 8>, PIB_Wrapper>("g5 "));
		auto lc1 = std::shared_ptr<AsyncFilter<PIB_Wrapper>>(new LogSIF<S, DatumArr<S, S, 8>, PIB_Wrapper>("c1 "));
		auto lc2 = std::shared_ptr<AsyncFilter<PIB_Wrapper>>(new LogSIF<S, DatumArr<S, S, 8>, PIB_Wrapper>("c2 "));
		auto lr1 = std::shared_ptr<AsyncFilter<PIB_Wrapper>>(new LogSIF<S, DatumArr<S, S, 6>, PIB_Wrapper>("r1 "));
		auto lr2 = std::shared_ptr<AsyncFilter<PIB_Wrapper>>(new LogSIF<S, DatumArr<S, S, 3>, PIB_Wrapper>("r2 "));
		auto lr3 = std::shared_ptr<AsyncFilter<PIB_Wrapper>>(new LogSIF<S, DatumArr<S, S, 3>, PIB_Wrapper>("r3 "));
		auto lr4 = std::shared_ptr<AsyncFilter<PIB_Wrapper>>(new LogSIF<S, DatumArr<S, S, 3>, PIB_Wrapper>("r4 "));
		auto lr5 = std::shared_ptr<AsyncFilter<PIB_Wrapper>>(new LogSIF<S, DatumArr<S, S, 8>, PIB_Wrapper>("r5 "));
#endif // _DEBUG_LOG_PIPE


#ifdef _DEBUG_LOG_PIPE
		posDiff->setOutput(lg1->getInput(0));
		lg1->setOutput(toLocalFrame->getInput(0));
		xyDiff->setOutput(lr1->getInput(0));
		lr1->setOutput(xyToLocalRot->getInput(0));
		toLocalFrame->setOutput(lg2->getInput(0));
		lg2->setOutput(gCompress->getInput(0));
		xyToLocalRot->setOutput(lr2->getInput(0));
		lr2->setOutput(rotCompress->getInput(0));
		gCompress->setOutput(lg3->getInput(0));
		lg3->setOutput(gDecay->getInput(0));
		rotCompress->setOutput(rotDecay->getInput(0));
		gDecay->setOutput(lg4->getInput(0));
		lg4->setOutput(gToScreen->getInput(0));
		rotDecay->setOutput(rotToScreen->getInput(0));
		gToScreen->setOutput(lg5->getInput(0));
		lg5->setOutput(gRotCombiner->getInput(0));
		rotToScreen->setOutput(gRotCombiner->getInput(1));
		gRotCombiner->setOutput(lc1->getInput(0));
		lc1->setOutput(finalCompress->getInput(0));
		finalCompress->setOutput(lc2->getInput(0));
		lc2->setOutput(finalConvolve->getInput(0));
#else
		posDiff->setOutput(toLocalFrame->getInput(0));
		xyDiff->setOutput(xyToLocalRot->getInput(0));
		toLocalFrame->setOutput(gCompress->getInput(0));
		xyToLocalRot->setOutput(rotCompress->getInput(0));
		gCompress->setOutput(gDecay->getInput(0));
		rotCompress->setOutput(rotDecay->getInput(0));
		gDecay->setOutput(gToScreen->getInput(0));
		rotDecay->setOutput(rotToScreen->getInput(0));
		gToScreen->setOutput(gRotCombiner->getInput(0));
		rotToScreen->setOutput(gRotCombiner->getInput(1));
		gRotCombiner->setOutput(finalCompress->getInput(0));
		finalCompress->setOutput(finalConvolve->getInput(0));
#endif // _DEBUG_LOG_PIPE

		mInputCamP = PIB_Wrapper::Unwrap<TimedDatum<S, DatumArr<S, S, 3>>>(posDiff->getInput(0));
		mInputCamFrame = PIB_Wrapper::Unwrap<TimedDatum<S, DatumMatrix<S, 3, 3>>>(toLocalFrame->getInput(1));
		mInputCamFrame2 = PIB_Wrapper::Unwrap<TimedDatum<S, DatumMatrix<S, 3, 3>>>(xyToLocalRot->getInput(1));
		mInputCamXY = PIB_Wrapper::Unwrap<TimedDatum<S, DatumArr<S, S, 6>>>(xyDiff->getInput(0));
		finalConvolve->setOutput(PIB_Wrapper::Wrap((std::shared_ptr<PostboxInputBase<TimedDatum<S, DatumArr<S, S, 8>>>>)mOutput));

		mFilters.push_back(posDiff);
		mFilters.push_back(xyDiff);
		mFilters.push_back(toLocalFrame);
		mFilters.push_back(xyToLocalRot);
		mFilters.push_back(gCompress);
		mFilters.push_back(rotCompress);
		mFilters.push_back(gDecay);
		mFilters.push_back(rotDecay);
		mFilters.push_back(gToScreen);
		mFilters.push_back(rotToScreen);
		mFilters.push_back(gRotCombiner);
		mFilters.push_back(finalCompress);
		mFilters.push_back(finalConvolve);

#ifdef _DEBUG_LOG_PIPE
		mFilters.push_back(lg1);
		mFilters.push_back(lg2);
		mFilters.push_back(lg3);
		mFilters.push_back(lg4);
		mFilters.push_back(lg5);
		mFilters.push_back(lc1);
		mFilters.push_back(lc2);
		mFilters.push_back(lr1);
		mFilters.push_back(lr2);
#endif // _DEBUG_LOG_PIPE
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