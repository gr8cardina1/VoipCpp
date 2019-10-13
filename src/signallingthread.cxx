#pragma implementation
#include <ptlib.h>
#include <ptlib/sockets.h>
#include "ss.hpp"
#include "allocatable.hpp"
#include "aftertask.hpp"
#include "ipport.hpp"
#include "icqcontact.hpp"
#include "radgwinfo.hpp"
#include "Conf.hpp"
#include "callcontrol.hpp"
#include <stdexcept>
#include "latencylimits.hpp"
#include "fakecalldetector.hpp"
#include "rtpstat.hpp"
#include "signallingoptions.hpp"
#include <boost/archive/text_oarchive.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/sequenced_index.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include "codecinfo.hpp"
#include "tarifround.hpp"
#include "tarifinfo.hpp"
#include "pricedata.hpp"
#include "sourcedata.hpp"
#include "outcarddetails.hpp"
#include "outchoicedetails.hpp"
#include "pointer.hpp"
#include "moneyspool.hpp"
#include <tr1/functional>
#include "smartguard.hpp"
#include <tr1/memory>
#include "commoncalldetails.hpp"
#include <cstring>
#include "asn.hpp"
#include "q931.hpp"
#include "h225.hpp"
#include "calldetails.hpp"
#include "signallingthread.hpp"
//---------------------
#include "h323common.hpp"
#include "h245handler.hpp"
#include "h245thread.hpp"
#include "sqloutthread.hpp"
#include "AddrUtils.h"
#include "session.hpp"
#include "rtpsession.hpp"
#include "legthread.hpp"
#include "h323call.hpp"
#include "answercall.hpp"
#include <ptlib/svcproc.h>
#include "ssconf.hpp"
#include "mysql.hpp"
#include "Log.h"
#include "h323.hpp"
#include "sipcommon.hpp" //toHex
#include "radiuscommon.hpp"
#include "printhex.hpp"
#include <boost/serialization/map.hpp>
#include <boost/serialization/vector.hpp>
#include "serializestring.hpp"
#ifdef linux
#include <mcheck.h>
#endif

//#define PROC_HACK
//#define KEEP_ALIVE


void SignallingThread :: accountCall ( ) {
	if ( ! call.setupInited )
		return;
	if ( ! call.begInited )
		call.endTime = call.begTime;
	else if ( ! call.endInited )
		call.endTime = PTime ( );
	if ( call.tries.empty ( ) )
		common.setPeerIndex ( - 1 );
	else
		common.setPeerIndex ( call.tries.back ( ).choiceIndex );
	ss :: ostringstream os;
	struct TmpStat {
		int frac, jitterMax, jitterAvg;
	};
	CodecInfo codec;
	if ( h ) {
		codec = h -> getCodec ( );
		safeDel ( h );
	}
	TmpStat st [ 4 ];
	std :: memset ( st, 0, sizeof ( st ) );
	for ( int i = 0; i < 4; i ++ )
		if ( int cnt = rtpStats [ i ].count ) {
			st [ i ].frac = rtpStats [ i ].fracLostSum / cnt;
			st [ i ].jitterMax = rtpStats [ i ].jitterMax;
			st [ i ].jitterAvg = rtpStats [ i ].jitterSum / cnt;
		}
	std :: time_t timeInSecs = call.begTime.GetTimeInSeconds ( );
	int secs = conf -> getRounded ( double ( ( call.endTime - call.begTime ).GetMilliSeconds ( ) ) / 1000.0 );
	int secs2 = secs;
	if ( call.delayedDisconnect && call.ddTime >= call.begTime && call.ddTime < call.endTime )
		secs2 = conf -> getRounded ( double ( ( call.ddTime - call.begTime ).GetMilliSeconds ( ) ) / 1000.0 );
	os << "insert into radacctSS values ( 0, '" << common.getCallingDigitsIn ( ) << "', '";
	const SourceData & source = common.getSource ( );
	os << getCallingDigits ( );
	os << "', '";
	ss :: string outCode, inCode = source.code, inEuCode = source.euCode;
	if ( inEuCode.empty ( ) )
		inEuCode = '-';
	int tarifedSeconds = conf -> getRounded ( conf -> getMinutesFromSeconds ( source.tarif, secs ) * 60 ),
		euTarifedSeconds = conf -> getRounded ( conf -> getMinutesFromSeconds ( source.euTarif, secs ) * 60 );
	int outSecs = 0, inSecs = source.round.round ( tarifedSeconds ),
		inEuSecs = source.euRound.round ( euTarifedSeconds );
	double outPrice = 0, outConnectPrice = 0, inPrice = source.price / 100000.0,
		inConnectPrice = inSecs ? source.connectPrice / 100000.0 : 0,
		inEuPrice = source.euPrice / 100000.0,
		inEuConnectPrice = inEuSecs ? source.euConnectPrice / 100000.0 : 0;
	inConnectPrice += getAmortisedMoney ( source.amortise, tarifedSeconds );
	inEuConnectPrice += getAmortisedMoney ( source.amortise, euTarifedSeconds );
	if ( source.type == SourceData :: card )
		os << "card " << source.acctn << ' ' << inEuPrice * source.valuteRate << ' ' << secs << ' ' <<
			( inEuSecs ? inEuConnectPrice * source.valuteRate : 0.0 ) << ' '
			<< inEuCode << ' ' << common.getRealDigits ( ) << ' ' << inEuSecs / 60.0;
	else
		os << common.getDialedDigits ( );
	bool hasCurChoice = common.curChoice ( );
	os << "', '";
	if ( hasCurChoice )
		os << common.getConvertedDigits ( );
	else
		os << '-';
	int ip;
	if ( source.type == SourceData :: inbound && source.peer > 0 )
		ip = source.peer;
	else
		ip = 0;
	os << ' ' << ip;
	ss :: string outDigits;
	int op = 0;
	if ( hasCurChoice ) {
		outDigits = common.curChoice ( ) -> getRealDigits ( ).substr ( common.curChoice ( ) -> getDepth ( ) );
		op = common.curChoice ( ) -> getPeer ( );
		outCode = common.curChoice ( ) -> getCode ( ).substr ( common.curChoice ( ) -> getDepth ( ) );
		outPrice = common.curChoice ( ) -> getPrice ( ) / 100000.0;
		int outTarifedSeconds = conf -> getRounded ( conf -> getMinutesFromSeconds ( common.curChoice ( ) -> getTarif ( ), secs2 ) * 60 );
		outSecs = common.curChoice ( ) -> getRound ( ).round ( outTarifedSeconds );
		outConnectPrice = outSecs ? common.curChoice ( ) -> getConnectPrice ( ) / 100000.0 : 0;
	}
	if ( inCode.empty ( ) )
		inCode = '-';
	if ( outCode.empty ( ) )
		outCode = '-';
	os << ' ' << op << ' ' << rtpStats [ 2 ].bytesCount << ' ' << rtpStats [ 0 ].bytesCount << ' ' << codec <<
		' ' << inCode << ' ' << inPrice << ' ' << inSecs << ' ' << inConnectPrice << ' ' << inEuCode << ' '
		<< inEuPrice << ' ' << inEuSecs << ' ' << inEuConnectPrice << ' ' << outCode << ' ' << outPrice << ' '
		<< outSecs << ' ' << outConnectPrice;
	if ( hasCurChoice && ! source.outAcctn.empty ( ) ) {
		const OutCardDetails & ocd = common.curChoice ( ) -> getOutCardDetails ( );
		int ocdTarifedSecs = conf -> getRounded ( conf -> getMinutesFromSeconds ( ocd.getEuTarif ( ), secs2 ) * 60 );
		int ocdSecs = ocd.getEuRound ( ).round ( ocdTarifedSecs ) ;
		os << " card " << source.outAcctn << ' ' << ocd.getEuPrice ( ) * source.valuteRate / 100000.0 << ' '
			<< secs2 << ' ' << ( ocdSecs ? ( ocd.getEuConnectPrice ( ) + getAmortisedMoney ( ocd.getAmortise ( ), ocdTarifedSecs ) ) * source.valuteRate / 100000.0 : 0.0 )
			<< ' ' << ocd.getDigits ( ) << ' ' << ocd.getEuCode ( ) << ' ' << ocdSecs / 60.0;
	}
	os << "', from_unixtime( " << timeInSecs << " ), from_unixtime( " << call.endTime.GetTimeInSeconds ( ) <<
		" ), " << secs << ", " << secs2 << ", '" << common.getCallerIp ( ) << "', '";
	if ( hasCurChoice )
		os << common.getCalledIp ( );
	os << "', " << common.getDisconnectCause ( );
	for ( int i = 0; i < 4; i ++ )
		os << ", " << st [ i ].frac << ", " << st [ i ].jitterMax << ", " << st [ i ].jitterAvg;
	os << ", '" << common.getRealDigits ( ) << "', '" << source.inPrefix << "', '" << outDigits << "', '";
	ss :: string printableCallId = H323 :: printableCallId ( common.getConfId ( ) );
	os << printableCallId << "', '" << printableCallId << "' )";
	for ( unsigned i = 0; i + 1 < call.tries.size ( ); i ++ ) {
		int index = call.tries [ i ].choiceIndex;
		const OutChoiceDetails * curChoice = common.choice ( index );
		os << ", ( 0, '', '";
		if ( curChoice -> getCallingDigits ( ) != "-" )
			os << curChoice -> getCallingDigits ( );
		os << "', '" << curChoice -> getSentDigits ( ) << "', '" << curChoice -> getDigits ( ) << ' ' << ip << ' ';
		op = curChoice -> getPeer ( );
		os << op << " 0 0 - - 0 0 0 - 0 0 0 ";
		ss :: string outCode = curChoice -> getCode ( ).substr ( curChoice -> getDepth ( ) );
		if ( outCode.empty ( ) )
			outCode = '-';
		double outPrice = curChoice -> getPrice ( ) / 100000.0,
			outConnectPrice = 0;//curChoice -> getConnectPrice ( ) / 100000.0;
		os << outCode << ' ' << outPrice << " 0 " << outConnectPrice << "', from_unixtime( " <<
			timeInSecs << " ), from_unixtime( " << timeInSecs << " ), 0, 0, '', '" <<
			curChoice -> getIp ( ) << "', " << call.tries [ i ].cause <<
			", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '" << common.getRealDigits ( ) << "', '', '" <<
			common.getRealDigits ( ).substr ( curChoice -> getDepth ( ) ) << "', '', '" << printableCallId << "' )";
	}
	sqlOut -> add ( os.str ( ) );
	sendRadiusAcc ( common, rtpStats, call.setupTime, call.outSetupTime, call.begTime, call.endTime, call.ref );
}

