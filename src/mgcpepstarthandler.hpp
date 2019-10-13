#ifndef MGCPEPSTARTHANDLER_HPP_
#define MGCPEPSTARTHANDLER_HPP_
#pragma interface

class EpStartHandler : public EpStateHandler {
	unsigned tid;
	int retries;
	bool ok;
	void sendAuep ( MGCP :: PduVector & ret );
public:
	EpStartHandler ( Ep * e, MGCP :: PduVector & ret, bool ok );
	void transactionTimedOut ( MGCP :: PduVector & ret, const MGCP :: PDU & pdu );
	void handleResponse ( MGCP :: PduVector & ret, const MGCP :: PDU & pdu );
};

#endif /*MGCPEPSTARTHANDLER_HPP_*/
