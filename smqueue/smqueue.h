/*
 * SMqueue.h - In-memory queue manager for Short Messages (SMS's) for OpenBTS.
 * Written by John Gilmore, July 2009.
 *
 * Copyright 2009, 2014 Free Software Foundation, Inc.
 * Copyright 2014 Range Networks, Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * See the COPYING file in the main directory for details.
 */


#ifndef SM_QUEUE_H
#define SM_QUEUE_H

#include "SmqGlobals.h"
#include <time.h>
//#include <osipparser2/osip_message.h>	/* from osipparser2 */
#include <stdlib.h>			/* for osipparser2 */
#include <sys/time.h>			/* for osip_init */
#include <osip2/osip.h>			/* for osip_init */
#include <list>
#include <map>
#include <string>
#include <iostream>
#include <stdio.h>

#include "smnet.h"			// My network support
#include <SubscriberRegistry.h>			// My home location register

#include <Logger.h>
void ProcessReceivedMsg();

// That's awful OSIP has a CR define.
// It clashes with our innocent L2Address::CR().
// Don't create 2-letter #defines, ever!
#undef CR
#include "SMSMessages.h"
using namespace SMS;


namespace SMqueue {

/* Maximum text size of an SMS message.  */
#define SMS_MESSAGE_MAX_LENGTH  160


/* strdup uses malloc, which doesn't play well with new/delete.
   The idiots who defined C++ don't provide one, so we will. */
char *new_strdup(const char *orig);

/*
 * States that a message can be in, while processed.
 * Messages come in, they need various lookups, then they go out.
 * Then they sit pending acknowledgment that they were successful.
 * The state machine is driven by incoming packets or information,
 * and in the absence of such incoming info, by timeouts.
 *
 * Note: If you change this enum, you MUST CHANGE SMqueue::timeouts and
 *       SMqueue::sm_state_strings.
 */

enum sm_state {				// timeout, next-state-if-timeout
	NO_STATE,
	INITIAL_STATE, // 1
	REQUEST_FROM_ADDRESS_LOOKUP,
	ASKED_FOR_FROM_ADDRESS_LOOKUP,  //3

	AWAITING_TRY_DESTINATION_IMSI,
	REQUEST_DESTINATION_IMSI, // 5
	ASKED_FOR_DESTINATION_IMSI,

	AWAITING_TRY_DESTINATION_SIPURL,
	REQUEST_DESTINATION_SIPURL,  // 8
	ASKED_FOR_DESTINATION_SIPURL,

	AWAITING_TRY_MSG_DELIVERY, // 10
	REQUEST_MSG_DELIVERY,  // 11
	ASKED_FOR_MSG_DELIVERY, // 12

	DELETE_ME_STATE,

	AWAITING_REGISTER_HANDSET,
	REGISTER_HANDSET,
	ASKED_TO_REGISTER_HANDSET,

	STATE_MAX_PLUS_ONE,		/* Keep this one last! */
};

#define STATE_MAX  (STATE_MAX_PLUS_ONE - 1)

// How to print a state
extern std::string sm_state_strings[STATE_MAX_PLUS_ONE];
extern string sm_state_string(enum sm_state astate);

/* Set this once we've called the initializer for the OSIP parser library. */
extern bool osip_initialized;
extern struct osip *osipptr;


/* In-memory object representing a Short Message.  These are kept as
   text strings (as we received them) and only parsed when we need to
   process them.  This keeps memory usage way down for medium to long
   term storage in the queue.  */
class short_msg {
  public:

	enum ContentType {
		UNSUPPORTED_CONTENT,
		TEXT_PLAIN,
		VND_3GPP_SMS
	};

	/* First just the text string.   A SIP message including body. */
	unsigned short text_length;
	char *text /* [text_length] */;  // C++ doesn't make it simple

	/* Now a flag for whether it's been parsed, and a parsed copy. */
	bool parsed_is_valid;
	/* If the parsed message has been modified, such that the string
	   copy is no longer valid, this will be true.  */
	bool parsed_is_better;
	osip_message_t *parsed;
	// from;
	// to;
	// time_t date;
	// expiration;
	ContentType content_type; // Content-Type of the message
	ContentType convert_content_type; // Content type to convert to, or UNSUPPORTED_CONTENT

	RPData *rp_data; // Parsed RP-DATA of an SMS. It's read from MESSAGE body if
	                 // it has application/vnd.3gpp.sms MIME-type. Note, that
	                 // it's parsed on request and may be NULL at any point.
	TLMessage *tl_message; // Parsed RPDU of an SMS. It's read from rp_data. Note,
	                       // that it's parsed on request and may be NULL at
	                       // any point.
	bool ms_to_sc; // Direction of the message. True is this is MS->SC SMS, false
	               // otherwise.
	bool need_repack; // Message should be packed into TPDU for delivery. E.g.
	                  // SIP MESSAGE sent to MS should be packed, while SIP
	                  // REGISTER should not.

	bool from_relay;

