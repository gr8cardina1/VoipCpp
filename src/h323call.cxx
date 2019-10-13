#pragma implementation
#include "ss.hpp"
#include "allocatable.hpp"
#include <ptlib.h>
#include <ptlib/sockets.h>
#include "session.hpp"
#include "aftertask.hpp"
#include "ipport.hpp"
#include "icqcontact.hpp"
#include "radgwinfo.hpp"
#include "Conf.hpp"
#include "rtpsession.hpp"
#include "rtpstat.hpp"
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/sequenced_index.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/member.hpp>
#include "q931.hpp"
#include "pointer.hpp"
#include "codecinfo.hpp"
#include "signallingoptions.hpp"
#include "tarifround.hpp"
#include "tarifinfo.hpp"
#include "pricedata.hpp"
#include "outcarddetails.hpp"
#include "outchoicedetails.hpp"
#include "h323call.hpp"
#include "callcontrol.hpp"
#include "sourcedata.hpp"
#include "moneyspool.hpp"
#include <tr1/functional>
#include "smartguard.hpp"
#include <tr1/memory>
#include "commoncalldetails.hpp"
#include "legthread.hpp"
#include <queue>
#include "originatelegthread.hpp"
#include "automutex.hpp"
#include <ptlib/svcproc.h>

H323Call :: H323Call ( ) : first ( 0 ), telephoneEventsPayloadType ( 0 ) { }

H323Call :: ~H323Call ( ) {
	if ( first )
		PSYSTEMLOG ( Error, "first thread did not terminate" );
}

const PTime & H323Call :: getInitTime ( ) const {
	return initTime;
}

int H323Call :: getLocalPort ( const LegThread * c ) {
	AutoMutex am ( mut );
	return getSession ( ) -> getLocalAddress ( c != first, false );
}

PIPSocket :: Address H323Call :: getLocalIp ( const LegThread * /*c*/ ) {
	return INADDR_ANY;
}

void H323Call :: setPeer ( const LegThread * c, const PIPSocket :: Address & local, bool fromNat ) {
	AutoMutex am ( mut );
	if ( c == first )
		getSession ( ) -> setFrom ( local, fromNat );
	else
		getSession ( ) -> setTo ( local, fromNat );
}

void H323Call :: setSendAddress ( const LegThread * c, const PIPSocket :: Address & remote, int port,
	const CodecInfo & /*inCodec*/, const CodecInfo & /*outCodec*/, const CodecInfo & /*changedInCodec*/,
		const CodecInfo & /*changedOutCodec*/ ) {
	AutoMutex am ( mut );
	getSession ( ) -> setSendAddress ( c == first, true, remote, WORD ( port + 1 ), 0, 0, false, "", "" );
	getSession ( ) -> setSendAddress ( c == first, false, remote, WORD ( port ), 0, 0, false, "", "" );
}

void H323Call :: setTelephoneEventsPayloadType ( const LegThread * /*c*/, unsigned payload ) {
	if ( payload ) {
		AutoMutex am ( mut );
		getSession ( ) -> setTelephoneEventsPayloadType ( telephoneEventsPayloadType = payload );
		// pohoge v ipss do setsendaddress eto en srabotaet
		// odnako chtobi eto zarabotalo v callbacke vse ravno nado mnogo dodelivat
	}
}

void H323Call :: getTimeout ( int & i, int & o ) {
	AutoMutex am ( mut );
	getSession ( ) -> getTimeout ( i, o );
}

OutTryVector H323Call :: getOutTries ( LegThread * /*c*/ ) {
	static OutTryVector tries;
	return tries;
}

bool H323Call :: fullAccount ( const LegThread * /*c*/ ) {
	return true;
}

