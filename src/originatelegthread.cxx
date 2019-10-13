#pragma implementation
#pragma implementation "printhex.hpp"
#include "ss.hpp"
#include "allocatable.hpp"
#include "pointer.hpp"
#include <boost/archive/text_oarchive.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/sequenced_index.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/member.hpp>
#include "codecinfo.hpp"
#include "signallingoptions.hpp"
#include "tarifround.hpp"
#include "tarifinfo.hpp"
#include "pricedata.hpp"
#include "outcarddetails.hpp"
#include "outchoicedetails.hpp"
#include "sourcedata.hpp"
#include "aftertask.hpp"
#include <ptlib.h>
#include "moneyspool.hpp"
#include <tr1/functional>
#include "smartguard.hpp"
#include <tr1/memory>
#include "commoncalldetails.hpp"
#include <ptlib/sockets.h>
#include <stdexcept>
#include <cstring>
#include "asn.hpp"
#include "q931.hpp"
#include "callcontrol.hpp"
#include "legthread.hpp"
#include <queue>
#include "originatelegthread.hpp"
#include "ixcudpsocket.hpp"
#include <boost/multi_index/mem_fun.hpp>
#include "sip2.hpp"
#include "siporiginatehandler.hpp"
#include "sipleg2.hpp"
#include "h225.hpp"
#include "latencylimits.hpp"
#include "fakecalldetector.hpp"
#include "h323leg.hpp"
#include "rtpstat.hpp"
#include "session.hpp"
#include "ipport.hpp"
#include "icqcontact.hpp"
#include "radgwinfo.hpp"
#include "Conf.hpp"
#include "rtpsession.hpp"
#include "h323call.hpp"
#include "radiuscommon.hpp"
#include "sipcommon.hpp"
#include "h323.hpp"
#include "sqloutthread.hpp"
#include "printhex.hpp"
#include "ssconf.hpp"
#include <ptlib/svcproc.h>
#include "automutex.hpp"
#include "mysql.hpp"
#include <boost/serialization/map.hpp>
#include <boost/serialization/vector.hpp>
#include "serializestring.hpp"
#include <boost/range.hpp>
#include "slprint.hpp"
#include "mgcpepthreadhandler.hpp"
#include "mgcpleg.hpp"
#include "dtmfrelay.hpp"

std :: ostream & operator<< ( std :: ostream & os, const RecodeInfo & r ) {
	return os << '(' << r.codec << ' ' << r.backCodec << ')';
}

void OriginateLegThread :: checkPeerMessages ( ) {
	PeerMessage f;
	while ( true ) {
		{
			AutoMutex am ( qm );
			if ( peerQueue.empty ( ) )
				return;
			f = peerQueue.front ( );
			peerQueue.pop ( );
		}
		f ( );
	}
}

void OriginateLegThread :: putMessage ( const PeerMessage & m ) {
	AutoMutex am ( qm );
	peerQueue.push ( m );
	if ( leg )
		leg -> wakeUp ( );
}

OriginateLegThread :: OriginateLegThread ( H323Call * c, LegThread * * p, unsigned _id, const CodecInfoVector & ic,
	const CodecInfoVector & oc ) : LegThread ( c, p, _id ), ref ( 0 ), maxForwards ( 70 ), inCodecs ( ic ),
	outCodecs ( oc ), end ( false ), goodCall ( false ), rtpTimeoutStarted ( false ), endInited ( false ),
	limitTries ( - 1 ), isSendAddressSet ( false ), gwTaken ( false ), differentCodec ( false ) { }

OriginateLegThread :: OriginateLegThread ( H323Call * c, LegThread * * p, unsigned _id, const CommonCallDetails & d,
	const CodecInfoVector & ic, const CodecInfoVector & oc, ProtoType origType, unsigned maxForwards, const ss :: string & remotePartyID,
	const ss :: string & pAssertID, const ss :: string& privacy ) :
	LegThread ( c, p, _id ), incomingCallId ( d.getConfId ( ) ), common ( d ), ref ( 0 ), maxForwards ( maxForwards ),
	inCodecs ( ic ), outCodecs ( oc ), end ( false ), goodCall ( false ), rtpTimeoutStarted ( false ), endInited ( false ),
	m_remotePartyID ( remotePartyID ), m_PAssertID ( pAssertID ), m_Privacy ( privacy ), limitTries ( origType ),
	isSendAddressSet ( false ), gwTaken ( false ), differentCodec ( false ) { }

