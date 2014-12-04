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
 * SmqGlobals.h
 *
 *  Created on: Nov 21, 2013
 *      Author:
 */

#ifndef SMQGLOBALS_H_
#define SMQGLOBALS_H_
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <string>


#define CLOCK_REALTIME 0 // Fix this  svgfix
#define NS_IN_SEC 1000000000L  // Number of nano seconds in a second

//#define mq_printf(str, arg...) printf("%d : "str, getpid(), ##arg)  // Original
#define mq_printf(str, arg...) printf("ms:%lu tid:%lu: "str, getCurrentMS(), pthread_self(), ##arg)
int WriteUDPMessage(char* Buffer, int BufferSize, std::string IPAddress, int portNum);


class SmqGlobals {
public:
	SmqGlobals();
	virtual ~SmqGlobals();
};

extern unsigned long getCurrentMS();
extern unsigned long getMSFromtimespec(struct timespec tv);
extern struct timespec addTimespec(struct timespec tv1, struct timespec tv2);
extern void msSleep(int ms);
extern time_t msgettime();
extern unsigned long getCurrentSeconds();

#endif /* SMQGLOBALS_H_ */
