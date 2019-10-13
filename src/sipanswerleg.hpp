#ifndef __SIPANSWERLEG_HPP
#define __SIPANSWERLEG_HPP
#pragma interface

class SipReceiver;
class SipCallDetails;
class SipAnswerLegThread : public AnswerLegThread {
	PCLASSINFO ( SipAnswerLegThread, AnswerLegThread )
	std :: queue < PeerMessage, std :: deque < PeerMessage, __SS_ALLOCATOR < PeerMessage > > > peerQueue;
	CommonCallDetails ccd;
	SIP :: URL from_, to_, contact_;
	ss :: string tag_, branch_;
	ss :: string via, myVia;
	SipReceiver * receiver;
	ss :: string callId_;
	ss :: string localIp;
	ss :: string inviteCseq;
	bool oked;
	bool byeSent;
	bool end;
	bool canceled_;
	bool acked_;
    ss::string intrfIp;


	StringVector vRecordRoute;
	ss :: string strContact1_;

	ss :: string strTag_;
	ss :: string strURI_;
	int savedLocalRtpPort;
	CodecInfo savedInCodec;
	PMutex receiverMut; // eto race, nado ispolzovat tot ge mutex chto i v receivere, t.e. sipsignallingthread::mut
    //bool directRTP_;

    void checkPeerMessages ( );
	bool handleInvite ( SIP :: PDU & mesg );
	bool receiveMessage ( SIP :: PDU & mesg );
	bool handleMessage ( SIP :: PDU & mesg );
	bool sendMessage ( SIP :: PDU & mesg, const ss :: string & addr, int port );
	bool handleBye ( SIP :: PDU & mesg );
	bool handleCancel ( SIP :: PDU & mesg );
	bool handleAck ( SIP :: PDU & mesg );
	bool handleError ( SIP :: PDU & mesg );
	bool handleTrying ( SIP :: PDU & mesg );
	bool handleOK ( SIP :: PDU & mesg );
	void sendTrying ( SIP :: PDU & mesg );
	void sendRinging ( int localRtpPort, const PIPSocket :: Address & localAddr,
		const CodecInfo & inCodec );
	void sendOk ( int localRtpPort, const PIPSocket :: Address & localAddr,
		const CodecInfo & inCodec );
	void sendBye ( );
	void sendOnHoldInvite ( );
	bool sendOnHoldAck ( );
	void sendOnHoldOK ( );

	bool handleInfo( SIP :: PDU & mesg );
	bool handleOptions( SIP :: PDU & mesg );

	void sendError ( );
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
	void peerOnHoldLeg ( int level );
	void peerOnHoldOKLeg ( );
	void peerDtmfLeg ( const DTMF :: Relay & /*r*/ ) { } // stub

	bool isRecordRouteRequired() const;
	ss :: string parseToTag(const ss :: string& strToTag);

protected:
	~SipAnswerLegThread ( );
public:
	SipAnswerLegThread ( H323Call * c, LegThread * * p, unsigned i, const SipCallDetails * d, const ss :: string & lip );
};

#endif
