#ifndef __RTPSESSION_HPP
#define __RTPSESSION_HPP
#pragma interface

#include "session.hpp"
class RTPSession : public Session {
	RecodersMap recoders;
	int id;
public:
	RTPSession ( RTPStat * st, const PIPSocket :: Address & from, const PIPSocket :: Address & to,
		int fromNat, bool tn );
	~RTPSession ( );
	int getLocalAddress ( bool fromCaller, bool fControlChannel );
	void setSendAddress ( bool fromCaller, bool fControlChannel, const PIPSocket :: Address & addr, WORD port,
		int rtpCodec, int rtpFrames, bool sendCodec, const ss :: string & recodeTo,
		const ss :: string & recodeFrom/*, unsigned telephoneEventsPayloadType*/ );
	void setTelephoneEventsPayloadType ( unsigned telephoneEventsPayloadType );
	void getTimeout ( int & i, int & o );
	void enableLog ( );
	void setTo ( const PIPSocket :: Address & addr, bool tn );
	void setFrom ( const PIPSocket :: Address & addr, bool fn );

	static void * operator new ( std :: size_t n );
	static void operator delete ( void * p, std :: size_t n ) throw ( );
};
#endif
