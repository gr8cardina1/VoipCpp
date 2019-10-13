#ifndef __H323ANSWERLEG_HPP
#define __H323ANSWERLEG_HPP
#pragma interface

class H323AnswerLegThread : public AnswerLegThread {
	PCLASSINFO ( H323AnswerLegThread, AnswerLegThread )
	PMutex qm;
	std :: queue < PeerMessage, std :: deque < PeerMessage, __SS_ALLOCATOR < PeerMessage > > > peerQueue;
	CallDetails details;
	PTCPSocket * sock, * h245Sock;
	CodecInfo answeredCodec;
	unsigned determinationNumber;
	bool sourceIsVoIP;
	bool useTunneling;
	bool rcSent;
	bool masterSlaveSent;
	bool capSetSent;
	void checkPeerMessages ( );
	bool receiveMesg ( );
	bool receiveH245Mesg ( );
	bool handleMesg ( Q931 & mesg );
	void handleH245Setup ( const H225 :: TransportAddress & addr );
	template < typename T > void handleH245Setup ( const T & mesg );
	bool handleSetup ( const Q931 & mesg, const H225 :: H323_UserInformation & uuField );
	void handleFastStartAndH245 ( Q931 & mesg, H225 :: H323_UserInformation & uuField,
		H225 :: ArrayOf_Asn_OctetString & fs, int localRtpPort, PIPSocket :: Address localAddr,
		const CodecInfo & inCodec, const CodecInfo & outCodec );
	void checkStartH245 ( H225 :: ArrayOf_Asn_OctetString & reply );
	bool encodeCapabilitySet ( unsigned seqNum, const Asn :: ObjectId & pid, H225 :: ArrayOf_Asn_OctetString & reply );
	bool handleRelease ( const Q931 & mesg, const H225 :: H323_UserInformation * uuField );
	bool handleFacility ( const Q931 & mesg, const H225 :: H323_UserInformation & origUuField );
	void setTunneling ( H225 :: H323_UU_PDU & pdu );
	void sendProceeding ( );
	void sendWithFacility ( Q931 & mesg, H225 :: H323_UserInformation & uuField );
	void sendAlerting ( int localRtpPort, const PIPSocket :: Address & localAddr,
		const CodecInfo & inCodec, const CodecInfo & outCodec );
	void sendConnect ( int localRtpPort, const PIPSocket :: Address & localAddr,
		const CodecInfo & inCodec, const CodecInfo & outCodec );
	bool processH245Control ( H225 :: ArrayOf_Asn_OctetString & reply );
	bool processH245Message ( const H245 :: MultimediaSystemControlMessage & mesg,
		H225 :: ArrayOf_Asn_OctetString & reply );
	bool handleResponse ( const H245 :: ResponseMessage & mesg, H225 :: ArrayOf_Asn_OctetString & reply );
	bool handleRequest ( const H245 :: RequestMessage & mesg, H225 :: ArrayOf_Asn_OctetString & reply );
	bool handleIndication ( const H245 :: IndicationMessage & mesg, H225 :: ArrayOf_Asn_OctetString & reply );
	bool handleMasterSlaveDeterminationRelease ( const H245 :: MasterSlaveDeterminationRelease & mesg,
		H225 :: ArrayOf_Asn_OctetString & reply );
	bool handleUserInput ( const H245 :: UserInputIndication & mesg,
		H225 :: ArrayOf_Asn_OctetString & reply );
	bool handleTerminalCapabilitySetRelease ( const H245 :: TerminalCapabilitySetRelease & mesg,
		H225 :: ArrayOf_Asn_OctetString & reply );
	bool handleMasterSlaveDetermination ( const H245 :: MasterSlaveDetermination & mesg,
		H225 :: ArrayOf_Asn_OctetString & reply );
	bool handleCapabilitySet ( const H245 :: TerminalCapabilitySet & mesg,
		H225 :: ArrayOf_Asn_OctetString & reply );
	bool handleRTDR ( const H245 :: RoundTripDelayRequest & mesg, H225 :: ArrayOf_Asn_OctetString & reply );
	bool handleOpenLogicalChannel ( const H245 :: OpenLogicalChannel & mesg,
		H225 :: ArrayOf_Asn_OctetString & reply );
	bool handleCloseLogicalChannel ( const H245 :: CloseLogicalChannel & mesg, H225 :: ArrayOf_Asn_OctetString & reply );
	bool handleRequestMode ( const H245 :: RequestMode & mesg, H225 :: ArrayOf_Asn_OctetString & reply );
	bool checkAddr ( const H245 :: TransportAddress & addr, const H245 :: UnicastAddress_iPAddress * & ip,
		PIPSocket :: Address & ipAddr );
	void sendReleaseComplete ( );
	void sendStatus ( );
	//virtuals
	void shutDownLeg ( );
	void init ( );
	bool ended ( ) const;
	bool iteration ( );
	void putMessage ( const PeerMessage & m );
	void peerAlertedLeg ( int localRtpPort, const PIPSocket :: Address & localAddr,
		const CodecInfo & inCodec, const CodecInfo & outCodec );
	void peerConnectedLeg ( int localRtpPort, const PIPSocket :: Address & localAddr,
		const CodecInfo & inCodec, const CodecInfo & outCodec );
	void peerOnHoldLeg ( int /*level*/ ) { } // stub
	void peerOnHoldOKLeg ( ) { } // stub
	void peerDtmfLeg ( const DTMF :: Relay & /*r*/ ) { } // stub
protected:
	~H323AnswerLegThread ( );
public:
	H323AnswerLegThread ( H323Call * c, LegThread * * p, PTCPSocket * s, unsigned i, const CallDetails & d );
};

#endif