SignallingThread :: SignallingThread ( PTCPSocket * socket, unsigned _id ) :
	PThread ( 1000, AutoDeleteThread, NormalPriority, PString ( PString :: Printf,
		"Q931 N%u", _id ) ), id ( _id ),
	callerSocket ( socket ), calledSocket ( 0 ), myH245Thread ( 0 ), h ( 0 ), common ( call.common ),
	directOut ( false ), sourceIsVoIP ( false ), setupTimeSaved ( false ), differentCodec ( false ) {
#ifdef linux
	mtrace ( );
#endif
	callerSocket -> SetReadTimeout ( 5000 );
	if ( ! callerSocket -> SetOption ( SO_KEEPALIVE, 1 ) )
		PSYSTEMLOG ( Error, "SO_KEEPALIVE: " << callerSocket -> GetErrorNumber ( ) << ' ' <<
			callerSocket -> GetErrorText ( ) );
	PIPSocket :: Address peerAddr;
	WORD peerPort;
	callerSocket -> GetPeerAddress ( peerAddr, peerPort );
	PString t = peerAddr.AsString ( );
	common.source ( ).ip = static_cast < const char * > ( t );
	common.setCallerIp ( static_cast < const char * > ( t ) );
	common.setCallerPort ( peerPort );
	if ( conf -> isDebugInIp ( common.getSource ( ).ip ) )
		common.setForceDebug ( true );
	Resume ( );
}

SignallingThread :: ~SignallingThread ( ) {
	if ( callerSocket ) {
		PSYSTEMLOG ( Error, "callerSocket not deleted" );
		safeDel ( callerSocket );
	}
#ifdef linux
	muntrace ( );
#endif
}

void SignallingThread :: Close ( ) {
	// Later on this should be improved to do a proper cleanup of the call
	// rather than just dropping it....
	PSYSTEMLOG ( Info, "SignallingThread::Close ( )" );
	callerSocket -> Close ( );
	if ( calledSocket != 0 )
		calledSocket -> Close ( );
	if ( myH245Thread != 0 )
		myH245Thread -> Close ( );
}

void SignallingThread :: beginShutDown ( ) {
	Close ( );
//	PXAbortBlock ( );
}

bool SignallingThread :: receiveMesg ( bool fromCaller ) {
// Task: to receive a Q931 message
	PTCPSocket * socket = ( fromCaller ? callerSocket : calledSocket );
	try {
		Q931 mesg ( readMsg ( socket ) );
		return handleMesg ( mesg, fromCaller );
	} catch ( std :: exception & e ) {
		PSYSTEMLOG ( Error, "unable to receive Q931 message from " <<
			( fromCaller ? "originator" : "destination" ) << ": " << e.what ( ) );
		return false;
	}
}

void SignallingThread :: closeThisChoice ( ) {
	if ( calledSocket ) {
		if ( call.goodCall )
			sendReleaseComplete ( calledSocket );
		calledSocket -> Close ( );
		safeDel ( calledSocket );
	}
	conf -> release ( common, true, false );
	call.tries.push_back ( OutTry ( common.getPeerIndex ( ), common.getDisconnectCause ( ), H323 :: printableCallId ( call.common.getConfId ( ) ) ) );
	call.h245Spool.clear ( );
	call.fastStartSpool.clear ( );
	progress.clear ( );
}

void SignallingThread :: translateSetup ( Q931 & mesg, H225 :: H323_UserInformation & uuField,
	const H225 :: TransportAddress & sourceCallSigAddr, const H225 :: TransportAddress & destCallSigAddr ) {
	PSYSTEMLOG(Info, "SignallingThread :: translateSetup");
	:: translateSetup ( common, call.setup, call.id, mesg, uuField, sourceCallSigAddr, destCallSigAddr );
	H225 :: H323_UU_PDU & pdu = uuField.m_h323_uu_pdu;
	H225 :: H323_UU_PDU_h323_message_body & body = pdu.m_h323_message_body;
	H225 :: Setup_UUIE & setup = body;
	if ( common.getSigOptions ( ).dropFastStart ( ) )
		setup.removeOptionalField ( H225 :: Setup_UUIE :: e_fastStart );
	else if ( /*! directRTP ( ) &&*/ call.fastStart.size ( ) ) {
		setup.get_fastStart ( ) = handleFastStart ( );
		if ( ! setup.get_fastStart ( ).size ( ) )
			setup.removeOptionalField ( H225 :: Setup_UUIE :: e_fastStart );
	}
	if ( common.getSigOptions ( ).dropTunneling ( ) ) {
		setup.removeOptionalField ( H225 :: Setup_UUIE :: e_parallelH245Control );
		pdu.includeOptionalField ( H225 :: H323_UU_PDU :: e_h245Tunneling );
		pdu.removeOptionalField ( H225 :: H323_UU_PDU :: e_h245Control );
	} else {
		handleH245 ( pdu, common.getSigOptions ( ).permitTunnelingInSetup ( ), true );
		if ( setup.hasOptionalField ( H225 :: Setup_UUIE :: e_parallelH245Control ) )
			handleH245 ( setup.get_parallelH245Control ( ), true );
	}
	if ( common.getSigOptions ( ).removeSymmetricOperation ( ) )
		setup.removeOptionalField ( H225 :: Setup_UUIE :: e_symmetricOperationRequired );
}

static void dropTunneling ( H225 :: H323_UserInformation & uuField ) {
	H225 :: H323_UU_PDU & pdu = uuField.m_h323_uu_pdu;
	pdu.includeOptionalField ( H225 :: H323_UU_PDU :: e_h245Tunneling );
	pdu.get_h245Tunneling ( ) = false;
	pdu.removeOptionalField ( H225 :: H323_UU_PDU :: e_h245Control );
}

void SignallingThread :: sendMesgToDest ( const Q931 & mesg, const H225 :: H323_UserInformation * uuField,
	PTCPSocket * destination ) {
	:: sendMesgToDest ( mesg, uuField, destination, common );
}

void SignallingThread :: sendSetup ( ) {
	Q931 q931 = call.setup;
	H225 :: H323_UserInformation uuField = call.uuField;
	// Translate setup message so that it references our call signalling
	// address
	PIPSocket :: Address sourceAddr;
	WORD sourcePort;
	PIPSocket :: Address destAddr;
	WORD destPort;

	calledSocket -> GetLocalAddress ( sourceAddr, sourcePort );
	calledSocket -> GetPeerAddress ( destAddr, destPort );
	translateSetup ( q931, uuField,
		AddrUtils :: convertToH225TransportAddr ( sourceAddr, sourcePort ),
		AddrUtils :: convertToH225TransportAddr ( destAddr, destPort ) );
	encodeH225IntoQ931 ( uuField, q931 );
	sendMesgToDest ( q931, & uuField, calledSocket );
	call.outSetupTime = PTime ( );
	if ( ! setupTimeSaved ) {
		setupTimeSaved = true;
		conf -> addSetupTime ( ( call.outSetupTime - call.begTime ).GetMilliSeconds ( ) );
	}
	int outPeerId = common.curChoice ( ) -> getPeer ( );
	_fakeCallDetector.initialize ( outPeerId, conf -> getMesgLatency ( outPeerId ) );
}

void SignallingThread :: sendProceeding ( ) {
	Q931 mesg ( Q931 :: msgCallProceeding, call.ref, true );
	H225 :: H323_UserInformation uuField;
	H225 :: H323_UU_PDU & pdu = uuField.m_h323_uu_pdu;
	if ( ! common.getSigOptions ( ).dropTunneling ( ) && call.uuField.m_h323_uu_pdu.hasOptionalField (
		H225 :: H323_UU_PDU :: e_h245Tunneling ) &&
		call.uuField.m_h323_uu_pdu.get_h245Tunneling ( ) ) {
		pdu.includeOptionalField ( H225 :: H323_UU_PDU :: e_h245Tunneling );
		pdu.get_h245Tunneling ( ) = true;
		if ( ! call.originatorVersion2 )
			pdu.includeOptionalField ( H225 :: H323_UU_PDU :: e_provisionalRespToH245Tunneling );
	}
	pdu.m_h323_message_body.setTag ( H225 :: H323_UU_PDU_h323_message_body :: e_callProceeding );
	H225 :: CallProceeding_UUIE & proceeding = pdu.m_h323_message_body;
	proceeding.m_protocolIdentifier = h225ProtocolID;
	proceeding.get_callIdentifier ( ) = call.id;
	encodeH225IntoQ931 ( uuField, mesg );
	sendMesgToDest ( mesg, & uuField, callerSocket );
}

