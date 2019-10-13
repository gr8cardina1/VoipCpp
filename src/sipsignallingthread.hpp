#ifndef __SIPSIGNALLINGTHREAD_HPP
#define __SIPSIGNALLINGTHREAD_HPP
#pragma interface
#include "sipcalldetails.hpp"

class SipFastStartElement;
//class SipCallDetails

typedef std :: vector < SipFastStartElement, __SS_ALLOCATOR < SipFastStartElement > > SipFastStartElementVector;

struct CallInternalData {
	CallInternalData ( ) : sequenceNumberTerminator_ ( -1 ), sequenceNumberOriginator_ ( -1 ) { }
	int sequenceNumberTerminator_; // terminator
    int sequenceNumberOriginator_; // originator
	ss :: string p_asserted_identity_; // recives P-Asserted-Identity value.
};

typedef	std :: map < ss :: string, CallInternalData, std :: less < ss :: string >,
	__SS_ALLOCATOR < std :: pair < const ss :: string, CallInternalData > > > MapOfSequences;

class SipReceiver : public Allocatable < __SS_ALLOCATOR > {
	typedef std :: list < SIP :: PDU, __SS_ALLOCATOR < SIP :: PDU > > MessagesList;
	MessagesList messages;
	PMutex & m;
	PCondVar v;
	SipReceiver ( const SipReceiver & );
public:
	SipReceiver ( PMutex & mut );
	~SipReceiver ( );
	bool get ( SIP :: PDU & mesg, int msec = 1000 );
	void put ( const SIP :: PDU & mesg );
	void wake ( );
};

class PInviteTimer : public PTimer {
  PCLASSINFO(PInviteTimer, PTimer);
public:
    PInviteTimer();

    void setCallID(const ss :: string& callID);
    ss :: string getCallID() const;

    PInviteTimer & operator=(
      const PTimeInterval & time    /// New time interval for timer.
    );

protected:
    ss :: string callID_;
};
//class SipCallDetails;


class SipSignallingThread : public PThread, public Allocatable < __SS_ALLOCATOR >, boost :: noncopyable {
	PCLASSINFO ( SipSignallingThread, PThread )
public:
	typedef std :: map < ss :: string, Pointer < SipCallDetails >, std :: less < ss :: string >,
		__SS_ALLOCATOR < std :: pair < const ss :: string, Pointer < SipCallDetails > > > > CallsMap;
	SipSignallingThread ( IxcUDPSocket * recvSocket, unsigned _id );
	~SipSignallingThread ( );
	void Close ( );
	SipReceiver * addReceiver ( const ss :: string & id );
	void removeReceiver ( const ss :: string & id );
	bool sendMessage ( SIP :: PDU & mesg, const PIPSocket :: Address & addr, int port );
	void GetInterfaceAddress(PIPSocket::Address& addr);
	void setTelephoneEventsPayloadType(const ss :: string& callId, bool f, unsigned payload);
	void asyncHunting ( Pointer < SipCallDetails > call );

	enum AdditionalNumber {
		anNone,
		anAnonymous, // *67
        anAnonymous1 // "Anonymous" into "FROM" fields
	};

	enum enTypeMessage {
		tmMessage,
		tmInfo
	};

protected:

	void Main ( );

	bool receiveMesg ( );

	bool handleInvite ( SIP :: PDU & mesg );

    bool handleReinviteFromOrig ( SipCallDetails * call, SIP :: PDU & mesg );
    bool handleReinviteFromTerm ( SipCallDetails * call, SIP :: PDU & mesg );

	bool autorization(SIP :: PDU & mesg, PIPSocket ::Address& fromAddr, WORD& fromPort, ss :: string& username, bool &isCard);

	bool createTimeOutInvite(SipCallDetails * call);
	bool createResponseOK(SipCallDetails * call, SIP :: PDU & mesg);

	bool handleOK ( SIP :: PDU & mesg );
	bool handleCancelOK ( SIP :: PDU & mesg );
	bool handleByeOK ( SIP :: PDU & mesg );
	bool handleMessageOK ( SIP :: PDU & mesg );
	void sendOk ( const SIP :: PDU & mesg, const CommonCallDetails & common, enTypeMessage ts );
	bool handleAck ( SIP :: PDU & mesg );
	bool handleBye ( SIP :: PDU & mesg );
	bool handleCancel ( SIP :: PDU & mesg );
	bool handleRegister ( SIP :: PDU & mesg );
	bool handleOptions ( SIP :: PDU & mesg );
	bool handleTrying ( SIP :: PDU & mesg );
	void sendTrying ( SIP :: PDU & mesg, SIP :: PDU & inviteMesg,
										 PIPSocket ::Address fromAddr, WORD fromPort );
	bool handleRinging ( SIP :: PDU & mesg );
	bool handleError ( SIP :: PDU & mesg );
	//void sendErrBack ( SIP :: PDU & mesg );

	bool handleRefer ( SIP :: PDU & mesg );
	bool handleMessage ( SIP :: PDU & mesg );
	bool handleNotify ( SIP :: PDU & mesg );
	bool handleSubcribe ( SIP :: PDU & mesg );
	bool handleInfo ( SIP :: PDU & mesg );