	short_msg () :
		text_length (0),
		text (NULL),
		parsed_is_valid (false),
		parsed_is_better (false),
		parsed (NULL),
		content_type(UNSUPPORTED_CONTENT),
		convert_content_type(UNSUPPORTED_CONTENT),
		rp_data(NULL),
		tl_message(NULL),
		ms_to_sc(false),
		need_repack(true),
		from_relay(false)
	{
	}
	// Make a short message, perhaps taking responsibility for deleting
	// the "new"-allocated memory passed in.
  	short_msg (int len, char * const cstr, bool use_my_memory) :
		text_length (len),
		text (cstr),
		parsed_is_valid (false),
		parsed_is_better (false),
		parsed (NULL),
		content_type(UNSUPPORTED_CONTENT),
		convert_content_type(UNSUPPORTED_CONTENT),
		rp_data(NULL),
		tl_message(NULL),
		ms_to_sc(false),
		need_repack(true),
		from_relay(false)
	{
		if (!use_my_memory) {
			text = new char [text_length+1];
			strncpy(text, cstr, text_length);
			text[text_length] = '\0';
		}
	};

	/* When created from another short_msg, must copy the string. */
	// We would've liked to make this private, so that we can check at
	// compile time that nobody's inadvertently making copies (malloc's)
	// of short_msg classes -- but unfortunately G++'s <list> doesn't
	// allow lists of classes that have no copy-constructors!
	//private:
  	short_msg (const short_msg &sm) :
		text_length (sm.text_length),
		text (0),
		parsed_is_valid (false),
		parsed_is_better (false),
		parsed (NULL),
		content_type(UNSUPPORTED_CONTENT),
		convert_content_type(UNSUPPORTED_CONTENT),
		rp_data(NULL),
		tl_message(NULL),
		ms_to_sc(false),
		need_repack(true),
		from_relay(sm.from_relay)
	{
		if (text_length) {
			text = new char [text_length+1];
			strncpy(text, sm.text, text_length);
			text[text_length] = '\0';
		}
	};

#if 0
	short_msg (std::string str) :
		text_length (str.length()),
		text (0),
		parsed_is_valid (false),
		parsed_is_better (false),
		parsed (NULL),
		content_type(UNSUPPORTED_CONTENT),
		convert_content_type(UNSUPPORTED_CONTENT),
		rp_data(NULL),
		tl_message(NULL),
		ms_to_sc(false),
		need_repack(false)
	{
		text = new char [text_length+1];
		strncpy(text, str.data(), text_length);
		text[text_length] = '\0';
	};
#endif
	
	/* Disable operator= to avoid pointer-sharing problems */
	private:
	short_msg & operator= (const short_msg &rvalue);
	public:

	/* Destructor */
	virtual ~short_msg ()
	{
		if (parsed)
			osip_message_free(parsed);
		delete [] text;
		delete rp_data;
		delete tl_message;
	}

	// Pseudo-constructor due to inability to run constructors on
	// members of lists.
	// Initialize a newly-default-constructed short message,
	// perhaps taking responsibility for deleting
	// the "new"-allocated memory passed in.
	void
  	initialize (int len, char * const cstr, bool use_my_memory)
	{
		// default constructor needs these things revised to
		// initialize with a message in a string.
		text_length = len;
		text = cstr;
		if (!use_my_memory) {
			text = new char [text_length+1];
			strncpy(text, cstr, text_length);
			text[text_length] = '\0';
		}
	}

	string scanWord(char *start) {
		char *ep = start;
		while (isalpha(*ep) || isdigit(*ep) || *ep == '_') ep++;
		return string(start,ep-start);
	}

