/*
* Copyright 2008 Free Software Foundation, Inc.
* Copyright 2011, 2013, 2014 Range Networks, Inc.
*
* This software is distributed under multiple licenses;
* see the COPYING file in the main directory for licensing
* information for this specific distribuion.
*
* This use of this software may be subject to additional restrictions.
* See the LEGAL file in the main directory for details.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/

/*
 * SmqReader.h
 *
 *  Created on: Nov 16, 2013
 *      Author: Scott
 */

#ifndef SMQREADER_H_
#define SMQREADER_H_


class SmqReader {
public:
	static std::string MQ_NAME;

	SmqReader() {
		// create message handler
		mqueHan = new SmqMessageHandler(SmqReader::MQ_NAME);
		// Start reader thread
		pthread_create(&mthread_ID, NULL, SmqReaderThread, (void*) NULL);
	}

	~SmqReader();
	static void* SmqReaderThread(void * ptr);

	// get handle to message queue
	SmqMessageHandler * getqueHan() {
		return mqueHan;
	}

private:
	pthread_t mthread_ID;
	SmqMessageHandler* mqueHan;
	bool please_re_exec;
};


#endif /* SMQREADER_H_ */
