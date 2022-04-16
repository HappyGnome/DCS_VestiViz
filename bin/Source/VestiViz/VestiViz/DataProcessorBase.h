#pragma once

#ifndef _DATAPROCESSORBASE_H_
#define _DATAPROCESSORBASE_H_

//#include<vector>
#include<thread>
#include<mutex>
#include <condition_variable>

#include"CircBuf.h"
#include"DifDatumVec.h"

template <typename Tin, typename Tout>
class DataProcessorBase {

	bool mCancelled = false;
	CircBuf<Tin> mOuterBuf;
	std::mutex mOuterBufMutex;
	std::condition_variable  mOuterBufCV;

	//Data passes from the outer buffer into the inner buffer
	CircBuf<Tin> mInnerBuf;

	Tout mLatest;
	bool mHasLatest;
	std::mutex mLatestMutex;	

	std::thread mWorkerThread;

	void _workerFunc() {
		while (true)
		{
			{//mOuterBufMutex lock scope
				std::lock_guard<std::mutex> lock(mOuterBufMutex);
				if (mCancelled) return;
				while (mBuf.empty()) {
					mOuterBufCV.wait(mOuterBufMutex);
					if (mCancelled) return;
				}
				mInnerBuf.collectfrom(mOuterBuf);
			}

			Tout newLatest = processStep();

			{//mLatestMutex lock scope
				std::lock_guard<std::mutex> lock(mLatestMutex);
				mLatest = std::move(newLatest);
				mHasLatest = true;
			}

		}
	}
protected:

	const CircBuf<Tin>& getInnerBuf() const {return mInnerBuf;}

	/*
	* Process data in the inner buffer and return an updated output
	*/
	virtual Tout processStep() {}
public:

	void startProcessing() {
		std::thread newThread(_workerFunc);
		mWorkerThread = std::move(newThread);
	}

	void stopProcessing() {
		std::lock_guard<std::mutex> lock(mOuterBufMutex);
		mCancelled = true;
		mOuterBufCV.notify_all();
	}

	bool getLatest (Tout &output) const{
		std::lock_guard<std::mutex> lock(mLatestMutex);
		if (mHasLatest) {
			output = mLatest;
			return true;
		}
		else return false;
	}

	void addDatum(Tin& input const) {
		std::lock_guard<std::mutex> lock(mOuterBufMutex);
		mOuterBuf.push_back(input);
		mOuterBufCV.notify_all();
	}

};

#endif