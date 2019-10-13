#ifndef MGCPEPNOTIFYHANDLER_HPP_
#define MGCPEPNOTIFYHANDLER_HPP_
#pragma interface

class EpNotifyHandler : public EpStateHandler {
	unsigned tid;
	int retries;
	ss :: string rid;
	ss :: string dialedDigits;
	bool hookUp;
	bool ok;

	void sendRqnt ( MGCP :: PduVector & ret );
	bool originateCall ( const OriginateCallArg & a );
	void transactionTimedOut ( MGCP :: PduVector & ret, const MGCP :: PDU & pdu );
	void handleRequest ( MGCP :: PduVector & ret, const MGCP :: PDU & pdu, const PIPSocket :: Address & ia );
	void handleResponse ( MGCP :: PduVector & ret, const MGCP :: PDU & pdu );
public:
	EpNotifyHandler ( Ep * e, MGCP :: PduVector & ret, bool hu, bool ok );
};

#endif /*MGCPEPNOTIFYHANDLER_HPP_*/
