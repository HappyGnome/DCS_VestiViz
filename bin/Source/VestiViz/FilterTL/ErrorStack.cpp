#include "ErrorStack.h"

bool ErrorStack::pop_message(std::string& message) {
	std::lock_guard<std::mutex> guard(mMessagesMutex);
	if (mMessages.empty())return false;
	message = mMessages.back();
	mMessages.pop_back();
	return true;
}

void ErrorStack::push_message(const std::string& message) {
	std::lock_guard<std::mutex> guard(mMessagesMutex);
	mMessages.push_back(message);
}

void ErrorStack::push_exception(const std::exception& e) {
	push_message(std::string(e.what()));
}