	/* Parsing, validating, and unparsing messages.
	 * Return
	 * 	False if failed
	*/
	bool parse() {
		int i;
		osip_message_t *sip;

		if (parsed_is_valid)
			return true;

		if (!osip_initialized) {
			//LOG(DEBUG) << "Calling osip_init";
			i = osip_init(&osipptr);
			if (i != 0) {
				LOG(DEBUG) << "osip_init failed error " << i;
				return false;
			}
			osip_initialized = true;
		}
			
		unparse();	// Free any previous one.

		// Parse SIP message
		//LOG(DEBUG) << "Calling osip_message_init";
		i = osip_message_init(&sip);
		if (i != 0)  {
			LOG(DEBUG) << "osip_message_init failed error " << i;
			return false;
		}

		//LOG(DEBUG) << "Calling osip_message_parse";
		i = osip_message_parse(sip, text, text_length);
		if (i != 0) {
			LOG(DEBUG) << "osip_message_parse failed error " << i;
			return false;
		}

		parsed = sip;
		parsed_is_valid = true;
		parsed_is_better = false;

		// Now parse SMS if needed
		if (parsed->content_type == NULL)
		{
			LOG(DEBUG) << "SMS content not set continue anyway??";
			// Most likely this is SIP response.
			content_type = UNSUPPORTED_CONTENT;
			// SVG Consider not continuing
		} else if (  strcmp(parsed->content_type->type, "text") == 0
		          && strcmp(parsed->content_type->subtype, "plain") == 0)
		{
			// If Content-Type is text/plain, then no decoding is needed.
			content_type = TEXT_PLAIN;
			//LOG(DEBUG) << "SMS message encoded text";
		} else if (  strcmp(parsed->content_type->type, "application") == 0
		          && strcmp(parsed->content_type->subtype, "vnd.3gpp.sms") == 0)
		{
			// This is an encoded SMS' TPDU.
			LOG(DEBUG) << "SMS message encoded 3GPP";
			content_type = VND_3GPP_SMS;

#if 1
			// (pat 10-2014) Originally we sent the RPDUs encoded as hex, which was incorrect.
			// OpenBTS now optionally sends the RPDU in base64 as per 3GPP 24.341, and adds the official "Content-Transfer-Encoding: base64" header.
			// Unfortunately this SIP library does not have any obvious way to get that information out, so just
			// grunge though the text of the SIP message.  This sucks.
			string encoding = "hex";	// (pat) Use hex if other encoding not explicitly specified for backward compatibility.
			static const char *cteHeader = "\r\nContent-Transfer-Encoding";
			char *contentTransferEncoding = strcasestr(this->text,cteHeader);
			if (contentTransferEncoding) {
				char *cp = contentTransferEncoding + strlen(cteHeader);
				while (isspace(*cp) || *cp == ':') { cp++; }
				encoding = scanWord(cp);
			}

			char *endp = strstr(this->text,"\r\n\r\n");	// Find the beginning of the message body.
			int content_len = parsed->content_length && parsed->content_length->value ? atoi(parsed->content_length->value) : 0;
			rp_data = decodeRPData(endp + 4, content_len, encoding);
#else
				// (pat 10-2014) This is the original code for hex encoded message body.
				// Decode it RP-DATA
				// (pat 10-2014) Add some checks so we dont crash.
				if (parsed->bodies.node == NULL || parsed->bodies.node->element == NULL) {
					LOG(ERR)<< "SIP Parse error 1";
					return true;
				}
				osip_body_t *bod1 = (osip_body_t *)parsed->bodies.node->element;
				const char *bods = bod1->body;
				if (bods == NULL) { // (pat 10-2014) More checks so we dont crash.
					LOG(ERR)<< "SIP Parse error 2";
					return true;
				}
				//LOG(DEBUG) << "Calling hex2rpdata";
				rp_data = hex2rpdata(bods,true);
#endif
			if (rp_data == NULL) {
				LOG(INFO) << "RP-DATA length is zero";  // This is okay
				return true;
			}

			// Decode RPDU
			tl_message = parseTPDU(rp_data->TPDU());
			if (tl_message == NULL) {
				LOG(INFO) << "TPDU parsing failed";
				return false;
			}
		}

		return true;
	} // parse

	/* Anytime a caller CHANGES the values in the parsed tree of the
	   message, they MUST call this, to let the caching system
	   know that the cached copy of the text-string message is no
	   longer valid.  Actually, we have TWO such cached copies!  FIXME
	   so we have to invalidate both of them. */
	void
	parsed_was_changed() {
		//LOG(DEBUG) << "Calling osip_message_force_update";
		parsed_is_better = true;
		osip_message_force_update(parsed);   // Tell osip library too
	}

	/* Make the text string valid, if the parsed copy is better.
	   (It gets "better" by being modified, and parsed_was_changed()
	   got called, but we deferred fixing up the text string till now.) */
	void
	make_text_valid() {
		if (parsed_is_better) {
			/* Make or remake text string from parsed version. */
			char *dest = NULL;
			size_t length = 0;

			if (!parsed_is_valid) {
				LOG(DEBUG) << "Parsed is not valid";
				return;
			}

			//LOG(DEBUG) << "Calling osip_message_to_str";
			int i = osip_message_to_str(parsed, &dest, &length);
			if (i != 0) {
				LOG(DEBUG) << "osip_message_to_str failed";  // Is this fatal
			}
			delete [] text;
			/* Because "osip_free" != "delete", we have to recopy
			   the string!!!  Don't you love C++?  */
			text_length = length;
			text = new char [text_length+1];
			strncpy(text, dest, text_length);
			text[text_length] = '\0';
			osip_free(dest);
			parsed_is_valid = true;
			parsed_is_better = false;
		}
		if (text == NULL) {
			LOG(DEBUG) << "text is null";
			return;
		}
	}

	/* Free up all memory used by parsed version of message. */
	void unparse() {
		// Free parsed TDPU.
		// FIXME -- We should check if it has been changed and update MESSAGE body.
		delete rp_data;
		rp_data = NULL;
		delete tl_message;
		tl_message = NULL;
		content_type = UNSUPPORTED_CONTENT;

		// Now unparse SIP
		if (parsed_is_better)
			make_text_valid();
		if (parsed)
			osip_message_free(parsed);
		parsed = NULL;
		parsed_is_valid = false;
		parsed_is_better = false;
	} //unparse


