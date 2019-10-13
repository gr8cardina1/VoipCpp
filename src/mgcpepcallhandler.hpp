#ifndef MGCPEPCALLHANDLER_HPP_
#define MGCPEPCALLHANDLER_HPP_
#pragma interface

class EpThreadHandler;
class EpCallHandler : public EpStateHandler {
	ss :: string dialedDigits;
	ss :: string callId;
	ss :: string connectionId;
	ss :: string answeredAddr;
	EpThreadHandler * handler;
	class Data;
	Data * data;
	PIPSocket :: Address ifAddr;
	unsigned tid;
	unsigned answeredPort;
	unsigned answeredTelephoneEventsPayloadType;
	int retries;
	enum State {
		cNotify,
		cCreateConnection,
		cWaitForModify,
		cModify,
		cRinging,
		cWaitForModify2,
		cModify2,
		cWaitForDetach,
		cNumStates
	} faState;
	void sendRqnt ( MGCP :: PduVector & ret );
	void sendCrcx ( MGCP :: PduVector & ret );
	void sendMdcx ( MGCP :: PduVector & ret );
	void sendRinging ( MGCP :: PduVector & ret );
	void sendMdcx2 ( MGCP :: PduVector & ret );
	void reSend ( MGCP :: PduVector & ret );
	void transactionTimedOut ( MGCP :: PduVector & ret, const MGCP :: PDU & pdu );
	void handleRequest ( MGCP :: PduVector & ret, const MGCP :: PDU & pdu, const PIPSocket :: Address & ia );
	void handleResponse ( MGCP :: PduVector & ret, const MGCP :: PDU & pdu );
	void createCall ( MGCP :: PduVector & ret, const MGCP :: PDU & pdu );
	void checkDeferredCommands ( MGCP :: PduVector & ret );
	~EpCallHandler ( );
	void sendRinging ( MGCP :: PduVector & ret, int localRtpPort, const PIPSocket :: Address & localAddr,
		const CodecInfo & inCodec );
	void sendOk ( MGCP :: PduVector & ret, int localRtpPort, const PIPSocket :: Address & localAddr,
		const CodecInfo & inCodec, unsigned telephoneEventsPayloadType );
public:
	EpCallHandler ( Ep * e, MGCP :: PduVector & ret, const ss :: string & dd, const PIPSocket :: Address & ia );
};

#endif /*MGCPEPCALLHANDLER_HPP_*/
