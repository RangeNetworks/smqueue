/*
* Copyright 2012-2014 Range Networks, Inc.
*
* This software is distributed under the terms of the GNU Affero Public License.
* See the COPYING file in the main directory for details.
*
* This use of this software may be subject to additional restrictions.
* See the LEGAL file in the main directory for details.

        This program is free software: you can redistribute it and/or modify
        it under the terms of the GNU Affero General Public License as published by
        the Free Software Foundation, either version 3 of the License, or
        (at your option) any later version.

        This program is distributed in the hope that it will be useful,
        but WITHOUT ANY WARRANTY; without even the implied warranty of
        MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
        GNU Affero General Public License for more details.

        You should have received a copy of the GNU Affero General Public License
        along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/


#include "smtest.h"

#include <string.h>
#include <stdlib.h>

#include <Sockets.h>

// (pat 8-9-2013) Unfortunately we need a default config to eliminate link errors.
#include <Configuration.h>
ConfigurationTable gConfig("/etc/OpenBTS/smqueue.db","smtest");

/** Submit an SMS for delivery.
 *  @return The server return code.
 */
int sendMessage(const char *smqueueIP, int smqueuePort, const char *myIP, int myPort,
		const char *smscCode, const char *from, const char *to,
		const char *txtBuf)
{
	static UDPSocket sock(myPort, smqueueIP, smqueuePort);

	static const char form[] =
		"MESSAGE sip:%s@%s SIP/2.0\n"
		"Via: SIP/2.0/UDP %s;branch=%x\n"
		"Max-Forwards: 2\n"
		"From: %s <sip:%s@%s:%d>;tag=%d\n"
		"To: sip:%s@%s\n"
		"Call-ID: %x@%s:%d\n"
		"CSeq: 1 MESSAGE\n"
		"Content-Type: text/plain\n" \
		"Content-Length: %u\n"
		"\n%s\n";
	static char buffer[1500];
	snprintf(buffer, 1499, form,
		smscCode, smqueueIP,
		myIP, (unsigned)random(),
		from, from, myIP, myPort, (unsigned)random(),
		to, smqueueIP,
		(unsigned)random(), myIP, myPort,
		strlen(txtBuf), txtBuf);
	sock.write(buffer);

	int numRead = sock.read(buffer,10000);
	if (numRead >= 0) {
		buffer[numRead] = '\0';

		printf("%s\n", buffer);
	} else {
		printf("%s\n", "Timed out");
	}

	return TEST_SUCCESS;
}

int main(int argc, const char *argv[])
{
	if (argc == 8) {
		const char *smqueueIP = argv[1]; //"127.0.0.1";
		int smqueuePort = atoi(argv[2]); //5062;
		const char *myIP = argv[3]; //"127.0.0.1";
		int myPort = atoi(argv[4]); //5070;
		const char *smscCode = "smsc";
		const char *from = argv[5];
		const char *to = argv[6];
		const char *msg = argv[7];

#define doTest(x,y,z) sendMessage(smqueueIP, smqueuePort, myIP, myPort, smscCode, x, y, z)
	
		//doTest("from", "to", "message");
		doTest(from, to, msg);
	} else {
		printf("usage: (All fields are required)\n"
			"smtest smqueueIP smqueuePort myIP myPort from to message\n\n");
	}

	return 1;
}

