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
#include "pointer.hpp"
#include "codecinfo.hpp"
#include "signallingoptions.hpp"
#include "tarifround.hpp"
#include "tarifinfo.hpp"
#include "pricedata.hpp"
#include "outcarddetails.hpp"
#include "outchoicedetails.hpp"
#include "h323call.hpp"
#include "answercall.hpp"
#include "automutex.hpp"
#include "callcontrol.hpp"
#include "moneyspool.hpp"
#include "sourcedata.hpp"
#include <tr1/functional>
#include "smartguard.hpp"
#include <tr1/memory>
#include "commoncalldetails.hpp"
#include "legthread.hpp"
#include "q931.hpp"
#include "answerlegthread.hpp"
#include <queue>
#include "originatelegthread.hpp"
#include <stdexcept>
#include <cstring>
#include "asn.hpp"
#include "h225.hpp"
#include "calldetails.hpp"
#include "h323common.hpp"
#include "h323answerleg.hpp"
#include <boost/multi_index/mem_fun.hpp>
#include "ixcudpsocket.hpp"
#include "sip.hpp"
#include "sipcalldetails.hpp"
#include "sipanswerleg.hpp"
#include <boost/lambda/lambda.hpp>
#include <ptlib/svcproc.h>
#include "ssconf.hpp"
#include "mgcpepthreadhandler.hpp"
#include "mgcpanswerleg.hpp"
#include "sdp.hpp"
#include "sipanswerhandler.hpp"
#include "sipanswerleg2.hpp"

void AnswerCall :: stop ( LegThread * c, int cause ) {
	bool del = false;
	{
		AutoMutex am ( mut );
		if ( c == first ) {
			PSYSTEMLOG ( Info, "AnswerCall :: stop: stopping first leg" );
			first = 0;
			if ( second )
				second -> Close ( cause );
			else if ( delayedThreads.empty ( ) )
				del = true;
			else
				for ( DelayedThreadsMap :: iterator i = delayedThreads.begin ( ); i != delayedThreads.end ( ); ++ i )
					i -> first -> Close ( cause );
		} else if ( c == second ) {
			PSYSTEMLOG ( Info, "AnswerCall :: stop: stopping second leg" );
			second = 0;
			if ( first )
				first -> Close ( cause );
			else if ( delayedThreads.empty ( ) )
				del = true;
		} else {
			DelayedThreadsMap :: iterator i = delayedThreads.find ( c );
			if ( i != delayedThreads.end ( ) )
				delayedThreads.erase ( i );
			if ( delayedThreads.empty ( ) && ! second ) {
				if ( ! first )
					del = true;
				else
					first -> Close ( cause );
			}
		}
	}
	if ( del ) {
		conf -> remove ( this );
		delete this; //nado chto-to odno
	}
}

void AnswerCall :: released ( int cause ) {
	AutoMutex am ( mut );
	if ( second )
		second -> Close ( cause );
	for ( DelayedThreadsMap :: iterator i = delayedThreads.begin ( ); i != delayedThreads.end ( ); ++ i )
		i -> first -> Close ( cause );
}

const RTPStat * AnswerCall :: getStats ( const LegThread * c ) {
	if ( c == second ) {
		AutoMutex am ( mut );
		session = 0;
	}
	return stats;
}

RTPSession * AnswerCall :: getSession ( ) {
	if ( ! session ) {
		PSYSTEMLOG(Info, "AnswerCall :: getSession: Create new RTPSession: " << stats << ' ' <<
		inLocalAddr << ' ' << INADDR_ANY << ' ' << fromNat << " false");
		session = new RTPSession ( stats, inLocalAddr, INADDR_ANY, fromNat, false );
	}
	//net zaranee informacii ob oboih adresah i ob nat tem bolee
	return session;
}

static PIPSocket :: Address getLocalAddr ( PIPSocket & s ) {
	PIPSocket :: Address a = INADDR_ANY;
	s.GetLocalAddress ( a );
	return a;
}