OriginateLegThread :: ~OriginateLegThread ( ) { }

void OriginateLegThread :: Close ( int cause ) {
	putMessage ( std :: tr1 :: bind ( & OriginateLegThread :: closeLocal, this, cause ) );
}

void OriginateLegThread :: closeLocal ( int cause ) {
	if ( common.getDisconnectCause ( ) == 0 && 0 != cause )
		common.setDisconnectCause ( Q931 :: CauseValues(cause) );
	shutDownLocal ( );
}

void OriginateLegThread :: shutDownLocal ( ) {
	if ( begInited && ! endInited )
		endTime = PTime ( );
	end = true;
}

void OriginateLegThread :: peerOnHold ( int level ) {
	putMessage ( std :: tr1 :: bind ( & OriginateLegThread :: peerOnHoldLocal, this, level ) );
}

void OriginateLegThread :: peerOnHoldLocal ( int level ) {
	leg -> peerOnHold ( level );
}

void OriginateLegThread :: peerOnHoldOK ( ) {
	putMessage ( std :: tr1 :: bind ( & OriginateLegThread :: peerOnHoldOKLocal, this ) );
}

void OriginateLegThread :: peerOnHoldOKLocal ( ) {
	leg -> peerOnHoldOK ( );
}

void OriginateLegThread :: peerDtmf ( const DTMF :: Relay & r ) {
	putMessage ( std :: tr1 :: bind ( & OriginateLegThread :: peerDtmfLocal, this, r ) );
}

void OriginateLegThread :: peerDtmfLocal ( const DTMF :: Relay & r ) {
	leg -> peerDtmf ( r );
}

static int getNewCallRef ( ) {
	static int r;
	while ( true ) {
		int o = r;
		int n = o + 1;
		if ( n > 0x7fff )
			n = 1;
		if ( __sync_bool_compare_and_swap ( & r, o, n ) )
			return n;
	}
}

static void makeRecodes ( const CodecInfoVector & offeredCodecs, const CodecInfoSet & allowedCodecs,
	bool allCodecs, const StringStringSetMap & recodesMap, RecodeInfoVector & recodes ) {
	RecodeInfoVector v;
	for ( CodecInfoContainer :: const_iterator i = offeredCodecs.c.begin ( ); i != offeredCodecs.c.end ( ); ++ i ) {
		const CodecInfo & c = * i;
		CodecInfoSet :: const_iterator a = allowedCodecs.find ( c );
		if ( allCodecs || a != allowedCodecs.end ( ) ) {
			int frames = 0;
			if ( a != allowedCodecs.end ( ) )
				frames = a -> getFrames ( );
			if ( frames != c.getFrames ( ) ) {
				CodecInfo t ( c );
				t.setFrames ( frames );
				v.push_back ( RecodeInfo ( t ) );
			} else
				v.push_back ( RecodeInfo ( c ) );
		}
	}
	for ( CodecInfoContainer :: const_iterator i = offeredCodecs.c.begin ( ); i != offeredCodecs.c.end ( ); ++ i ) {
		const CodecInfo & c = * i;
		StringStringSetMap :: const_iterator j = recodesMap.find ( c.getCodec ( ) );
		if ( j == recodesMap.end ( ) )
			continue;
		for ( StringSet :: const_iterator k = j -> second.begin ( ); k != j -> second.end ( ); ++ k ) {
			CodecInfoSet :: const_iterator r = allowedCodecs.find ( * k );
			if ( r == allowedCodecs.end ( ) || ! r -> getCanRecode ( ) || v.get < Codec > ( ).count ( * r ) ||
				! CodecInfo :: isTrivialRecoder ( c.getCodec ( ), r -> getCodec ( ) ) )
				continue;
			CodecInfo t ( * r );
			if ( ! t.getFrames ( ) )
				t.setFrames ( c.getFrames ( ) );
			v.push_back ( RecodeInfo ( t, c ) );
		}
	}
	for ( CodecInfoContainer :: const_iterator i = offeredCodecs.c.begin ( ); i != offeredCodecs.c.end ( ); ++ i ) {
		const CodecInfo & c = * i;
		StringStringSetMap :: const_iterator j = recodesMap.find ( c.getCodec ( ) );
		if ( j == recodesMap.end ( ) )
			continue;
		for ( StringSet :: const_iterator k = j -> second.begin ( ); k != j -> second.end ( ); ++ k ) {
			CodecInfoSet :: const_iterator r = allowedCodecs.find ( * k );
			if ( r == allowedCodecs.end ( ) || ! r -> getCanRecode ( ) || v.get < Codec > ( ).count ( * r ) )
				continue;
			CodecInfo t ( * r );
			if ( ! t.getFrames ( ) )
				t.setFrames ( c.getFrames ( ) );
			v.push_back ( RecodeInfo ( t, c ) );
		}
	}
	std :: swap ( recodes, v );
	PSYSTEMLOG(Info, "3OriginateLegThread :: makeRecodes: recodes.size() = " << recodes.size() );
}

