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
 * SmqMessageHandler.cpp
 *
 *  Created on: Nov 16, 2013
 *      Author:
 *
 *
 */

#include "smqueue.h"
#include "QueuedMsgHdrs.h"
#include "SmqMessageHandler.h"
#include "SmqReader.h"
#include "SmqWriter.h"

// Pointers to handlers for the reader and writer queues
SmqReader* smqReader;
SmqWriter* smqWriter;


// Static function to start the threads
void SmqMessageHandler::StartThreads() {
	LOG(INFO) << "Start reader and writer threads";
	smqReader = new SmqReader();
	smqWriter = new SmqWriter();
}


SmqMessageHandler::~SmqMessageHandler() {
	/* Close the message queue */
	int ret = mq_close(mqdes);
	if (ret)
		LOG(DEBUG) << "Message queue close failed";
	else
		LOG(DEBUG) << "Message queue closed";
}


/*
returns
	0 = sent okay
	-1 failed
Should only be used for test need header for all msgs
Delete after clean up

*/
int SmqMessageHandler::SmqSendMessage(SimpleWrapper * pMsg) {

	int returnValue;
	int ret;

	//LOG(DEBUG) << "Sending message to queue:" << getQueueName();;

	// Wait for queue to open
	while (!queueOpened()) {
		LOG(DEBUG) << "Try to open queue:" << getQueueName() << "in SmqSendMessage";
		mqueueOpened = SmqInitReceiver(); // Set up the queue name
		if (queueOpened()) {
			LOG(DEBUG) << "Successfully opened queue for sender queue:" << getQueueName();
			//break;
		}
		msSleep(2000);
	} // while

	ret = mq_send(mqdes, (char*) pMsg, pMsg->getSize(), 0);
	if (ret) {
		LOG(DEBUG) << "Message failed to be sent error:" << ret << " errno:" << errno;
		returnValue = -1;
	}
	else {
		//LOG(DEBUG) << "Message sent okay queue:" << getQueueName();
		returnValue = 0;
	}

	return returnValue;
}


int SmqMessageHandler::SmqDeleteMessage() {
	return 0;
}


std::string SmqMessageHandler::getQueueName() {
	return mqueueName;
}


bool SmqMessageHandler::queueOpened() {
	return mqueueOpened;
}


/*
 * Return
 *	 Message okay = size of message
 *	 -x = Failed
 *	 0 = Timeout
 *
 */
int SmqMessageHandler::SmqWaitforMessage(int TimeoutMS, char * MsgBuffer, int MsgBufferSize) {
	int msgLength = 0;
	struct mq_attr attrList;

	//LOG(DEBUG) << "Waiting for message in queue:" << getQueueName() << " tmo:" << TimeoutMS << " bufsize:" << MsgBufferSize;

	// Wait for queue to open.  This may not be needed
	while (!queueOpened()) {
		LOG(DEBUG) << "Try to open queue:" << getQueueName() << " in SmqWaitforMessage";
		mqueueOpened = SmqInitReceiver(); // Set up the queue name
		if (mqueueOpened) {
			LOG(DEBUG) << "Successfully opened queue:" << getQueueName() << " in SmqWaitforMessage";
			break;
		}
		msSleep(2000);
	} // while

	//mq_getattr(mqdes, &attrList);
	//LOG(DEBUG) << "mq_attr flags:%ld maxmsg:%ld mq_msgsize:%ld mq_curmsg:%ld",
	//attrList.mq_flags, attrList.mq_maxmsg, attrList.mq_msgsize, attrList.mq_curmsgs);

	/* get the current time at which mq_receive() is going to be called
	   i.e time since the Epoch time, 1970-01-01 00:00:00 +0000 (UTC) */
	clock_gettime(CLOCK_REALTIME, &currenttime_before);

	struct timespec wait_timeout;  // Put delay in time spec
	wait_timeout.tv_sec = (TimeoutMS / 1000);
	wait_timeout.tv_nsec = ((TimeoutMS % 1000) * 1000000);
	abs_timeout = addTimespec(currenttime_before, wait_timeout);

	// Wait for the message
	// LOG(DEBUG) << "Real enter wait for message in queue:"<< getQueueName().c_str() << " tmo:" << TimeoutMS;
	msgLength = mq_timedreceive(mqdes, MsgBuffer, MsgBufferSize, &msg_prio, &abs_timeout);

#if 0
	// only enable for debugging
	clock_gettime(CLOCK_REALTIME, &currenttime_after);

	/* Get the time at which the message queue returns */
	LOG(DEBUG) << "Message queue:%s size:%ld", mqueueName.c_str(), getMessageQueueSize());

	/* Print all time values. We didn't print anything before to avoid the delay caused by printf() */
	LOG(DEBUG) << "Time before mq_timedreceive is called sec:%ld ns:%ld ms:%lu",
				currenttime_before.tv_sec, currenttime_before.tv_nsec, getMSFromtimespec(currenttime_before));
	LOG(DEBUG) << "Absolute time passed to mq_timedreceive sec:%ld ns:%ld ms:%lu",
				abs_timeout.tv_sec, abs_timeout.tv_nsec, getMSFromtimespec(abs_timeout));
	LOG(DEBUG) << "Time at which mq_timedreceive returned sec:%ld ns:%ld ms:%lu",
				currenttime_after.tv_sec, currenttime_after.tv_nsec, getMSFromtimespec(currenttime_after));
	LOG(DEBUG) << "Actual wait time ms:%ld",
			getMSFromtimespec(currenttime_after) - getMSFromtimespec(currenttime_before));
	//LOG(DEBUG) << "Actual passed to wait ms:%lu", getMSFromtimespec(abs_timeout));
