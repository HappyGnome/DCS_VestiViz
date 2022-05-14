#pragma once


#ifndef _FILTERACTIONBASE_H_
#define _FILTERACTIONBASE_H_

template <typename IOWrapper>
class FilterActionBase {
public:
	virtual ~FilterActionBase() = default;

	virtual bool action() = 0;
	virtual std::size_t inputCount() = 0;

	virtual bool setOutput(typename IOWrapper::Wrapped&& wrappedInput) = 0;
	virtual typename IOWrapper::Wrapped getInput(int index, bool enableBlocking = true) const= 0;
	virtual void setBlockForOutput(bool enable) = 0;
	virtual void cancel() = 0;
};

#endif