bool OriginateLegThread :: tryNextChoice ( ) {
	if ( goodCall )
		return false;
	if ( conf -> shuttingDown ( ) ) {
		shutDownLocal ( );
		return false;
	}
	bool limited = false;
	int limitedCause = conf -> getDefaultDisconnectCause();/* Q931 :: cvNoCircuitChannelAvailable;*/
	while ( common.hasNextPeerIndex ( ) ) {
		call -> newChoice ( this );
		common.nextPeerIndex ( );
		if ( disableSIP && common.curChoice ( ) -> getType ( ) == ptSip )
			continue;
		PSYSTEMLOG ( Info, "Trying destination " << common.getCalledIp ( ) << ':' << common.getCalledPort ( )
			<< ':' << common.getSentDigits ( ) );
		bool takeGw = limitTries != - 1 && common.curChoice ( ) -> getType ( ) != limitTries;
		if ( conf -> take ( common, limitedCause, takeGw ? & gwTaken : 0 ) ) {
			limited = false;
			limitedCause = conf -> getDefaultDisconnectCause(); /*Q931 :: cvNoCircuitChannelAvailable;*/
			ref = getNewCallRef ( );
			RecodeInfoVector inRecodes;
			makeRecodes ( inCodecs, common.getOutAllowedCodecs ( ), common.getOutAllCodecs ( ),
				conf -> getFullRecodes ( ), inRecodes );
			if ( inRecodes.empty ( ) ) {
				PSYSTEMLOG ( Error, "tryNextChoice failed: inRecodes is empty." );
				ss :: for_each ( inCodecs.c, SLPrint ( "inCodecs: " ) );
				ss :: for_each ( common.getOutAllowedCodecs ( ), SLPrint ( "outAllowedCodecs: " ) );
				conf -> release ( common, false, gwTaken );
				continue;
			}
			ss :: for_each ( inRecodes, SLPrint ( "recode: " ) );
			switch ( common.curChoice ( ) -> getType ( ) ) {
				case ptMgcp:
					leg = new MgcpLeg ( this, common, inRecodes, common.curChoice ( ) -> getMgcpGw ( ),
						common.curChoice ( ) -> getMgcpEp ( ) );
					break;
				case ptH323:
					leg = new H323Leg ( this, common, inRecodes, inRecodes, ref );
					break;
				default:
//					leg = new SipLeg ( this, common, inRecodes, & qm, ref, isRecordRouteRequired () );
					leg = new SipLeg2 ( this, common, inRecodes, maxForwards, m_remotePartyID, m_PAssertID, m_Privacy );
			}
			call -> setDirect ( this, common.curChoice ( ) -> getDirectRTP ( ) );
			try {
				if ( leg -> tryChoice ( ) ) {
					PSYSTEMLOG(Info, "OriginateLegThread :: tryNextChoice ( ). leg -> tryChoice ( ) is good.");
					return true;
				}
			} catch ( std :: exception & e ) {
				PSYSTEMLOG ( Error, "exception: " << e.what ( ) );
			}
			conf -> release ( common, false, gwTaken );
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

void OriginateLegThread :: closeThisChoice ( ) {
	differentCodec = false;
	leg -> closeChoice ( );
	conf -> release ( common, true, gwTaken );
	ss :: string t;
	{
		AutoMutex am ( lm );
		t = printableCallId;
	}
	tries.push_back ( OutTry ( common.getPeerIndex ( ), common.getDisconnectCause ( ), t ) );
}

bool OriginateLegThread :: admissibleIP ( PIPSocket :: Address & ip ) {
	PSYSTEMLOG ( Info, "rtp ip check: " << ip );
	return conf -> validOutRtp ( common.curChoice ( ) -> getPeer ( ), static_cast < const char * > ( ip.AsString ( ) ) );
}

void OriginateLegThread :: calledSocketConnected ( const PIPSocket :: Address & local ) {
	sqlSetPeer ( );
	call -> setPeer ( this, local, common.curChoice ( ) -> getFromNat ( ) );
}

void OriginateLegThread :: Main ( ) {
	if ( ! init ( ) )
		return;
	sqlStart ( );
	PSYSTEMLOG( Info, "OriginateLegThread :: Main. this = " << this);
	while ( ! end && tryNextChoice ( ) ) {
		setupTime = PTime ( );
		choiceAnswered = false;
		if ( ! leg -> initChoice ( ) ) {
			PSYSTEMLOG ( Error, "initChoice failed" );
			closeThisChoice ( );
			continue;
		}
		PSYSTEMLOG(Info, "OriginateLegThread :: Main ( ). leg -> tryChoice ( ) is good.");

		int timeoutCnt = 0;
		while ( ! end && ! leg -> ended ( ) ) {
			if ( conf -> shuttingDown ( ) ) {
				shutDownLocal ( );
				break;
			}
			if ( ! leg -> iteration ( ) ) {
				PSYSTEMLOG ( Info, "iteration failed" );
				checkPeerMessages ( ); // na sluchay chto tam legit cancel on first leg
				break;
			}
			checkPeerMessages ( ); // race v slichae sip - wakeup moget priyti posle etogo no do next iteration
			if ( ! choiceAnswered && ( PTime ( ) - setupTime ).GetSeconds ( ) > 5 ) {
				PSYSTEMLOG ( Error, "no answer in 5 seconds" );
				break;
			}
			if ( int t = common.getSigOptions ( ).connectTimeout ( ) )
				if ( ! begInited && ( PTime ( ) - setupTime ).GetSeconds ( ) > t ) {
					PSYSTEMLOG ( Error, "leg connect timeout" );
					break;
				}
			if ( int t = common.getSigOptions ( ).maxCallDurationMin ( ) )
				if ( begInited && t <= ( PTime ( ) - begTime ).GetMinutes ( ) ) {
					PSYSTEMLOG ( Error, "max call duration: " << t << ", " << PTime ( ) << ", "
						<< begTime );
					closeLocal ( Q931 :: cvNormalUnspecified );
					break;
				}
			timeoutCnt ++;
			if ( common.getSigOptions ( ).rtpTimeoutEnable ( ) && begInited &&
				!( common.curChoice ( ) -> getDirectRTP ( ) || common.getDirectIn ( ))&&
				timeoutCnt % ( common.getSigOptions ( ).rtpTimeoutPeriod ( ) / 3 ) == 0 &&
				( PTime ( ) - begTime ).GetSeconds ( ) > 30 ) {
				int i, o;
				call -> getTimeout ( i, o );
				if ( std :: max ( i, o ) > common.getSigOptions ( ).rtpTimeoutOne ( ) ||
					std :: min ( i, o ) > common.getSigOptions ( ).rtpTimeoutBoth ( ) ) {
					PSYSTEMLOG ( Error, "rtp timeout: " << i << ", " << o << ", "
						<< common.getSigOptions ( ).rtpTimeoutOne ( ) << ", " <<
						common.getSigOptions ( ).rtpTimeoutBoth ( ) );
					closeLocal ( Q931 :: cvNormalUnspecified );
					leg -> rtpTimeoutDetected ( );
					break;
				}
			}

		}
		PSYSTEMLOG ( Info, "closing choice: " << end << ", " << leg -> ended ( ) );
		closeThisChoice ( );
	}
	//if ( begInited )
		conf -> removeCall ( common.source ( ), eaters );
	accountCall ( );
	sqlEnd ( );
	call -> stop ( this, common.getDisconnectCause ( ) );
}

void OriginateLegThread :: connected ( ) {
	if ( isSendAddressSet ) {
		if ( ! pddTime )
			pddTime = PTime ( );
		call -> connected ( this );
	}
	if ( begInited )
		return;
	goodCall = true;
	begTime = PTime ( );
	begInited = true;
	conf -> addCall ( common, eaters, this );
	sqlConnect ( );
}

void OriginateLegThread :: released ( Q931 :: CauseValues cause ) {
	if ( begInited && ! endInited ) {
		endTime = PTime ( );
		endInited = true;
	}
	PSYSTEMLOG ( Info, "get release complete 0x" << std :: hex << cause );
	common.setDisconnectCause ( cause );
	conf -> replaceReleaseCompleteCause ( common, false );
	PSYSTEMLOG ( Info, "OriginateLegThread: initialized disconnectCause to " << common.getDisconnectCause ( ) );
	if ( ! endInited || conf -> getRounded ( double ( ( endTime - begTime ).GetMilliSeconds ( ) ) / 1000.0 ) == 0 )
		conf -> addRelease ( common.curChoice ( ) -> getPeer ( ), common.getRealDigits ( ), cause );
	//conf -> removeCall ( common.getSource ( ), eaters );
}

void OriginateLegThread :: alerted ( ) {
	if ( ! pddTime )
		pddTime = PTime ( );
	call -> alerted ( this );
	goodCall = true;
}

void OriginateLegThread :: answered ( ) {
	choiceAnswered = true;
}

void OriginateLegThread :: onHold ( int level ) {
	call -> onHold ( this, level );
}

void OriginateLegThread :: sendDtmf ( const DTMF :: Relay & r ) {
	call -> sendDtmf ( this, r );
}

void OriginateLegThread :: accountCall ( ) {
	bool fullAccount = call -> fullAccount ( this );
	int skippedTries = 1;
	if ( ! fullAccount )
		skippedTries = 0;
	ss :: ostringstream os;
	os << "insert into radacctSS values ";
	const SourceData & source = common.getSource ( );
	int ip = source.peer;
	std :: time_t timeInSecs = begTime.GetTimeInSeconds ( );
	if ( fullAccount ) {
		if ( tries.empty ( ) )
			common.setPeerIndex ( - 1 );
		else
			common.setPeerIndex ( tries.back ( ).choiceIndex );
		struct TmpStat {
			int frac, jitterMax, jitterAvg;
		};
		TmpStat st [ 4 ];
		std :: memset ( st, 0, sizeof ( st ) );
		const RTPStat * rtpStats = call -> getStats ( this );
		for ( int i = 0; i < 4; i ++ )
			if ( int cnt = rtpStats [ i ].count ) {
				st [ i ].frac = rtpStats [ i ].fracLostSum / cnt;
				st [ i ].jitterMax = rtpStats [ i ].jitterMax;
				st [ i ].jitterAvg = rtpStats [ i ].jitterSum / cnt;
			}
		int secs = conf -> getRounded ( double ( ( endTime - begTime ).GetMilliSeconds ( ) ) / 1000.0 );
		int secs2 = secs;
		os << "( 0, '" << common.getCallingDigitsIn ( ) << "', '" << getCallingDigits ( ) << "', '";
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
			os << "card " << source.acctn << ' ' << inEuPrice * source.valuteRate << ' ' << secs <<
				' ' << ( inEuSecs ? inEuConnectPrice * source.valuteRate : 0.0 ) << ' ' <<
				inEuCode << ' ' << common.getRealDigits ( ) << ' ' << inEuSecs / 60.0;
		else
			os << common.getDialedDigits ( );
		os << "', '";
		bool hasCurChoice = common.curChoice ( );
		if ( hasCurChoice )
			os << common.getConvertedDigits ( );
		else
			os << '-';
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
			' ' << inCode << ' ' << inPrice << ' ' << inSecs << ' ' << inConnectPrice << ' ' << inEuCode << ' ' <<
			inEuPrice << ' ' << inEuSecs << ' ' << inEuConnectPrice << ' ' << outCode << ' ' << outPrice << ' ' <<
			outSecs << ' ' << outConnectPrice;
		if ( hasCurChoice && ! source.outAcctn.empty ( ) ) {
			const OutCardDetails & ocd = common.curChoice ( ) -> getOutCardDetails ( );
			int ocdTarifedSecs = conf -> getRounded ( conf -> getMinutesFromSeconds ( ocd.getEuTarif ( ), secs2 ) * 60 );
			int ocdSecs = ocd.getEuRound ( ).round ( ocdTarifedSecs ) ;
			os << " card " << source.outAcctn << ' ' << ocd.getEuPrice ( ) * source.valuteRate / 100000.0 << ' ' << secs2 << ' ' <<
				( ocdSecs ? ( ocd.getEuConnectPrice ( ) + getAmortisedMoney ( ocd.getAmortise ( ), ocdTarifedSecs ) ) * source.valuteRate / 100000.0 : 0.0 )
				<< ' ' << ocd.getDigits ( ) << ' ' << ocd.getEuCode ( ) << ' ' << ocdSecs / 60.0;
		}
		os << "', from_unixtime( " << timeInSecs << " ), from_unixtime( " << endTime.GetTimeInSeconds ( ) <<
			" ), " << secs << ", " << secs2 << ", '" << common.getCallerIp ( ) << "', '";
		if ( hasCurChoice )
			os << common.getCalledIp ( );
		os << "', " << common.getDisconnectCause ( );
		for ( int i = 0; i < 4; i ++ )
			os << ", " << st [ i ].frac << ", " << st [ i ].jitterMax << ", " << st [ i ].jitterAvg;
		os << ", '" << common.getRealDigits ( ) << "', '" << source.inPrefix << "', '" << outDigits << "', '"
			<< incomingCallId << "', '" << printableCallId << "' )";
		sendRadiusAcc ( common, rtpStats, setupTime, setupTime, begTime, endTime, ref, incomingCallId );
	}
	for ( unsigned i = 0; i + skippedTries < tries.size ( ); i ++ ) {
		int index = tries [ i ].choiceIndex;
		const OutChoiceDetails * curChoice = common.choice ( index );
		if ( i || skippedTries )
			os << ", ";
		os << "( 0, '', '";
		if ( curChoice -> getCallingDigits ( ) != "-" )
			os << curChoice -> getCallingDigits ( );
		os << "', '" << curChoice -> getSentDigits ( ) << "', '" << curChoice -> getDigits ( ) << ' ' << ip << ' ';
		int op = curChoice -> getPeer ( );
		os << op << " 0 0 - - 0 0 0 - 0 0 0 ";
		ss :: string outCode = curChoice -> getCode ( ).substr ( curChoice -> getDepth ( ) );
		if ( outCode.empty ( ) )
			outCode = '-';
		double outPrice = curChoice -> getPrice ( ) / 100000.0,
			outConnectPrice = 0;//curChoice -> getConnectPrice ( ) / 100000.0;
		os << outCode << ' ' << outPrice << " 0 " << outConnectPrice << "', from_unixtime( " <<
			timeInSecs << " ), from_unixtime( " << timeInSecs << " ), 0, 0, '', '" << curChoice -> getIp ( )
			<< "', " << tries [ i ].cause << ", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '" <<
			common.getRealDigits ( ) << "', '', '" <<
			common.getRealDigits ( ).substr ( curChoice -> getDepth ( ) ) << "', '', '" << tries [ i ].callId << "' )";
	}
	sqlOut -> add ( os.str ( ) );
}

void OriginateLegThread :: setCodec ( const CodecInfo & c ) {
	codec = c;
}

void OriginateLegThread :: setSendAddress ( const PIPSocket :: Address & addr, int port, const CodecInfo & inCodec,
	const CodecInfo & outCodec, const CodecInfo & changedInCodec, const CodecInfo & changedOutCodec ) {
	differentCodec = isDifferentCodec ( inCodec, changedInCodec ) || isDifferentCodec ( outCodec, changedOutCodec );
	call -> setSendAddress ( this, addr, port, inCodec, outCodec, changedInCodec, changedOutCodec );
	if ( begInited && ! isSendAddressSet ) {
		if ( ! pddTime )
			pddTime = PTime ( );
		call -> connected ( this );
	}
	isSendAddressSet = true;
}

void OriginateLegThread :: setTelephoneEventsPayloadType ( unsigned payload ) {
	call -> setTelephoneEventsPayloadType ( this, payload );
}

Leg :: Leg ( OriginateLegThread * t, CommonCallDetails & c ) : thread ( t ), common ( c ) { }

CallBackLegThread :: CallBackLegThread ( H323Call * c, LegThread * * p, const ss :: string & an, const ss :: string & ac,
	const ss :: string & n, unsigned _id, const CodecInfoVector & ic, const CodecInfoVector & oc ) :
	OriginateLegThread ( c, p, _id, ic, oc ) {
	SetThreadName ( "CallBackLeg " + PString ( id ) );
	common.setCallingDigitsIn ( an );
	common.setDialedDigits ( n );
	common.source ( ).acctn = ac;
	common.setConfId ( H323 :: globallyUniqueId ( ) );

	Resume ( );
}

void CallBackLegThread :: sqlSetPeer ( ) {
	ss :: ostringstream os;
	os << "update SmsCallBackActiveCalls set convertedDigits = '" << common.getConvertedDigits ( ) << "', ip = '"
		<< common.getCalledIp ( ) << "', name = '" << common.getOutName ( ) << "' where id = " << id;
	sqlOut -> add ( os.str ( ) );
}

void CallBackLegThread :: sqlStart ( ) {
	ss :: ostringstream os;
	os << "replace SmsCallBackActiveCalls set id = " << id << ", acctn = '" << common.getSource ( ).acctn <<
		"', startTime = from_unixtime( " << PTime ( ).GetTimeInSeconds ( ) << " ), digits = '" <<
		common.getDialedDigits ( ) << '\'';
	sqlOut -> add ( os.str ( ) );
}

void CallBackLegThread :: sqlEnd ( ) {
	conf -> removeCall ( common.source ( ), eaters );
	ss :: ostringstream os;
	os << "delete from SmsCallBackActiveCalls where id = " << id;
	sqlOut -> add ( os.str ( ) );
}

void CallBackLegThread :: sqlConnect ( ) {
	ss :: ostringstream os;
	os << "update SmsCallBackActiveCalls set connectTime = from_unixtime( " <<
		begTime.GetTimeInSeconds ( ) << " ) where id = " << id;
	sqlOut -> add ( os.str ( ) );
}

bool CallBackLegThread :: init ( ) {
	common.source ( ).type = SourceData :: card;
	OutChoiceDetailsVectorVector choiceForks; //FIXME
	StringVector forkOutAcctns;
	if ( ! conf -> getCallInfo ( choiceForks, forkOutAcctns, common, true, true, true ) ) {
		call -> stop ( this, common.getDisconnectCause ( ) );
		return false;
	}
	return true;
}

void CallBackLegThread :: accountCall ( ) {
	OriginateLegThread :: accountCall ( );
	common.release ( );
}

OriginateFullThread :: OriginateFullThread ( H323Call * c, LegThread * * p, unsigned _id,
	const CommonCallDetails & com, const CodecInfoVector & ic, const CodecInfoVector & oc, ProtoType origType,
	unsigned maxForwards, const ss :: string & remotePartyID, const ss :: string & pAssertId, const ss :: string & privacy ) :
	OriginateLegThread ( c, p, _id, com, ic, oc, origType, maxForwards, remotePartyID, pAssertId, privacy ) {
	SetThreadName ( "OriginateFullLeg " + PString ( id ) );
	Resume ( );
}

bool OriginateFullThread :: init ( ) {
	return true;
}

void OriginateFullThread :: sqlSetPeer ( ) {
	ss :: ostringstream os;
	os << "update ActiveCalls set convertedDigits = '" << common.getConvertedDigits ( ) << "', outIP = '"
		<< common.getCalledIp ( ) << "', outName = '" << common.getOutName ( ) << "' where id = " << id;
	sqlOut -> add ( os.str ( ) );
}

void OriginateFullThread :: sqlStart ( ) {
	conf -> addActiveCall ( id, this );
}

void OriginateFullThread :: sqlEnd ( ) {
	ss :: ostringstream os;
	os << "delete from ActiveCalls where id = " << id;
	sqlOut -> add ( os.str ( ) );
	conf -> removeActiveCall ( id );
}

void OriginateFullThread :: sqlConnect ( ) {
	ss :: ostringstream os;
	os << "replace ActiveCalls set id = " << id << ", startTime = from_unixtime( " << PTime ( ).GetTimeInSeconds ( )
		<< " ), setupTime = from_unixtime( " << PTime ( ).GetTimeInSeconds ( ) << " ), inCallId = '" <<
		incomingCallId << "', outCallId = '" << printableCallId << "', name = '" << common.getSource ( ).name <<
		"', rname = '" << common.getSource ( ).rname << "', inIP = '" << common.getCallerIp ( ) << '\'';
	if ( common.getSource ( ).type == SourceData :: card )
		os << ", dialedDigits = '#" << common.getSource ( ).acctn << '#' << common.getRealDigits ( ) << '\'';
	else if ( common.getSource ( ).type == SourceData :: inbound )
		os << ", dialedDigits = '" << common.getDialedDigits ( ) << '\'';
	os << ", callingDigits = '" << common.getCallingDigitsIn ( ) <<
		"', convertedDigits = '" << common.getConvertedDigits ( ) << "', outIP = '"
		<< common.getCalledIp ( ) << "', outName = '" << common.getOutName ( ) <<
		"', connectTime = from_unixtime( " <<
		begTime.GetTimeInSeconds ( ) << " ), priceCache = '";
	ss :: ostringstream ot;
	const CommonCallDetails & common = this -> common;
	const OutTryVector & tries = call -> getOutTries ( this );
	{
		boost :: archive :: text_oarchive oa ( ot );
		oa << common << tries;
	}
	os << MySQL :: escape ( ot.str ( ) ) << '\'';
	sqlOut -> add ( os.str ( ) );
}

int OriginateLegThread :: getLocalPort ( ) const {
	return call -> getLocalPort ( this );
}

PIPSocket :: Address OriginateLegThread :: getLocalIp ( ) const {
	return call -> getLocalIp ( this );
}

void OriginateLegThread :: beginShutDown ( ) {
	Close ( Q931 :: cvNormalUnspecified );
}

int OriginateLegThread :: getCallSeconds ( ) const {
	return conf -> getRounded ( double ( ( endTime - begTime ).GetMilliSeconds ( ) ) / 1000.0 );
}

bool OriginateLegThread :: getCallConnected ( ) const {
	return begInited;
}

int OriginateLegThread :: getCallRef ( ) const {
	return ref;
}

ss :: string OriginateLegThread :: getInIp ( ) const {
	return common.getCallerIp ( );
}

int OriginateLegThread :: getInPeerId ( ) const {
	return common.getSource ( ).peer;
}

ss :: string OriginateLegThread :: getInPeerName ( ) const {
	return common.getSource ( ).name;
}

ss :: string OriginateLegThread :: getInResponsibleName ( ) const {
	return common.getSource ( ).rname;
}

ss :: string OriginateLegThread :: getSetupTime ( ) const {
	return static_cast < const char * > ( setupTime.AsString ( ) );
}

ss :: string OriginateLegThread :: getConnectTime ( ) const {
	if ( ! begInited )
		return "-";
	return static_cast < const char * > ( begTime.AsString ( ) );
}

ss :: string OriginateLegThread :: getOutIp ( ) const {
	const OutChoiceDetails * c = common.curChoice ( );
	if ( ! c )
		return "";
	return c -> getIp ( );
}

int OriginateLegThread :: getOutPeerId ( ) const {
	const OutChoiceDetails * c = common.curChoice ( );
	if ( ! c )
		return 0;
	return c -> getPeer ( );
}

void OriginateLegThread :: setPrintableCallId ( const ss :: string & s ) {
	AutoMutex am ( lm );
	printableCallId = s;
}

ss :: string OriginateLegThread :: getCallId ( ) const {
	ss :: string r;
	{
		AutoMutex am ( lm );
		r = printableCallId;
	}
	return r;
}

ss :: string OriginateLegThread :: getOutPeerName ( ) const {
	return common.getOutName ( );
}

ss :: string OriginateLegThread :: getOutResponsibleName ( ) const {
	return common.getOutRname ( );
}

ss :: string OriginateLegThread :: getCallingDigits ( ) const {
	const OutChoiceDetails * c = common.curChoice ( );
	if ( ! c )
		return "-";
	return c -> getCallingDigits ( );
}

ss :: string OriginateLegThread :: getSentDigits ( ) const {
	const OutChoiceDetails * c = common.curChoice ( );
	if ( ! c )
		return "-";
	return c -> getSentDigits ( );
}

ss :: string OriginateLegThread :: getDialedDigits ( ) const {
	return common.getDialedDigits ( );
}

ss :: string OriginateLegThread :: getInCode ( ) const {
	return common.getSource ( ).code;
}

ss :: string OriginateLegThread :: getOutCode ( ) const {
	const OutChoiceDetails * c = common.curChoice ( );
	if ( ! c )
		return "";
	return c -> getCode ( ).substr ( c -> getDepth ( ) );
}

ss :: string OriginateLegThread :: getInAcctn ( ) const {
	return common.getSource ( ).acctn;
}

ss :: string OriginateLegThread :: getOutAcctn ( ) const {
	return common.getSource ( ).outAcctn;
}

ss :: string OriginateLegThread :: getPdd ( ) const {
	PTime t = pddTime ? pddTime.get ( ) : PTime ( );
	ss :: ostringstream os;
	os << t - call -> getInitTime ( );
	return os.str ( );
}

bool OriginateLegThread :: getDifferentProtocol ( ) const {
	return limitTries == - 1 || gwTaken;
}

bool OriginateLegThread :: getDifferentCodec ( ) const {
	return differentCodec;
}