	bool handleUnAuthorised ( SIP :: PDU & mesg, int authType );
	void sendErrBack ( const SipCallDetails * call );
	void sendErrMessage ( const SIP :: PDU & mesg, const CommonCallDetails & common );
	bool sendAck ( SIP :: PDU & mesg );
	void sendUnAuthorised ( SIP :: PDU & mesg );
	void accountCall ( SipCallDetails * call );
	void checkStalled ( );
	bool tryNextChoice ( SipCallDetails * call );
	void doBalance ( );
	void sendByeForward ( const SipCallDetails * call );
	void sendCancelForward1 ( const SipCallDetails * call );
	void sendByeBackward ( const SipCallDetails * call );
	bool sendInviteForward ( SipCallDetails * call, const ss :: string & authLine = "", int authType = -1 );
	bool hasReceiver ( const SIP :: PDU & mesg );
	void removeUnsupported ( SipCallDetails * call, SipFastStartElementVector & fs ) const;
	void removeReverseUnsupported ( SipCallDetails * call, SipFastStartElementVector & fs ) const;
	void getNoRecodesMapIn ( SipCallDetails * call, SipFastStartElementVector & fs );
	void getRecodesMapIn ( SipCallDetails * call, SipFastStartElementVector & in );
	void getNoRecodesMapOut ( SipCallDetails * call, SipFastStartElementVector & fs );
	void getRecodesMapOut ( SipCallDetails * call, SipFastStartElementVector & out );
//    bool SipSignallingThread :: isCodecPresentInOriginalFastStart(const SipCallDetails :: FastStartMap& mapFS,
//                                                                  const ss :: string& codecName);

//    void changeURITelephonNumber( SipCallDetails * call, const ss :: string& accountCard);
    //void changeToTelephonNumber ( SipCallDetails * call, const ss :: string& accountCard);

    bool handleFastStart ( SipCallDetails * call, SIP :: SessionDescription & sdp );
	bool handleFastStartResponse ( SipCallDetails * call, SIP :: MediaDescription * media );
	void handleAnswerSDP ( SIP :: PDU & mesg, SipCallDetails * call, const ss :: string & myIP );

	bool handleUnAuthorized( const SIP :: PDU & mesg );

	/**
	 * @fn bool handleViaFields(const SIP::PDU& mesg, PIPSocket::Address& fromAddr, WORD& fromPort) const;
	 * @brief Function processes Via header and replaces (if requered) socket source address/port from header ones
	 */

	bool handleViaFields(const SIP::PDU& mesg, PIPSocket::Address& fromAddr, WORD& fromPort) const;

	void startInviteNotifier();
	void stopInviteNotifier();

    bool isInviteFromTerminator(const SIP::PDU& newMesg, const SIP::PDU& oldMesg);
    bool isRepitedInvite(const ss :: string& callId, const ss :: string& newCSeq, bool isTerminatorsInvite );
	bool insertCSeq(const ss :: string& callId, const ss :: string& newCSeq, bool isTerminatorsInvite );
	void deleteCSeq(const ss :: string& callId);

	ss :: string createPAssertId(const SIP::URL& from);
	void insertPAssertId(SIP :: PDU & mesg); // insert into mime
	void setPAssertId(const ss :: string& callId, const ss :: string& p_asserted_identity);

    void deletePAssertId(SIP :: PDU & mesg); // if isPAssertRequired on the out-peer == false P-Asserted-Identity would be removed
	ss :: string getPAssertId(const ss :: string& callId) const;

	bool isRecordRouteRequired ( const SIP :: PDU & mesg);
	bool isRecordRouteRequired ( const SipCallDetails * call);

	bool isForceInsertingDTMFintoInvite (const SIP :: PDU & mesg) const;
	bool isForceInsertingDTMFintoInvite (const SipCallDetails * call) const;

	bool isCNonceTerm(CommonCallDetails* call) const;
	bool isCNonceOrig(CommonCallDetails* call) const;
	bool isCNonce(CommonCallDetails* call) const;

	ss :: string parsesAdditionalNumber(const ss :: string& strFrom,
										ss :: string* newNumber, ss :: string* addNumber);
	AdditionalNumber choiceAdditionalData(const ss :: string& addNumber);

	ss :: string editNumberTo ( const ss :: string& fullOldNumber, const ss :: string& newNumber );
	ss :: string editNumberFromAnonymous ( ) const;
	void checkMessages ( );

	IxcUDPSocket * callerSocket;
	typedef std :: map < ss :: string, Pointer < SipReceiver >, std :: less < ss :: string >,
		__SS_ALLOCATOR < std :: pair < const ss :: string, Pointer < SipReceiver > > > > ReceiversMap;
	ReceiversMap receivers;
	PMutex mut;
	ss :: string myIP;
	MapOfSequences mapOfSequences_;
	static CallsMap calls;
	SIPAuthenticateValidator sipValidator;

	PInviteTimer   inviteTimer_;
	unsigned id;
	int      inviteTimerInterval_;
    bool isAccSend_;
    ss :: string accountCard_;
	PDECLARE_NOTIFIER(PTimer, SipSignallingThread, OnInviteTimeout);
	typedef std :: tr1 :: function < void ( ) > AsyncMessage;
	ThreadMessageQueue < AsyncMessage > q;

    /// Protectes for several invites with same cseq.
//    unsigned lastCSeq_; odnoy peremennoy na 1000 parallelnih zvonkov yavno nedostatochno
};
extern SipSignallingThread * sipThread;
#endif
