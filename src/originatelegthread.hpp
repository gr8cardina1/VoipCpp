#ifndef ORIGINATELEGTHREAD_HPP_
#define ORIGINATELEGTHREAD_HPP_
#pragma interface

class OriginateLegThread;

namespace DTMF {
struct Relay;
}

struct RecodeInfo {
	CodecInfo codec;
	CodecInfo backCodec;
	explicit RecodeInfo ( const CodecInfo & c ) : codec ( c ), backCodec ( c ) { }
	RecodeInfo ( const CodecInfo & c, const CodecInfo & bc ) : codec ( c ), backCodec ( bc ) { }
};

std :: ostream & operator<< ( std :: ostream & os, const RecodeInfo & r );

typedef boost :: multi_index :: multi_index_container < RecodeInfo,
	boost :: multi_index :: indexed_by < boost :: multi_index :: sequenced < >,
	boost :: multi_index :: ordered_unique < boost :: multi_index :: tag < Codec >,
	boost :: multi_index :: member < RecodeInfo, CodecInfo, & RecodeInfo :: codec > > >,
	__SS_ALLOCATOR < RecodeInfo > > RecodeInfoVector;

class Leg : public Allocatable < __SS_ALLOCATOR >, boost :: noncopyable {
protected:
	OriginateLegThread * thread;
	CommonCallDetails & common;
public:
	virtual ~Leg ( ) { };
	virtual bool tryChoice ( ) = 0;
	virtual bool initChoice ( ) = 0;
	virtual bool iteration ( ) = 0;
	virtual void closeChoice ( ) = 0;
	virtual bool ended ( ) const = 0;
	virtual void rtpTimeoutDetected ( ) { };
	virtual void wakeUp ( ) = 0;
	virtual bool peerOnHold ( int level ) = 0;
	virtual bool peerOnHoldOK ( ) = 0;
	virtual bool peerDtmf ( const DTMF :: Relay & r ) = 0;
protected:
	Leg ( OriginateLegThread * t, CommonCallDetails & c );
};

class OriginateLegThread : public LegThread, public CallControl {
	PCLASSINFO ( OriginateLegThread, LegThread )
	EaterDetails eaters;
	typedef std :: tr1 :: function < void ( ) > PeerMessage;
	PMutex qm;
	std :: queue < PeerMessage, std :: deque < PeerMessage, __SS_ALLOCATOR < PeerMessage > > > peerQueue;
	ss :: string incomingCallId;
	ss :: string printableCallId;
	mutable PMutex lm;
	void putMessage ( const PeerMessage & m ); // nonlocal
	void checkPeerMessages ( );
	void closeLocal ( int );
	void peerOnHoldLocal ( int level );
	void peerOnHoldOKLocal ( );
	void peerDtmfLocal ( const DTMF :: Relay & r );
protected:
	void beginShutDown ( );
	int getCallSeconds ( ) const;
	bool getCallConnected ( ) const;
	int getCallRef ( ) const;
	ss :: string getInIp ( ) const;
	int getInPeerId ( ) const;
	ss :: string getInPeerName ( ) const;
	ss :: string getInResponsibleName ( ) const;
	ss :: string getSetupTime ( ) const;
	ss :: string getConnectTime ( ) const;
	ss :: string getOutIp ( ) const;
	int getOutPeerId ( ) const;
	ss :: string getCallId ( ) const;
	ss :: string getOutPeerName ( ) const;
	ss :: string getOutResponsibleName ( ) const;
	ss :: string getCallingDigits ( ) const;
	ss :: string getSentDigits ( ) const;
	ss :: string getDialedDigits ( ) const;
	ss :: string getInCode ( ) const;
	ss :: string getOutCode ( ) const;
	ss :: string getInAcctn ( ) const;
	ss :: string getOutAcctn ( ) const;
	ss :: string getPdd ( ) const;
	bool getDifferentProtocol ( ) const;
	bool getDifferentCodec ( ) const;