void SignallingThread :: sendTunneling ( ) {
	Q931 mesg ( Q931 :: msgFacility, call.ref, false );
	H225 :: H323_UserInformation uuField;
	H225 :: H323_UU_PDU & pdu = uuField.m_h323_uu_pdu;
	pdu.includeOptionalField ( H225 :: H323_UU_PDU :: e_h245Tunneling );
	pdu.get_h245Tunneling ( ) = true;
	pdu.m_h323_message_body.setTag ( H225 :: H323_UU_PDU_h323_message_body :: e_empty );
	handleH245 ( pdu, true, true );
	encodeH225IntoQ931 ( uuField, mesg );
	sendMesgToDest ( mesg, & uuField, calledSocket );
}

void SignallingThread :: sendKeepalive ( ) {
	Q931 mesg ( Q931 :: msgFacility, call.ref, false );
	H225 :: H323_UserInformation uuField;
	H225 :: H323_UU_PDU & pdu = uuField.m_h323_uu_pdu;
	pdu.m_h323_message_body.setTag ( H225 :: H323_UU_PDU_h323_message_body :: e_empty );
	encodeH225IntoQ931 ( uuField, mesg );
	sendMesgToDest ( mesg, & uuField, callerSocket );
}

void SignallingThread :: getChoices ( ) {
	if ( ! call.setupInited )
		return;
	getCallDetailsForSetup ( );
}

void SignallingThread :: shutDown ( ) {
	if ( call.begInited )
		common.setDisconnectCause ( Q931 :: cvNormalCallClearing );
	else {
		common.setDisconnectCause ( Q931 :: cvNoRouteToDestination );
		call.begTime = call.endTime = PTime ( );
		call.endInited = call.begInited = true;
	}
	if ( ! call.endInited ) {
		call.endTime = PTime ( );
		call.endInited = true;
	}
	sendReleaseComplete ( );
	if ( calledSocket && calledSocket -> IsOpen ( ) )
		sendReleaseComplete ( calledSocket );
}

void SignallingThread :: calledSocketConnected ( ) {
	call.rcSentToDest = false;
	if ( h ) {
		PIPSocket :: Address addr;
		if ( calledSocket -> GetLocalAddress ( addr ) )
			h -> setTo ( addr, common.curChoice ( ) -> getFromNat ( ) );
		h -> setOutPeer ( common.curChoice ( ) -> getPeer ( ), common.getOutAllCodecs ( ),
			common.getOutAllowedCodecs ( ), common.getSigOptions ( ).outAnswerToRTDR ( ) );
	}
}

bool SignallingThread :: tryNextChoiceIteration ( ) {
	safeDel ( h );
	if ( tryNextChoiceIterationH323 ( common, calledSocket ) ) {
		const SourceData & source = common.getSource ( );
		directOut = common.curChoice ( ) -> getDirectRTP ( ) ||
			( source.type == SourceData :: card && ! source.outAcctn.empty ( ) &&
			! common.curChoice ( ) -> getOutCardDetails ( ).getRedirect ( ) &&
			source.ip == common.curChoice ( ) -> getIp ( ) );
		calledSocketConnected ( );
		return true;
	}
	return false;
}

bool SignallingThread :: tryNextChoice ( ) {
	if ( call.goodCall )
		return false;
	if ( conf -> shuttingDown ( ) ) {
		shutDown ( );
		return false;
	}
	bool limited = false;
	int limitedCause = conf -> getDefaultDisconnectCause(); /*Q931 :: cvNoCircuitChannelAvailable;*/
	while ( common.hasNextPeerIndex ( ) ) {
		common.nextPeerIndex ( );
		PSYSTEMLOG ( Info, "Trying destination " << common.getCalledIp ( ) << ':' << common.getCalledPort ( )
			<< ':' << common.getConvertedDigits ( ) );
		if ( conf -> take ( common, limitedCause, 0 ) ) {
			limited = false;
			limitedCause = conf -> getDefaultDisconnectCause(); /*Q931 :: cvNoCircuitChannelAvailable;*/
			if ( tryNextChoiceIteration ( ) )
				return true;
			conf -> release ( common, false, false );
		} else {
			limited = true;
			PSYSTEMLOG ( Error, "Limit reached: " << common.getCalledIp ( ) );
		}
	}
	if ( limited ) {
		common.setDisconnectCause ( limitedCause );
		conf -> replaceReleaseCompleteCause ( common, false );
	}
	return false;
}

void SignallingThread :: sendReleaseComplete ( PTCPSocket * sock ) {
// Task: to send a releasecomplete message to caller
	if ( ! sock )
		sock = callerSocket;
	if ( sock == callerSocket ) {
		if ( call.rcSentToOrigin )
			return;
		call.rcSentToOrigin = true;
	} else {
		if ( call.rcSentToDest )
			return;
		call.rcSentToDest = true;
	}
	:: sendReleaseComplete ( sock, common, call.id, call.ref, sock == callerSocket );
}

#ifdef NDEBUG
#define TIMEMULT 1
#else
#define TIMEMULT 100
#endif

void SignallingThread :: getSetup ( ) {
	while ( callerSocket -> IsOpen ( ) ) {
		int Selection = PSocket :: Select ( * callerSocket, * callerSocket, 3000 * TIMEMULT );
		PSYSTEMLOG ( Info, "After getSetup.Select" );
		if ( Selection > 0 ) {
			PSYSTEMLOG ( Error, "select: " << PChannel :: GetErrorText (
				static_cast < PChannel :: Errors > ( Selection ) ) );
			break;
		}
		if ( Selection < 0 && receiveMesg ( true ) )
			break;
		callerSocket -> Close ( );
		break;
	}
}

static bool availableFromCaller ( int s ) {
	return s == - 1 || s == - 3;
}

static bool availableFromCalled ( int s ) {
	return s == - 2 || s == - 3;
}

void SignallingThread :: sqlStart ( ) {
	conf -> addActiveCall ( id, this );
}

void SignallingThread :: sqlEnd ( ) {
	ss :: ostringstream os;
	os << "delete from ActiveCalls where id = " << id;
	sqlOut -> add ( os.str ( ) );
	conf -> removeActiveCall ( id );
}

static void checkTimeInterval ( PTime & t, const char * name ) {
	PTime t2;
	long secs = ( t2 - t ).GetSeconds ( );
	if ( secs > 60 )
		PSYSTEMLOG ( Error, name << " taken " << secs << " seconds !!!!!!!!!!!!" );
	t = t2;
}

