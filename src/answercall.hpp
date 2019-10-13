#ifndef __ANSWERCALL_HPP
#define __ANSWERCALL_HPP
#pragma interface

class PTCPSocket;
class CallDetails;
class SipCallDetails;
class CommonCallDetails;

namespace SDP {
	class SessionDescription;
}

namespace SIP2 {
class AnswerHandler;
class ServerTransactionId;
}

namespace H225 {
	class H323_UserInformation;
}

class EpThreadHandler;

class AnswerCall : public H323Call {
	struct SetDirect : public Allocatable < __SS_ALLOCATOR > {
		bool direct;
		SetDirect ( bool d ) : direct ( d ) { }
		operator bool ( ) const {
			return direct;
		}
	};
	struct SetPeer : public Allocatable < __SS_ALLOCATOR > {
		PIPSocket :: Address local;
		bool fromNat;
		SetPeer ( const PIPSocket :: Address & l, bool fn ) : local ( l ), fromNat ( fn ) { }
	};
	struct SetSendAddress : public Allocatable < __SS_ALLOCATOR > {
		PIPSocket :: Address remote;
		CodecInfo inCodec;
		CodecInfo outCodec;
		CodecInfo changedInCodec;
		CodecInfo changedOutCodec;
		int port;
		SetSendAddress ( const PIPSocket :: Address & r, int p, const CodecInfo & ic,
			const CodecInfo & oc, const CodecInfo & cic, const CodecInfo & coc ) : remote ( r ),
			inCodec ( ic ), outCodec ( oc ), changedInCodec ( cic ), changedOutCodec ( coc ), port ( p ) { }
	};
	struct Delays {
		Pointer < SetDirect > setDirect;
		Pointer < SetPeer > setPeer;
		Pointer < SetSendAddress > setSendAddress;
		unsigned setTelephoneEventsPayloadType;
		bool accounted;
		Delays ( ) : setTelephoneEventsPayloadType ( 0 ), accounted ( false ) { }
	};
	LegThread * second;
	Pointer < RTPSession > session;
	RTPStat stats [ 4 ];
	CodecInfoVector toTermCodecs, fromTermCodecs;
	int fromInRtpCodec, fromInRtpFrames, fromOutRtpCodec, fromOutRtpFrames;
	PIPSocket :: Address inAddr, outAddr, inLocalAddr;
	int inPort, outPort;
	ss :: string inRecodeFrom, inRecodeTo, outRecodeFrom, outRecodeTo;
	typedef std :: map < LegThread *, Delays, std :: less < LegThread * >,
		__SS_ALLOCATOR < std :: pair < LegThread * const, Delays > > > DelayedThreadsMap;
	DelayedThreadsMap delayedThreads;
	CodecInfo answeredInCodec, answeredOutCodec;
	bool fromInSendCodec, fromOutSendCodec;
	bool directIn, directOut;
	bool hasFastStart;
	bool fromNat;
	bool useDelays;
	bool forkAccounted;

	RTPSession * getSession ( );
	void setSendAddress ( const LegThread * c, const PIPSocket :: Address & remote, int port,
		const CodecInfo & inCodec, const CodecInfo & outCodec, const CodecInfo & changedInCodec,
		const CodecInfo & changedOutCodec );
	void setPeer ( const LegThread * c, const PIPSocket :: Address & local, bool fromNat );
	bool directRTP ( ) const;
	OutTryVector getOutTries ( LegThread * c );
	void newChoice ( LegThread * c );
	bool fullAccount ( const LegThread * c );
	void setTelephoneEventsPayloadType ( const LegThread * c, unsigned payload );
	void onHold ( const LegThread * c, int level );
	void onHoldOK ( const LegThread * c );
	void sendDtmf ( const LegThread * c, const DTMF :: Relay & r );
public:
	AnswerCall ( const OutChoiceDetailsVectorVector & choiceForks, const StringVector & forkOutAcctns,
		PTCPSocket * s, int i, const CallDetails & c ); // h323 -> sip
	AnswerCall ( const OutChoiceDetailsVectorVector & choiceForks, const StringVector & forkOutAcctns,
		const SipCallDetails * call, const ss :: string & localIp ); // sip -> h323
	AnswerCall ( const OutChoiceDetailsVectorVector & choiceForks, const StringVector & forkOutAcctns, // mgcp -> any
		const CommonCallDetails & c, const SDP :: SessionDescription & sdp, EpThreadHandler * & t,
		const ss :: string & gw, const ss :: string & ep, const PTime & it );
	AnswerCall ( const OutChoiceDetailsVectorVector & choiceForks, const StringVector & forkOutAcctns, // sip2 -> any
		const CommonCallDetails & c, const SDP :: SessionDescription & sdp, SIP2 :: AnswerHandler * & t,
		const SIP2 :: ServerTransactionId & tid, unsigned expires, unsigned maxForwards, 
		const ss :: string & remotePartyID, const ss :: string & PAssertID, const ss :: string & Privacy );
	void connected ( LegThread * c );
	void alerted ( LegThread * c );
	const RTPStat * getStats ( const LegThread * c );
	void stop ( LegThread * c, int cause );
	void setDirect ( LegThread * c, bool d );
	int getLocalPort ( const LegThread * c );
	PIPSocket :: Address getLocalIp ( const LegThread * c );
	void released ( int cause );
	~AnswerCall ( );
};

#endif