AnswerCall :: AnswerCall ( const OutChoiceDetailsVectorVector & choiceForks, const StringVector & forkOutAcctns,
	PTCPSocket * s, int i, const CallDetails & d ) : second ( 0 ), fromInRtpCodec ( 0 ), fromInRtpFrames ( 0 ),
	fromOutRtpCodec ( 0 ), fromOutRtpFrames ( 0 ), inAddr ( d.fsAddr ), inLocalAddr ( getLocalAddr ( * s ) ),
	inPort ( d.fsPort ), outPort ( 0 ), fromInSendCodec ( false ), fromOutSendCodec ( false ),
	directIn ( d.common.getDirectIn ( ) ), directOut ( false ), hasFastStart ( ! d.fastStart.empty ( ) ),
	fromNat ( d.common.getFromNat ( ) ), useDelays ( choiceForks.size ( ) > 1 ), forkAccounted ( false ) {
	initTime = d.setupTime;
	if ( d.common.getIncomingCodecs ( ).c.empty ( ) )
		std :: remove_copy_if ( d.common.getInAllowedCodecs ( ).begin ( ), d.common.getInAllowedCodecs ( ).end ( ),
			std :: back_inserter ( toTermCodecs.c ), std :: mem_fun_ref ( & CodecInfo :: getSupported ) );
	else
		toTermCodecs = d.common.getIncomingCodecs ( );
	fromTermCodecs = toTermCodecs;

	AutoMutex am ( mut );

	new H323AnswerLegThread ( this, & first, s, i, d );
	if ( ! useDelays )
		new OriginateFullThread ( this, & second, i, d.common, toTermCodecs, fromTermCodecs, ptH323, 69 );
	else {
		for ( unsigned i = 0; i < choiceForks.size ( ); i ++ ) {
			LegThread * p = 0;
			CommonCallDetails common = d.common;
			common.source ( ).outAcctn = forkOutAcctns [ i ];
			const OutChoiceDetailsVector & choices = choiceForks [ i ];
			common.initChoices ( int ( choices.size ( ) ) );
			for ( unsigned j = 0; j < choices.size ( ); j ++ )
				common.addChoice ( choices [ j ] );
			new OriginateFullThread ( this, & p, getNewCallId ( ), common, toTermCodecs, fromTermCodecs, ptH323, 69 );
			delayedThreads.insert ( std :: make_pair ( p, Delays ( ) ) );
		}
	}
}

AnswerCall :: AnswerCall ( const OutChoiceDetailsVectorVector & choiceForks, const StringVector & forkOutAcctns,
	const SipCallDetails * call, const ss :: string & localIp ) : second ( 0 ),
	fromInRtpCodec ( 0 ), fromInRtpFrames ( 0 ), fromOutRtpCodec ( 0 ), fromOutRtpFrames ( 0 ),
	inAddr ( call -> origRtpAddr ), inLocalAddr ( INADDR_ANY ), inPort ( call -> origRtpPort ), outPort ( 0 ),
	fromInSendCodec ( false ), fromOutSendCodec ( false ), directIn ( call -> common.getDirectIn ( ) ),
	directOut ( false ), hasFastStart ( true ), fromNat ( call -> common.getFromNat ( ) ),
	useDelays ( choiceForks.size ( ) > 1 ), forkAccounted ( false ) {
	initTime = call -> startTime;
	if ( call -> common.getIncomingCodecs ( ).c.empty ( ) )
		std :: remove_copy_if ( call -> common.getInAllowedCodecs ( ).begin ( ),
			call -> common.getInAllowedCodecs ( ).end ( ),
			std :: back_inserter ( toTermCodecs.c ), std :: mem_fun_ref ( & CodecInfo :: getSupported ) );
	else
		toTermCodecs = call -> common.getIncomingCodecs ( );
	fromTermCodecs = toTermCodecs;
	telephoneEventsPayloadType = call -> common.getTelephoneEventsPayloadType ( );
	AutoMutex am ( mut );
	new SipAnswerLegThread ( this, & first, call -> id, call, localIp );
	if ( ! useDelays )
		new OriginateFullThread ( this, & second, call -> id, call -> common, toTermCodecs, fromTermCodecs, ptSip, 69 );
	else {
		for ( unsigned i = 0; i < choiceForks.size ( ); i ++ ) {
			LegThread * p = 0;
			CommonCallDetails common = call -> common;
			common.source ( ).outAcctn = forkOutAcctns [ i ];
			const OutChoiceDetailsVector & choices = choiceForks [ i ];
			OutChoiceDetailsVector :: size_type csz = choices.size ( );
			common.initChoices ( int ( csz ) );
			for ( OutChoiceDetailsVector :: size_type j = 0; j < csz; j ++ )
				common.addChoice ( choices [ j ] );
			new OriginateFullThread ( this, & p, getNewCallId ( ), common, toTermCodecs, fromTermCodecs, ptSip, 69 );
			delayedThreads.insert ( std :: make_pair ( p, Delays ( ) ) );
		}
	}
}