void SignallingThread :: Main ( ) {
	PSYSTEMLOG ( Info, "SignallingThread::Main()" );
	sqlStart ( );
	bool vi = conf -> isValidInPeerAddress ( common.getCallerIp ( ) );
	PSYSTEMLOG ( Info, "Checking inip done" );
	if ( ! vi ) {
		if ( ! usePrefixAuth ) {
			PSYSTEMLOG ( Error, "invalid InPeerAddress " << common.getCallerIp ( ) );
			sendReleaseComplete ( );
			Close ( );
		}
	} else
		common.source ( ).type = SourceData :: inbound;
	getSetup ( );
	getChoices ( );
	PSYSTEMLOG ( Info, "after getChoices" );
	if ( ! common.getIncomingCodecs ( ).c.empty ( )	&& ( hasSipChoices ( common.getChoices ( ) ) ||
		call.choiceForks.size ( ) > 1 || common.getUseNormalizer ( ) ) ) {
		sqlEnd ( );
		call.common.setConfId ( H323 :: printableCallId ( call.common.getConfId ( ) ) ); // tut on ne printable
		new AnswerCall ( call.choiceForks, call.forkOutAcctns, callerSocket, id, call );
		callerSocket = 0;
		return;
	}
	if ( ! common.getChoices ( ).empty ( ) )
		sendProceeding ( );
	PTime tt;
#ifdef PROC_HACK
	PTime procTime;
#endif
	while ( callerSocket -> IsOpen ( ) && tryNextChoice ( ) ) {
		checkTimeInterval ( tt, "tryNextChoice" );
		try {
			sendSetup ( );
		} catch ( logic_error & e ) {
			PSYSTEMLOG ( Error, "exception: " << e.what ( ) );
			closeThisChoice ( );
			break;
		}
		checkTimeInterval ( tt, "sendSetup" );
#ifdef KEEP_ALIVE
		int idle = 0;
#endif
		int timeoutCnt = 0;
		try {
		while ( callerSocket -> IsOpen ( ) && ( calledSocket -> IsOpen ( ) || call.delayedDisconnect ) ) {
//			PSYSTEMLOG ( Info, "Before Select" );
			int Selection = PSocket :: Select ( * callerSocket,
				* ( call.delayedDisconnect ? callerSocket : calledSocket ), 3000 );
//			PSYSTEMLOG ( Info, "After Select" );
			checkTimeInterval ( tt, "select" );
			if ( conf -> shuttingDown ( ) ) {
				shutDown ( );
				break;
			}
			if ( Selection > 0 ) {
				PSYSTEMLOG ( Error, "select: " << PChannel :: GetErrorText (
					static_cast < PChannel :: Errors > ( Selection ) ) );
				break;
			}
			if ( availableFromCaller ( Selection ) && ! receiveMesg ( true ) ) {
				checkTimeInterval ( tt, "receive from caller" );
				sendReleaseComplete ( );
				callerSocket -> Close ( );
				break;
			} else
				checkTimeInterval ( tt, "receive from caller" );
			if ( ! call.delayedDisconnect && availableFromCalled ( Selection ) && ! receiveMesg ( false ) ) {
				checkTimeInterval ( tt, "receive from called" );
				break;
			} else
				checkTimeInterval ( tt, "receive from caller" );
			if ( h && ( h -> getCallerBreak ( ) || h -> getCalledBreak ( ) ) )
				throw ( logic_error ( "break from h245 thread" ) );
#ifdef KEEP_ALIVE
			if ( Selection )
				idle = 0;
			else {
				idle ++;
				if ( idle > 50 ) {
					sendKeepalive ( );
					idle = 0;
				}
			}
#endif
#ifdef PROC_HACK
			if ( ! call.goodCall && ( PTime ( ) - procTime ).GetSeconds ( ) > 2 ) {
				sendProceeding ( );
				procTime = PTime ( );
			}
#endif
			if ( int t = common.getSigOptions ( ).connectTimeout ( ) )
				if ( ! call.begInited && ( PTime ( ) - call.outSetupTime ).GetSeconds ( ) > t ) {
					PSYSTEMLOG ( Error, "q931 connect timeout" );
					break;
				}
			++ timeoutCnt;
			if ( common.getSigOptions ( ).rtpTimeoutEnable ( ) && call.begInited && h && ! directRTP ( ) &&
				timeoutCnt % ( common.getSigOptions ( ).rtpTimeoutPeriod ( ) / 3 ) == 0 &&
				( call.rtpTimeoutStarted || ( ( PTime ( ) - call.begTime ).GetSeconds ( ) > 30 &&
				( call.rtpTimeoutStarted = true ) ) ) ) {
				int i, o;
				h -> getRtpSession ( ) -> getTimeout ( i, o );
				if ( std :: max ( i, o ) > common.getSigOptions ( ).rtpTimeoutOne ( ) ||
					std :: min ( i, o ) > common.getSigOptions ( ).rtpTimeoutBoth ( ) ) {
					PSYSTEMLOG ( Error, "rtp timeout: " << i << ", " << o << ", "
						<< common.getSigOptions ( ).rtpTimeoutOne ( ) << ", " <<
						common.getSigOptions ( ).rtpTimeoutBoth ( ) );
					common.setDisconnectCause ( Q931 :: cvNormalUnspecified );
					_fakeCallDetector.registerRTPtimeout ( );
					break;
				}
			}
			if ( int t = common.getSigOptions ( ).maxCallDurationMin ( ) )
				if ( timeoutCnt % 20 == 0 && t <= ( PTime ( ) - call.begTime ).GetMinutes ( ) ) {
					PSYSTEMLOG ( Error, "max call duration: " << t << ", " << PTime ( ) << ", "
						<< call.begTime );
					common.setDisconnectCause ( Q931 :: cvNormalUnspecified );
					break;
				}
			if ( call.delayedDisconnect && ( PTime ( ) - call.ddTime ).GetSeconds ( ) >= disconnectDelay ) {
				PSYSTEMLOG ( Info, "delayed disconnect" );
				break;
			}
		}
		} catch ( logic_error & e ) {
			PSYSTEMLOG ( Error, "exception: " << e.what ( ) );
			if ( h && h -> getCallerBreak ( ) ) {
				closeThisChoice ( );
				break;
			}
		}
		closeThisChoice ( );
		checkTimeInterval ( tt, "closeThisChoice" );
	}
	PSYSTEMLOG ( Info, "after while" );
	if ( callerSocket -> IsOpen ( ) ) {
		sendReleaseComplete ( );
		checkTimeInterval ( tt, "sendReleaseComplete" );
	}
	if ( ! call.endInited && call.begInited ) {
		call.endInited = true;
		call.endTime = PTime ( );
	}
	if ( myH245Thread ) {
		myH245Thread -> Close ( );
		if ( ! myH245Thread -> IsTerminated ( ) ) {
			Sleep ( 17000 );
			PSYSTEMLOG ( Info, "before waitforterminaton" );
			while ( ! myH245Thread -> WaitForTermination ( 1 ) ) {
				PSYSTEMLOG ( Error, "myH245Thread did not terminate!" );
				PSYSTEMLOG ( Error, "RUN GDB !!!!!!!!!!!!!!!!!!!" );
				Sleep ( 1000000 );
			}
		}
		safeDel ( myH245Thread );
	}
	conf -> removeCall ( call );
	accountCall ( );
	PSYSTEMLOG ( Info, "RTP Stats: " << rtpStats [ 0 ].count << ", " <<
		rtpStats [ 1 ].count );
	callerSocket -> Close ( );
	safeDel ( callerSocket );
	common.release ( );
	sqlEnd ( );
	checkTimeInterval ( tt, "sqlEnd" );
	PSYSTEMLOG ( Info, "SignallingThread::Main() - end" );
}

static void SetH245Address ( H225 :: H323_UserInformation & uuField, const H225 :: TransportAddress & addr ) {
// Task: given an Q931 message sets the H245 transport address
	H225 :: H323_UU_PDU_h323_message_body & body =
		uuField.m_h323_uu_pdu.m_h323_message_body;
	switch ( body.getTag ( ) ) {
		case H225 :: H323_UU_PDU_h323_message_body :: e_callProceeding: {
			H225 :: CallProceeding_UUIE & callProceeding = body;
			callProceeding.includeOptionalField ( H225 :: CallProceeding_UUIE :: e_h245Address );
			callProceeding.get_h245Address ( ) = addr;
			break;
		} case H225 :: H323_UU_PDU_h323_message_body :: e_connect: {
			H225 :: Connect_UUIE & connect = body;
			connect.includeOptionalField ( H225 :: Connect_UUIE :: e_h245Address );
			connect.get_h245Address ( ) = addr;
			break;
		} case H225 :: H323_UU_PDU_h323_message_body :: e_alerting: {
			H225 :: Alerting_UUIE & alerting = body;
			alerting.includeOptionalField ( H225 :: Alerting_UUIE :: e_h245Address );
			alerting.get_h245Address ( ) = addr;
			break;
		} case H225 :: H323_UU_PDU_h323_message_body :: e_setup: {
			H225 :: Setup_UUIE & setup = body;
			setup.includeOptionalField ( H225 :: Setup_UUIE :: e_h245Address );
			setup.get_h245Address ( ) = addr;
			break;
		} case H225 :: H323_UU_PDU_h323_message_body :: e_facility: {
			H225 :: Facility_UUIE & facility = body;
			facility.includeOptionalField ( H225 :: Facility_UUIE :: e_h245Address );
			facility.get_h245Address ( ) = addr;
			break;
		} case H225 :: H323_UU_PDU_h323_message_body :: e_progress: {
			H225 :: Progress_UUIE & progress = body;
			progress.includeOptionalField ( H225 :: Progress_UUIE :: e_h245Address );
			progress.get_h245Address ( ) = addr;
			break;
		}
	}
}

static bool hasH245Address ( H225 :: H323_UserInformation & uuField,
	H225 :: TransportAddress & addr, bool goodCall ) {
// Task: given an Q931 message, returns true iff it includes an H245 transport address
// and if it does, return it in the Addr parameter
	H225 :: H323_UU_PDU_h323_message_body & body = uuField.m_h323_uu_pdu.m_h323_message_body;
	switch ( body.getTag ( ) ) {
		case H225 :: H323_UU_PDU_h323_message_body :: e_callProceeding: {
			H225 :: CallProceeding_UUIE & callProceeding = body;
			callProceeding.removeOptionalField ( H225 :: CallProceeding_UUIE :: e_h245Address );
			return false;
		} case H225 :: H323_UU_PDU_h323_message_body :: e_connect: {
			H225 :: Connect_UUIE & connect = body;
			if ( connect.hasOptionalField ( H225 :: Connect_UUIE :: e_h245Address ) ) {
				addr = connect.get_h245Address ( );
				return true;
			}
			return false;
		} case H225 :: H323_UU_PDU_h323_message_body :: e_alerting: {
			H225 :: Alerting_UUIE & alerting = body;
			if ( alerting.hasOptionalField ( H225 :: Alerting_UUIE :: e_h245Address ) ) {
				addr = alerting.get_h245Address ( );
				return true;
			}
			return false;
		} case H225 :: H323_UU_PDU_h323_message_body :: e_setup: {
			H225 :: Setup_UUIE & setup = body;
			setup.removeOptionalField ( H225 :: Setup_UUIE :: e_h245Address );
			return false;
		} case H225 :: H323_UU_PDU_h323_message_body :: e_facility: {
			H225 :: Facility_UUIE & facility = body;
			if ( facility.hasOptionalField ( H225 :: Facility_UUIE :: e_h245Address ) ) {
				if ( goodCall ) {
					addr = facility.get_h245Address ( );
					return true;
				}
				facility.removeOptionalField ( H225 :: Facility_UUIE :: e_h245Address );
			}
			return false;
		} case H225 :: H323_UU_PDU_h323_message_body :: e_progress: {
			H225 :: Progress_UUIE & progress = body;
			if ( progress.hasOptionalField ( H225 :: Progress_UUIE :: e_h245Address ) ) {
				if ( goodCall ) {
					addr = progress.get_h245Address ( );
					return true;
				}
				progress.removeOptionalField ( H225 :: Progress_UUIE :: e_h245Address );
			}
			return false;
		}
	}
	return false;
}

