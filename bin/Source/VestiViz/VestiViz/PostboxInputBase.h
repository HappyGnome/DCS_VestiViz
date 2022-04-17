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
	 */
	virtual void addDatum(const T& input) = 0;

	/**
	 * Add rvalue datum to the postbox.  
	 * Call from any thread. 
	 * 
	 * @param input Data to post
	 */
	virtual void addDatum(T&& input) = 0;

	/**
	 * Add rvalue datum to the postbox if the read receipt flag matches the given value.
	 * Call from any thread.
	 *
	 * @param input: Data to post, matchRead: read receipt status must match this to post the data
	 * @return true if datum posted
	 */
	virtual bool addDatumIfReadMatches(T&& input, bool match) = 0;

	/**
	 * Add datum to the postbox if the read receipt flag matches the given value.
	 * Call from any thread.
	 *
	 * @param input: Data to post, matchRead: read receipt status must match this to post the data
	 * @return true if datum posted
	 */
	virtual bool addDatumIfReadMatches(T& input, bool match) = 0;

	virtual void resetRead() = 0;

	/**
	 * Close the postbox. Further waitForPost calls will return false. 
	 * Call from any thread.
	 * @return
	 */
	virtual void cancel() = 0;

};
#endif