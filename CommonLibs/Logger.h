/*
* Copyright 2009, 2010 Free Software Foundation, Inc.
* Copyright 2010 Kestrel Signal Processing, Inc.
*
* This software is distributed under multiple licenses; see the COPYING file in the main directory for licensing information for this specific distribuion.
*
* This use of this software may be subject to additional restrictions.
* See the LEGAL file in the main directory for details.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

*/


#ifndef LOGGER_H
#define LOGGER_H

#include <syslog.h>
#include <stdint.h>
#include <stdio.h>
#include <sstream>
#include <list>
#include <map>
#include <string>
#include "Threads.h"


#define _LOG(level) \
	Log(LOG_##level).get() << pthread_self() \
	<< " " __FILE__  ":"  << __LINE__ << ":" << __FUNCTION__ << ": "
#define LOG(wLevel) \
	if (gGetLoggingLevel(__FILE__)>=LOG_##wLevel) _LOG(wLevel)
#define OBJLOG(wLevel) \
	if (gGetLoggingLevel(__FILE__)>=LOG_##wLevel) _LOG(wLevel) << "obj: " << this << ' '

#define LOG_ASSERT(x) { if (!(x)) LOG(EMERG) << "assertion " #x " failed"; } assert(x);


#define DEFAULT_MAX_ALARMS 10


/**
	A C++ stream-based thread-safe logger.
	Derived from Dr. Dobb's Sept. 2007 issue.
	Updated to use syslog.
	This object is NOT the global logger;
	every log record is an object of this class.
*/
class Log {

	public:

	protected:

	std::ostringstream mStream;		///< This is where we buffer up the log entry.
	int mPriority;					///< Priority of current repot.

	public:

	Log(int wPriority)
		:mPriority(wPriority)
	{ }

	// Most of the work is in the desctructor.
	/** The destructor actually generates the log entry. */
	~Log();

	std::ostringstream& get();
};



std::list<std::string> gGetLoggerAlarms();		///< Get a copy of the recent alarm list.


/**@ Global control and initialization of the logging system. */
//@{
/** Initialize the global logging system. */
void gLogInit(const char* name, const char* level=NULL, int facility=LOG_USER);
/** Get the logging level associated with a given file. */
int gGetLoggingLevel(const char *filename=NULL);
//@}


#endif

// vim: ts=4 sw=4