static void dropH245Address ( H225 :: H323_UserInformation & uuField ) {
	H225 :: H323_UU_PDU_h323_message_body & body = uuField.m_h323_uu_pdu.m_h323_message_body;
	switch ( body.getTag ( ) ) {
		case H225 :: H323_UU_PDU_h323_message_body :: e_callProceeding: {
			H225 :: CallProceeding_UUIE & callProceeding = body;
			callProceeding.removeOptionalField ( H225 :: CallProceeding_UUIE :: e_h245Address );
			return;
		} case H225 :: H323_UU_PDU_h323_message_body :: e_connect: {
			H225 :: Connect_UUIE & connect = body;
			connect.removeOptionalField ( H225 :: Connect_UUIE :: e_h245Address );
			return;
		} case H225 :: H323_UU_PDU_h323_message_body :: e_alerting: {
			H225 :: Alerting_UUIE & alerting = body;
			alerting.removeOptionalField ( H225 :: Alerting_UUIE :: e_h245Address );
			return;
		} case H225 :: H323_UU_PDU_h323_message_body :: e_setup: {
			H225 :: Setup_UUIE & setup = body;
			setup.removeOptionalField ( H225 :: Setup_UUIE :: e_h245Address );
			return;
		} case H225 :: H323_UU_PDU_h323_message_body :: e_facility: {
			H225 :: Facility_UUIE & facility = body;
			facility.removeOptionalField ( H225 :: Facility_UUIE :: e_h245Address );
			return;
		} case H225 :: H323_UU_PDU_h323_message_body :: e_progress: {
			H225 :: Progress_UUIE & progress = body;
			progress.removeOptionalField ( H225 :: Progress_UUIE :: e_h245Address );
			return;
		}
	}
}

static bool compareIp ( const H225 :: TransportAddress_ipAddress & addr, const PIPSocket :: Address ip ) {
	const ss :: string & t = addr.m_ip;
	if ( t [ 0 ] < ip.Byte1 ( ) )
		return true;
	if ( t [ 0 ] > ip.Byte1 ( ) )
		return false;
	if ( t [ 1 ] < ip.Byte2 ( ) )
		return true;
	if ( t [ 1 ] > ip.Byte2 ( ) )
		return false;
	if ( t [ 2 ] < ip.Byte3 ( ) )
		return true;
	if ( t [ 2 ] > ip.Byte3 ( ) )
		return false;
	return t [ 3 ] < ip.Byte4 ( );
}

bool SignallingThread :: handleH245Setup ( H225 :: H323_UserInformation & uuField, bool fromCaller ) {
// Task: to handle potential setting up of H.245 channel
	H225 :: TransportAddress addr;
	if ( ! hasH245Address ( uuField, addr, call.goodCall ) )
		return true;
	if ( common.getSigOptions ( ).tunnelingWithH245Address ( ) == useTunneling &&
		uuField.m_h323_uu_pdu.hasOptionalField ( H225 :: H323_UU_PDU :: e_h245Tunneling ) &&
		uuField.m_h323_uu_pdu.get_h245Tunneling ( ) ) {
		dropH245Address ( uuField );
		return false;
	}
	if ( addr.getTag ( ) != H225 :: TransportAddress :: e_ipAddress )
		return true;
	if ( common.getFromNat ( ) && fromCaller )
		return false;
	PIPSocket :: Address ip_laddr;
	if ( common.getSigOptions ( ).tunnelingWithH245Address ( ) == useH245Address &&
		uuField.m_h323_uu_pdu.hasOptionalField ( H225 :: H323_UU_PDU :: e_h245Tunneling ) )
		uuField.m_h323_uu_pdu.get_h245Tunneling ( ) = false;
	( fromCaller ? calledSocket : callerSocket ) -> GetLocalAddress ( ip_laddr );
	if ( ! myH245Thread ) {
		// Create the H.245 handler thread....
		if ( ! h )
			h = createH245Handler ( );
		myH245Thread = new H245Thread ( addr, h, call.ref, common.getForceDebug ( ), fromCaller );
	} else {
		PSYSTEMLOG ( Info, "H245Thread already exist" );
		if ( myH245Thread -> getBC ( ) != fromCaller ) {
			if ( compareIp ( addr, ip_laddr ) )
				return false;
			PIPSocket :: Address ha;
			WORD hp;
			AddrUtils :: convertToIPAddress ( addr, ha, hp );
			myH245Thread -> abortListen ( ha, hp );
			return false;
		}
	}
	// Substitute the H.245 address with ours
	SetH245Address ( uuField, AddrUtils :: convertToH225TransportAddr ( ip_laddr,
		myH245Thread ->	getListenerPort ( ) ) );
	return true;
}

void SignallingThread :: startH245 ( H225 :: H323_UserInformation & uuField ) {
// Task: to handle potential setting up of H.245 channel
	PIPSocket :: Address ip_laddr;
	callerSocket -> GetLocalAddress ( ip_laddr );
	if ( ! h )
		h = createH245Handler ( );
	myH245Thread = new H245Thread ( H225 :: TransportAddress ( ), h, call.ref, common.getForceDebug ( ), false, true );
	// Substitute the H.245 address with ours
	SetH245Address ( uuField, AddrUtils :: convertToH225TransportAddr ( ip_laddr,
		myH245Thread ->	getListenerPort ( ) ) );
	calledSocket -> GetLocalAddress ( ip_laddr );
	Q931 facm ( Q931 :: msgFacility, call.ref, false );
	H225 :: H323_UserInformation facuu;
	H225 :: H323_UU_PDU & pdu = facuu.m_h323_uu_pdu;
	H225 :: H323_UU_PDU_h323_message_body & body = pdu.m_h323_message_body;
	body.setTag ( H225 :: H323_UU_PDU_h323_message_body :: e_facility );
	H225 :: Facility_UUIE & fac = body;
	fac.m_reason.setTag ( H225 :: FacilityReason :: e_startH245 );
	fac.get_callIdentifier ( ) = call.id;
	fac.m_protocolIdentifier = h225ProtocolID;
	SetH245Address ( facuu, AddrUtils :: convertToH225TransportAddr ( ip_laddr,
		myH245Thread ->	getListenerPort2 ( ) ) );
	encodeH225IntoQ931 ( facuu, facm );
	sendMesgToDest ( facm, & facuu, calledSocket );
	SetH245Address ( facuu, AddrUtils :: convertToH225TransportAddr ( ip_laddr,
		myH245Thread ->	getListenerPort ( ) ) );
	encodeH225IntoQ931 ( facuu, facm );
	facm.setFromDest ( true );
	sendMesgToDest ( facm, & facuu, callerSocket );
}

bool SignallingThread :: handleProceeding ( Q931 & mesg, H225 :: H323_UserInformation & uuField ) {
// Task: to handle a call proceeding
// Retn: true iff message should be forwarded
	PSYSTEMLOG(Info, "SignallingThread :: handleProceeding");
	H225 :: H323_UU_PDU & pdu = uuField.m_h323_uu_pdu;
	H225 :: H323_UU_PDU_h323_message_body & body = pdu.m_h323_message_body;
	if ( body.getTag ( ) != H225 :: H323_UU_PDU_h323_message_body :: e_callProceeding ) {
		PSYSTEMLOG ( Error, "Bad body tag for CallProceeding: " << body.getTag ( ) );
		return false;
	}
	if ( common.getSigOptions ( ).replaceCallProceedingWithAlerting ( ) ) {
		makeAlertingFromCallProceeding ( mesg, body );
		return handleAlerting ( mesg, uuField );
	}
	handleH245 ( pdu, call.goodCall, false );
	handleProgressIndicator ( mesg );
	H225 :: CallProceeding_UUIE & proc = body;
	handleFastStartResponse ( proc );
	return false;
}

bool SignallingThread :: handleAlerting ( Q931 & mesg, H225 :: H323_UserInformation & uuField ) {
// Task: to handle an alerting message
// Retn: true iff message should be forwarded
	PSYSTEMLOG(Info, "SignallingThread :: handleAlerting");
	if ( ! progress.empty ( ) ) {
		for ( StringVector :: size_type i = 0, j = progress.size ( ); i < j; i ++ )
			mesg.addIE ( Q931 :: ieProgressIndicator, progress [ i ] );
		progress.clear ( );
	}
	call.goodCall = true;
	H225 :: H323_UU_PDU & pdu = uuField.m_h323_uu_pdu;
	H225 :: H323_UU_PDU_h323_message_body & body = pdu.m_h323_message_body;
	if ( body.getTag ( ) != H225 :: H323_UU_PDU_h323_message_body :: e_alerting ) {
		PSYSTEMLOG ( Error, "Bad body tag for Alerting: " << body.getTag ( ) );
		return false;
	}
	H225 :: Alerting_UUIE & alert = body;
	handleMultipleCalls ( alert );
	handleCallId ( alert );
	handleFastStartResponse ( alert );
	if ( common.getFromNat ( ) && ! pdu.hasOptionalField ( H225 :: H323_UU_PDU :: e_h245Control ) &&
		! alert.hasOptionalField ( H225 :: Alerting_UUIE :: e_h245Address ) && ! myH245Thread
		&& ( ! pdu.hasOptionalField ( H225 :: H323_UU_PDU :: e_h245Tunneling ) ||
		pdu.get_h245Tunneling ( ) == false ) )
		startH245 ( uuField );
	else
		handleH245Setup ( uuField, false );
	handleH245 ( pdu, call.goodCall, false );
	dropTokens ( alert );
	if ( call.h245SpoolDest.size ( ) )
		sendTunneling ( );
	call.aSent = true;
	return true;
}

