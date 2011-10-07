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



#include "Regexp.h"
#include <iostream>

using namespace std;


int main(int argc, char *argv[])
{

	Regexp email("^[[:graph:]]+@[[:graph:]]+ ");
	Regexp simple("^dburgess@");

	const char text1[] = "dburgess@jcis.net test message";
	const char text2[] = "no address text message";

	cout << email.match(text1) << " " << text1 << endl;
	cout << email.match(text2) << " " << text2 << endl;

	cout << simple.match(text1) << " " << text1 << endl;
	cout << simple.match(text2) << " " << text2 << endl;
}
