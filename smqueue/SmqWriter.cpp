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
 * SmqWriter.cpp
 *
 *  Created on: Nov 16, 2013
 *      Author: Scott Van Gundy
 */


#include <string>
#include "smqueue.h"
#include "QueuedMsgHdrs.h"

#include "SmqMessageHandler.h"
#include "SmqWriter.h"
#include <Logger.h>

extern SmqWriter* smqWriter;


extern SMqueue::SMq smq;

std::string SmqWriter::MQ_NAME = "/SmqWriter";



SmqWriter::~SmqWriter() {
	delete mqueHan;
}


void* SmqWriter::SmqWriterThread(void * ptr) {
	LOG(DEBUG) <<"Start SMQ writer thread";

	char msgBuffer[MQ_MESSAGE_MAX_SIZE+10];
	int bytesRead = 0;
	int msgCount = 0;
	SimpleWrapper* pWrap;
	QueuedMsgHdrs* pMsg;
	unsigned long currentSeconds;
	unsigned long lastRunSeconds;
	SMqueue::short_msg_pending pendingMsg;

	// Queue opened  Process messages
	currentSeconds = getCurrentSeconds();
	lastRunSeconds = currentSeconds;
	LOG(DEBUG) << "Enter writer thread loop";
	while (!smq.stop_main_loop) {

		//LOG(DEBUG) <<"Start SMQ writer thread loop";
		bytesRead = smqWriter->getqueHan()->SmqWaitforMessage(200, msgBuffer, (int) sizeof(msgBuffer));
		currentSeconds = getCurrentSeconds();
		//LOG(DEBUG) << "Got return from SmqWaitforMessage in writer status:" << bytesRead;
		if (bytesRead < 0) {
			LOG(DEBUG) << "Writer failed to get message:" << bytesRead;

		} else if (bytesRead == 0) {
			// ******** Got timeout ***************
			//LOG(DEBUG) << "Got timeout in writer thread";

			smq.process_timeout();  // Process entries in queue
			//LOG(DEBUG) << "Return from process_timeout";

			if ((currentSeconds - lastRunSeconds ) > 60) {
				LOG(DEBUG) << "Run once a minute stuff";
				smq.InitInsideReaderLoop(); // Updates configuration

				int queueSize = smq.time_sorted_list.size();
				if (queueSize > 0) { LOG(DEBUG) << "Queue size " << queueSize;}
				// Save queue to file on timeout
				//LOG(DEBUG) << "Enter save_queue_to_file";
				if (!smq.save_queue_to_file(smq.savefile)) {  // Save queue file each timeout  may want to slow this down
					LOG(WARNING) << "Failed to read queue file on timeout file:" << smq.savefile;
				}
				lastRunSeconds = currentSeconds;
			}

		} else if (bytesRead > 0) {
			// ********* Got message *************
			//LOG(DEBUG) << "Received message in writer thread length:" << bytesRead;
			pWrap = (SimpleWrapper*) msgBuffer;
			pMsg = pWrap->getMsgPtr();

			// PROCESS MESSAGES HERE
			pMsg->ProcessMessage();

			delete pMsg;
		} // Got message
	} // while

	LOG(DEBUG) << "End SMQ writer thread";
	return NULL;
}