bool SignallingThread :: handleConnect ( Q931 & mesg, H225 :: H323_UserInformation & uuField ) {
// Task: to handle a connect message
// Retn: true iff message should be forwarded
	PSYSTEMLOG(Info, "SignallingThread :: handleConnect");

	if ( ! call.setupInited ) {
		PSYSTEMLOG ( Error, "connect without setup" );
		return false;
	}
	H225 :: H323_UU_PDU & pdu = uuField.m_h323_uu_pdu;
	H225 :: H323_UU_PDU_h323_message_body & body = pdu.m_h323_message_body;
	if ( body.getTag ( ) != H225 :: H323_UU_PDU_h323_message_body :: e_connect ) {
		PSYSTEMLOG ( Error, "Bad body tag for Connect: " << body.getTag ( ) );
		return false;
	}
	call.goodCall = true;
	call.begTime = PTime ( );
	call.begInited = true;
	conf -> addCall ( call, this );
	H225 :: Connect_UUIE & conn = body;
	handleMultipleCalls ( conn );
	handleCallId ( conn );
	handleFastStartResponse ( conn );
	handleH245Setup ( uuField, false );
	handleH245 ( pdu, call.goodCall, false );
	dropTokens ( conn );
	if ( common.getSigOptions ( ).addBearerToConnect ( ) && ! mesg.hasIE ( Q931 :: ieBearerCapability ) ) {
		ss :: string data;
		data.push_back ( '\x80' );
		data.push_back ( '\x90' );
		data.push_back ( '\xa3' );
		mesg.setIE ( Q931 :: ieBearerCapability, data );
	}
	ss :: ostringstream os;
	os << "replace ActiveCalls set id = " << id << ", startTime = from_unixtime( " <<
		PTime ( ).GetTimeInSeconds ( ) << " ), inIP = '" << common.getCallerIp ( ) <<
		"', convertedDigits = '";
	if ( usePrefixAuth && ! isMaster && common.getSource ( ).type == SourceData :: unknown )
		os << "forwarded to master";
	else
		os << common.getConvertedDigits ( );
	ss :: string printableCallId = H323 :: printableCallId ( common.getConfId ( ) );
	os << "', outIP = '" << common.getCalledIp ( ) << "', outName = '" << common.getOutName ( ) <<
		"', outRname = '" << common.getOutRname ( ) <<
		"', name = '" << common.getSource ( ).name << "', rname = '" << common.getSource ( ).rname <<
		"', setupTime = from_unixtime( " << PTime ( ).GetTimeInSeconds ( ) << " ), ref = " << call.ref <<
		", inCallId = '" << printableCallId << "', outCallId = '" << printableCallId << '\'';
	if ( common.getSource ( ).type == SourceData :: card )
		os << ", dialedDigits = '#" << common.getSource ( ).acctn << '#' << common.getRealDigits ( ) << '\'';
	else if ( common.getSource ( ).type == SourceData :: inbound )
		os << ", dialedDigits = '" << common.getDialedDigits ( ) << '\'';
	os << ", callingDigits = '" << common.getCallingDigitsIn ( ) <<
	"', connectTime = from_unixtime( " <<
		call.begTime.GetTimeInSeconds ( ) << " ), priceCache = '";
	ss :: ostringstream ot;
	const CommonCallDetails & common = call.common;
	const OutTryVector & tries = call.tries;
	{
		boost :: archive :: text_oarchive oa ( ot );
		oa << common << tries;
	}
	os << MySQL :: escape ( ot.str ( ) ) << '\'';
	sqlOut -> add ( os.str ( ) );
	return true;
}

bool SignallingThread :: handleRelease ( Q931 & mesg, const H225 :: H323_UserInformation * uuField,
	bool fromCaller, bool & end ) {
	end = true;
	Q931 :: CauseValues cause = getCause ( mesg, uuField );
	PSYSTEMLOG ( Info, "get release complete 0x" << std :: hex << cause << " from " << ( fromCaller ? "caller" : "destination" ) );
	common.setDisconnectCause ( cause );
	if ( ! fromCaller )
		sendReleaseComplete ( calledSocket );
	else
		sendReleaseComplete ( );
	conf -> replaceReleaseCompleteCause ( common, fromCaller );
	PSYSTEMLOG ( Info, "SignallingThread: initialized disconnectCause to " << common.getDisconnectCause ( ) );
	mesg.setCause ( Q931 :: CauseValues ( common.getDisconnectCause ( ) ) );
	if ( ! fromCaller && ! call.goodCall ) {
		conf -> addRelease ( common.curChoice ( ) -> getPeer ( ), common.getRealDigits ( ), cause );
		return false;
	}
	if ( ! fromCaller && disconnectDelay && common.getSigOptions ( ).delayedDisconnect ( ) ) {
		call.ddTime = PTime ( );
		call.delayedDisconnect = true;
		end = false;
		return false;
	}
	if ( ! call.endInited ) {
		call.endTime = PTime ( );
		call.endInited = true;
	}
	if ( ! fromCaller )
		call.rcSentToOrigin = true;
	else
		call.rcSentToDest = true;
	return true;
}

bool SignallingThread :: handleFacility ( Q931 & mesg, H225 :: H323_UserInformation & uuField,
	bool fromCaller ) {
// Task: to handle a facility message
// Retn: true iff message should be forwarded
//	if ( ! MyCall.goodCall )
//		PSYSTEMLOG ( Error, "facility when ! goodCall: " << Mesg << uuField
//			<< "!!!!!!!!!!!!!!!" );
	PSYSTEMLOG(Info, "SignallingThread :: handleFacility");

	handleProgressIndicator ( mesg );
	H225 :: H323_UU_PDU & pdu = uuField.m_h323_uu_pdu;
	_RTDRsendBack = false;
	handleH245 ( pdu, call.goodCall, fromCaller );
	if ( _RTDRsendBack ) {
		H225 :: TransportAddress addr;
		if ( pdu.get_h245Control ( ).empty ( ) && ! hasH245Address ( uuField, addr, call.goodCall ) ) {
			return false;
		}
	}
	H225 :: H323_UU_PDU_h323_message_body & body = pdu.m_h323_message_body;
	switch ( body.getTag ( ) ) {
		case H225 :: H323_UU_PDU_h323_message_body :: e_facility: {
			H225 :: Facility_UUIE & fac = body;
			switch ( fac.m_reason.getTag ( ) ) {
				case H225 :: FacilityReason :: e_startH245:
					if ( sourceIsVoIP )
						return false;
				case H225 :: FacilityReason :: e_undefinedReason:
				case H225 :: FacilityReason :: e_forwardedElements:
				case H225 :: FacilityReason :: e_transportedInformation:
					break;
				default:
					PSYSTEMLOG ( Error, "unknown facility reason: " << mesg <<
						uuField << "!!!!!!!!!!!!!!!!" );
			}
			handleMultipleCalls ( fac );
			handleCallId ( fac );
			handleFastStartResponse ( fac );
			if ( ! handleH245Setup ( uuField, fromCaller ) )
				return false;
			if ( fac.hasOptionalField ( H225 :: Facility_UUIE :: e_alternativeAddress ) )
				PSYSTEMLOG ( Error, "facility has e_alternativeAddress: " <<
					mesg << fac << "!!!!!!!!!!!!!!!!" );
			if ( fac.hasOptionalField ( H225 :: Facility_UUIE :: e_alternativeAliasAddress ) )
				PSYSTEMLOG ( Error, "facility has e_alternativeAliasAddress: " <<
					mesg << fac << "!!!!!!!!!!!!!!!!" );
			dropTokens ( fac );
			if ( common.getSigOptions ( ).emptyFacility ( ) &&
				! fac.hasOptionalField ( H225 :: Facility_UUIE :: e_fastStart ) &&
				! fac.hasOptionalField ( H225 :: Facility_UUIE :: e_h245Address ) ) {
				PSYSTEMLOG ( Info, "emptying facility" );
				body.setTag ( H225 :: H323_UU_PDU_h323_message_body :: e_empty );
			}
		} case H225 :: H323_UU_PDU_h323_message_body :: e_empty:
			return call.goodCall;
		default:
			PSYSTEMLOG ( Error, "Bad body tag for Facility: " << body.getTag ( ) );
			return false;
	}
}

bool SignallingThread :: handleProgress ( Q931 & mesg, H225 :: H323_UserInformation & uuField, bool fromCaller ) {
	if ( ! common.getSigOptions ( ).dropProgressBeforeAlerting ( ) )
		call.goodCall = true;
	if ( call.goodCall )
		return handleProgressReal ( mesg, uuField, fromCaller );
	if ( fromCaller )
		return false;
	call.pMesg = mesg;
	call.pUUField = uuField;
	call.pInited = true;
	return false;
}

void SignallingThread :: handleProgressIndicator ( const Q931 & mesg ) {
	if ( call.goodCall )
		return;
	for ( std :: size_t i = 0, j = mesg.hasIE ( Q931 :: ieProgressIndicator ); i < j; i ++ )
		progress.push_back ( mesg.getIE ( Q931 :: ieProgressIndicator, i ) );
}

