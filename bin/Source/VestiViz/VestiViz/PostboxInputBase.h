#pragma once


#ifndef _POSTBOXINPUTBASE_H_
#define _POSTBOXINPUTBASE_H_

template <typename T>
class PostboxInputBase {
public:
	/**
	 * Add datum to the postbox.  
	 * Call from any thread. 
	 * 
	 * @param input Data to post
	 * @return false if postbox is in a cancelled state, or already cancelled
	 */
	virtual bool addDatum(const T& input,bool waitForRead = false) = 0;

	/**
	 * Add rvalue datum to the postbox.  
	 * Call from any thread. 
	 * 
	 * @param input Data to post
	 * @return false if postbox is in a cancelled state, or already cancelled
	 */
	virtual bool addDatum(T&& input, bool waitForRead = false) = 0;

	/**
	 * Close the postbox. Further blocking calls will return false. 
	 * Call from any thread.
	 * @return
	 */
	virtual void cancel() = 0;

};
#endif