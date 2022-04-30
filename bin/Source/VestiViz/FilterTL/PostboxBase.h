#pragma once


#ifndef _POSTBOXBASE_H_
#define _POSTBOXBASE_H_

#include "PostboxInputBase.h"

template <typename T, template<typename,typename> typename L, typename LAlloc = std::allocator<T>>
class PostboxBase : public PostboxInputBase<T> {
public:

	/**
	 * Get output buffer object.
	 * Call from receiving thread only.
	 *
	 * @return iterator to beginning of output.
	 */
	virtual const L<T,LAlloc>& output() const = 0;

	/**
	 * Test for empty output.
	 * Call from receiving thread only.
	 *
	 * @return true if output is empty.
	 */
	virtual bool empty() const = 0;

	/**
	 * Wait until new data is ready for output.
	 * Call from receiving thread only.
	 *
	 * @return true if data is ready, false if the operation was cancelled.
	 */
	virtual bool waitForPost() = 0;

	/**
	 * Get new data for output, or return immediately if none is available
	 * Call from receiving thread only.
	 * 
	 * @return true if data is ready, false if the operation was cancelled.
	 */
	virtual bool flushPost() = 0;

};
#endif