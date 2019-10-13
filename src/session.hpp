#ifndef __SESSION_HPP
#define __SESSION_HPP
#pragma interface

class RTPStat;
class Session : public Allocatable < __SS_ALLOCATOR > {
protected:
	int ports [ 4 ];
	RTPStat * stats;
public:
	Session ( RTPStat * st ) : stats ( st ) { }
	virtual ~Session ( ) { }
	virtual int getLocalAddress ( bool fromCaller, bool fControlChannel ) = 0;
	virtual void setSendAddress ( bool fromCaller, bool fControlChannel, const PIPSocket :: Address & addr,
		WORD port, int rtpCodec, int rtpFrames, bool sendCodec, const ss :: string & recodeTo,
		const ss :: string & recodeFrom ) = 0;
	virtual void getTimeout ( int & i, int & o ) = 0;
	virtual void enableLog ( ) = 0;
	virtual void setTo ( const PIPSocket :: Address & addr, bool tn ) = 0;
	virtual void setFrom ( const PIPSocket :: Address & addr, bool fn ) = 0;
	virtual void setTelephoneEventsPayloadType ( unsigned telephoneEventsPayloadType ) = 0;
//	virtual unsigned getTelephoneEventsPayloadType() const = 0;
};
#endif
