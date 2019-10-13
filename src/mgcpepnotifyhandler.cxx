#pragma implementation
#include "ss.hpp"
#include "allocatable.hpp"
#include "mgcp.hpp"
#include <ptlib.h>
#include <ptlib/sockets.h>
#include "mgcpepstatehandler.hpp"
#include "mgcpepnotifyhandler.hpp"
#include <ptlib/svcproc.h>
#include "mgcpep.hpp"
#include "mgcpepcallhandler.hpp"
#include "mgcpeporiginatehandler.hpp"
#include "pointer.hpp"
#include "mgcpsignalparser.hpp"
//#include "codecinfo.hpp"

void EpNotifyHandler :: sendRqnt ( MGCP :: PduVector & ret ) {
	MGCP :: PDU pdu ( ep -> getName ( ), ep -> getGwName ( ), ep -> getIp ( ), ep -> getPort ( ),
		MGCP :: PDU :: vNotificationRequest );
	tid = pdu.getId ( );
	MGCP :: MIMEInfo & mime = pdu.getMIME ( );
	if ( hookUp ) {
		rid = "0001";
		mime.setRequestedEvents ( "L/hd(N)" );
//		mime.setRequestedEvents ( "L/hd(E(R(D/[0-9#*T](D),L/hu(N)),S(L/dl),D([0-9*].[#T])))" );
		mime.setSignalRequests ( ss :: string ( ) );
	} else {
		rid = "0002";
		mime.setRequestedEvents ( "D/[0-9#*T](D),L/hu(N),L/hf(N)" );
		if ( ! dialedDigits.empty ( ) )
			mime.setSignalRequests ( ss :: string ( ) );
		else {
			static ss :: string dialTone = "L/dl";
			static ss :: string errorTone = "L/ro(to=4000)";
			if ( ok )
				mime.setSignalRequests ( dialTone );
			else {
				ok = true;
				mime.setSignalRequests ( errorTone );
			}
		}
		mime.setDigitMap ( "(123|x.T)" );
//		mime.setDigitMap ( ss :: string ( ) );
	}
	mime.setRequestIdentifier ( rid );
//	mime.setNotifiedEntity ( "193.108.123.245" );
	ret.push_back ( pdu );
}

EpNotifyHandler :: EpNotifyHandler ( Ep * e, MGCP :: PduVector & ret, bool hu, bool o ) : EpStateHandler ( e ), tid ( 0 ),
	retries ( 1 ), hookUp ( hu ), ok ( o ) {
	sendRqnt ( ret );
}

void EpNotifyHandler :: transactionTimedOut ( MGCP :: PduVector & ret, const MGCP :: PDU & pdu ) {
	if ( pdu.getId ( ) != tid )
		PSYSTEMLOG ( Error, "unknown transaction timeout: " << pdu.getId ( ) );
	if ( ++ retries <= 3 )
		sendRqnt ( ret ); //else just wait for rsip
}

void EpNotifyHandler :: handleRequest ( MGCP :: PduVector & ret, const MGCP :: PDU & pdu, const PIPSocket :: Address & ia ) {
	if ( pdu.getVerb ( ) != MGCP :: PDU :: vNotify ) {
		PSYSTEMLOG ( Error, "unknown request: " << int ( pdu.getVerb ( ) ) );
		EpStateHandler :: handleRequest ( ret, pdu, ia );
		return;
	}
	ss :: string oe;
	const MGCP :: MIMEInfo & mime = pdu.getMIME ( );
	if ( ! mime.getObservedEvents ( oe ) ) {
		PSYSTEMLOG ( Error, "no observed events" );
		EpStateHandler :: handleRequest ( ret, pdu, ia );
		return;
	}
	MGCP :: SignalRequestVector events ( MGCP :: parseSignalRequests ( oe ) );
	if ( events.empty ( ) ) {
		PSYSTEMLOG ( Error, "parsed empty observed events list" );
		EpStateHandler :: handleRequest ( ret, pdu, ia );
		return;
	}
	makeResponse ( ret, pdu, MGCP :: PDU :: rcOK );
	bool hookChanged = false;
	for ( size_t i = 0; i < events.size ( ); i ++ ) {
		const MGCP :: EventName en = events [ i ].getName ( );
		PSYSTEMLOG ( Info, "handling event: " << events [ i ] );
		if ( en.getPackage ( ) == "L" && en.getEvent ( ) == "hd" ) {
			hookUp = false;
			hookChanged = true;
			dialedDigits.clear ( );
			continue;
		}
		if ( en.getPackage ( ) == "L" && en.getEvent ( ) == "hu" ) {
			hookUp = true;
			hookChanged = true;
			dialedDigits.clear ( );
			continue;
		}
		if ( en.getPackage ( ) == "D" ) {
			const ss :: string & d = en.getEvent ( );
			for ( ss :: string :: const_iterator j = d.begin ( ); j != d.end ( ); ++ j ) {
				const char c = * j;
				if ( c == 't' ) {
					PSYSTEMLOG ( Info, "T, dialedDigits: " << dialedDigits );
					//dialedDigits.clear ( );
					continue;
				} else
					dialedDigits += c;
			}
			PSYSTEMLOG ( Info, "dialedDigits: " << dialedDigits );
			continue;
		}
		PSYSTEMLOG ( Error, "neponyatniy event" );
	}
	if ( hookUp || dialedDigits.empty ( ) )
		sendRqnt ( ret );
	else
		ep -> changeHandler ( new EpCallHandler ( ep, ret, dialedDigits, ia ) );
}

void EpNotifyHandler :: handleResponse ( MGCP :: PduVector & ret, const MGCP :: PDU & pdu ) {
	if ( tid != pdu.getId ( ) ) {
		PSYSTEMLOG ( Error, "unknown transaction response: " << pdu.getId ( ) );
		EpStateHandler :: handleResponse ( ret, pdu );
		return;
	}
	switch ( pdu.getCode ( ) ) {
		case MGCP :: PDU :: rcOK:
			return;
		case MGCP :: PDU :: rcOffHook:
			hookUp = false;
			sendRqnt ( ret );
			return;
		case MGCP :: PDU :: rcOnHook:
			hookUp = true;
			sendRqnt ( ret );
			return;
		default:
			PSYSTEMLOG ( Error, "unknown result code: " << int ( pdu.getCode ( ) ) );
			return;
	}
}

bool EpNotifyHandler :: originateCall ( const OriginateCallArg & a ) {
	if ( ! hookUp )
		return false;
	ep -> changeHandler ( new EpOriginateHandler ( ep, a ) );
	return true;
}
