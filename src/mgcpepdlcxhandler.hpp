#ifndef MGCPEPDLCXHANDLER_HPP_
#define MGCPEPDLCXHANDLER_HPP_
#pragma interface

class EpDlcxHandler : public EpStateHandler {
	unsigned tid;
	int retries;
	bool hookUp;
	bool ok;
	void sendDlcx ( MGCP :: PduVector & ret );
public:
	EpDlcxHandler ( Ep * e, MGCP :: PduVector & ret, bool hu, bool ok );
	void transactionTimedOut ( MGCP :: PduVector & ret, const MGCP :: PDU & pdu );
	void handleResponse ( MGCP :: PduVector & ret, const MGCP :: PDU & pdu );
};

#endif /*MGCPEPDLCXHANDLER_HPP_*/