	CommonCallDetails common;
	unsigned ref;
	unsigned maxForwards;
	CodecInfoVector inCodecs, outCodecs;
	bool end;
	bool goodCall;
	bool rtpTimeoutStarted;
	bool endInited;
	PTime setupTime, endTime;
	OutTryVector tries;
	CodecInfo codec;
	Pointer < Leg > leg;
	boost :: optional < PTime > pddTime;
	ss :: string m_remotePartyID;
	ss :: string m_PAssertID;
	ss :: string m_Privacy;
	int limitTries;
	bool choiceAnswered;
	bool isSendAddressSet;
	bool gwTaken;
	bool differentCodec;
	void Close ( int ); //nonlocal
public:
	virtual ~OriginateLegThread ( );
	void calledSocketConnected ( const PIPSocket :: Address & local );
	virtual void connected ( );
	void released ( Q931 :: CauseValues cause );
	virtual void alerted ( );
	void setCodec ( const CodecInfo & c );
	bool admissibleIP ( PIPSocket :: Address & ip );
	virtual void answered ( );
	void setSendAddress ( const PIPSocket :: Address & addr, int port, const CodecInfo & inCodec,
		const CodecInfo & outCodec, const CodecInfo & changedInCodec, const CodecInfo & changedOutCodec );
	void setTelephoneEventsPayloadType ( unsigned payload );
	bool isGoodCall ( ) const { return goodCall; }
	const OutTryVector & getTries ( ) { return tries; }
	int getLocalPort ( ) const;
	PIPSocket :: Address getLocalIp ( ) const;
	void peerOnHold ( int level );
	void peerOnHoldOK ( );
	void onHold ( int level );
	void sendDtmf ( const DTMF :: Relay & r );
	void setPrintableCallId ( const ss :: string & c );
	void peerDtmf ( const DTMF :: Relay & r );
protected:
	OriginateLegThread ( H323Call * c, LegThread * * p, unsigned _id, const CodecInfoVector & ic,
		const CodecInfoVector & oc );
	OriginateLegThread ( H323Call * c, LegThread * * p, unsigned _id, const CommonCallDetails & d,
		const CodecInfoVector & ic, const CodecInfoVector & oc, ProtoType origType, unsigned maxForwards,
		const ss :: string & remotePartyID, const ss :: string&, const ss :: string& );
	void shutDownLocal ( );
	bool tryNextChoice ( );
	void closeThisChoice ( );
	virtual void Main ( );
	virtual void accountCall ( );
	virtual bool init ( ) = 0;
	virtual void sqlSetPeer ( ) = 0;
	virtual void sqlStart ( ) = 0;
	virtual void sqlEnd ( ) = 0;
	virtual void sqlConnect ( ) = 0;
};

class CallBackLegThread : public OriginateLegThread {
	PCLASSINFO ( CallBackLegThread, OriginateLegThread );
	void sqlSetPeer ( );
	void sqlStart ( );
	void sqlEnd ( );
	void sqlConnect ( );
	bool init ( );
	void accountCall ( );

	//void beginShutDown ( );
	//int getCallSeconds ( ) const;

public:
	CallBackLegThread ( H323Call * c, LegThread * *p, const ss :: string & an, const ss :: string & ac, const ss :: string & n,
		unsigned _id, const CodecInfoVector & ic, const CodecInfoVector & oc );
};

class OriginateFullThread : public OriginateLegThread {
	PCLASSINFO ( OriginateFullThread, OriginateLegThread );
	bool init ( );
	void sqlSetPeer ( );
	void sqlStart ( );
	void sqlEnd ( );
	void sqlConnect ( );
public:
	OriginateFullThread ( H323Call * c, LegThread * * p, unsigned _id, const CommonCallDetails & com,
		const CodecInfoVector & ic, const CodecInfoVector & oc, ProtoType origType, unsigned maxForwards,
		const ss :: string & remotePartyID = ss :: string ( ), const ss :: string & passertid = ss :: string ( ),
		const ss :: string & privacy = ss :: string ( ) );
};

#endif /*ORIGINATELEGTHREAD_HPP_*/