AnswerCall :: AnswerCall ( const OutChoiceDetailsVectorVector & choiceForks, const StringVector & forkOutAcctns,
	const CommonCallDetails & c, const SDP :: SessionDescription & sdp, EpThreadHandler * & t,
	const ss :: string & gw, const ss :: string & ep, const PTime & it ) : second ( 0 ), fromInRtpCodec ( 0 ),
	fromInRtpFrames ( 0 ), fromOutRtpCodec ( 0 ), fromOutRtpFrames ( 0 ), inAddr ( INADDR_ANY ),
	inLocalAddr ( INADDR_ANY ), inPort ( 0 ), outPort ( 0 ), fromInSendCodec ( false ), fromOutSendCodec ( false ),
	directIn ( c.getDirectIn ( ) ), directOut ( false ), hasFastStart ( true ), fromNat ( c.getFromNat ( ) ),
	useDelays ( choiceForks.size ( ) > 1 ), forkAccounted ( false ) {
	initTime = it;
	if ( const SDP :: MediaDescription * media = sdp.getMediaDescription ( SDP :: mediaAudio ) ) {
		if ( const ss :: string * ip = media -> getConnectionAddress ( ) )
			inAddr = ip -> c_str ( );
		else
			inAddr = sdp.getConnectionAddress ( ).c_str ( );
		inPort = media -> getPort ( );
	}
	if ( c.getIncomingCodecs ( ).c.empty ( ) )
		std :: remove_copy_if ( c.getInAllowedCodecs ( ).begin ( ), c.getInAllowedCodecs ( ).end ( ),
			std :: back_inserter ( toTermCodecs.c ), std :: mem_fun_ref ( & CodecInfo :: getSupported ) );
	else
		toTermCodecs = c.getIncomingCodecs ( );
	fromTermCodecs = toTermCodecs;
	telephoneEventsPayloadType = c.getTelephoneEventsPayloadType ( );
	unsigned id = getNewCallId ( );
	AutoMutex am ( mut );
	t = new MgcpAnswerLegThread ( this, & first, id, c, gw, ep );
	if ( ! useDelays )
		new OriginateFullThread ( this, & second, id, c, toTermCodecs, fromTermCodecs, ptMgcp, 69, "", "" );
	else {
		for ( unsigned i = 0; i < choiceForks.size ( ); i ++ ) {
			LegThread * p = 0;
			CommonCallDetails common = c;
			common.source ( ).outAcctn = forkOutAcctns [ i ];
			const OutChoiceDetailsVector & choices = choiceForks [ i ];
			OutChoiceDetailsVector :: size_type csz = choices.size ( );
			common.initChoices ( int ( csz ) );
			for ( OutChoiceDetailsVector :: size_type j = 0; j < csz; j ++ )
				common.addChoice ( choices [ j ] );
			new OriginateFullThread ( this, & p, getNewCallId ( ), common, toTermCodecs, fromTermCodecs, ptMgcp, 69, "", "" );
			delayedThreads.insert ( std :: make_pair ( p, Delays ( ) ) );
		}
	}
}

