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


#ifndef REGEXPW_H
#define REGEXPW_H

#include <regex.h>
#include <iostream>
#include <stdlib.h>



class Regexp {

	private:

	regex_t mRegex;


	public:

	Regexp(const char* regexp, int flags=REG_EXTENDED)
	{
		int result = regcomp(&mRegex, regexp, flags);
		if (result) {
			char msg[256];
			regerror(result,&mRegex,msg,255);
			std::cerr << "Regexp compilation of " << regexp << " failed: " << msg << std::endl;
			abort();
		}
	}

	~Regexp()
		{ regfree(&mRegex); }

	bool match(const char *text, int flags=0) const
		{ return regexec(&mRegex, text, 0, NULL, flags)==0; }

};


#endif
