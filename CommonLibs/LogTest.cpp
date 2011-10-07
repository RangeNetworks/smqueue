/*
* Copyright 2009 Free Software Foundation, Inc.
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

#include <iostream>
#include <iterator>

#include "Logger.h"
#include "Configuration.h"

ConfigurationTable gConfig;
//ConfigurationTable gConfig("example.config");

void printAlarms()
{
    std::ostream_iterator<std::string> output( std::cout, "\n" );
    std::list<std::string> alarms = gGetLoggerAlarms();
    std::cout << "#alarms = " << alarms.size() << std::endl;
    std::copy( alarms.begin(), alarms.end(), output );
}

int main(int argc, char *argv[])
{
	gLogInit("LogTest","NOTICE",LOG_LOCAL7);

	LOG(EMERG) << " testing the logger.";
	LOG(ALERT) << " testing the logger.";
	LOG(CRIT) << " testing the logger.";
	LOG(ERR) << " testing the logger.";
	LOG(WARNING) << " testing the logger.";
	LOG(NOTICE) << " testing the logger.";
	LOG(INFO) << " testing the logger.";
	LOG(DEBUG) << " testing the logger.";
    std::cout << "\n\n\n";
    std::cout << "testing Alarms\n";
	LOG(ALERT) << " testing the logger alarm.";
    std::cout << "you should see three lines:" << std::endl;
    printAlarms();
    std::cout << "----------- generating 20 alarms ----------" << std::endl;
    for (int i = 0 ; i < 20 ; ++i) {
        LOG(ALERT) << i;
    }
    std::cout << "you should see ten lines with the numbers 10..19:" << std::endl;
    printAlarms();
}