bool SignallingThread :: handleProgressReal ( Q931 & mesg, H225 :: H323_UserInformation & uuField,
	bool fromCaller ) {
// Task: to handle a progress message
// Retn: true iff message should be forwarded
	PSYSTEMLOG(Info, "SignallingThread :: handleProgressReal");
	handleProgressIndicator ( mesg );
	H225 :: H323_UU_PDU & pdu = uuField.m_h323_uu_pdu;
	H225 :: H323_UU_PDU_h323_message_body & body = pdu.m_h323_message_body;
	if ( body.getTag ( ) != H225 :: H323_UU_PDU_h323_message_body :: e_progress ) {
		PSYSTEMLOG ( Error, "Bad body tag for Progress: " << body.getTag ( ) );
		return false;
	}
	H225 :: Progress_UUIE & prog = body;
	handleMultipleCalls ( prog );
//	handleCallId ( prog );
	prog.m_callIdentifier = call.id;
	handleFastStartResponse ( prog );
	handleH245Setup ( uuField, fromCaller );
	handleH245 ( pdu, call.goodCall, fromCaller );
	dropTokens ( prog );
	return call.goodCall;
}

bool SignallingThread :: handleMesg ( Q931 & mesg, bool fromCaller ) {
// Task: to handle the given Q931 message
	mesg.removeIE ( Q931 :: ieDisplay );
	bool forwardMesg;
	Pointer < H225 :: H323_UserInformation > uuField;
	if ( mesg.hasIE ( Q931 :: ieUserUser ) ) {
		const ss :: string & s ( mesg.getIE ( Q931 :: ieUserUser ) );
		try {
			Asn :: istream is ( s );
			uuField = new H225 :: H323_UserInformation ( is );
			if ( is.hasException ( ) )
				PSYSTEMLOG ( Error, "decode exception: " << is.getException ( ) );
		} catch ( std :: exception & e ) {
			PSYSTEMLOG ( Error, "decode c++ exception: " << e.what ( ) );
		}
	}

	PTCPSocket * socket = ( fromCaller ? callerSocket : calledSocket );
	PIPSocket :: Address peerAddr;
	WORD peerPort;
	socket -> GetPeerAddress ( peerAddr, peerPort );
	Log -> logQ931Msg ( mesg, uuField, OpengateLog :: Receiving, peerAddr, peerPort, common.getForceDebug ( ) );

	if ( ! uuField ) {
		switch ( mesg.getMessageType ( ) ) {
			case Q931 :: msgStatus:
			case Q931 :: msgStatusEnquiry:
			case Q931 :: msgReleaseComplete:
			case Q931 :: msgNotify:
			case Q931 :: msgSetupAck:
				break;
			default:
				PSYSTEMLOG ( Error, "SignallingThread::handleMesg ( " <<
					mesg << ", " << fromCaller << " ) - ! HasUU" );
				return false;
		}
	}
	bool end = false;
	_fakeCallDetector.registerMesg ( mesg.getMessageType ( ) );
	switch ( mesg.getMessageType ( ) ) {
		case Q931 :: msgSetup:
			if ( ! fromCaller )
				return false;
			forwardMesg = false;
			handleSetup ( mesg, * uuField );
			break;
		case Q931 :: msgCallProceeding:
			if ( fromCaller )
				return false;
			forwardMesg = handleProceeding ( mesg, * uuField );
			break;
		case Q931 :: msgAlerting:
			if ( fromCaller )
				return false;
			forwardMesg = handleAlerting ( mesg, * uuField );
			break;
		case Q931 :: msgConnect:
			if ( fromCaller )
				return false;
			if ( ! call.aSent && call.pInited ) {
				call.pInited = false;
				call.goodCall = true;
				forwardMesg = handleProgressReal ( call.pMesg, call.pUUField, false );
				if ( calledSocket && // Will be created when we receive a setup mesg
					forwardMesg ) {
					if ( common.getSigOptions ( ).dropTunneling ( ) )
						dropTunneling ( call.pUUField );
					if ( common.getSigOptions ( ).dropNonStandard ( ) )
						dropNonStandard ( call.pUUField );
					encodeH225IntoQ931 ( call.pUUField, call.pMesg );
					PTCPSocket * socket = ( fromCaller ? calledSocket : callerSocket );
					sendMesgToDest ( call.pMesg, & call.pUUField, socket );
				}
			}
			forwardMesg = handleConnect ( mesg, * uuField );
			break;
		case Q931 :: msgReleaseComplete:
			forwardMesg = handleRelease ( mesg, uuField, fromCaller, end );
			break;
		case Q931 :: msgFacility:
			forwardMesg = handleFacility ( mesg, * uuField, fromCaller );
			break;
		case Q931 :: msgProgress:
			if ( fromCaller )
				return false;
			forwardMesg = handleProgress ( mesg, * uuField, fromCaller );
			break;
		case Q931 :: msgStatus:
		case Q931 :: msgStatusEnquiry:
		case Q931 :: msgInformation:
		case Q931 :: msgSetupAck:
		case Q931 :: msgNotify:
			forwardMesg = call.goodCall;
			break;
		default:
			PSYSTEMLOG ( Error, "Unknown message type: " << mesg << " !!!!!!!!!!!!!!!!!!!!!\n" );
			if ( ! common.getForceDebug ( ) )
				Log -> logQ931Msg ( mesg, uuField, OpengateLog :: Receiving, PIPSocket :: Address ( ), 0, true );
			forwardMesg = false;
			break;
	}
	if ( calledSocket && // Will be created when we receive a setup mesg
		forwardMesg ) {
		if ( call.goodCall && ! call.pddTime )
			call.pddTime = PTime ( );
		if ( uuField ) {
			if ( common.getSigOptions ( ).dropTunneling ( ) )
				dropTunneling ( * uuField );
			if ( common.sigOptions ( ).dropNonStandard ( ) )
				dropNonStandard ( * uuField );
			encodeH225IntoQ931 ( * uuField, mesg );
		}
		PTCPSocket * socket = ( fromCaller ? calledSocket : callerSocket );
		sendMesgToDest ( mesg, uuField, socket );
	}
	return ! end;
}

void SignallingThread :: getInPrice ( const H225 :: H323_UU_PDU & pdu ) {
	:: getInPrice ( pdu, common );
}

bool SignallingThread :: handleSetup ( const Q931 & mesg, const H225 :: H323_UserInformation & uuField ) {
// Task: to handle a setup message
// Retn: true iff message should be forwarded
	const H225 :: H323_UU_PDU & pdu = uuField.m_h323_uu_pdu;
	const H225 :: H323_UU_PDU_h323_message_body & body = pdu.m_h323_message_body;
	if ( body.getTag ( ) != H225 :: H323_UU_PDU_h323_message_body :: e_setup )
		return false;
	if ( call.setupInited )
		return false;
	call.setupTime = PTime ( );
	call.setupInited = true;
	call.setup = mesg;
	call.uuField = uuField;
	const H225 :: Setup_UUIE & setup = body;
	const H225 :: EndpointType & sourceInfo = setup.m_sourceInfo;
	if ( sourceInfo.hasOptionalField ( H225 :: EndpointType :: e_vendor ) ) {
		const H225 :: VendorIdentifier & vendorId = sourceInfo.get_vendor ( );
		if ( vendorId.hasOptionalField ( H225 :: VendorIdentifier :: e_productId ) ) {
			if ( static_cast < const ss :: string & > ( vendorId.get_productId ( ) ) == "VoIP" )
				sourceIsVoIP = true;
		}
	}
	if ( sourceIsVoIP )
		PSYSTEMLOG ( Info, "source is VoIP !!!!!!!!!!" );
	if ( setup.hasOptionalField ( H225 :: Setup_UUIE :: e_fastStart ) )
		call.fastStart = setup.get_fastStart ( );
	if ( setup.m_protocolIdentifier.str ( ) == "0.0.8.2250.0.2" )
		call.originatorVersion2 = true;
	return true;
}

bool SignallingThread :: getCallDetailsForSetup ( ) {
// Task: to obtain a call details and destination endpoint relative to the SetupMesg.
// Search the CallDetails object in the calls table and if not found try to create
// a new call item.
	call.ref = call.setup.getCallReference ( );
	SetThreadName ( "Q931 " + PString ( call.ref ) );
	const H225 :: H323_UU_PDU & pdu = call.uuField.m_h323_uu_pdu;
	const H225 :: H323_UU_PDU_h323_message_body & body = pdu.m_h323_message_body;
	const H225 :: Setup_UUIE & setup = body;
	if ( ! getCallID ( setup, call.id ) ) {
		PSYSTEMLOG ( Error, "Setup hasnt CallID" );
		call.id.m_guid = H323 :: globallyUniqueId ( );
	}
	common.setDialedDigits ( call.setup.getCalledPartyNumber ( ) );
	if ( common.getDialedDigits ( ).empty ( ) ) {
		PSYSTEMLOG ( Error, "Setup hasnt destinationAddress" );
		return false;
	}
//	MyCall.dialedDigits.Replace ( "A", "#", true );
	common.setCallingDigitsIn ( call.setup.getCallingPartyNumber ( ) );
	getInPrice ( pdu );
	common.setConfId ( setup.m_conferenceID );
	getIncomingCodecsFromSetup ( pdu, common.incomingCodecs ( ), call.fsAddr, call.fsPort );
	if ( ! conf -> getCallInfo ( call.choiceForks, call.forkOutAcctns, common, true, true, true ) ) {
		return false;
	}
	return true;
}

H225 :: ArrayOf_Asn_OctetString SignallingThread :: handleFastStart ( ) {
	PSYSTEMLOG(Info, "SignallingThread :: handleFastStart");
	if ( ! h )
		h = createH245Handler ( );
	return h -> handleFastStartNew ( call.fastStart );
}

