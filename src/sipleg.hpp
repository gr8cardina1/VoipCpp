#ifndef __SIPLEG_HPP
#define __SIPLEG_HPP
#pragma interface

class SipReceiver;

class SipLeg : public Leg {
	PMutex * wakeMut;
	SIP :: URL from_, to_;
	const ss :: string tag;
	ss :: string branch;
	ss :: string via;
	SipReceiver * receiver;
	const ss :: string callId;
	ss :: string localIp;
	bool oked;
	bool byeSent;
	RecodeInfoVector inRecodes;
	bool sendInvite ( const ss :: string & authLine = "", int authType = 0 );
	bool sendMessage ( SIP :: PDU & mesg, const ss :: string & addr, int port );
	bool receiveMessage ( SIP :: PDU & mesg );
	bool isPreviousHuntTo ( const SIP :: MIMEInfo & mime );
	bool isPreviousHuntFrom ( const SIP :: MIMEInfo & mime );
	bool handleTrying ( SIP :: PDU & mesg );
	bool handleDialogEstablishement ( SIP :: PDU & mesg );
	bool handleAnswerSDP ( SIP :: PDU & mesg );
    bool handleSessionProgress ( SIP :: PDU & mesg );
	bool handleRinging ( SIP :: PDU & mesg );
	bool sendAck ( const ss :: string & cseq, const StringVector* recordRoute, const ss :: string * contact );
	bool handleCancelOK ( SIP :: PDU & );
	bool handleByeOK ( SIP :: PDU & );
	bool handleOK ( SIP :: PDU & mesg );
	bool handleError ( SIP :: PDU & mesg );
	bool handleBye ( SIP :: PDU & mesg );
	bool handleCancel ( SIP :: PDU & mesg );
	bool handleOptions ( SIP :: PDU & mesg );
	bool handleMessage ( SIP :: PDU & mesg );
	bool handleUnAuthorised ( SIP :: PDU & mesg, int authType );
	bool handleInfo ( SIP :: PDU & mesg );
	bool handleInvite ( SIP :: PDU & mesg );
	void sendTrying ( SIP :: PDU & mesg );

	ss :: string parseContact(const ss :: string& strContact);
	ss :: string parseToTag(const ss :: string& strContact);

/// Convertations metods:
//	bool handle503( SIP :: PDU & mesg );
//	bool sendResponse( SIP :: PDU :: StatusCodes statusCode );
	//Q931 :: CauseValues getResponse( SIP :: PDU :: StatusCodes statusCode );
	Q931 :: CauseValues getResponse( int statusCode );

	ss :: string createPAssertId(const SIP::URL& from);
	void insertPAssertId(SIP :: PDU & mesg); // insert into mime
	bool isRecordRouteRequired() const;

	ss :: string createAnonimousFromHeader(const ss :: string& strFrom, const ss :: string& strAddr);
	ss :: string createToHeader(const ss :: string& strAddr);

	bool tryChoice ( );
	bool initChoice ( );
	bool iteration ( );
	void closeChoice ( );
	bool ended ( ) const;
	void wakeUp ( );
	void sendReleaseComplete ( );
	bool peerOnHold ( int level );
	bool peerOnHoldOK ( );
public:
	SipLeg ( OriginateLegThread * t, CommonCallDetails & c, const RecodeInfoVector & inCodecs,
		PMutex * wm, unsigned ref, bool isRecordRouteRequired );

private:
	SIPAuthenticateValidator sipValidator;
	ss :: string p_asserted_identity_; // recives P-Asserted-Identity value.
	bool isRecordRouteRequired_;
    bool isAccSend_; // Zamena A-num imenem kartochki.
    ss :: string strContactRoute_; // Route from "Contact" field of MIME.
	ss :: string strToTag_;
    ss :: string strContactFromOK_;

    ss :: string strSentDigits_;
    ss :: string accountCard_;
    bool directRTP_;
};

#endif
