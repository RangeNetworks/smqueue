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
 * SmqWriter.h
 *
 *  Created on: Nov 16, 2013
 *      Author: Scott Van Gundy
 */

#ifndef SMQWRITER_H_
#define SMQWRITER_H_


class SmqWriter {
public:
	static std::string MQ_NAME;

	SmqWriter() {
		mqueHan = new SmqMessageHandler(SmqWriter::MQ_NAME);
		pthread_create(&mthread_ID, NULL, SmqWriterThread, (void*) NULL);
	}

	~SmqWriter();
	static void* SmqWriterThread(void * ptr);

	SmqMessageHandler* getqueHan() {
		return mqueHan;
	}

private:
	pthread_t mthread_ID;
	SmqMessageHandler* mqueHan;
};

#endif /* SMQWRITER_H_ */