	// Get text for short message
	std::string get_text() const
	{
		switch (content_type) {
		case TEXT_PLAIN: {
			if (parsed->bodies.node != 0) {
				osip_body_t *bod1 = (osip_body_t *)parsed->bodies.node->element;
				return bod1->body;
			} else {
				return "";
			}
		}
		break;

		case VND_3GPP_SMS: {
			if (tl_message == NULL) {
				return "";
			}

			LOG(DEBUG) << "Trying to decode message " << (ms_to_sc?"MS->SC":"SC->MS")
			           << " MTI=" << tl_message->MTI() << " tl_message: " << *tl_message;

			try {
				std::string decoded_text;
				// MS->SC and SC-MS messages have to be handled separately
				if (ms_to_sc) {
					// Only SUBMIT messages have text data.
					if (tl_message->MTI() != TLMessage::SUBMIT) {
						LOG(NOTICE) << "Can't decode MS->SC message with MTI=" << tl_message->MTI();
					} else {
						const TLSubmit *submit = (TLSubmit*)tl_message;
						decoded_text = submit->UD().decode();
					}
				} else {
					// Only DELIVER messages have text data.
					if (tl_message->MTI() != TLMessage::DELIVER) {
						LOG(NOTICE) << "Can't decode SC->MS message with MTI=" << tl_message->MTI();
					} else {
						const TLDeliver *deliver = (TLDeliver*)tl_message;
						decoded_text = deliver->UD().decode();
					}
				}
				LOG(NOTICE) << "Decoded text: " << decoded_text;
				return decoded_text;
			}
			catch (SMSReadError) {
				LOG(DEBUG) << "SMS parsing failed (above L3)";
				// TODO:: Should we send error back to the phone?
				return "";
			}

		}
		break;

		case UNSUPPORTED_CONTENT:
		default:
			return "";
		} // switch
	} // get_text


	/* Kind of a nasty hack to convert a message as it is going out. Generally from rpdu to text. */
	void convert_message(ContentType to) {
		convert_content_type = to;
	}


	void do_not_convert_message() {
		convert_content_type = UNSUPPORTED_CONTENT;
	}

};

// I couldn't figure out how to make these static members of the class...
/* Timeouts when going from NO_STATE into each subsequent state */
//    /*static*/ int timeouts_NO_STATE[STATE_MAX_PLUS_ONE];

/* Index to all timeouts */
extern /*static*/ int (*timeouts[STATE_MAX_PLUS_ONE])[STATE_MAX_PLUS_ONE];

class SMq;

class short_msg_pending: public short_msg {
	public:
	enum sm_state state;		// State of processing
	time_t next_action_time;	// When to do something different
	int retries;			// How many times we've retried
					// this message.
	char srcaddr[16];		// Source address (ipv4 or 6 or ...)
	socklen_t srcaddrlen;		// Valid length of src address.
	char *qtag;			// Tag that identifies this msg
					// uniquely in the queue.
					// (It is set 1st time msg is parsed.)
	int qtaghash;			// Simple hash of the qtag.
	char *linktag;			// Tag of a message that this message
					// is related to.  (We use this in
					// handset register messages, to find
					// the original SMS message that
					// prompted us to send the register.)

	static const char *smp_my_ipaddress;	// Static copy of my IP address
					// for validity checking of msgs.
					// (We get our own copy because
					// it's the only thing we need to
					// inherit from SMq.)
	static const char *smp_my_2nd_ipaddress; // Idiocy for NAT

	/* Constructors */
	short_msg_pending () :
		state (NO_STATE),
		next_action_time (0),
		retries (0),
		// srcaddr({0}),  // can't seem to initialize an array?
		srcaddrlen(0),
		qtag (NULL),
		qtaghash (0),
		linktag (NULL)
	{ 
	}

	// Make a pending short message, perhaps taking responsibility for 
	// deleting the "new"-allocated memory passed in.
  	short_msg_pending (int len, char * const cstr, bool use_my_memory)
	    : short_msg (len, cstr, use_my_memory),
		state (NO_STATE),
		next_action_time (0),
		retries (0),
		// srcaddr({0}),  // can't seem to initialize an array?
		srcaddrlen(0),
		qtag (NULL),
		qtaghash (0),
		linktag (NULL)
	{
	}

#if 0
	short_msg_pending (std::string str) : 
		short_msg (str),
		state (NO_STATE),
		next_action_time (0),
		retries (0),
		// srcaddr({0}),  // can't seem to initialize an array?
		srcaddrlen(0),
		qtag (NULL),
		qtaghash (0),
		linktag (NULL)
	{
	}
#endif

	// 
	// We would've liked to declare this next function PRIVATE,
	// since that would prevent anyone from carelessly duplicating
	// short_msg_pending's in constructors.
	// Unfortunately, libstdc++'s <list> doesn't allow lists to be
	// created from classes that lack a copy constructor.
	//private:
	short_msg_pending (const short_msg_pending &smp) :
		short_msg (static_cast<const short_msg &>(smp)),
		state (smp.state),
		next_action_time (smp.next_action_time),
		retries (smp.retries),
		// srcaddr({0}),  // can't seem to initialize an array?
		srcaddrlen(smp.srcaddrlen),
		qtag (NULL),
		qtaghash (smp.qtaghash),
		linktag (NULL)
	{
		if (smp.srcaddrlen) {
			if (smp.srcaddrlen > sizeof (srcaddr)) {
				LOG(DEBUG) << "Srcaddr data too large";
				return;  // Can't continue
			} else {
				memcpy(srcaddr, smp.srcaddr, smp.srcaddrlen);
			}
		}
			
		if (smp.qtag) {
			int len = strlen(smp.qtag);
			this->qtag = new char[len+1];
			strncpy(this->qtag, smp.qtag, len);
			this->qtag[len] = '\0';
		}
		
		if (smp.linktag) {
			int len = strlen(smp.linktag);
			this->linktag = new char[len+1];
			strncpy(this->linktag, smp.linktag, len);
			this->linktag[len] = '\0';
		}
	}

