#ifndef MGCPEPORIGINATEHANDLER_HPP_
#define MGCPEPORIGINATEHANDLER_HPP_
#pragma interface
class EpOriginateThreadHandler;
class EpOriginateHandler : public EpStateHandler {
	EpOriginateThreadHandler * handler;
	enum {
		cCreateConnection,
		cNotifyRequest1,
		cWaitForNotify,
		cNotifyRequest2,
		cWaitForDetach
	} faState;
	int retries;
	unsigned tid;
	bool hookUp;
	ss :: string callId;
	ss :: string mediaIp, signalIp;
	CodecInfo * codec;
	unsigned telephoneEventsPayloadType;
	int rtpPort;
	void reSend ( MGCP :: PduVector & ret );
	void sendCrcx ( MGCP :: PduVector & ret );
	void sendRqnt1 ( MGCP :: PduVector & ret );
	void sendRqnt2 ( MGCP :: PduVector & ret );
	void handleResponse ( MGCP :: PduVector & ret, const MGCP :: PDU & pdu );
	void handleRequest ( MGCP :: PduVector & ret, const MGCP :: PDU & pdu, const PIPSocket :: Address & ia );
	void transactionTimedOut ( MGCP :: PduVector & ret, const MGCP :: PDU & pdu );
	~EpOriginateHandler ( );
public:
	EpOriginateHandler ( Ep * e, const OriginateCallArg & a );
};
#endif /*MGCPEPORIGINATEHANDLER_HPP_*/
