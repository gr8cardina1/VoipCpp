#ifndef SIPLEG2_HPP_
#define SIPLEG2_HPP_
#pragma interface

namespace SIP2 {
class ContactHeader;
}

class SipLeg2 : public Leg, private SIP2 :: OriginateHandler {
	RecodeInfoVector inRecodes;
	ss :: string m_remotePartyID;
	ss :: string m_PAssertID;
	ss :: string m_Privacy;
	struct Data;
	Data * p; // ukazatel tolko poslednim constructorom !
	// eto znachit nikakih stringov nige etogo mesta !
	unsigned maxForwards;
	unsigned lastCseq;
	unsigned redirectCount;
	bool gotUnauthorized;
	bool tryChoice ( );
	bool initChoice ( ) {
		return true;
	}
	bool iteration ( );
	void closeChoice ( );
	bool ended ( ) const {
		return false;
	}
	void wakeUp ( );
	void disconnectReceived ( int c, const SIP2 :: ResponseMIMEInfo & m );
	bool disconnectReceivedLocal ( int c );
	bool redirectReceivedLocal ( int c, const SIP2 :: ContactHeader & contact );
	void tryingReceived ( );
	bool tryingReceivedLocal ( );
	void ringingReceived ( const SIP2 :: Response & r );
	bool ringingReceivedLocal ( const SIP2 :: Response & r );
	void okReceived ( const SIP2 :: Response & r );
	bool okReceivedLocal ( const SIP2 :: Response & r );
	void unauthorizedReceived ( const StringVector & realms );
	bool unauthorizedReceivedLocal ( const StringVector & realms );
	void onholdReceived ( int level, int port, const ss :: string & addr );
	bool onholdReceivedLocal ( int level, int port, const ss :: string & addr );
	void dtmfRelayReceived ( const DTMF :: Relay & r );
	bool dtmfRelayReceivedLocal ( const DTMF :: Relay & r );
	bool handleAnswerSDP ( const SIP2 :: Response & r );
	bool peerOnHold ( int level );
	bool peerOnHoldOK ( ) { return true; } // stub
	bool peerDtmf ( const DTMF :: Relay & r );
	bool acceptsDtmfOverride ( ) const;
public:
	SipLeg2 ( OriginateLegThread * t, CommonCallDetails & c, const RecodeInfoVector & inCodecs, unsigned maxForwards, 
	const ss :: string & remotePartyID, const ss :: string& passertid, const ss :: string& privacy );
	~SipLeg2 ( );
};

#endif /*SIPLEG2_HPP_*/
