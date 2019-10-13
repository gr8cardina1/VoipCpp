#ifndef __H323LEG_HPP
#define __H323LEG_HPP
#pragma interface

class H323Leg : public Leg {
	PTCPSocket * sock, * h245Sock;
	H225 :: CallIdentifier callId;
	unsigned ref;
	bool useTunneling;
	unsigned determinationNumber;
	bool fastConnectRefused;
	bool fastStartAnswered;
	bool capsSent;
	bool masterSlaveSent;
	RecodeInfoVector inRecodes, outRecodes;
	bool slowStartInited;
	bool rcSent;
	bool tryChoice ( );
	bool initChoice ( );
	bool iteration ( );
	void closeChoice ( );
	bool ended ( ) const;
	void rtpTimeoutDetected ( );
	void wakeUp ( );
	bool peerOnHold ( int /*level*/ ) { return true; } //stub
	bool peerOnHoldOK ( ) { return true; } //stub
	bool peerDtmf ( const DTMF :: Relay & /*r*/ ) { return true; } //stub
public:
	H323Leg ( OriginateLegThread * t, CommonCallDetails & c, const RecodeInfoVector & inCodecs,
		const RecodeInfoVector & outCodecs, unsigned r );
	~H323Leg ( );
protected:
	void sendReleaseComplete ( );
	void calledSocketConnected ( );
	void addFastStartFromIn ( H225 :: ArrayOf_Asn_OctetString & v, const PIPSocket :: Address & local, int port );
	void addFastStartFromOut ( H225 :: ArrayOf_Asn_OctetString & v, const PIPSocket :: Address & local, int port );
	void addFastStart ( H225 :: Setup_UUIE & setup, const PIPSocket :: Address & local );
	void addH245 ( H225 :: Setup_UUIE & setup );
	void addCaps ( H225 :: ArrayOf_Asn_OctetString & reply );
	void addMasterSlave ( H225 :: ArrayOf_Asn_OctetString & reply,
		const H245 :: MasterSlaveDetermination * incDet = 0 );
	void sendSetup ( );
	bool handleH245 ( const H225 :: H323_UU_PDU & pdu );
	bool processH245Message ( const H245 :: MultimediaSystemControlMessage & mesg,
		H225 :: ArrayOf_Asn_OctetString & reply );
	bool handleIndication ( const H245 :: IndicationMessage & mesg, H225 :: ArrayOf_Asn_OctetString & reply );
	bool handleMasterSlaveDeterminationRelease ( const H245 :: MasterSlaveDeterminationRelease & mesg,
		H225 :: ArrayOf_Asn_OctetString & reply );
	bool handleUserInput ( const H245 :: UserInputIndication & mesg,
		H225 :: ArrayOf_Asn_OctetString & reply );
	bool handleTerminalCapabilitySetRelease ( const H245 :: TerminalCapabilitySetRelease & mesg,
		H225 :: ArrayOf_Asn_OctetString & reply );
	bool handleCommand ( const H245 :: CommandMessage & mesg, H225 :: ArrayOf_Asn_OctetString & reply );
	bool handleEndSession ( const H245 :: EndSessionCommand & mesg,
		H225 :: ArrayOf_Asn_OctetString & reply );
	bool handleResponse ( const H245 :: ResponseMessage & mesg, H225 :: ArrayOf_Asn_OctetString & /*reply*/ );
	bool handleOpenLogicalChannelAck ( const H245 :: OpenLogicalChannelAck & msg );
	bool handleRequest ( const H245 :: RequestMessage & mesg, H225 :: ArrayOf_Asn_OctetString & reply );
	bool handleCloseLogicalChannel ( const H245 :: CloseLogicalChannel & mesg,
		H225 :: ArrayOf_Asn_OctetString & reply );
	bool handleOpenLogicalChannel ( const H245 :: OpenLogicalChannel & mesg,
		H225 :: ArrayOf_Asn_OctetString & reply );
	bool handleRTDR ( const H245 :: RoundTripDelayRequest & mesg, H225 :: ArrayOf_Asn_OctetString & reply );
	bool handleMasterSlaveDetermination ( const H245 :: MasterSlaveDetermination & mesg,
		H225 :: ArrayOf_Asn_OctetString & reply );
	bool handleCapabilitySet ( const H245 :: TerminalCapabilitySet & mesg,
		H225 :: ArrayOf_Asn_OctetString & reply );
	bool checkAddr ( H245 :: TransportAddress & addr, H245 :: UnicastAddress_iPAddress * & ip,
		PIPSocket :: Address & ipAddr );
	bool checkAddr ( const H245 :: TransportAddress & addr, const H245 :: UnicastAddress_iPAddress * & ip,
		PIPSocket :: Address & ipAddr );
	bool handleFastStartResponse2 ( const H225 :: ArrayOf_Asn_OctetString & v );
	template < class T > bool handleFastStartResponse ( T & uuie );
	template < class T > bool handleH245Setup ( const T & uuie );
	bool handleProceeding ( Q931 & mesg, H225 :: H323_UserInformation & uuField );
	bool handleAlerting ( Q931 & mesg, H225 :: H323_UserInformation & uuField );
	bool handleConnect ( Q931 & mesg, H225 :: H323_UserInformation & uuField );
	bool handleRelease ( const Q931 & mesg, const H225 :: H323_UserInformation * uuField );
	bool handleFacility ( const Q931 & mesg, const H225 :: H323_UserInformation & uuField );
	bool handleProgress ( const Q931 & mesg, const H225 :: H323_UserInformation & uuField );
	bool handleRequestMode ( const H245 :: RequestMode & mesg, H225 :: ArrayOf_Asn_OctetString & reply );
	bool handleMesg ( Q931 & mesg );
	bool receiveMesg ( );
	bool receiveH245Mesg ( );
	void initSlowStart ( );
	bool initSlowStart ( H225 :: ArrayOf_Asn_OctetString & reply );
	void sendStatus ( );
	std :: pair < bool, bool > getMasterSlaveDecision ( const H245 :: MasterSlaveDetermination & mesg ) const;
	void disableTunneling ( );
private:
	FakeCallDetector _fakeCallDetector;
};

#endif