AnswerCall :: AnswerCall ( const OutChoiceDetailsVectorVector & choiceForks, const StringVector & forkOutAcctns,
	const CommonCallDetails & c, const SDP :: SessionDescription & sdp, SIP2 :: AnswerHandler * & t,
	const SIP2 :: ServerTransactionId & tid, unsigned expires, unsigned maxForwards, 
	const ss :: string & remotePartyID, const ss :: string & PAssertID, const ss :: string & Privacy ) 
	: second ( 0 ),
	fromInRtpCodec ( 0 ), fromInRtpFrames ( 0 ), fromOutRtpCodec ( 0 ), fromOutRtpFrames ( 0 ), inAddr ( INADDR_ANY ),
	inLocalAddr ( INADDR_ANY ), inPort ( 0 ), outPort ( 0 ), fromInSendCodec ( false ), fromOutSendCodec ( false ),
	directIn ( c.getDirectIn ( ) ), directOut ( false ), hasFastStart ( true ), fromNat ( c.getFromNat ( ) ),
	useDelays ( choiceForks.size ( ) > 1 ), forkAccounted ( false ) {
	if ( const SDP :: MediaDescription * media = sdp.getMediaDescription ( SDP :: mediaAudio ) ) {
		if ( const ss :: string * ip = media -> getConnectionAddress ( ) )
			inAddr = ip -> c_str ( );
		else
			inAddr = sdp.getConnectionAddress ( ).c_str ( );
		inPort = media -> getPort ( );
	}
	if ( c.getIncomingCodecs ( ).c.empty ( ) )
		std :: remove_copy_if ( c.getInAllowedCodecs ( ).begin ( ), c.getInAllowedCodecs ( ).end ( ),
			std :: back_inserter ( toTermCodecs.c ), std :: mem_fun_ref ( & CodecInfo :: getSupported ) );
	else
		toTermCodecs = c.getIncomingCodecs ( );
	fromTermCodecs = toTermCodecs;
	telephoneEventsPayloadType = c.getTelephoneEventsPayloadType ( );
	unsigned id = getNewCallId ( );
	AutoMutex am ( mut );
	t = new SipAnswerLegThread2 ( this, & first, id, c, tid, expires );
	if ( ! useDelays )
		new OriginateFullThread ( this, & second, id, c, toTermCodecs, fromTermCodecs, ptSip, maxForwards, remotePartyID, PAssertID, Privacy );
	else {
		for ( unsigned i = 0; i < choiceForks.size ( ); i ++ ) {
			LegThread * p = 0;
			CommonCallDetails common = c;
			common.source ( ).outAcctn = forkOutAcctns [ i ];
			const OutChoiceDetailsVector & choices = choiceForks [ i ];
			OutChoiceDetailsVector :: size_type csz = choices.size ( );
			common.initChoices ( int ( csz ) );
			for ( OutChoiceDetailsVector :: size_type j = 0; j < csz; j ++ )
				common.addChoice ( choices [ j ] );
			new OriginateFullThread ( this, & p, getNewCallId ( ), common, toTermCodecs, fromTermCodecs, ptSip,
				maxForwards, remotePartyID, PAssertID, Privacy );
			delayedThreads.insert ( std :: make_pair ( p, Delays ( ) ) );
		}
	}
}

bool AnswerCall :: directRTP ( ) const {
	return hasFastStart && ( directIn || directOut );
}

