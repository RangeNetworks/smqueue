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
 * SmqReader.cpp
 *
 *  Created on: Nov 16, 2013
 *      Author: Scott Van Gundy
 */

#include <string>
#include "smqueue.h"
#include "QueuedMsgHdrs.h"
#include "SmqMessageHandler.h"
#include "SmqReader.h"

#include <Logger.h>

std::string SmqReader::MQ_NAME = "/SmqReader";

extern SmqReader* smqReader;


SmqReader::~SmqReader() {
	delete mqueHan;
}


void* SmqReader::SmqReaderThread(void* ptr) {
	LOG(DEBUG) <<"Start SMQ reader thread";

	char msgBuffer[MQ_MESSAGE_MAX_SIZE+10];  // Must be larger than size speced in attr
	int bytesRead = 0;
	int msgCount = 0;

	smq.InitBeforeMainLoop();
	smq.InitInsideReaderLoop(); // Updates configuration  do here to make sure everything is setup for threads

	// Queue opened  Process messages
	LOG(DEBUG) << "Enter reader thread loop";
	while (!smq.stop_main_loop) {

		smq.main_loop(60000);

#if 0
		// Put this back in if process messages in reader thread
		LOG(DEBUG) <<"Start SMQ reader thread loop";
		bytesRead = smqReader->getqueHan()->SmqWaitforMessage(2000, msgBuffer, sizeof(msgBuffer));  // Wait for message

		//LOG(DEBUG) << "Got return from SmqWaitforMessage in reader thread status:%d", bytesRead);
		if (bytesRead < 0) {
			// GOT ERROR
			LOG(DEBUG) << "Reader failed to get message:" << bytesRead;

		} else if (bytesRead == 0) {
			// TIMEOUT
			LOG(DEBUG) << "Got timeout waiting for reader message";
			//Add timeout stuff here

			//LOG(DEBUG) << "Enter InitInsideReaderLoop";
			smq.InitInsideReaderLoop(); // Updates configuration  Moved this to writer queue
			//LOG(DEBUG) << "Return from InitInsideReaderLoop";

			// This is where all the work is done
			// Message are read in here
			//LOG(DEBUG) << "Enter main_loop";
			smq.main_loop(2000);
			//LOG(DEBUG) << "Return from main_loop";

		} else if (bytesRead > 0) {
			// GOT MESSAGE
			LOG(DEBUG) << "Received message in reader thread length;" << bytesRead;
			// PROCESS MESSAGES HERE
#if 0
			switch (msgType) {
			case QueuedMsgHdrs::TestMessage:

				break;

			case QueuedMsgHdrs::SendResponse:   // this is a writer message

				break;
			} // switch
#endif
		}
#endif
	} // while

	smq.CleaupAfterMainreaderLoop();

	LOG(DEBUG) << "End SMQ reader thread";
	return NULL;
}