	/* Override operator= to avoid pointer-sharing problems */
	private:
	short_msg_pending & operator= (const short_msg_pending &rvalue);
	public:

	/* Destructor */
	virtual ~short_msg_pending () {
		delete [] qtag;
		delete [] linktag;
	}

	/* Methods */

	/* 
	 * Most of the time we can't use a constructor, because we
	 * want to create our short_msg_pending and put it in a list, without
	 * copying it (which would involve lots of useless new/delete's,
	 * particularly if we've parsed it into a huge nested struct).
	 * So we usually make a list with one element (which uses the
	 * default constructor) and then run initialize() on that element
	 * with the same arguments we would've used for the constructor.
	 * This 'constructs' the new short_msg_pending in a temporary list,
	 * and we can then trivially move it into the real message queue,
	 * removing it from the temporary list in the process.
	 */
	// Make a pending short message, perhaps taking responsibility for 
	// deleting the "new"-allocated memory passed in.
	void
  	initialize (int len, char * const cstr, bool use_my_memory)
	{
		short_msg::initialize (len, cstr, use_my_memory);
		// initguts();
	}

#if 0
	short_msg_pending (std::string str) : 
		short_msg (str),
		state (NO_STATE),
		next_action_time (0),
		retries (0),
		// srcaddr({0}),  // can't seem to initialize an array?
		srcaddrlen(0),
		qtag (NULL),
		qtaghash (0),
		linktag (NULL)
	{
	}
#endif


	time_t msgettime() {
		struct timespec tv;
		clock_gettime(CLOCK_REALTIME, &tv);
		unsigned long time_in_mill =
			 (tv.tv_sec * 1000UL) + (tv.tv_nsec / 1000000UL) ; // convert tv_sec & tv_usec to millisecond
		return time_in_mill;
	}


	/* Reset the message's state and timeout.  Timeout is set based
	   on the current state and the new state.  */
	void set_state(enum sm_state newstate) {
		next_action_time = msgettime() +
			(*SMqueue::timeouts[state])[newstate];
		LOG(DEBUG) << "Set state Current: " << sm_state_string(state) << " Newstate: " << sm_state_string(newstate)
				<< " Timeout value " << *SMqueue::timeouts[state][newstate];
		state = newstate;
		/* If we're in a queue, some code in another class is now going
		   to have to change our queue position.  */
	}

	/* Reset the message's state and timeout.  Timeout is argument.  */
	void set_state(enum sm_state newstate, time_t timeout) {
		next_action_time = timeout;
		state = newstate;
		/* If we're in a queue, some code in another class is now going
		   to have to change our queue position.  */
	}

	/* Check that the message is valid, and set the qtag and qtaghash
	   from the message's contents.   Result is 0 for valid, or
	   SIP response error code (e.g. 405).  */
	int validate_short_msg(SMq *manager, bool should_early_check);

	// Set the qtag and qtaghash from the parsed fields.
	// Whenever we change any of these fields, we have to recalculate
	// the qtag.  
	// FIXME, we assume that the CSEQ, Call-ID, and From tag all are
	// components that, in combination, identify the message uniquely.
	// FIXME!  The spec is unpleasantly unclear about this.
	// Result is 0 for success, or 3-digit integer error code if error.
	int set_qtag();

	// Hash the tag to an int, for speedier searching.
	int taghash_of(const char *tag);

	/* Check host and port for validity.  */
	bool
	check_host_port(char *host, char *port);

	/* Generate a billing record. */
	void write_cdr(SubscriberRegistry& hlr) const;

};

typedef std::list<short_msg_pending> short_msg_p_list;

/*
 * Function parameters and return value for short-code "command" functions that
 * process SMS messages internally rather than sending the SMS message
 * on to somebody else.
 */
class SMq;

enum short_code_action {
	SCA_DONE = 0, ///< No further processing is needed. Free message.
	SCA_INTERNAL_ERROR = 1, //< Just report error and bail out.
	SCA_REPLY = 2, ///< Free this message and send replay back to the msg sender
	               ///< with a text from params.scp_reply
	SCA_RETRY_AFTER_DELAY = 3, ///< HLR is busy. Retry query later.
	SCA_REPLY_AND_RETRY = 4, ///< UNUSED.
	SCA_QUIT_SMQUEUE = 5, ///< Self-explanatory. Exit smqueue.
	SCA_AWAIT_REGISTER = 6, ///< HLR response is delayed. Wait.
	SCA_REGISTER = 7, ///< HLR record for this phone has been retrieved.
	                  ///< Proceed to registration with Asterisk.
	SCA_TREAT_AS_ORDINARY = 8, ///< Continue msg processing as if it were non-shortcode msg.
	SCA_EXEC_SMQUEUE = 9, ///< Fork new smqueue instance and exit this one.
	SCA_RESTART_PROCESSING = 10 ///< Return from this short code processing
	                                 ///< and run another short code.
};

class short_code_params {
  public:
	int scp_retries;		// in: 0 if first call for this msg
	SMq *scp_smq;			// in: The entire "global" SMq structure
	short_msg_p_list::iterator scp_qmsg_it; // in: iterator for our msg 
	char *scp_reply;		// out: Reply msg to sender
	int scp_delay;			// out: Delay before re-call