#endif

	if (msgLength < 0)
	{ 	/* mq_timedreceive returned with an error */
		//LOG(DEBUG) << "Wait for message receive got errno:%d error:%d", errno, msgLength);
		if (errno == ETIMEDOUT) {  // errno = 110
			// ********* TIMEOUT ***************************
			//LOG(DEBUG) << "Wait for message got timeout");
			msgLength = 0;
		} else {
			// *********** GOT ERROR ******************
			//Check for other errors like signal to stop EINTR
			LOG(DEBUG) << "Wait for message receive got errno:" << errno <<  " error:" << msgLength;
			// Return current value of msgLength
		}
	}
	else {
		/* *****************GOT MESSAGE **************** */
		//LOG(DEBUG) << "Successfully received %d bytes", (int) msgLength);
	}

	return msgLength;
}


/*

Returns
	true = okay

*/
bool SmqMessageHandler::SmqInitReceiver() {
	bool retValue = false;

	LOG(DEBUG) << "SmqInitReceiver queue name:" << mqueueName;

	/* Create message queue */
	mqdes = mq_open(mqueueName.c_str(), O_RDWR | O_CREAT, MQ_MODE, &attr);  // Should be this one

	if (mqdes != (mqd_t)-1 ) {
		LOG(DEBUG) << "Message queue opened:" << mqueueName;
		retValue = true;
		mqueueOpened = true;
	}
	else {
		LOG(DEBUG) << "Message Queue FAILED to open";
		LOG(DEBUG) << "mq_open error:" << errno << " mqdes:" << mqdes;
		retValue = false;
	}

	return retValue;
}


long SmqMessageHandler::getMessageQueueSize() {
	int error;
	long count = -1;
	struct mq_attr localAttr;

	error = mq_getattr(mqdes, &localAttr);
	if (!error)
		count = localAttr.mq_curmsgs;

	return count;
}


/*
 * Send a QueuedMsgHdrs to writer queue
 * Deleted SimpleWrapper receiver deletes attached message
 */
int SendWriterMsg(SimpleWrapper* pMsg) {
	int iret;
	iret = smqWriter->getqueHan()->SmqSendMessage(pMsg);
	delete pMsg;  // Delete SimpleWrapper
	return iret;
}


/*
    Send a message to the writer thread asking for a sip ack to be sent
 */
void queue_respond_sip_ack(int errcode, SMqueue::short_msg_pending *shortmsg, char * netaddr, size_t netaddrlen) {
	LOG(DEBUG) << "Send SIP ACK queue request";
	QueuedMsgHdrs* pMsg = new SIPAckMessage(errcode, shortmsg, netaddr, netaddrlen);
	SimpleWrapper* sWrap = new SimpleWrapper(pMsg);
	SendWriterMsg(sWrap);  // Message gets deleted in here
}


// Signal writer thread to process incoming message
void ProcessReceivedMsg() {
	LOG(DEBUG) << "Signal writer thread ProcessReceivedMsg";
	QueuedMsgHdrs* pMsg = new ProcessIncommingMsg();
	SimpleWrapper* sWrap = new SimpleWrapper(pMsg);
	SendWriterMsg(sWrap);
}


// Only used for testing
void SendTestMessage() {
	QueuedMsgHdrs* pMsg = new TestMessage("Test message from reader thread");
	SimpleWrapper* sWrap = new SimpleWrapper(pMsg);
	SendWriterMsg(sWrap);  // Message gets deleted in here
}