void AnswerCall :: connected ( LegThread * c ) {
	PSYSTEMLOG ( Error, "AnswerCall :: connected" );
	if ( useDelays && c != first ) {
		Delays delays;
		{
			AutoMutex am ( mut );
			if ( second )
				return;
			second = c;
			useDelays = false;
			for ( DelayedThreadsMap :: const_iterator i = delayedThreads.begin ( ); i != delayedThreads.end ( ); ++ i ) {
				if ( i -> first == c )
					delays = i -> second;
				else
					i -> first -> Close ( 0 );
			}
			delayedThreads.erase ( second );
		}
		if ( delays.setDirect )
			setDirect ( c, * delays.setDirect );
		if ( delays.setPeer )
			setPeer ( c, delays.setPeer -> local, delays.setPeer -> fromNat );
		if ( delays.setSendAddress )
			setSendAddress ( c, delays.setSendAddress -> remote, delays.setSendAddress -> port,
				delays.setSendAddress -> inCodec, delays.setSendAddress -> outCodec,
				delays.setSendAddress -> changedInCodec,
				delays.setSendAddress -> changedOutCodec );
		if ( delays.setTelephoneEventsPayloadType )
			setTelephoneEventsPayloadType ( c, delays.setTelephoneEventsPayloadType );
		alerted ( c );
	}
	AutoMutex am ( mut );
	if ( c == second ) {
		if ( AnswerLegThread * f = dynamic_cast < AnswerLegThread * > ( first ) ) {
			if ( directRTP ( ) ) {
				PSYSTEMLOG ( Info, "calling peerConnected with addr " << outAddr );
				f -> peerConnected ( outPort, outAddr, answeredInCodec, answeredOutCodec,
					telephoneEventsPayloadType );
			} else {
				f -> peerConnected ( getSession ( ) -> getLocalAddress ( false, false ), INADDR_ANY,
					answeredInCodec, answeredOutCodec, telephoneEventsPayloadType );
				getSession ( ) -> setSendAddress ( true, true, inAddr, WORD ( inPort + 1 ), 0, 0, false, "", "" );
				getSession ( ) -> setSendAddress ( true, false, inAddr, WORD ( inPort ), fromOutRtpCodec,
					fromOutRtpFrames, fromOutSendCodec, inRecodeFrom, inRecodeTo );
				if ( telephoneEventsPayloadType )
					getSession ( ) -> setTelephoneEventsPayloadType ( telephoneEventsPayloadType );
			}
		}
	} else if ( c != first )
		PSYSTEMLOG ( Error, "unknown leg connected: " << c << " !!!!!!!!!!!!" );
}

void AnswerCall :: alerted ( LegThread * c ) {
	if ( c != first && useDelays )
		return;
	AutoMutex am ( mut );
	if ( c == second ) {
	PSYSTEMLOG(Info, "AnswerCall :: alerted: " << inAddr << "; " << answeredInCodec.getCodec ( ) );
		if ( AnswerLegThread * f = dynamic_cast < AnswerLegThread * > ( first ) ) {
			if ( directRTP ( ) )
				f -> peerAlerted ( outPort, outAddr, answeredInCodec, answeredOutCodec );
			else {
				f -> peerAlerted ( getSession ( ) -> getLocalAddress ( false, false ), INADDR_ANY,
					answeredInCodec, answeredOutCodec );
				getSession ( ) -> setSendAddress ( true, true, inAddr, WORD ( inPort + 1 ), 0, 0, false, "", "" );
				getSession ( ) -> setSendAddress ( true, false, inAddr, WORD ( inPort ), fromOutRtpCodec,
					fromOutRtpFrames, fromOutSendCodec, inRecodeFrom, inRecodeTo );
			}
		}
	} else if ( c != first )
		PSYSTEMLOG ( Error, "unknown leg alerted: " << c << " !!!!!!!!!!!!" );

}

