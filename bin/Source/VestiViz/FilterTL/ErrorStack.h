#pragma once

#ifndef _ERRORSTACKBASE_H_
#define _ERRORSTACKBASE_H_

#include<vector>
#include<string>
#include<mutex>
#include "CircBuf.h"

class ErrorStack {
	CircBufL<std::string> mMessages;
	std::mutex mMessagesMutex;
public:
	explicit ErrorStack(std::size_t size);
	bool pop_message(std::string& message);
	void push_message(const std::string& message);
	void push_exception(const std::exception& e);
};

#endif