	short_code_params() :
		scp_retries(0), scp_smq (NULL), scp_qmsg_it (),
		scp_reply (NULL), scp_delay (0) 
	{ }
  private:
	// Avoid copy-construction and assignment by making private.
	short_code_params(const short_code_params &) :
		scp_retries(0), scp_smq (NULL), scp_qmsg_it (),
		scp_reply (NULL), scp_delay (0) 
	{ }
	short_code_params & operator=(const short_code_params &);
  public:

	~short_code_params() {
		delete [] scp_reply;
	}
};

// Function pointer declaration for short-code action functions
typedef enum short_code_action (*short_func_t)
			(const char *imsi, const char *msgtext, 
			 short_code_params *scp);

/*
 * Associative map between target phone numbers (short codes) and
 * function pointers that implement those numbers.
 */
typedef std::map<std::string,short_func_t> short_code_map_t;  // Maps description (phone number) to function to call

/* What fills in that map */
void init_smcommands(short_code_map_t *scm);

/* 
 * Main class for SIP Short Message processing.
 * The daemon is designed to be running one copy of this class.
 */
class SMq {
	public:

	// Put all timeouts in one place
	const static int TIMEOUTMS = 5000;
	const static int SMSRATELIMITMS = 1000;
	const static int LONGDELETMS = 5000000;   // 83 minutes  Used by SC.ZapQueued.Password
	const static int INCREASEACKEDMSGTMOMS = 60000;  // 5 minutes

	void InitBeforeMainLoop();
	void CleaupAfterMainreaderLoop();
	void InitInsideReaderLoop();

	/* A list of all messages we know about, sorted by time of next
	   action (assuming nothing arrives to change our mind before that
	   time). */
	short_msg_p_list time_sorted_list;

	std::string savefile; //SMq
	bool please_re_exec;

	pthread_mutexattr_t mutexSLAttr;
	pthread_mutex_t sortedListMutex;

	void unlockSortedList() {
		// LOG(DEBUG) << "UNLOCK";  // debug opnly
		pthread_mutex_unlock(&sortedListMutex);
	}

	void lockSortedList() {
		// LOG(DEBUG) << "LOCK"; // debug only
		pthread_mutex_lock(&sortedListMutex);
	}

	/* We may later want other accessors for faster access to various
	   messages when things DO arrive.  For now, linear search!  */

	/* The network sockets that we're using for I/O */
	SMnet my_network;

	/* The interface to the Host Location Register for routing
	   messages and looking up their return and destination addresses.  */
	SubscriberRegistry my_hlr;

	/* Where to send SMS's that we can't route locally. */
	std::string global_relay;
	std::string global_relay_port;
	short_msg::ContentType global_relay_contenttype;

	/* My IP address (I can't tell how I look to others). */
	std::string my_ipaddress;
	/* Idiocy for NAT */
	std::string my_2nd_ipaddress;

	/* My port number. */
	std::string my_udp_port;

	/* The IP addr:port of the HLR, where we send SIP REGISTER
	   messages to associate IMSIs with cell site addr:port numbers. */
	std::string my_register_hostport;

	/* The call-ID, CSeq, and flag we use in registration requests */
	std::string register_call_id;
	int register_call_seq;
	bool have_register_call_id;

	/* Set this to true when you want main loop to stop.  */
	bool stop_main_loop;

	/* Set this to true when you want the program to re-exec itself
	   instead of terminating after the main loop stops.  */
	bool reexec_smqueue;

	// Input from command line
	int argc;
	char **argv;

	/* Constructor */
	SMq () : 
		time_sorted_list (),
		my_network (),
		my_hlr(),
		global_relay(""),
		my_ipaddress(""),
		my_2nd_ipaddress(""),
		my_udp_port(""),
		my_register_hostport(""),
		register_call_id(""),
		register_call_seq(0),
		have_register_call_id(false),
		stop_main_loop (false),
		reexec_smqueue (false)
	{
		// We need recursive attribute set
		int mStatus;
		mStatus = pthread_mutexattr_init(&mutexSLAttr);
		if (mStatus != 0) { LOG(DEBUG) << "Mutex pthread_mutexattr_init error " << mStatus; }
		pthread_mutexattr_settype(&mutexSLAttr, PTHREAD_MUTEX_RECURSIVE);
		if (mStatus != 0) { LOG(DEBUG) << "Mutex pthread_mutexattr_settype error " << mStatus; }
		pthread_mutex_init(&sortedListMutex, &mutexSLAttr);
		if (mStatus != 0) { LOG(DEBUG) << "Mutex pthread_mutex_init error " << mStatus; }

		my_hlr.init();
	}


	/* Destructor */
	~SMq() {
		pthread_mutex_destroy(&sortedListMutex);
	}


	// Override operator= so -Weffc++ doesn't complain
	// *DISABLE* assignments by making the = operation private.
	private:
	SMq & operator= (const SMq &rvalue);
	public:

