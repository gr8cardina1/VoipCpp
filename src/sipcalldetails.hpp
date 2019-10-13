#ifndef __SIPCALLDETAILS_HPP
#define __SIPCALLDETAILS_HPP
#pragma interface

#include "rtpstat.hpp"
#include "ipport.hpp"
#include "rtpsession.hpp"

struct SipFastStartElement {
	SIP :: MediaFormat format;
	CodecInfo codec;
	int changeFrames, outChangeFrames;
	CodecInfo recodeTo;
	SipFastStartElement ( const SIP :: MediaFormat & f, int ch = 0 );
	SipFastStartElement ( ) : format ( SIP :: MediaFormat :: G729 ) { }
};

typedef std :: vector < SipFastStartElement, __SS_ALLOCATOR < SipFastStartElement > > SipFastStartElementVector;

extern int getNewCallId ( );

struct SipCallDetails : public CallControl, public Allocatable < __SS_ALLOCATOR > {
	typedef std :: map < ss :: string, SipFastStartElement, std :: less < ss :: string >,
		__SS_ALLOCATOR < std :: pair < const ss :: string, SipFastStartElement > > > FastStartMap;
	CommonCallDetails common;
	ss :: string via, myVia, calledVia, origCSeq, termContact;
	SIP :: URL from, to, myTo, myFrom, contact;
	SIP :: URL origCardName, termCardName; // Esli stoit flag v SigOptionsPeer (isAccSend), cto nado podmenyat A-number na Ima kartochki.
	RTPStat st [ 4 ];
	Pointer < RTPSession > session;
	OutTryVector tries;
	CodecInfo codec;
	SipCallDetails ( const PIPSocket :: Address & local, const PIPSocket :: Address & fromAddr,
		bool fromNat, bool toNat ) : session ( new RTPSession ( st, local, local, fromNat, toNat ) ),
		id ( getNewCallId ( ) ), balancedSeconds ( 0 ), oked ( false ), cancelled ( false ), byed ( false ),
		outTaken ( false ), answered ( false ), goodCall ( false ), removed ( false ), errored ( false ),
		killed ( false ), differentCodec ( false ), isAnonymous_( false ), m_errorCode( SIP :: PDU :: scFaiureServiceUnavailable ) {
		common.setCallerIp ( static_cast < const char * > ( fromAddr.AsString ( ) ) );
		common.setFromNat ( fromNat );
		origInterface = static_cast < const char * > ( local.AsString ( ) );
		termInterface = static_cast < const char * > ( local.AsString ( ) );
	}
	PTime startTime, okTime, endTime, inviteTime, lastTimeoutCheck, errorTime;
	boost :: optional < PTime > pddTime;
	SIP :: PDU invite, inviteTemp;
	int id;
	int balancedSeconds;
	EaterDetails eaters;
	OutChoiceDetailsVectorVector choiceForks;
	StringVector forkOutAcctns;
	ss :: string origInterface, termInterface;
	FastStartMap originalFastStartIn, originalFastStartOut;
	PIPSocket :: Address origRtpAddr;
	StringVector vRecordRoute_;
	StringVector vOKRecordRoute_;
	WORD origRtpPort;
	bool oked, cancelled, byed, outTaken, answered, goodCall, removed, errored, killed, differentCodec;
	bool isAnonymous_;
	SIP :: PDU :: StatusCodes m_errorCode;

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
	const ss :: string & getOrigInterface() const;
	const ss :: string & getTermInterface() const;
	bool getDirectRTP ( ) const;
	bool getDifferentProtocol ( ) const;
	bool getDifferentCodec ( ) const;
};

class CompareCodecs
{
public:
    CompareCodecs (const SipCallDetails :: FastStartMap& mapFS, const SipFastStartElement& codecName);
    SipFastStartElement getFastStartElement() const;

public:
    static bool isAnnexB (const ss :: string& codecName);
    static bool is729 (const ss :: string& codecName);

    static void initClassData();



protected:
    static std :: set <ss :: string> m_alias729;
    static std :: set <ss :: string> m_annexB729;

    SipFastStartElement m_Codec;
    static bool isInit;
};


#endif
