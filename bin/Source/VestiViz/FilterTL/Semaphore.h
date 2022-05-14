#pragma once
#ifndef _SEMAPHORE_H_
#define _SEMAPHORE_H_

#include<mutex>
#include <condition_variable>

class Semaphore {
	std::mutex mMutex;
	unsigned long mCount;

};

#endif