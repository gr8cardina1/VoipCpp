#ifndef __H323CALL_HPP
#define __H323CALL_HPP
#pragma interface
class LegThread;
class CodecInfo;

namespace DTMF {
struct Relay;
}

class H323Call : public Allocatable < __SS_ALLOCATOR >, boost :: noncopyable {
	virtual RTPSession * getSession ( ) = 0;
protected:
	PMutex mut;
	LegThread * first;
	unsigned telephoneEventsPayloadType;
	PTime initTime;
	H323Call ( );
public:
	void getTimeout ( int & i, int & o );
	const PTime & getInitTime ( ) const;
	virtual int getLocalPort ( const LegThread * c );
	virtual PIPSocket :: Address getLocalIp ( const LegThread * c );
	virtual void stop ( LegThread * c, int cause ) = 0;
	virtual void setPeer ( const LegThread * c, const PIPSocket :: Address & local, bool fromNat );
	virtual void setSendAddress ( const LegThread * c, const PIPSocket :: Address & remote, int port,
		const CodecInfo & inCodec, const CodecInfo & outCodec, const CodecInfo & changedInCodec,
		const CodecInfo & changedOutCodec );
	virtual void setTelephoneEventsPayloadType ( const LegThread * c, unsigned payload );

	virtual const RTPStat * getStats ( const LegThread * c ) = 0;
	virtual void connected ( LegThread * c ) = 0;
	virtual void alerted ( LegThread * c ) = 0;
	virtual void setDirect ( LegThread * c, bool d ) = 0;
	virtual void released ( int /*cause*/ ) { }
	virtual OutTryVector getOutTries ( LegThread * c );
	virtual void newChoice ( LegThread * /*c*/ ) { };
	virtual bool fullAccount ( const LegThread * c );
	virtual void onHold ( const LegThread * c, int level ) = 0;
	virtual void onHoldOK ( const LegThread * c ) = 0;
	virtual void sendDtmf ( const LegThread * c, const DTMF :: Relay & r ) = 0;
	virtual ~H323Call ( );
};

class CallBackCall : public H323Call {
	LegThread * second;
	Pointer < RTPSession > session;
	RTPStat stats [ 4 ];
	int secondId;
	ss :: string secondNumber;
	ss :: string ani;
	ss :: string acctn;
	bool secondStarted;
	RTPSession * getSession ( );
	void onHold ( const LegThread *, int level );
	void onHoldOK ( const LegThread * );
	void sendDtmf ( const LegThread * c, const DTMF :: Relay & r );
public:
	CallBackCall ( bool parallelStart, int i, const ss :: string & an, const ss :: string & acc, const ss :: string & f,
		const ss :: string & s );
	void connected ( LegThread * c );
	void alerted ( LegThread * c );
	const RTPStat * getStats ( const LegThread * c );
	void stop ( LegThread * c, int cause );
	void setDirect ( LegThread * c, bool d );
	~CallBackCall ( );
};

#endif
