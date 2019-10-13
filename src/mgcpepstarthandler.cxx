#pragma implementation
#include "ss.hpp"
#include "allocatable.hpp"
#include "mgcp.hpp"
#include <ptlib.h>
#include <ptlib/sockets.h>
#include "mgcpepstatehandler.hpp"
#include "mgcpepstarthandler.hpp"
#include <ptlib/svcproc.h>
#include "mgcpep.hpp"
#include "mgcpepnotifyhandler.hpp"
#include "mgcpepdlcxhandler.hpp"

void EpStartHandler :: sendAuep ( MGCP :: PduVector & ret ) {
	MGCP :: PDU pdu ( ep -> getName ( ), ep -> getGwName ( ), ep -> getIp ( ), ep -> getPort ( ),
		MGCP :: PDU :: vAuditEndpoint );
	MGCP :: MIMEInfo & mime = pdu.getMIME ( );
	ss :: string ri;
	ri = "R,D,S,X,Q,N,I,T,O,ES";
	if ( ep -> sBearerInformation )
		ri += ",B";
	if ( ep -> sRestartMethod )
		ri += ",RM";
	if ( ep -> sRestartDelay )
		ri += ",RD";
	if ( ep -> sReasonCode )
		ri += ",E";
	if ( ep -> sPackageList )
		ri += ",PL";
	if ( ep -> sMaxMGCPDatagram )
		ri += ",MD";
	ri += ",A";
	if ( ep -> sPersistentEvents )
		ri += ",B/PR";
	if ( ep -> sNotificationState )
		ri += ",B/NS";
//	mime.setRequestedInfo ( "R,D,S,X,Q,N,I,T,O,ES,B,RM,RD,E,PL,MD,A,B/PR,B/NS" );
	mime.setRequestedInfo ( ri );
	ret.push_back ( pdu );
	tid = pdu.getId ( );
}

EpStartHandler :: EpStartHandler ( Ep * e, MGCP :: PduVector & ret, bool o ) : EpStateHandler ( e ), tid ( 0 ), retries ( 1 ), ok ( o ) {
	sendAuep ( ret );
}

void EpStartHandler :: transactionTimedOut ( MGCP :: PduVector & ret, const MGCP :: PDU & pdu ) {
	if ( pdu.getId ( ) != tid )
		PSYSTEMLOG ( Error, "unknown transaction timeout: " << pdu.getId ( ) );
	if ( ++ retries <= 3 )
		sendAuep ( ret ); //else just wait for rsip
}

void EpStartHandler :: handleResponse ( MGCP :: PduVector & ret, const MGCP :: PDU & pdu ) {
	if ( tid != pdu.getId ( ) ) {
		PSYSTEMLOG ( Error, "unknown transaction response: " << pdu.getId ( ) );
		return;
	}
	if ( pdu.getCode ( ) == MGCP :: PDU :: rcOK ) {
		ss :: string es;
		pdu.getMIME ( ).getEventStates ( es );
		bool hookUp = ( es != "L/hd" );
		if ( hookUp )
			ok = true;
		if ( pdu.getMIME ( ).hasConnectionId ( ) )
			ep -> changeHandler ( new EpDlcxHandler ( ep, ret, hookUp, ok ) );
		else
			ep -> changeHandler ( new EpNotifyHandler ( ep, ret, hookUp, ok ) );
	}
}
