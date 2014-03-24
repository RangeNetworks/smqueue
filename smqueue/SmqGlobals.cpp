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
 * SmqGlobals.cpp
 *
 *  Created on: Nov 21, 2013
 *      Author: scott
 *      Some common resource used by SMQ
 */


#include "SmqGlobals.h"
#include <Logger.h>

SmqGlobals::SmqGlobals() {

}

SmqGlobals::~SmqGlobals() {

}



unsigned long getCurrentSeconds() {
	struct timespec tv;
	clock_gettime(CLOCK_REALTIME, &tv);;
	unsigned long time_in_sec =
			 tv.tv_sec + (tv.tv_nsec / 1000000000UL) ; // convert tv_sec & tv_usec to millisecond
	return time_in_sec;
}
/*
 * Returns current MS
 */
unsigned long getCurrentMS() {
	struct timespec tv;
	clock_gettime(CLOCK_REALTIME, &tv);
	unsigned long time_in_mill =
			 (tv.tv_sec * 1000UL) + (tv.tv_nsec / 1000000UL) ; // convert tv_sec & tv_usec to millisecond
	return time_in_mill;
}


/*
 * Returns MS from timespec
 */
unsigned long getMSFromtimespec(struct timespec tv) {
	unsigned long time_in_mill =
			 (tv.tv_sec * 1000UL) + (tv.tv_nsec / 1000000UL) ; // convert tv_sec & tv_usec to millisecond
	return time_in_mill;
}


struct timespec addTimespec(struct timespec tv1, struct timespec tv2) {
	struct timespec tvret;

	tvret.tv_sec = tv1.tv_sec + tv2.tv_sec;
	tvret.tv_nsec = tv1.tv_nsec + tv2.tv_nsec;
	if (tvret.tv_nsec >= NS_IN_SEC) {
		tvret.tv_sec++;
		tvret.tv_nsec = tvret.tv_nsec - NS_IN_SEC;
	}
	return tvret;
}


void msSleep(int ms) {
	usleep(ms*1000); // Convert to microseconds
	return;
}


time_t msgettime() {
	//LOG(DEBUG) << "Enter msgettime";
	struct timespec tv;
	clock_gettime(CLOCK_REALTIME, &tv);
	unsigned long time_in_mill =
		 (tv.tv_sec * 1000UL) + (tv.tv_nsec / 1000000UL) ; // convert tv_sec & tv_usec to millisecond
	//LOG(DEBUG) << "Return msgettime ms" << time_in_mill;
	return time_in_mill;
}


