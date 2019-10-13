#pragma implementation
#include "ss.hpp"
#include "allocatable.hpp"
#include "mgcp.hpp"
#include <ptlib.h>
#include <ptlib/sockets.h>
#include "mgcpepstatehandler.hpp"
#include "mgcpepdlcxhandler.hpp"
#include <ptlib/svcproc.h>
#include "mgcpep.hpp"
#include "mgcpepnotifyhandler.hpp"

void EpDlcxHandler :: sendDlcx ( MGCP :: PduVector & ret ) {
	MGCP :: PDU pdu ( ep -> getName ( ), ep -> getGwName ( ), ep -> getIp ( ), ep -> getPort ( ),
		MGCP :: PDU :: vDeleteConnection );
	ret.push_back ( pdu );
	tid = pdu.getId ( );
}

EpDlcxHandler :: EpDlcxHandler ( Ep * e, MGCP :: PduVector & ret, bool hu, bool o ) : EpStateHandler ( e ), tid ( 0 ),
	retries ( 1 ), hookUp ( hu ), ok ( o ) {
	sendDlcx ( ret );
}

void EpDlcxHandler :: transactionTimedOut ( MGCP :: PduVector & ret, const MGCP :: PDU & pdu ) {
	if ( pdu.getId ( ) != tid )
		PSYSTEMLOG ( Error, "unknown transaction timeout: " << pdu.getId ( ) );
	if ( ++ retries <= 3 )
		sendDlcx ( ret ); //else just wait for rsip
}

void EpDlcxHandler :: handleResponse ( MGCP :: PduVector & ret, const MGCP :: PDU & pdu ) {
	if ( tid != pdu.getId ( ) ) {
		PSYSTEMLOG ( Error, "unknown transaction response: " << pdu.getId ( ) );
		return;
	}
	if ( pdu.getCode ( ) == MGCP :: PDU :: rcDeleted || pdu.getCode ( ) == MGCP :: PDU :: rcOK )
		ep -> changeHandler ( new EpNotifyHandler ( ep, ret, hookUp, ok ) );
	else
		PSYSTEMLOG ( Error, "unknown response to dlcx: " << pdu.getCode ( ) );
}