	/* Set my own IP address, since I can't tell how I look to others. */
	void set_my_ipaddress(std::string myip) {
		my_ipaddress = myip;
		// Point to it for message validity checking.
		// NOTE: that copy shares same storage as this one.
		short_msg_pending::smp_my_ipaddress = myip.c_str();
	}

	/* Set my 2nd IP address, since NAT is widespread among idiots. */
	void set_my_2nd_ipaddress(std::string myip) {
		my_2nd_ipaddress = myip;
		// Point to it for message validity checking.
		// NOTE: that copy shares same storage as this one.LOG(DEBUG) << "Run once a minute stuff"
		short_msg_pending::smp_my_2nd_ipaddress = myip.c_str();
	}

	/* Set the global relay address (host:port string) */
	void set_global_relay(std::string gr, std::string port, std::string contentType) {
		global_relay = gr;
		global_relay_port = port;
		if (contentType.length()) {
			if (contentType == "text/plain") {
				global_relay_contenttype = short_msg::TEXT_PLAIN;
			} else if (contentType == "application/vnd.3gpp.sms") {
				global_relay_contenttype = short_msg::VND_3GPP_SMS;
			}
		} else {
			global_relay_contenttype = short_msg::VND_3GPP_SMS;
		}
	}

	/* Set the register host & port -- where to register handsets */
	void set_register_hostport(std::string hp) {
		my_register_hostport = hp;
	}

	/* Initialize the listener -- sets up and opens my_network. */
	bool init_listener (std::string port) {
		my_udp_port = port;
		return my_network.listen_on_port (port);
	}

	bool to_is_deliverable(const char *username);
	bool from_is_deliverable(const char *from);

	bool convert_content_type(short_msg_pending *message, short_msg::ContentType to_type);

	/* Convert a short_msg to a given content type */
	//void convert_message(short_msg_pending *qmsg, short_msg::ContentType toType);

	// Main loop listening for dgrams and processing them.
	void main_loop(int tmo);

	/* If nothing happens for a while, handle that.  */
	void process_timeout();

	/* Send a SIP response to acknowledge reciept of a short msg. */
	void respond_sip_ack(int errcode, short_msg_pending *smp, char *netaddr, size_t netaddrlen);

	/*
	 * Originate a short message
	 * Put it in the queue and start handling it.
	 * From is a shortcode (currently),
	 * To is an IMSI (currently),
	 * msgtext is plain ASCII text.
	 * Result is 0 for success, negative for error.
	 */
	int
	originate_sm(const char *from, const char *to, const char *msgtext,
			enum sm_state firststate);

	/*
	 * Originate half of a short message
	 * Put it in the queue and start handling it, but don't actually
	 * finish it or send it; return it to the caller for further mucking.
	 *
	 * In particular, the caller must set the:
	 *    uri
	 *    From:
	 *    To:
	 *    Content-Type and message body, if any.
	 *
	 * Method is which type of SIP packet (MESSAGE, RESPONSE, etc).
	 * Result is a short_msg_p_list containing one short_msg_pending, with
	 * the half-initialized message in it.
	 */
	short_msg_p_list *
	originate_half_sm(std::string method);

	/*
	 * Send a bounce message, based on an existing queued message.
	 * Return the state to set the original bouncing message to.
	 */
	enum sm_state
	bounce_message(short_msg_pending *sent_msg, const char *errstr);
	
	/*
	 * See if the handset's imsi and phone number are in the HLR
	 * database yet, since if it isn't, we can't register the imsi
	 * at its cell's host:port yet.
	 */
	bool
	ready_to_register (short_msg_p_list::iterator qmsg);

	/*
	 * Register a handset's IMSI with its cell, by sending Asterisk
	 * a SIP REGISTER message that we "relay" from the cell.  (We
	 * actually originate it, but we pretend that the cell sent it to us.
	 * Actually the cell sent us an incoming shortcode SMS from an
	 * unregistered phone, which is in the queue in REGISTER_HANDSET state.)
	 */
	enum sm_state
	register_handset (short_msg_p_list::iterator qmsg);

	/* Check if this is a short-code message and handle it.
	 * Return true if message has been handled, false if you should continue
	 * message handling as usual. In latter case \p next_state is untouched.
	 */
	bool
	handle_short_code(const short_code_map_t &short_code_map,
	                  short_msg_p_list::iterator qmsg, enum sm_state &next_state);

	/* When a SIP response arrives, search the queue for its matching
	   MESSAGE and handle both.  */
	void
	handle_response(short_msg_p_list::iterator qmsg);

	/* Search the message queue to find a message whose tag matches.  */
	bool
	find_queued_msg_by_tag(short_msg_p_list::iterator &mymsg,
				    const char *tag, int taghash);
	/* Same, but without a known taghash. */
	bool
	find_queued_msg_by_tag(short_msg_p_list::iterator &mymsg,
				    const char *tag);

	/*
	 * Look up the hostname and port number where we should send Short
	 * Messages for the IMSI in the To address.  
	 * 
	 * This is also where we assign a new Call-ID to the message, so that
	 * re-sends will use the same Call-ID, but re-locate's (looking up the
	 * recipient's location again) will use a new one.
	 */
	enum sm_state
	lookup_uri_hostport (short_msg_pending *qmsg);