void AnswerCall :: setSendAddress ( const LegThread * c, const PIPSocket :: Address & remote, int port,
	const CodecInfo & inCodec, const CodecInfo & outCodec, const CodecInfo & changedInCodec,
	const CodecInfo & changedOutCodec ) {
	AutoMutex am ( mut );
	if ( c != first && useDelays ) {
		DelayedThreadsMap :: iterator i = delayedThreads.find ( const_cast < LegThread * > ( c ) );
		i -> second.setSendAddress = new SetSendAddress ( remote, port, inCodec, outCodec,
			changedInCodec, changedOutCodec );
		return;
	}
	int rtpCodec = 0;
	int rtpFrames = 0;
	bool sendCodec = false;
	CodecInfo ic = inCodec, oc = outCodec;
	const CodecInfo * answerFromInCodec = & ic;
	const CodecInfo * answerFromOutCodec = & oc;
	if ( c == second ) {
		typedef CodecInfoContainer :: index < Codec > :: type ByCodec;
		ByCodec & toTermByCodec = toTermCodecs.c.get < Codec > ( );
		ByCodec :: const_iterator i = toTermByCodec.find ( ic );
		if ( noAnnexB && i == toTermByCodec.end ( ) && ic.getCodec ( ) == "g729br8" ) {
			ic.setCodec ( "g729r8" );
			i = toTermByCodec.find ( ic );
		}
		if ( i != toTermByCodec.end ( ) /*&& i -> getFrames ( ) != ic.getFrames ( )*/ ) {
			rtpCodec = fromInRtpCodec = getRtpCodec ( changedInCodec );
			rtpFrames = fromInRtpFrames = changedInCodec.getFrames ( );
			sendCodec = fromInSendCodec = fromInRtpCodec && fromInRtpFrames;
			answerFromInCodec = & * i;
		}
		ByCodec & fromTermByCodec = fromTermCodecs.c.get < Codec > ( );
		ByCodec :: const_iterator j = fromTermByCodec.find ( oc );
		if ( noAnnexB && j == fromTermByCodec.end ( ) && oc.getCodec ( ) == "g729br8" ) {
			oc.setCodec ( "g729r8" );
			j = fromTermByCodec.find ( oc );
		}
		if ( j != fromTermByCodec.end ( ) /*&& j -> getFrames ( ) != oc.getFrames ( )*/ ) {
			answerFromOutCodec = & * j;
			fromOutRtpCodec = getRtpCodec ( * answerFromOutCodec );
			fromOutRtpFrames = answerFromOutCodec -> getFrames ( );
			fromOutSendCodec = fromOutRtpCodec && fromOutRtpFrames;
		}
	}
	if ( c == first ) {
		inAddr = remote;
		inPort = port;
	} else {
		PSYSTEMLOG ( Info, "setting outAddr to " << remote );
		outAddr = remote;
		outPort = port;
	}
	if ( ! directRTP ( ) ) {
		if ( c == second ) {
			if ( changedOutCodec.getCodec ( ) != outCodec.getCodec ( ) ) {
				inRecodeTo = outCodec.getCodec ( );
				inRecodeFrom = changedOutCodec.getCodec ( );
			}
			if ( changedInCodec.getCodec ( ) != inCodec.getCodec ( ) ) {
				outRecodeTo = changedInCodec.getCodec ( );
				outRecodeFrom = inCodec.getCodec ( );
			}
		}
		getSession ( ) -> setSendAddress ( c == first, true, remote, WORD ( port + 1 ), 0, 0, false, "", "" );
		getSession ( ) -> setSendAddress ( c == first, false, remote, WORD ( port ), rtpCodec, rtpFrames, sendCodec,
			c == second ? outRecodeFrom : inRecodeFrom, c == second ? outRecodeTo : inRecodeTo );
	}
	if ( c == second ) {
		answeredInCodec = * answerFromInCodec;
		answeredOutCodec = * answerFromOutCodec;
	} else if ( c != first )
		PSYSTEMLOG ( Error, "unknown leg setSendAddress: " << c << " !!!!!!!!!!!!" );
}

AnswerCall :: ~AnswerCall ( ) {
	if ( second )
		PSYSTEMLOG ( Error, "second thread did not terminate" );
	if ( session )
		PSYSTEMLOG ( Error, "session not stopped" );
}

void AnswerCall :: setDirect ( LegThread * c, bool d ) {
	if ( c != first && useDelays ) {
		AutoMutex am ( mut );
		DelayedThreadsMap :: iterator i = delayedThreads.find ( c );
		i -> second.setDirect = new SetDirect ( d );
	} else if ( c == second )
		directOut = d;
	else
		PSYSTEMLOG ( Error, "unknown leg in setDirect" );
}

void AnswerCall :: setPeer ( const LegThread * c, const PIPSocket :: Address & local, bool fromNat ) {
	if ( c != first && useDelays ) {
		AutoMutex am ( mut );
		DelayedThreadsMap :: iterator i = delayedThreads.find ( const_cast < LegThread * > ( c ) );
		i -> second.setPeer = new SetPeer ( local, fromNat );
		return;
	}
	if ( directRTP ( ) )
		return;
	H323Call :: setPeer ( c, local, fromNat );
}