void CallBackCall :: stop ( LegThread * c, int cause ) {
	bool del = false;
	{
		AutoMutex am ( mut );
		if ( c == first ) {
			PSYSTEMLOG ( Info, "CallBackCall :: stop: stopping first leg" );
			first = 0;
			if ( second )
				second -> Close ( cause );
			else
				del = true;
		} else if ( c == second ) {
			PSYSTEMLOG ( Info, "CallBackCall :: stop: stopping second leg" );
			second = 0;
			if ( first )
				first -> Close ( cause );
			else
				del = true;
		} else
			PSYSTEMLOG ( Error, "unknown leg stopped: " << c << " !!!!!!!!!!!!" );
	}
	if ( del )
		conf -> remove ( this );
}

const RTPStat * CallBackCall :: getStats ( const LegThread * /*c*/ ) {
	AutoMutex am ( mut );
	session = 0;
	return stats;
}

RTPSession * CallBackCall :: getSession ( ) {
	if ( ! session )
		session = new RTPSession ( stats, INADDR_ANY, INADDR_ANY, false, false );
		//net zaranee informacii ob oboih adresah i ob nat tem bolee
	return session;
}

CallBackCall :: CallBackCall ( bool parallelStart, int i, const ss :: string & an, const ss :: string & acc,
	const ss :: string & f, const ss :: string & s ) : second ( 0 ), secondId ( i + 1 ), secondNumber ( s ), ani ( an ),
	acctn ( acc ), secondStarted ( parallelStart ) {
	CodecInfoVector c;
	c.c.push_back ( CodecInfo ( "g729r8", 4 ) );
	new CallBackLegThread ( this, & first, ani, acctn, f, i, c, c );
	if ( parallelStart )
		new CallBackLegThread ( this, & second, ani, acctn, secondNumber, secondId, c, c );
}

void CallBackCall :: connected ( LegThread * c ) {
	AutoMutex am ( mut );
	if ( c != first && c != second )
		PSYSTEMLOG ( Error, "unknown leg connected: " << c << " !!!!!!!!!!!!" );
	if ( c == first && ! secondStarted ) {
		secondStarted = true;
		CodecInfoVector ci;
		ci.c.push_back ( CodecInfo ( "g729r8", 4 ) );
		new CallBackLegThread ( this, & second, ani, acctn, secondNumber, secondId, ci, ci );
	}
}

void CallBackCall :: alerted ( LegThread * /*c*/ ) { }

CallBackCall :: ~CallBackCall ( ) {
	if ( second )
		PSYSTEMLOG ( Error, "second thread did not terminate" );
	if ( session )
		PSYSTEMLOG ( Error, "session not stopped" );
}

void CallBackCall :: setDirect ( LegThread *, bool ) { }

void CallBackCall :: onHold ( const LegThread * c, int level ) {
	AutoMutex am ( mut );
	if ( c == first ) {
		if ( second )
			dynamic_cast < OriginateLegThread * > ( second ) -> peerOnHold ( level );
	} else if ( c == second ) {
		if ( first )
			dynamic_cast < OriginateLegThread * > ( first ) -> peerOnHold ( level );
	} else
		PSYSTEMLOG ( Error, "onHold for unknown leg" );
}

void CallBackCall :: onHoldOK ( const LegThread * c ) {
	AutoMutex am ( mut );
	if ( c == first ) {
		if ( second )
			dynamic_cast < OriginateLegThread * > ( second ) -> peerOnHoldOK ( );
	} else if ( c == second ) {
		if ( first )
			dynamic_cast < OriginateLegThread * > ( first ) -> peerOnHoldOK ( );
	} else
		PSYSTEMLOG ( Error, "onHoldOK for unknown leg" );
}

void CallBackCall :: sendDtmf ( const LegThread * c, const DTMF :: Relay & relay ) {
	AutoMutex am ( mut );
	if ( c == first ) {
		if ( second )
			dynamic_cast < OriginateLegThread * > ( second ) -> peerDtmf ( relay );
	} else if ( c == second ) {
		if ( first )
			dynamic_cast < OriginateLegThread * > ( first ) -> peerDtmf ( relay );
	} else
		PSYSTEMLOG ( Error, "sendDtmf for unknown leg" );
}
