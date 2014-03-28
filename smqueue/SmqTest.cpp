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
 * SmqTest.cpp
 *
 *  Created on: Dec 16, 2013
 *      Author: scott
 */

#include "SmqTest.h"

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

#include <sys/socket.h>

#include <netinet/in.h>

using namespace std;

#include "smqueue.h"
#include "QueuedMsgHdrs.h"
#include "SmqMessageHandler.h"
#include "SmqReader.h"
#include "SmqWriter.h"
#include "SmqGlobals.h"

extern SmqReader* smqReader;
extern SmqWriter* smqWriter;

#define REPEAT 10000

pthread_t thread1, thread2;


void* ReaderTestThread(void *ptr) {
	LOG(DEBUG) << "Starting reader test thread";
	int msgCount = 0;
	int repeatCount = 0;
	//while (true) {

	while (++repeatCount < REPEAT) {

		//LOG(DEBUG) << "Send TestMessage from reader test thread");
		SendTestMessage(); // Send message to writer thread

		//LOG(DEBUG) << "Call originate_sm from readertestthread")
		//smq.originate_sm("TEST1", "TEST2", "This is a test msg from reader test", SMqueue::INITIAL_STATE);  // fake an  incommming message

		msSleep(1000);
	}

	LOG(DEBUG) << "Reader test thread stopped";

	return NULL;
}


static int gLevel = 1;
void RecursiveMutextTest(int Level) {
	int iLevel = Level;

	if (Level > 3) return;
	LOG(DEBUG) << "Enter Level " << iLevel;
	smq.lockSortedList();

	LOG(DEBUG) << "In lock level " << iLevel;

	gLevel++;
	RecursiveMutextTest(gLevel);
	LOG(DEBUG) << "Back from recursive call " << iLevel;

	smq.unlockSortedList();
	LOG(DEBUG) <<  "Unlock from Level " << iLevel;
} // RecursiveMutextTest



// Send fake input to the input socket  incomming default 5063
#define MAXBUFFER 100
#define TPORT 5063
#define REPEAT 10000
void WriteToUDP() {
	int sfd, n;
	char buffer[MAXBUFFER];
	struct sockaddr_in saddr;
	char IPAddress[100];
	socklen_t len;
	int ret;
	int portNum;

	// Set up test
	int repeat = REPEAT;
	int pattern = 0;
	strcpy(IPAddress, "127.0.0.1");
	portNum = TPORT;

	sfd = socket(AF_INET, SOCK_DGRAM, 0);

	bzero(&saddr, sizeof(saddr));
	saddr.sin_family = AF_INET;
	inet_pton(AF_INET, IPAddress, &saddr.sin_addr);
	saddr.sin_port = htons(portNum);

	LOG(DEBUG) << "WriteToUDP running, repeat " << repeat;
	while(repeat-- > 0) {
		for (int i = 0; i < sizeof(buffer); i++) {  // Write a pattern to the buffer for each write.
			buffer[i] = pattern;
		}
		len=sizeof(saddr);
		LOG(DEBUG) << "Write to UDP port, repeat " << repeat;
		ret = sendto(sfd, buffer, sizeof(buffer), 0, (struct sockaddr *) &saddr, len);
		LOG(DEBUG) << "Send length " << ret;
		pattern++;
		msSleep(2000);
	} // while
} // WriteToUDP



void* WriterTestThread(void *ptr) {
	LOG(DEBUG) << "Starting writer test thread";
	int msgCount = 0;
	int repeatCount = 0;

	WriteToUDP();  // just run once
	//RecursiveMutextTest(gLevel);


	//while (true) {
	while (++repeatCount < REPEAT) {
		// Send message to writer
		//LOG(DEBUG) << "Send msg %d from Writer test thread repeat: %d", msgCount, repeatCount);
		//msgCount++;
		//char buffer[50];
		//sprintf(buffer, "Writer test thread message:%d", msgCount);
		msSleep(2000);
	}

	return NULL;
} // WriterTestThread


void StartTestThreads() {
	LOG(DEBUG) << "Start test threads version 14";

#if 0

	// msSleep(5000);  //svgfix

	//pthread_create(&thread1, NULL, ReaderTestThread, (void*) NULL);

	pthread_create(&thread2, NULL, WriterTestThread, (void*) NULL);

#endif
}



