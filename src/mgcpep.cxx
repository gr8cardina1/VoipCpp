#pragma implementation
#include "ss.hpp"
#include "allocatable.hpp"
#include "mgcp.hpp"
#include <ptlib.h>
#include <ptlib/sockets.h>
#include "mgcpep.hpp"
#include "mgcpepstatehandler.hpp"
#include "mgcpepstarthandler.hpp"
#include "mgcpgw.hpp"
#include <ptlib/svcproc.h>
#include "mgcpconf.hpp"

Ep :: Ep ( const ss :: string & n, Gw * g, unsigned ip, const MgcpClassInfo & cl ) : name ( n ), gw ( g ), handler ( 0 ),
	inPeer ( ip ), sBearerInformation ( cl.sBearerInformation ), sRestartMethod ( cl.sRestartMethod ),
	sRestartDelay ( cl.sRestartDelay ), sReasonCode ( cl.sReasonCode ), sPackageList ( cl.sPackageList ),
	sMaxMGCPDatagram ( cl.sMaxMGCPDatagram ), sPersistentEvents ( cl.sPersistentEvents ),
	sNotificationState ( cl.sNotificationState ) {
//	if ( g -> getName ( ) == "ata1-h323.whaterxc.com" )
//		sBearerInformation = sRestartMethod = sRestartDelay = sReasonCode = sPackageList =
//			sMaxMGCPDatagram = sPersistentEvents = sNotificationState = false;
//	else if ( g -> getName ( ) == "addpac.whaterxc.com" )
//		sPackageList = sMaxMGCPDatagram = sPersistentEvents = sNotificationState = false;
}

Ep :: ~Ep ( ) {
	delete handler;
}

const ss :: string & Ep :: getGwName ( ) const {
	return gw -> getName ( );
}

const ss :: string & Ep :: getIp ( ) const {
	return gw -> getIp ( );
}

unsigned short Ep :: getPort ( ) const {
	return gw -> getPort ( );
}

void Ep :: init ( MGCP :: PduVector & ret ) {
	changeHandler ( new EpStartHandler ( this, ret, true ) );
}

void Ep :: detachThreadHandler ( MGCP :: PduVector & ret, bool ok ) {
	changeHandler ( new EpStartHandler ( this, ret, ok ) ); //mogno i chto-to poumnee
}

bool Ep :: originateCall ( const OriginateCallArg & a ) {
	return handler -> originateCall ( a );
}

void Ep :: handleRestart ( MGCP :: PduVector & ret ) {
	init ( ret );
}

void Ep :: handleRequest ( MGCP :: PduVector & ret, const MGCP :: PDU & pdu, const PIPSocket :: Address & ia ) {
	switch ( pdu.getVerb ( ) ) {
		case MGCP :: PDU :: vRestartInProgress: {
			ss :: string rm;
			pdu.getMIME ( ).getRestartMethod ( rm );
			int rd = 0;
			if ( rm == "restart" && ( ! pdu.getMIME ( ).getRestartDelay ( rd ) || ! rd ) ) {
				makeResponse ( ret, pdu, MGCP :: PDU :: rcOK );
				return handleRestart ( ret );
			}
			PSYSTEMLOG ( Error, "unsupported restart method " << rm );
			makeResponse ( ret, pdu, MGCP :: PDU :: rcUnknownRestartMethod );
		} default:
			handler -> handleRequest ( ret, pdu, ia );
	}
}

void Ep :: handleResponse ( MGCP :: PduVector & ret, const MGCP :: PDU & pdu ) {
	handler -> handleResponse ( ret, pdu );
}

void Ep :: transactionTimedOut ( MGCP :: PduVector & ret, const MGCP :: PDU & pdu ) {
	handler -> transactionTimedOut ( ret, pdu );
}

void Ep :: changeHandler ( EpStateHandler * h ) {
	delete handler;
	handler = h;
}

void Ep :: sendRinging ( MGCP :: PduVector & ret, int localRtpPort, const PIPSocket :: Address & localAddr,
	const CodecInfo & inCodec ) {
	handler -> sendRinging ( ret, localRtpPort, localAddr, inCodec );
}

void Ep :: sendOk ( MGCP :: PduVector & ret, int localRtpPort, const PIPSocket :: Address & localAddr,
	const CodecInfo & inCodec, unsigned telephoneEventsPayloadType ) {
	handler -> sendOk ( ret, localRtpPort, localAddr, inCodec, telephoneEventsPayloadType );
}

void Ep :: reloadConf ( unsigned ip, const MgcpClassInfo & cl ) {
	inPeer = ip;
	sBearerInformation = cl.sBearerInformation;
	sRestartMethod = cl.sRestartMethod;
	sRestartDelay = cl.sRestartDelay;
	sReasonCode = cl.sReasonCode;
	sPackageList = cl.sPackageList;
	sMaxMGCPDatagram = cl.sMaxMGCPDatagram;
	sPersistentEvents = cl.sPersistentEvents;
	sNotificationState = cl.sNotificationState;
}
