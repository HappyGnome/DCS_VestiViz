#pragma once

#ifndef _ERRORSTACKBASE_H_
#define _ERRORSTACKBASE_H_

#include<vector>
#include<string>
#include<mutex>

class ErrorStack {
	std::vector<std::string> mMessages;
	std::mutex mMessagesMutex;
public:
	bool pop_message(std::string& message);
	void push_message(const std::string& message);
	void push_exception(const std::exception& e);
};

#endif