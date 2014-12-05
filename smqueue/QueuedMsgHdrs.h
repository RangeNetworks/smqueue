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
 * QueuedMsgHdrs.h
 *
 *  Created on: Nov 27, 2013setbufferSize
 *      Author: scott
 *      Header for message being sent in SMQ
 */

#ifndef HEADERFORMSGS_H_
#define HEADERFORMSGS_H_


class QueuedMsgHdrs {
public:

	enum MessageType {
		TestMessage = 1,
		SIPAckMsg = 2,
		SendResponse = 3,
		ProcessIncommingMsg = 4
	};

	QueuedMsgHdrs() {}

	QueuedMsgHdrs(enum MessageType eMessageType, void * ptr, int MessageSize) {
		this->msgType = eMessageType;
		pData = ptr;
		smp = 0;
		setMessageSize(sizeof(QueuedMsgHdrs));
	}

	virtual ~QueuedMsgHdrs() {
		//LOG(DEBUG) << "QueuedMsgHdrs DTOR";
	}

	void setpData(void * data);
	void * getpData();
	void setMsgType(enum MessageType msgType);
	MessageType getMsgType() const;
	SMqueue::short_msg_pending* getSmp() const;
	void setSmp(SMqueue::short_msg_pending* smp);
	int getMessageSize() { return messageSize; }
	void setMessageSize(int iSize) { messageSize = iSize; }
	virtual int ProcessMessage() {
		// Not currently used
		LOG(DEBUG) << "Process message QueuedMsgHdrs DID NOTHING";
		return 0;
	}

private:
	MessageType msgType;
	SMqueue::short_msg_pending *smp;  // the message in the queue
	void * pData;
	int messageSize;
};


class SIPAckMessage : public QueuedMsgHdrs {
public:

	SIPAckMessage(int err, SMqueue::short_msg_pending * smp, char * netaddr, size_t netaddrlen) {
		setMsgType(QueuedMsgHdrs::SIPAckMsg);
		setSmp(smp);       // Use the pointer later
		setpData(0);

		this->errcode = err;
		this->netaddr = netaddr;
		this->netaddrlen = netaddrlen;
		setMessageSize(sizeof(SIPAckMessage));
	}

	virtual ~SIPAckMessage() {
		// QueuedMsgHdrs dtor will delete smp
		//LOG(DEBUG) << "SIPAckMessage DTOR";
	}

	int getErrcode() const
	{
		return errcode;
	}

	void setErrcode(int errcode)
	{
		this->errcode = errcode;
	}

	char* getNetaddr() const
	{
		return netaddr;
	}

	void setNetaddr(char* netaddr)
	{
		this->netaddr = netaddr;
	}

	size_t getNetaddrlen() const
	{
		return netaddrlen;
	}

	void setNetaddrlen(size_t netaddrlen)
	{
		this->netaddrlen = netaddrlen;
	}

	virtual int ProcessMessage() {
		LOG(DEBUG) << "Process message SIPAckMessage";
		smq.respond_sip_ack(this->getErrcode(), (SMqueue::short_msg_pending *) this->getSmp(), // Real call to respond_sip_ack on writer thread
					(char *) this->getNetaddr(), (int) this->getNetaddrlen());
		return 0;
	}

private:
	int errcode;
	char* netaddr;
	size_t netaddrlen;
};


class TestMessage : public QueuedMsgHdrs {
public:

	TestMessage(std::string sMsg) {
		strcpy(msg, sMsg.c_str());

		setMsgType(QueuedMsgHdrs::TestMessage);
		setSmp(0);
		setpData(0);
		setMessageSize(sizeof(TestMessage));
	}
	virtual ~TestMessage() {
		//LOG(DEBUG) << "TestMessage DTOR";
	}

	virtual int ProcessMessage() {
		LOG(DEBUG) << "Process message TestMessage msg:" << msg;  // Log is all this message has to do currently
		return 0;
	}//

private:
	char msg[50];
};


class ProcessIncommingMsg : public QueuedMsgHdrs {
public:
	// Message will be sent to the writer thread so that msgs will be processed
	ProcessIncommingMsg() {
		setMsgType(QueuedMsgHdrs::ProcessIncommingMsg);
		setSmp(0);
		setpData(0);
		setMessageSize(sizeof(ProcessIncommingMsg));
	}

	virtual ~ProcessIncommingMsg() {
		//LOG(DEBUG) << "ProcessIncommingMsg DTOR";

	}

	virtual int ProcessMessage() {
		// Got a message in the queue go handle it
		LOG(DEBUG) << "Process message ProcessIncommingMsg";
		smq.process_timeout();
		return 0;
	}

};


class SimpleWrapper {
public:
	SimpleWrapper(QueuedMsgHdrs* pData) {
		msgPtr = pData;
	}

	~SimpleWrapper() {
		// don't delete msgPtr this will be doe by receiving client
		//LOG(DEBUG) << "SimpleWrapper DTOR";
	}
	int getSize() { return sizeof(SimpleWrapper); }
	QueuedMsgHdrs* getMsgPtr() { return msgPtr; }
	void setMsgPtr(QueuedMsgHdrs* pData) { msgPtr = pData; }

private:
	QueuedMsgHdrs* msgPtr;
}; // SimpleWrapper



#endif   /* HEADERFORMSGS_H_ */
