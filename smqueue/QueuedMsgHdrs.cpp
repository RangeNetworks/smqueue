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
 * HeaderforMsgs.cpp
 *
 *  Created on: Nov 27, 2013
 *      Author: scott
 */

#include "smqueue.h"
#include "QueuedMsgHdrs.h"



void * QueuedMsgHdrs::getpData()
{
	return pData;
}


void QueuedMsgHdrs::setpData(void * data)
{
	pData = data;
}


QueuedMsgHdrs::MessageType QueuedMsgHdrs::getMsgType() const
{
	return msgType;
}


void QueuedMsgHdrs::setMsgType(MessageType msgType)
{
	this->msgType = msgType;
}


SMqueue::short_msg_pending* QueuedMsgHdrs::getSmp() const
{
	return smp;
}


void QueuedMsgHdrs::setSmp(SMqueue::short_msg_pending* smp)
{
	this->smp = smp;
}


