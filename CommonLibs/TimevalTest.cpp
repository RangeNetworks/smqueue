/*
* Copyright 2008 Free Software Foundation, Inc.
*
* This software is distributed under multiple licenses; see the COPYING file in the main directory for licensing information for this specific distribuion.
*
* This use of this software may be subject to additional restrictions.
* See the LEGAL file in the main directory for details.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

*/




#include "Timeval.h"
#include <iostream>

using namespace std;

int main(int argc, char *argv[])
{

	Timeval then(10000);
	cout << then.elapsed() << endl;

	while (!then.passed()) {
		cout << "now: " << Timeval() << " then: " << then << " remaining: " << then.remaining() << endl;
		usleep(500000);
	}
	cout << "now: " << Timeval() << " then: " << then << " remaining: " << then.remaining() << endl;
}