static void append ( H225 :: ArrayOf_Asn_OctetString & to, const H225 :: ArrayOf_Asn_OctetString & from ) {
	std :: size_t ts = to.size ( ), fs = from.size ( );
	to.setSize ( ts + fs );
	for ( std :: size_t i = 0; i < fs; i ++ )
		to [ i + ts ] = from [ i ];
}

void SignallingThread :: handleH245 ( H225 :: H323_UU_PDU & pdu, bool goodCall, bool fromCaller ) {
	if ( common.getSigOptions ( ).dropTunneling ( ) )
		return;
	if ( ! goodCall ) {
		if ( pdu.hasOptionalField ( H225 :: H323_UU_PDU :: e_h245Control ) ) {
			if ( ! fromCaller )
				append ( call.h245Spool, pdu.get_h245Control ( ) );
			else
				append ( call.h245SpoolDest, pdu.get_h245Control ( ) );
			pdu.removeOptionalField ( H225 :: H323_UU_PDU :: e_h245Control );
		}
	} else {
		if ( ! fromCaller && call.h245Spool.size ( ) ) {
			pdu.includeOptionalField ( H225 :: H323_UU_PDU :: e_h245Control );
			appendBack ( pdu.get_h245Control ( ), call.h245Spool );
			call.h245Spool.clear ( );
		} else if ( fromCaller && call.h245SpoolDest.size ( ) ) {
			pdu.includeOptionalField ( H225 :: H323_UU_PDU :: e_h245Control );
			appendBack ( pdu.get_h245Control ( ), call.h245SpoolDest );
			call.h245SpoolDest.clear ( );
		}
		if ( pdu.hasOptionalField ( H225 :: H323_UU_PDU :: e_h245Control ) )
			handleH245 ( pdu.get_h245Control ( ), fromCaller );
	}
}

void SignallingThread :: handleH245 ( H225 :: ArrayOf_Asn_OctetString & control,
	bool fromCaller ) {
	for ( std :: size_t i = control.size ( ); i --; ) {
		if ( ! h )
			h = createH245Handler ( );
		try {
			Asn :: istream is ( control [ i ] );
			H245 :: MultimediaSystemControlMessage h245Mesg ( is );
			if ( is.hasException ( ) ) {
				PSYSTEMLOG ( Error, "tunneled h245: can't decode" << control [ i ] << ": " << is.getException ( ) );
				return;
			}
			bool changeTarget = false;
			if ( ! h -> handleMesg ( h245Mesg, fromCaller, changeTarget ) ) {
				control.erase ( i );
				continue;
			}
			if ( changeTarget ) { //need to send back
				Asn :: ostream os;
				h245Mesg.encode ( os );
				Q931 facility ( Q931 :: msgFacility, call.ref, true );
				H225 :: H323_UserInformation uuField;
				H225 :: H323_UU_PDU & pdu = uuField.m_h323_uu_pdu;
				pdu.m_h323_message_body.setTag ( H225 :: H323_UU_PDU_h323_message_body :: e_empty );
				pdu.includeOptionalField ( H225 :: H323_UU_PDU :: e_h245Tunneling );
				pdu.get_h245Tunneling ( ) = true;
				pdu.includeOptionalField ( H225 :: H323_UU_PDU :: e_h245Control );
				H225 :: ArrayOf_Asn_OctetString & reply = pdu.get_h245Control ( );
				reply.push_back ( os.str ( ) );
				encodeH225IntoQ931 ( uuField, facility );
				facility.setFromDest ( fromCaller ? true : false );
				sendMesgToDest ( facility, & uuField, fromCaller ? callerSocket : calledSocket );
				control.erase ( i );
				_RTDRsendBack = true;
				continue;
			}
			Asn :: ostream os;
			h245Mesg.encode ( os );
			control [ i ] = os.str ( );
		} catch ( std :: exception & e ) {
			PSYSTEMLOG ( Error, "tunneled h245: can't decode control [ " << i << " ] : " << e.what ( ) );
			return;
		}
	}
}

template < class T > void SignallingThread :: handleFastStartResponse ( T & uuie ) {
	PSYSTEMLOG ( Info, "handleFastStartResponse0" );
	if ( ! call.goodCall ) {
		if ( uuie.hasOptionalField ( T :: e_fastStart ) ) {
			call.fastStartSpool = uuie.get_fastStart ( );
			uuie.removeOptionalField ( T :: e_fastStart );
		}
		return;
	}
	if ( call.fastStartSpool.size ( ) ) {
		uuie.includeOptionalField ( T :: e_fastStart );
		uuie.get_fastStart ( ) = call.fastStartSpool;
		call.fastStartSpool.clear ( );
	}
	if ( ! uuie.hasOptionalField ( T :: e_fastStart ) )
		return;
	if ( /*directRTP ( ) ||*/ ( h && h -> handleFastStartResponseNew ( uuie.get_fastStart ( ) ) ) )
		return;
	uuie.removeOptionalField ( T :: e_fastStart );
}

H245Handler * SignallingThread :: createH245Handler ( ) {
	H245Handler * h = new H245Handler ( differentCodec, common, rtpStats, directRTP ( ) );
	PIPSocket :: Address addr;
	if ( callerSocket ) {
		if ( callerSocket -> GetLocalAddress ( addr ) )
			h -> setFrom ( addr );
		else
			PSYSTEMLOG ( Error, "Cant't get local address from callerSocket: " << callerSocket -> GetErrorText ( ) );
	} else
		PSYSTEMLOG ( Error, "Cant't get local address from callerSocket: socket is null" );
	if ( calledSocket ) {
		if ( calledSocket -> GetLocalAddress ( addr ) )
			h -> setTo ( addr, common.curChoice ( ) -> getFromNat ( ) );
		else
			PSYSTEMLOG ( Error, "Cant't get local address from calledSocket: " << calledSocket -> GetErrorText ( ) );
	} else
		PSYSTEMLOG ( Error, "Cant't get local address from calledSocket: socket is null" );
/*	if ( ! directRTP ( ) && MyCall.realDigits == "70955555555" )
		h -> getRtpSession ( ) -> enableLog ( );*/
	return h;
}

bool SignallingThread :: directRTP ( ) const {
	return common.getDirectIn ( ) || directOut;
}

template < class T > void SignallingThread :: handleCallId ( T & uuie ) {
	:: handleCallId ( uuie, call.id );
}

int SignallingThread :: getCallSeconds ( ) const {
	return conf -> getRounded ( double ( ( call.endTime - call.begTime ).GetMilliSeconds ( ) ) / 1000.0 );
}

bool SignallingThread :: getCallConnected ( ) const {
	return call.begInited;
}

int SignallingThread :: getCallRef ( ) const {
	return call.ref;
}

ss :: string SignallingThread :: getInIp ( ) const {
	return common.getCallerIp ( );
}

int SignallingThread :: getInPeerId ( ) const {
	return common.getSource ( ).peer;
}

ss :: string SignallingThread :: getInPeerName ( ) const {
	return common.getSource ( ).name;
}

ss :: string SignallingThread :: getInResponsibleName ( ) const {
	return common.getSource ( ).rname;
}

ss :: string SignallingThread :: getSetupTime ( ) const {
	return static_cast < const char * > ( call.setupTime.AsString ( ) );
}

ss :: string SignallingThread :: getConnectTime ( ) const {
	if ( ! call.begInited )
		return "-";
	return static_cast < const char * > ( call.begTime.AsString ( ) );
}

ss :: string SignallingThread :: getOutIp ( ) const {
	const OutChoiceDetails * c = common.curChoice ( );
	if ( ! c )
		return "";
	return c -> getIp ( );
}

int SignallingThread :: getOutPeerId ( ) const {
	const OutChoiceDetails * c = common.curChoice ( );
	if ( ! c )
		return 0;
	return c -> getPeer ( );
}

ss :: string SignallingThread :: getCallId ( ) const {
	const ss :: string & confId = common.getConfId ( );
	return toHex ( confId );
}

ss :: string SignallingThread :: getOutPeerName ( ) const {
	return common.getOutName ( );
}

ss :: string SignallingThread :: getOutResponsibleName ( ) const {
	return common.getOutRname ( );
}

ss :: string SignallingThread :: getCallingDigits ( ) const {
	if ( common.getPeerIndex ( ) == -1 )
		return "-";
	else
		return common.getCallingDigits ( );
}

ss :: string SignallingThread :: getSentDigits ( ) const {
	if ( common.getPeerIndex ( ) == -1 )
		return "-";
	else
		return common.getSentDigits ( );
}

ss :: string SignallingThread :: getDialedDigits ( ) const {
	if ( common.getPeerIndex ( ) == -1 )
		return "-";
	else
		return common.getDialedDigits ( );
}

ss :: string SignallingThread :: getInCode ( ) const {
	return common.getSource ( ).code;
}

ss :: string SignallingThread :: getOutCode ( ) const {
	const OutChoiceDetails * c = common.curChoice ( );
	if ( ! c )
		return "";
	return c -> getCode ( ).substr ( c -> getDepth ( ) );
}

ss :: string SignallingThread :: getInAcctn ( ) const {
	return common.getSource ( ).acctn;
}

ss :: string SignallingThread :: getOutAcctn ( ) const {
	return common.getSource ( ).outAcctn;
}

ss :: string SignallingThread :: getPdd ( ) const {
	PTime t = call.pddTime ? call.pddTime.get ( ) : PTime ( );
	ss :: ostringstream os;
	os << t - call.setupTime;
	return os.str ( );
}

bool SignallingThread :: getDifferentProtocol ( ) const {
	return false;
}

bool SignallingThread :: getDifferentCodec ( ) const {
	return differentCodec;
}

