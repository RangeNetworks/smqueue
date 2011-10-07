/*
* Copyright 2008 Free Software Foundation, Inc.
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




#include "Threads.h"
#include "Interthread.h"
#include <iostream>

using namespace std;


InterthreadQueue<int> gQ;
InterthreadMap<int,int> gMap;

void* qWriter(void*)
{
	int *p;
	for (int i=0; i<20; i++) {
		p = new int;
		*p = i;
		COUT("queue write " << *p);
		gQ.write(p);
		if (random()%2) sleep(1);
	}
	p = new int;
	*p = -1;
	gQ.write(p);
	return NULL;
}

void* qReader(void*)
{
	bool done = false;
	while (!done) {
		int *p = gQ.read();
		COUT("queue read " << *p);
		if (*p<0) done=true;
		delete p;
	}
	return NULL;
}


void* mapWriter(void*)
{
	int *p;
	for (int i=0; i<20; i++) {
		p = new int;
		*p = i;
		COUT("map write " << *p);
		gMap.write(i,p);
		if (random()%2) sleep(1);
	}
	return NULL;
}

void* mapReader(void*)
{
	for (int i=0; i<20; i++) {
		int *p = gMap.read(i);
		COUT("map read " << *p);
		delete p;
	}
	return NULL;
}






int main(int argc, char *argv[])
{
	Thread qReaderThread;
	qReaderThread.start(qReader,NULL);
	Thread mapReaderThread;
	mapReaderThread.start(mapReader,NULL);

	Thread qWriterThread;
	qWriterThread.start(qWriter,NULL);
	Thread mapWriterThread;
	mapWriterThread.start(mapWriter,NULL);

	qReaderThread.join();
	qWriterThread.join();
	mapReaderThread.join();
	mapWriterThread.join();
}


// vim: ts=4 sw=4