int AnswerCall :: getLocalPort ( const LegThread * c ) {
	if ( directRTP ( ) )
		return c == first ? outPort : inPort;
	return H323Call :: getLocalPort ( c );
}

PIPSocket :: Address AnswerCall :: getLocalIp ( const LegThread * c ) {
	if ( directRTP ( ) )
		return c == first ? outAddr : inAddr;
	return H323Call :: getLocalIp ( c );
}

OutTryVector AnswerCall :: getOutTries ( LegThread * c ) {
	AutoMutex am ( mut );
	if ( c == first )
		if ( OriginateLegThread * s = dynamic_cast < OriginateLegThread * > ( second ) )
			return s -> getTries ( );
	return H323Call :: getOutTries ( c );
}

void AnswerCall :: newChoice ( LegThread * c ) {
	if ( c != first && useDelays ) {
		AutoMutex am ( mut );
		DelayedThreadsMap :: iterator i = delayedThreads.find ( c );
		i -> second = Delays ( );
	}
}

bool AnswerCall :: fullAccount ( const LegThread * c ) {
	AutoMutex am ( mut );
	if ( forkAccounted )
		return false;
	if ( c == first )
		return false;
	if ( c == second ) {
		forkAccounted = true;
		return true;
	}
	if ( second )
		return false;
	DelayedThreadsMap :: iterator i = delayedThreads.find ( const_cast < LegThread * > ( c ) );
	if ( i == delayedThreads.end ( ) )
		return false;
	i -> second.accounted = true;
	using boost :: lambda :: _1;
	if ( std :: find_if ( delayedThreads.begin ( ), delayedThreads.end ( ),
		! ( & ( & _1 ->* & DelayedThreadsMap :: value_type :: second ) ->* & Delays :: accounted ) )
		!= delayedThreads.end ( ) )
		return false;
	forkAccounted = true; //superfluous ?
	return true;
}

void AnswerCall :: setTelephoneEventsPayloadType ( const LegThread * c, unsigned payload ) {
	if ( ! payload )
		return;
	AutoMutex am ( mut );
	if ( c != first && useDelays ) {
		DelayedThreadsMap :: iterator i = delayedThreads.find ( const_cast < LegThread * > ( c ) );
		i -> second.setTelephoneEventsPayloadType = payload;
		return;
	}
	telephoneEventsPayloadType = payload;
}

void AnswerCall :: onHold ( const LegThread * c, int level ) {
	AutoMutex am ( mut );
	if ( c == first ) {
		if ( second )
			dynamic_cast < OriginateLegThread * > ( second ) -> peerOnHold ( level );
	} else if ( c == second ) {
		if ( first )
			dynamic_cast < AnswerLegThread * > ( first ) -> peerOnHold ( level );
	} else
		PSYSTEMLOG ( Error, "onHold for unknown leg" );
}

void AnswerCall :: onHoldOK ( const LegThread * c ) {
	AutoMutex am ( mut );
	if ( c == first ) {
		if ( second )
			dynamic_cast < OriginateLegThread * > ( second ) -> peerOnHoldOK ( );
	} else if ( c == second ) {
		if ( first )
			dynamic_cast < AnswerLegThread * > ( first ) -> peerOnHoldOK ( );
	} else
		PSYSTEMLOG ( Error, "onHoldOK for unknown leg" );
}

void AnswerCall :: sendDtmf ( const LegThread * c, const DTMF :: Relay & relay ) {
	PSYSTEMLOG(Info, "AnswerCall :: sendDtmf");
	AutoMutex am ( mut );
	if ( c == first ) {
		if ( second )
			dynamic_cast < OriginateLegThread * > ( second ) -> peerDtmf ( relay );
	} else if ( c == second ) {
		if ( first )
			dynamic_cast < AnswerLegThread * > ( first ) -> peerDtmf ( relay );
	} else
		PSYSTEMLOG ( Error, "onHold for unknown leg" );
}

