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

#include "DatumMatrix.h"
#include "PIB_Wrapper.h"

template<typename S>
class VestivizPipeline {

	std::vector< std::shared_ptr<AsyncFilter<PIB_Wrapper>>> mFilters;

	std::shared_ptr<PostboxInputBase<TimedDatum<S, DatumArr<S, S, 3>>>> mInputCamP;
	std::shared_ptr<PostboxInputBase<TimedDatum<S, DatumMatrix<S, 3, 3>>>> mInputCamFrame;
	std::shared_ptr<PostboxInputBase<TimedDatum<S, DatumMatrix<S, 3, 3>>>> mInputCamFrame2;
	std::shared_ptr<PostboxInputBase<TimedDatum<S, DatumArr<S, S, 6>>>> mInputCamXY;

	std::shared_ptr<PostboxInputBase<TimedDatum<S, DatumArr<S, S, 8>>>> mOutput;
public:

	VestivizPipeline() :mOutput(new SimplePostbox<TimedDatum<S, DatumArr<S, S, 8>>>()) {};

	void startPipeline() {
		for (auto it = mFilters.begin(); it!=mFilters.end();it++)
		{
			it->startProcessing();
		}
	}
	void stopPipeline() {
		for (auto it = mFilters.begin(); it != mFilters.end(); it++)
		{
			it->stopProcessing();
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


		mInputCamP = PIB_Wrapper::Unwrap<TimedDatum<S, DatumArr<S, S, 3>>>(posDiff->getInput(0));
		mInputCamFrame = PIB_Wrapper::Unwrap<TimedDatum<S, DatumMatrix<S, 3, 3>>>(toLocalFrame->getInput(1));
		mInputCamFrame2 = PIB_Wrapper::Unwrap<TimedDatum<S, DatumMatrix<S, 3, 3>>>(xyToLocalRot->getInput(1));
		mInputCamXY = PIB_Wrapper::Unwrap<TimedDatum<S, DatumArr<S, S, 6>>>(xyDiff->getInput(0));
		finalConvolve->setOutput(PIB_Wrapper::Wrap(mOutput));

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
	}

	void addDatum(S t, DatumMatrix<S, 3, 3> && frame, DatumArr<S, S, 3>&& point) {
		if (mInputCamP == nullptr || mInputCamFrame == nullptr) return;

		mInputCamP->addDatum(TimedDatum<S, DatumArr<S, S, 3>> {t, point});
		mInputCamFrame->addDatum(TimedDatum<S, DatumMatrix<S, 3, 3>> {t, frame});
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