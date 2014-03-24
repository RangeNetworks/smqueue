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
 * SmqMessageHandler.h
 *
 *  Created on: Nov 16, 2013
using namespace std;
using namespace SMqueue;
extern ConfigurationTable gConfig;
 *      Author: Scott
 *      This code implements the wait for event function
 */

#ifndef SMQMESSAGEHANDLER_H_
#define SMQMESSAGEHANDLER_H_

#include <stdio.h>
#include <stdlib.h>
#include <mqueue.h>
#include <time.h>
#include <errno.h>

#include <string>
#include "SmqGlobals.h"
#include <unistd.h>

void queue_respond_sip_ack(int errcode, SMqueue::short_msg_pending *shortmsg, char * netaddr, size_t netaddrlen);
void ProcessReceivedMsg();
void SendTestMessage();


using namespace std;
/*
These are the upper limit on queue sizes
/proc/sys/fs/mqueue/msg_max: 1100
/proc/sys/fs/mqueue/msgsize_max: 1000
/proc/sys/fs/mqueue/queues_max: 256
 */

#define MQ_MESSAGE_MAX_SIZE 1100
#define MQ_MAX_NUM_OF_MESSAGES 100
#define MQ_MODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)

/*
errno
13 permission denied
14
22 EINVAL
90 EMSGSIZE Message too long (POSIX.1)
110 ETIMEDOUT
 */
class SmqMessageHandler {
public:
	static void StartThreads();

	SmqMessageHandler(string queueName) {
		/* Form the queue attributes */
		attr.mq_flags = 0; /* i.e mq_send will be block if message queue is full */
		attr.mq_maxmsg = MQ_MAX_NUM_OF_MESSAGES;
		attr.mq_msgsize = MQ_MESSAGE_MAX_SIZE;
		attr.mq_curmsgs = 0;
		msg_prio = 0;
		mqueueName = queueName;
		mqueueOpened = false;
	}

	~SmqMessageHandler();

	int SmqDeleteMessage();

	int SmqWaitforMessage(int TimeoutMS, char * MsgBuffer, int MsgBufferSize);
	bool SmqInitReceiver();
	string getQueueName();
	bool queueOpened();
	long getMessageQueueSize();

	// Send functions
	int SmqSendMessage(SimpleWrapper * pMsg);

private:
	// Message queue
	mqd_t mqdes;  // Message queue
	struct mq_attr attr;
	unsigned msg_prio;
	struct timespec currenttime_before;
	struct timespec abs_timeout;
	struct timespec currenttime_after;

	string mqueueName; // Name used with this message queue
	bool mqueueOpened;
};

#endif /* SMQMESSAGEHANDLER_H_ */
