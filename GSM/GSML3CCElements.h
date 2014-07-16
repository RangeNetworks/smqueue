/**@file Elements for Call Control, GSM 04.08 10.5.4.  */
/*
* Copyright 2008, 2009, 2014 Free Software Foundation, Inc.
* Copyright 2014 Range Networks, Inc.
*
* This software is distributed under multiple licenses; see the COPYING file in the main directory for licensing information for this specific distribuion.
*
* This use of this software may be subject to additional restrictions.
* See the LEGAL file in the main directory for details.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

*/




#ifndef GSML3CCELEMENTS_H
#define GSML3CCELEMENTS_H

#include "GSML3Message.h"
#include <iostream>
#include <Logger.h>

namespace GSM {



/** A general class for BCD numbers as they normally appear in L3. */
class L3BCDDigits {

	private:

	static const unsigned maxDigits = 20;
	char mDigits[maxDigits+1];					///< ITU-T E.164 limits address to 15 digits

	public:

	L3BCDDigits() { mDigits[0]='\0'; }

	// (pat) The -1 below and +1 above are mutually redundant.
	L3BCDDigits(const char* wDigits) { strncpy(mDigits,wDigits,sizeof(mDigits)-1); mDigits[sizeof(mDigits)-1]='\0'; }

	L3BCDDigits(const L3BCDDigits &other) {
		memcpy(mDigits,other.mDigits,sizeof(mDigits));
	}
	void parse(const L3Frame& src, size_t &rp, size_t numOctets, bool international = false);
	void write(L3Frame& dest, size_t &wp) const;

	/** Return number of octets needed to encode the digits. */
	size_t lengthV() const;

	unsigned size() const { return strlen(mDigits); }
	const char* digits() const { return mDigits; }
};


std::ostream& operator<<(std::ostream&, const L3BCDDigits&);







/** Calling Party BCD Number, GSM 04.08 10.5.4.9 */
// (pat) 24.018 10.5.4.9 quote: "This IE is not used in the MS to network direction."
class L3CallingPartyBCDNumber : public L3ProtocolElement {

private:

	TypeOfNumber mType;
	NumberingPlan mPlan;

	L3BCDDigits mDigits;

	/**@name Octet 3a */
	//@{
	bool mHaveOctet3a;
	int mPresentationIndicator;
	int mScreeningIndicator;
	//@}


public:

	L3CallingPartyBCDNumber()
		:mType(UnknownTypeOfNumber),mPlan(UnknownPlan),
		mHaveOctet3a(false)
	{ }

	L3CallingPartyBCDNumber( const char * wDigits)
		:mPlan(E164Plan), mDigits(wDigits),
		mHaveOctet3a(false)
	{
		mType = (wDigits[0] == '+') ?  InternationalNumber : NationalNumber;
		LOG(DEBUG) << "L3CallingPartyBCDNumber ctor type=" << mType << " Digits " << wDigits;
	}

	L3CallingPartyBCDNumber(const L3CallingPartyBCDNumber &other)
		:mType(other.mType),mPlan(other.mPlan),mDigits(other.mDigits),
		mHaveOctet3a(other.mHaveOctet3a),
		mPresentationIndicator(other.mPresentationIndicator),
		mScreeningIndicator(other.mScreeningIndicator)
	{}


	NumberingPlan plan() const { return mPlan; }
	TypeOfNumber type() const { return mType; }
	const char* digits() const { return mDigits.digits(); }

	size_t lengthV() const;
	void writeV( L3Frame& dest, size_t &wp  ) const;
	void parseV( const L3Frame& src, size_t &rp, size_t expectedLength);	
	void parseV(const L3Frame&, size_t&) { assert(0); }
	void text(std::ostream&) const;
};


/** Called Party BCD Number, GSM 04.08 10.5.4.7 */
class L3CalledPartyBCDNumber : public L3ProtocolElement {


private:

	TypeOfNumber mType;
	NumberingPlan mPlan;
	L3BCDDigits mDigits;

public:

	L3CalledPartyBCDNumber()
		:mType(UnknownTypeOfNumber),
		mPlan(UnknownPlan)
	{ }

	L3CalledPartyBCDNumber(const char * wDigits)
		:mPlan(E164Plan), mDigits(wDigits)
	{
		mType = (wDigits[0] == '+') ?  GSM::InternationalNumber : GSM::NationalNumber;
		//LOG(DEBUG) << "L3CallingPartyBCDNumber ctor type=" << mType << " Digits " << wDigits;
	}

	// (pat) This auto-conversion from CallingParty to CalledParty was used in the SMS code,
	// however, it was creating a disaster during unintended auto-conversions of L3Messages,
	// which are unintentionally sprinkled throughout the code base due to incomplete constructors.
	// The fix would be to add 'explicit' keywords everywhere.
	explicit L3CalledPartyBCDNumber(const L3CallingPartyBCDNumber& other)
		:mType(other.type()),mPlan(other.plan()),mDigits(other.digits())
	{ }

	// (pat) We must have this constructor as a choice also.
	L3CalledPartyBCDNumber(const L3CalledPartyBCDNumber& other)
		:mType(other.mType),mPlan(other.mPlan),mDigits(other.mDigits)
	{ }


	NumberingPlan plan() const { return mPlan; }
	TypeOfNumber type() const { return mType; }
	const char* digits() const { return mDigits.digits(); }

	size_t lengthV() const ; 
	void writeV( L3Frame& dest, size_t &wp  ) const;
	void parseV( const L3Frame& src, size_t &rp, size_t expectedLength );	
	void parseV(const L3Frame&, size_t&) { assert(0); }
	void text(std::ostream&) const;
};








} // GSM

#endif
// vim: ts=4 sw=4