	/* 
	 * Change the From address username to a valid phone number in format:
	 *     +countrycodephonenum
	 */
	enum sm_state
	lookup_from_address(short_msg_pending *qmsg);

	/* Change the Request-URI's address to a valid IMSI.  */
	enum sm_state
	lookup_uri_imsi (short_msg_pending *qmsg);

	// For memory allocation simplicity, it's easiest to create
	// new messages as a 1-entry short_msg_p_list and then move
	// them to the real list.  Note that this moves the message's list
	// entry itself off the original list (which can then be discarded).
	// Push_front only does a copy so use splice ??
	void insert_new_message(short_msg_p_list &smp) {
		lockSortedList();
		time_sorted_list.splice (time_sorted_list.begin(), smp);
		time_sorted_list.begin()->set_state (INITIAL_STATE);  // Note set state can move the entries in the queue
		// time_sorted_list.begin()->timeout = 0;  // it is already
		// Low timeout will cause this msg to be at front of queue.
		unlockSortedList();
		debug_dump(); //svgfix
		ProcessReceivedMsg();
	}

	// This version lets the initial state be set.
	void insert_new_message(short_msg_p_list &smp, enum sm_state s) {
		LOG(DEBUG) << "Insert message into queue 2";
		lockSortedList();
		time_sorted_list.splice (time_sorted_list.begin(), smp);
		time_sorted_list.begin()->set_state (s);
		// time_sorted_list.begin()->timeout = 0;  // it is already
		// Low timeout will cause this msg to be at front of queue.
		unlockSortedList();
		debug_dump(); //svgfix
		ProcessReceivedMsg();
	}
	// This version lets the state and timeout be set.
	void insert_new_message(short_msg_p_list &smp, enum sm_state s, time_t t) {
		LOG(DEBUG) << "Insert message into queue 3";
		lockSortedList();
		time_sorted_list.splice (time_sorted_list.begin(), smp);
		time_sorted_list.begin()->set_state (s, t);
		unlockSortedList();
		debug_dump(); //svgfix
		ProcessReceivedMsg();
	}

	/* Debug dump of the queue and the SMq class in general. */
	void debug_dump();

	// Set the linktag of "newmsg" to point to oldmsg.
	void set_linktag(short_msg_p_list::iterator newmsg,
			 short_msg_p_list::iterator oldmsg);

	// Get the old message that this message links to.
	bool get_link(short_msg_p_list::iterator &oldmsg,
		      short_msg_p_list::iterator qmsg);

	/*
 	 * When we reset the state and timestamp of a message,
	 * we need to reinsert it into the queue.  C++ doesn't seem
	 * to have a standard container that lets one do this cleanly.
	 * Even with lists it's a kludge -- we have to do our own search.
	 * With multisets you can't splice an element out while keeping the
	 * element... etc.
	 */
	void set_state(short_msg_p_list::iterator sm, enum sm_state newstate) {
		short_msg_p_list temp;
		lockSortedList();
		/* Extract the current sm from the time_sorted_list */
		temp.splice(temp.begin(), time_sorted_list, sm);
		sm->set_state(newstate);
		// Insert it according to the new timestamp.
		// One would think that "multisets" could do this simply,
		// but they don't appear to have this capability.
		// Note: if list is empty, or all are too early, insert at end.
		for (short_msg_p_list::iterator x = time_sorted_list.begin(); 
		     true; x++) {
			if (x == time_sorted_list.end() 
			    || x->next_action_time >= sm->next_action_time) {
				time_sorted_list.splice(x, temp);
				break;
			}
		}
		unlockSortedList();
	} // set_state

	/*
 	 * When we reset the state and timestamp of a message,
	 * we need to reinsert it into the queue.  C++ doesn't seem
	 * to have a standard container that lets one do this cleanly.
	 * Even with lists it's a kludge -- we have to do our own search.
	 * With multisets you can't splice an element out while keeping the
	 * element... etc.
	 */
	void set_state(short_msg_p_list::iterator sm, enum sm_state newstate, time_t timestamp) {
		lockSortedList();
		short_msg_p_list temp;
		/* Extract the current sm from the time_sorted_list */
		temp.splice(temp.begin(), time_sorted_list, sm);
		sm->set_state(newstate, timestamp);
		// Insert it according to the new timestamp.
		// One would think that "multisets" could do this simply,
		// but they don't appear to have this capability.
		// Note: if list is empty, or all are too early, insert at end.
		for (short_msg_p_list::iterator x = time_sorted_list.begin(); 
		     true; x++) {
			if (x == time_sorted_list.end() 
			    || x->next_action_time >= sm->next_action_time) {
				time_sorted_list.splice(x, temp);
				break;
			}
		}
		unlockSortedList();
	} // set_state

	/* Save the queue to a file; read it back from a file.
	   Reading a queue file doesn't delete things that might already
 	   be in the queue; if you want a clean queue, delete anything
	   already in the queue first.  */
	bool
	save_queue_to_file(std::string qfile);
	bool
	read_queue_from_file(std::string qfile);


}; // SMq class
} // namespace SMqueue

extern SMqueue::SMq smq;

#endif

