#pragma implementation
#include "ss.hpp"
#include "allocatable.hpp"
#include "tarifinfo.hpp"
#include "tarifround.hpp"
#include "pricedata.hpp"
#include "sourcedata.hpp"
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/sequenced_index.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include "codecinfo.hpp"
#include <iomanip>
#include <stdexcept>
#include <cstring>
#include "asn.hpp"
#include <ptlib.h>
#include <ptlib/sockets.h>
#include "q931.hpp"
#include "h225.hpp"
#include "h323common.hpp"
#include "h245handler.hpp"
#include <ptlib/svcproc.h>
#include "automutex.hpp"
#include "aftertask.hpp"
#include "ipport.hpp"
#include "icqcontact.hpp"
#include "radgwinfo.hpp"
#include "Conf.hpp"
#include "session.hpp"
#include "rtpsession.hpp"
#include "signallingoptions.hpp"
#include "pointer.hpp"
#include "outcarddetails.hpp"
#include "outchoicedetails.hpp"
#include "moneyspool.hpp"
#include <tr1/functional>
#include "smartguard.hpp"
#include <tr1/memory>
#include "commoncalldetails.hpp"
#include <boost/range.hpp>
#include "slprint.hpp"

H245Handler :: H245Handler ( volatile bool & differentCodec, const CommonCallDetailsBaseOut & call, RTPStat * st, bool directRTP ) :
	differentCodec ( differentCodec ), rtpSession ( 0 ), stats ( st ), direct ( directRTP ), fromNat ( call.getFromNat ( ) ),
	toNat ( call.curChoice ( ) -> getFromNat ( ) ), source ( call.getSource ( ) ),
	inAllCodecs ( call.getInAllCodecs ( ) ), outAllCodecs ( call.getOutAllCodecs ( ) ),
	inAllowedCodecs ( call.getInAllowedCodecs ( ) ), outAllowedCodecs ( call.getOutAllowedCodecs ( ) ),
	outPeer ( call.curChoice ( ) -> getPeer ( ) ), inAnswerToRTDR ( call.getSigOptions ( ).inAnswerToRTDR ( ) ),
	outAnswerToRTDR ( call.getSigOptions ( ).outAnswerToRTDR ( ) ), callerBreak ( false ),
	calledBreak ( false ), dropTele ( call.getSigOptions ( ).dropTunneling ( ) ) {
	differentCodec = false;
	PIPSocket :: GetHostAddress ( from );
	PIPSocket :: GetHostAddress ( to );
	ss :: for_each ( inAllowedCodecs, SLPrint ( "codec in in list: " ) );
	ss :: for_each ( outAllowedCodecs, SLPrint ( "codec in out list: " ) );
	ss :: for_each ( call.getInAllowedCodecs ( ), SLPrint ( "codec in original in list: " ) );
	ss :: for_each ( call.getOutAllowedCodecs ( ), SLPrint ( "codec in original out list: " ) );
}

Session * H245Handler :: getRtpSession ( ) {
	if ( ! rtpSession )
		rtpSession = new RTPSession ( stats, from, to, fromNat, toNat );
//		rtpSession = new MGCPSession ( stats, from, to, fromNat, toNat );
	return rtpSession;
}

bool H245Handler :: admissiblePort ( WORD port ) {
	return :: admissiblePort ( port );
}

bool H245Handler :: admissibleIP ( bool caller, PIPSocket :: Address & ip ) {
// Task: Used on logical channel procedures to check if the ip address used
// for the logical channel is admissible. The admissible ips are the
// ip used for RAS addresses or the ip of the call signal addresses
// of the indicated endpoint (caller or called).
	PSYSTEMLOG ( Info, "rtp ip check: " << caller << ", " << ip );
	if ( caller ) {
		if ( source.type == SourceData :: inbound &&
			! conf -> validInRtp ( source.peer, static_cast < const char * > ( ip.AsString ( ) ) ) ) {
			callerBreak = true;
			throw std :: logic_error ( static_cast < const char * > ( "rtp ip " + ip.AsString ( ) +
				" not admissible" ) );
		}
	} else {
		if ( ! conf -> validOutRtp ( outPeer, static_cast < const char * > ( ip.AsString ( ) ) ) ) {
				calledBreak = true;
				throw std :: logic_error ( static_cast < const char * > ( "rtp ip " + ip.AsString ( ) +
					" not admissible" ) );
		}
	}
	return true;
}

void H245Handler :: handleOpenLogicalChannelReject ( H245 :: OpenLogicalChannelReject & /*msg*/,
	bool /*FromCaller*/ ) { }

bool H245Handler :: handleOpenLogicalChannelAck ( H245 :: OpenLogicalChannelAck & msg, bool fromCaller ) {
	if ( ! msg.hasOptionalField ( H245 :: OpenLogicalChannelAck :: e_forwardMultiplexAckParameters ) ) {
		PSYSTEMLOG ( Error, "no forwardMultiplexAckParameters - not supported" );
		return false;
	}
	H245 :: OpenLogicalChannelAck_forwardMultiplexAckParameters & ackparam =
		msg.get_forwardMultiplexAckParameters ( );
	if ( ackparam.getTag ( ) != H245 :: OpenLogicalChannelAck_forwardMultiplexAckParameters ::
		e_h2250LogicalChannelAckParameters ) {
		PSYSTEMLOG ( Error, "unsupported ackparams tag: " << ackparam.getTag ( ) );
		return false;
	}
	H245 :: H2250LogicalChannelAckParameters & h225Params = ackparam;
	if ( msg.hasOptionalField ( H245 :: OpenLogicalChannelAck :: e_reverseLogicalChannelParameters ) ) {
		PSYSTEMLOG ( Error, "reverse params - not supported" );
		return false;
	}
	if ( ! h225Params.hasOptionalField ( H245 :: H2250LogicalChannelAckParameters :: e_mediaChannel ) ) {
		PSYSTEMLOG ( Error, "no mediaChannel - unsupported" );
		return false;
	}
	if ( ! h225Params.hasOptionalField ( H245 :: H2250LogicalChannelAckParameters :: e_mediaControlChannel ) ) {
		PSYSTEMLOG ( Error, "no mediaControlChannel - not supported" );
		return false;
	}
	int channel = msg.m_forwardLogicalChannelNumber;
	const RequestMap & channelRequests = fromCaller ? outChannelRequests : inChannelRequests;
	RequestMap :: const_iterator p = channelRequests.find ( channel );
	if ( p == channelRequests.end ( ) ) {
		PSYSTEMLOG ( Error, "ack unknown channel " << channel );
		return false;
	}
	H245 :: TransportAddress & controlAddr = h225Params.get_mediaControlChannel ( );
	H245 :: UnicastAddress_iPAddress * controlIp;
	PIPSocket :: Address controlIpAddr;
	if ( ! checkAddr ( controlAddr, controlIp, controlIpAddr, fromCaller ) )
		return false;
	H245 :: TransportAddress & mediaAddr = h225Params.get_mediaChannel ( );
	H245 :: UnicastAddress_iPAddress * mediaIp;
	PIPSocket :: Address mediaIpAddr;
	if ( ! checkAddr ( mediaAddr, mediaIp, mediaIpAddr, fromCaller ) )
		return false;
	if ( direct )
		return true;
	Session * s = getRtpSession ( );
	bool sendCodec = p -> second.rtpFrames;
	differentCodec = isDifferentCodec ( p -> second.recodeTo, p -> second.recodeFrom );
	s -> setSendAddress ( fromCaller, false, mediaIpAddr, WORD ( mediaIp -> m_tsapIdentifier ), p -> second.rtpCodec,
		p -> second.rtpFrames, sendCodec, p -> second.recodeTo, p -> second.recodeFrom );
	setIPAddress ( * mediaIp, getLocalAddress ( ! fromCaller ), s -> getLocalAddress ( fromCaller, false ) );
	setIPAddress ( * controlIp, getLocalAddress ( ! fromCaller ), s -> getLocalAddress ( fromCaller, true ) );
	return true;
}

bool H245Handler :: handleResponse ( H245 :: ResponseMessage & response, bool fromCaller ) {
// Task: to handle the given H.245 response
	switch ( response.getTag ( ) ) {
		case H245 :: ResponseMessage :: e_openLogicalChannelAck:
			return handleOpenLogicalChannelAck ( response, fromCaller );
		case H245 :: ResponseMessage :: e_openLogicalChannelReject:
			handleOpenLogicalChannelReject ( response, fromCaller );
		default:
			return true;
	}
}

bool H245Handler :: checkAddr ( H245 :: TransportAddress & addr, H245 :: UnicastAddress_iPAddress * & ip,
	PIPSocket :: Address & ipAddr, bool fromCaller ) {
	if ( addr.getTag ( ) != H245 :: TransportAddress :: e_unicastAddress ) {
		PSYSTEMLOG ( Error, "RTP Addr isn't e_unicastAddress" );
		return false;
	}
	H245 :: UnicastAddress & unicastAddr = addr;
	if ( unicastAddr.getTag ( ) != H245 :: UnicastAddress :: e_iPAddress ) {
		PSYSTEMLOG ( Error, "RTP Addr isn't e_iPAddress" );
		return false;
	}
	ip = & static_cast < H245 :: UnicastAddress_iPAddress & > ( unicastAddr );
	const Asn :: string & nw = ip -> m_network;
	ipAddr = PIPSocket :: Address ( nw [ 0 ], nw [ 1 ], nw [ 2 ], nw [ 3 ] );
	// Security check. Check if the given ip belongs to the endpoint and if the
	// port number is not a system port.
	if ( ! admissibleIP ( fromCaller, ipAddr ) ) {
		PSYSTEMLOG ( Error, "RTP Addr isn't admissible" );
		return false;
	}
	if ( ! admissiblePort ( WORD ( ip -> m_tsapIdentifier ) ) ) {
		PSYSTEMLOG ( Error, "RTP Port " << ip -> m_tsapIdentifier << " isn't admissible" );
		return false;
	}
	return true;
}

void H245Handler :: handleRTDR ( H245 :: MultimediaSystemControlMessage & msg, bool fromCaller,
	bool & changeTarget ) {
	if ( ( fromCaller && ! inAnswerToRTDR ) || ( ! fromCaller && ! outAnswerToRTDR ) )
		return;
	int num;
	{
		const H245 :: RequestMessage & rm = msg;
		const H245 :: RoundTripDelayRequest & req = rm;
		num = req.m_sequenceNumber;
	}
	changeTarget = true;
	msg.setTag ( H245 :: MultimediaSystemControlMessage :: e_response );
	H245 :: ResponseMessage & response = msg;
	response.setTag ( H245 :: ResponseMessage :: e_roundTripDelayResponse );
	H245 :: RoundTripDelayResponse & rtdr = response;
	rtdr.m_sequenceNumber = num;
}

void H245Handler :: handleOpenLogicalChannelConfirm ( H245 :: OpenLogicalChannelConfirm & /*msg*/, bool /*fromCaller*/ ) { }

void H245Handler :: handleIndication ( H245 :: IndicationMessage & indication, bool fromCaller ) {
// Task: to handle the given H.245 indication
	switch ( indication.getTag ( ) ) {
		case H245 :: IndicationMessage :: e_openLogicalChannelConfirm:
			handleOpenLogicalChannelConfirm ( indication, fromCaller );
			break;
	}
}

static void changeCodecAndFrames ( H245 :: AudioCapability & audioData, const ss :: string & recodeTo, int f ) {
	setCodec ( audioData, recodeTo );
	if ( f )
		setFrames ( audioData, f );
}

bool H245Handler :: handleOpenLogicalChannel ( H245 :: OpenLogicalChannel & olc, bool fromCaller ) {
	if ( olc.hasOptionalField ( H245 :: OpenLogicalChannel :: e_reverseLogicalChannelParameters ) ) {
		PSYSTEMLOG ( Error, "open with reverse params - unsupported" );
		return false;
	}
	H245 :: OpenLogicalChannel_forwardLogicalChannelParameters & forwardParams =
		olc.m_forwardLogicalChannelParameters;
//	if ( forwardParams.m_dataType.getTag ( ) != H245 :: DataType :: e_audioData ) {
//		PSYSTEMLOG ( Error, "unsupported dataType tag: " << forwardParams.m_dataType.getTag ( ) );
//		return false;
//	}
	H245 :: OpenLogicalChannel_forwardLogicalChannelParameters_multiplexParameters & multiParams =
		forwardParams.m_multiplexParameters;
	if ( multiParams.getTag ( ) != H245 :: OpenLogicalChannel_forwardLogicalChannelParameters_multiplexParameters
		:: e_h2250LogicalChannelParameters ) {
		PSYSTEMLOG ( Error, "unsupported multiParams tag: " << multiParams.getTag ( ) );
		return false;
	}
	H245 :: H2250LogicalChannelParameters & h225Params = multiParams;
	if ( h225Params.hasOptionalField ( H245 :: H2250LogicalChannelParameters :: e_mediaChannel ) ) {
		PSYSTEMLOG ( Error, "open with mediaChannel - unsupported" );
		return false;
	}
	if ( ! h225Params.hasOptionalField ( H245 :: H2250LogicalChannelParameters :: e_mediaControlChannel ) ) {
		PSYSTEMLOG ( Error, "no mediaControlChannel - not supported" );
		return false;
	}
	H245 :: TransportAddress & addr = h225Params.get_mediaControlChannel ( );
	H245 :: UnicastAddress_iPAddress * ip;
	PIPSocket :: Address ipAddr;
	if ( ! checkAddr ( addr, ip, ipAddr, fromCaller ) )
		return false;
	int channel = olc.m_forwardLogicalChannelNumber;
	H245 :: DataType & dataType = forwardParams.m_dataType;
	bool isAudioData = dataType.getTag ( ) == H245 :: DataType :: e_audioData;
//	H245 :: AudioCapability & audioData = forwardParams.m_dataType;
	RequestElement r ( dataType );
	bool myAllCodecs = fromCaller ? inAllCodecs : outAllCodecs;
	if ( ! myAllCodecs ) {
		const CodecInfoSet & myAllowedCodecs = fromCaller ? inAllowedCodecs : outAllowedCodecs;
		CodecInfoSet :: const_iterator i = myAllowedCodecs.find ( r.recodeFrom );
		if ( i == myAllowedCodecs.end ( ) ) {
			PSYSTEMLOG ( Error, "unallowed codec " << r.recodeFrom << " from " <<
				( fromCaller ? "caller" : "called" ) << " - bug" );
			for ( CodecInfoSet :: const_iterator i = myAllowedCodecs.begin ( );
				i != myAllowedCodecs.end ( ); ++ i )
				PSYSTEMLOG ( Error, "allowed: " << * i );
			return false;
		}
	}
	bool theirAllCodecs = fromCaller ? outAllCodecs : inAllCodecs;
	if ( ! theirAllCodecs ) {
		const CodecInfoSet & theirAllowedCodecs = fromCaller ? outAllowedCodecs : inAllowedCodecs;
		CodecInfoSet :: const_iterator i = theirAllowedCodecs.find ( r.recodeFrom );
		if ( i == theirAllowedCodecs.end ( ) ) {
			if ( direct )
				return false;
			i = findRecode ( r.recodeFrom, theirAllowedCodecs, conf -> getRecodes ( ) );
			if ( i == theirAllowedCodecs.end ( ) ) {
				PSYSTEMLOG ( Error, "cant find recode" );
				return false;
			}
			r.recodeTo = i -> getCodec ( );
		} else
			r.recodeFrom.clear ( );
		r.rtpFrames = i -> getFrames ( );
	} else
		r.recodeFrom.clear ( );
	codec = :: getCodec ( dataType );
	if ( isAudioData ) {
		if ( ! r.recodeTo.empty ( ) )
			setCodec ( dataType, r.recodeTo );
		r.rtpCodec = getRtpCodec ( dataType );
	}
	( fromCaller ? inChannelRequests : outChannelRequests ).insert ( std :: make_pair ( channel, r ) );
	if ( isAudioData && r.rtpFrames )
		setFrames ( dataType, r.rtpFrames );
	if ( direct )
		return true;
	Session * s = getRtpSession ( );
	s -> setSendAddress ( fromCaller, true, ipAddr, WORD ( ip -> m_tsapIdentifier ), 0, 0, false, "", "" );
	setIPAddress ( * ip, getLocalAddress ( ! fromCaller ), s -> getLocalAddress ( fromCaller, false ) );
	return true;
}

bool H245Handler :: checkCap ( const CodecInfo & codec, bool fromCaller, H245 :: AudioCapability & cap ) const {
	for ( CodecInfoSet :: const_iterator i = inAllowedCodecs.begin ( ); i != inAllowedCodecs.end ( ); ++ i )
		PSYSTEMLOG ( Info, "codec in in list: " << * i );
	for ( CodecInfoSet :: const_iterator i = outAllowedCodecs.begin ( ); i != outAllowedCodecs.end ( ); ++ i )
		PSYSTEMLOG ( Info, "codec in out list: " << * i );

	bool changeIn = false, changeOut = false;
	int frames = 0;
	if ( ! inAllCodecs ) {
		CodecInfoSet :: const_iterator i = inAllowedCodecs.find ( codec );
		if ( i == inAllowedCodecs.end ( ) ) {
			if ( fromCaller )
				return false;
			changeIn = true;
		} else if ( ! fromCaller )
			frames = i -> getFrames ( );
	}
	if ( ! outAllCodecs ) {
		CodecInfoSet :: const_iterator i = outAllowedCodecs.find ( codec );
		if ( i == outAllowedCodecs.end ( ) ) {
			if ( ! fromCaller )
				return false;
			changeOut = true;
		} else if ( fromCaller )
			frames = i -> getFrames ( );
	}
	if ( changeIn ) {
		CodecInfoSet :: const_iterator i = findRecode ( codec, inAllowedCodecs, conf -> getRecodes ( ) );
		if ( i == inAllowedCodecs.end ( ) )
			return false;
		changeCodecAndFrames ( cap, i -> getCodec ( ), i -> getFrames ( ) );
		return true;
	}
	if ( changeOut ) {
		CodecInfoSet :: const_iterator i = findRecode ( codec, outAllowedCodecs, conf -> getRecodes ( ) );
		if ( i == outAllowedCodecs.end ( ) )
			return false;
		changeCodecAndFrames ( cap, i -> getCodec ( ), i -> getFrames ( ) );
		return true;
	}
	if ( frames )
		setFrames ( cap, frames );
	return true;
}

void H245Handler :: handleCapabilitySet ( H245 :: TerminalCapabilitySet & caps, bool fromCaller ) {
	PSYSTEMLOG(Info, "H245Handler :: handleCapabilitySet");
	if ( ! caps.hasOptionalField ( H245 :: TerminalCapabilitySet :: e_capabilityTable ) ||
		! caps.hasOptionalField ( H245 :: TerminalCapabilitySet :: e_capabilityDescriptors ) )
		return;
	H245 :: TerminalCapabilitySet_capabilityTable & table = caps.get_capabilityTable ( );
	IntSet removedCaps;
	StringSet passedCaps;
	for ( std :: size_t i = table.size ( ); i --; ) {
		H245 :: CapabilityTableEntry & entry = table [ i ];
		if ( ! entry.hasOptionalField ( H245 :: CapabilityTableEntry :: e_capability ) )
			continue;
		H245 :: Capability & cap = entry.get_capability ( );
		switch ( cap.getTag ( ) ) {
			case H245 :: Capability :: e_receiveAudioCapability:
			case H245 :: Capability :: e_transmitAudioCapability:
			case H245 :: Capability :: e_receiveAndTransmitAudioCapability: {
				CodecInfo codec = :: getCodec ( cap );
				if ( codec.getCodec ( ) == "unknown" )
					PSYSTEMLOG ( Error, "unknown capability !!!!!!!!!!!!!!!!!: " << cap );
				else
					PSYSTEMLOG ( Info, "capability " << codec );
				if ( checkCap ( codec, fromCaller, cap ) &&
					passedCaps.insert ( :: getCodec ( cap ).getCodec ( ) ).second )
					continue;
				break;
			} case H245 :: Capability :: e_receiveRTPAudioTelephonyEventCapability:
				if ( dropTele )
					break;
			default:
				continue;
		}
		removedCaps.insert ( entry.m_capabilityTableEntryNumber );
		table.erase ( i );
	}
	if ( table.empty ( ) )
		caps.removeOptionalField ( H245 :: TerminalCapabilitySet :: e_capabilityTable );
	if ( removedCaps.empty ( ) )
		return;
	H245 :: TerminalCapabilitySet_capabilityDescriptors & descrs = caps.get_capabilityDescriptors ( );
	for ( std :: size_t i = 0, ds = descrs.size ( ); i < ds; i ++ ) {
		H245 :: CapabilityDescriptor & descr = descrs [ i ];
		if ( ! descr.hasOptionalField ( H245 :: CapabilityDescriptor :: e_simultaneousCapabilities ) )
			continue;
		H245 :: CapabilityDescriptor_simultaneousCapabilities & capSetArray =
			descr.get_simultaneousCapabilities ( );
		for ( std :: size_t j = capSetArray.size ( ); j --; ) {
			H245 :: AlternativeCapabilitySet & capSet = capSetArray [ j ];
			for ( std :: size_t k = capSet.size ( ); k --; ) {
				if ( removedCaps.count ( capSet [ k ] ) )
					capSet.erase ( k );
			}
			if ( capSet.empty ( ) )
				capSetArray.erase ( j );
		}
		if ( capSetArray.empty ( ) )
			descr.removeOptionalField ( H245 :: CapabilityDescriptor :: e_simultaneousCapabilities );
	}
}

bool H245Handler :: handleRequest ( H245 :: MultimediaSystemControlMessage & msg, bool fromCaller,
	bool & changeTarget ) {
// Task: to handle the given H.245 request
	PSYSTEMLOG ( Info, "H245Handler_Proxy::HandleRequest()" );
	H245 :: RequestMessage & request = msg;
	switch ( request.getTag ( ) ) {
		case H245 :: RequestMessage :: e_openLogicalChannel:
			return handleOpenLogicalChannel ( request, fromCaller );
		case H245 :: RequestMessage :: e_terminalCapabilitySet:
			handleCapabilitySet ( request, fromCaller );
			return true;
		case H245 :: RequestMessage :: e_roundTripDelayRequest:
			handleRTDR ( msg, fromCaller, changeTarget );
		default:
			return true;
	}
	PSYSTEMLOG ( Info, "H245Handler_Proxy::HandleRequest() - fine" );
}

bool H245Handler :: handleMesg ( H245 :: MultimediaSystemControlMessage & mesg, bool fromCaller,
	bool & changeTarget ) {
// Task: to handle the given H.245 message
	AutoMutex am ( mut );
	switch ( mesg.getTag ( ) ) {
		case H245 :: MultimediaSystemControlMessage :: e_request:
			return handleRequest ( mesg, fromCaller, changeTarget );
		case H245 :: MultimediaSystemControlMessage :: e_response:
			return handleResponse ( mesg, fromCaller );
		case H245 :: MultimediaSystemControlMessage :: e_indication:
		case H245 :: MultimediaSystemControlMessage :: e_command:
			return true;
		default :
			PSYSTEMLOG ( Error, "Unknown H245 message type not handled : " << mesg.getTag ( ) );
			return false;
	}
}

PIPSocket :: Address H245Handler :: getLocalAddress ( bool callerSide ) {
// Task: return the address of the interface to the caller host or the called host.
	PIPSocket :: Address addr;
/*	if ( callerSide && CallerSocket )
		CallerSocket -> getLocalAddress ( addr );
	else if ( ! callerSide && CalledSocket )
		CalledSocket -> getLocalAddress ( addr );
	else
		PIPSocket :: GetHostAddress ( addr );*/
	addr = ( callerSide ? from : to );
	PSYSTEMLOG ( Info, "H245Handler_Proxy::getLocalAddress(" <<
		callerSide << ") = " << addr );
	return addr;
}

void H245Handler :: setFrom ( const PIPSocket :: Address & f ) {
	from = f;
}

PIPSocket :: Address H245Handler :: getFrom ( ) const {
	return from;
}

void H245Handler :: setTo ( const PIPSocket :: Address & t, bool tn ) {
	to = t;
	toNat = tn;
	if ( rtpSession )
		rtpSession -> setTo ( t, tn );
}

PIPSocket :: Address H245Handler :: getTo ( ) const {
	return to;
}

H245Handler :: ~H245Handler ( ) {
	differentCodec = false;
	delete rtpSession;
}

void H245Handler :: removeUnsupported ( FastStartElementVector & fs ) const {
	if ( inAllCodecs )
		return;
	for ( FastStartElementVector :: size_type i = fs.size ( ); i --; ) {
		CodecInfoSet :: const_iterator c = inAllowedCodecs.find ( fs [ i ].codec );
		if ( c == inAllowedCodecs.end ( ) )
			fs.erase ( fs.begin ( ) + i );
		else
			fs [ i ].changeFrames = c -> getFrames ( );
	}
}

void H245Handler :: removeReverseUnsupported ( FastStartElementVector & fs ) const {
	if ( outAllCodecs )
		return;
	for ( FastStartElementVector :: size_type i = fs.size ( ); i --; ) {
		if ( outAllowedCodecs.count ( fs [ i ].codec ) == 0 )
			fs.erase ( fs.begin ( ) + i );
	}
}

static CodecInfo findRecode ( const CodecInfo & codec, const StringStringSetMap & recodes ) {
	StringStringSetMap :: const_iterator j = recodes.find ( codec.getCodec ( ) );
	if ( j == recodes.end ( ) )
		return CodecInfo ( );
	return * j -> second.begin ( );
}

void H245Handler :: getRecodesMapIn ( FastStartElementVector & in ) {
	for ( FastStartElementVector :: size_type i = in.size ( ); i --; ) {
		FastStartElement & e = in [ i ];
		if ( outAllCodecs )
			e.recodeTo = findRecode ( e.codec, conf -> getRecodes ( ) );
		else {
			CodecInfoSet :: const_iterator f = findRecode ( e.codec, outAllowedCodecs,
				conf -> getRecodes ( ) );
			if ( f != outAllowedCodecs.end ( ) )
				e.recodeTo = * f;
		}
		if ( e.recodeTo.getCodec ( ) == "unknown" ) {
			in.erase ( in.begin ( ) + i );
			continue;
		}
		e.changeFrames = e.recodeTo.getFrames ( );
		originalFastStartIn [ e.recodeTo.getCodec ( ) ] = e;
	}
}

struct FastStartTemp {
	FastStartElement e;
	int origPref;
	int codecPref;
	FastStartTemp ( const FastStartElement & ee, int o, int c ) : e ( ee ), origPref ( o ), codecPref ( c ) { }
	FastStartTemp ( ) { }
};

void H245Handler :: getRecodesMapOut ( FastStartElementVector & out ) {
	typedef std :: map < ss :: string, FastStartTemp, std :: less < ss :: string >,
		__SS_ALLOCATOR < std :: pair < const ss :: string, FastStartTemp > > > StringFastStartTempMap;
	StringFastStartTempMap tmp;
	for ( FastStartElementVector :: size_type i = out.size ( ); i --; ) {
		FastStartElement e = out [ i ];
		StringStringSetMap :: const_iterator f = conf -> getBackRecodes ( ).find ( e.codec.getCodec ( ) );
		if ( f == conf -> getBackRecodes ( ).end ( ) ) {
			out.erase ( out.begin ( ) + i );
			continue;
		}
		int cp = 0;
		int chfr = 0;
		CodecInfoSet :: const_iterator ic = inAllowedCodecs.find ( e.codec );
		if ( ic != inAllowedCodecs.end ( ) && ic -> getFrames ( ) )
			chfr = ic -> getFrames ( );
		for ( StringSet :: const_iterator r = f -> second.begin ( ); r != f -> second.end ( ); ++ r, cp ++ ) {
			CodecInfo c;
			if ( outAllCodecs )
				c = * r;
			else {
				CodecInfoSet :: const_iterator j = outAllowedCodecs.find ( * r );
				if ( j != outAllowedCodecs.end ( ) )
					c = * j;
			}
			if ( c.getCodec ( ) == "unknown" )
				continue;
			e.recodeTo = c;
			e.changeFrames = chfr;
			e.outChangeFrames = c.getFrames ( );
			tmp [ c.getCodec ( ) ] = FastStartTemp ( e, int ( i ), cp );
		}
	}
	typedef std :: map < int, FastStartElement, std :: less < int >,
		__SS_ALLOCATOR < std :: pair < const int, FastStartElement > > > IntFastStartElementMap;
	typedef std :: map < int, IntFastStartElementMap, std :: less < int >,
		__SS_ALLOCATOR < std :: pair < const int, IntFastStartElementMap > > > IntIntFastStartElementMap;

	IntIntFastStartElementMap tmp2;
	for ( StringFastStartTempMap :: const_iterator i = tmp.begin ( ); i != tmp.end ( ); ++ i ) {
		const FastStartTemp & t = i -> second;
		tmp2 [ t.origPref ] [ t.codecPref ] = t.e;
		originalFastStartOut [ t.e.recodeTo.getCodec ( ) ] = t.e;
	}
	out.clear ( );
	for ( IntIntFastStartElementMap :: const_iterator i = tmp2.begin ( ); i != tmp2.end ( ); ++ i )
		for ( IntFastStartElementMap :: const_iterator j = i -> second.begin ( );
			j != i -> second.end ( ); ++ j )
			out.push_back ( j -> second );
}

void H245Handler :: getNoRecodesMapIn ( FastStartElementVector & fs ) {
	for ( unsigned i = 0; i < fs.size ( ); i ++ ) {
		CodecInfoSet :: const_iterator c = outAllowedCodecs.find ( fs [ i ].codec );
		if ( c != outAllowedCodecs.end ( ) && c -> getFrames ( ) )
			fs [ i ].changeFrames = c -> getFrames ( );
		originalFastStartIn [ fs [ i ].codec.getCodec ( ) ] = fs [ i ];
	}
}

void H245Handler :: getNoRecodesMapOut ( FastStartElementVector & fs ) {
	for ( unsigned i = 0; i < fs.size ( ); i ++ ) {
		CodecInfoSet :: const_iterator c = inAllowedCodecs.find ( fs [ i ].codec );
		if ( c != inAllowedCodecs.end ( ) && c -> getFrames ( ) )
			fs [ i ].changeFrames = c -> getFrames ( );
		c = outAllowedCodecs.find ( fs [ i ].codec );
		if ( c != outAllowedCodecs.end ( ) && c -> getFrames ( ) )
			fs [ i ].outChangeFrames = c -> getFrames ( );
		originalFastStartOut [ fs [ i ].codec.getCodec ( ) ] = fs [ i ];
	}
}

H225 :: ArrayOf_Asn_OctetString H245Handler :: handleFastStartNew ( const H225 :: ArrayOf_Asn_OctetString & fastStart ) {
	PSYSTEMLOG ( Info, "H245Handler :: handleFastStartNew" );
	FastStartElementVector fsin, fsout;
	originalFastStartIn.clear ( );
	originalFastStartOut.clear ( );
	for ( unsigned i = 0; i < fastStart.size ( ); i ++ ) {
		try {
			Asn :: istream is ( fastStart [ i ] );
			H245 :: OpenLogicalChannel open ( is );
			if ( is.hasException ( ) ) {
				PSYSTEMLOG ( Error, "fastStart: can't decode " << fastStart [ i ] << ": "
					<< is.getException ( ) );
				return H225 :: ArrayOf_Asn_OctetString ( );
			}
			H245 :: OpenLogicalChannel_forwardLogicalChannelParameters & forwardParams =
				open.m_forwardLogicalChannelParameters;
			H245 :: UnicastAddress_iPAddress * testIp;
			PIPSocket :: Address testAddr;
			switch ( forwardParams.m_dataType.getTag ( ) ) {
				case H245 :: DataType :: e_audioData: {
					if ( open.hasOptionalField (
						H245 :: OpenLogicalChannel :: e_reverseLogicalChannelParameters ) ) {
						PSYSTEMLOG ( Error, "forward & reverse - not supported" );
						continue;
					}
					if ( forwardParams.m_multiplexParameters.getTag ( ) != H245 ::
						OpenLogicalChannel_forwardLogicalChannelParameters_multiplexParameters
						:: e_h2250LogicalChannelParameters ) {
						PSYSTEMLOG ( Error,
							"not h2250LogicalChannelParameters - not supported" );
						continue;
					}
					H245 :: H2250LogicalChannelParameters & h225Params =
						forwardParams.m_multiplexParameters;
					if ( ! h225Params.hasOptionalField (
						H245 :: H2250LogicalChannelParameters :: e_mediaControlChannel ) ) {
						PSYSTEMLOG ( Error, "no rtcp - not supported" );
						continue;
					}
					if ( ! checkAddr ( h225Params.get_mediaControlChannel ( ),
						testIp, testAddr, true ) )
						continue;
					h225Params.removeOptionalField (
						H245 :: H2250LogicalChannelParameters :: e_mediaChannel );
					fsin.push_back ( FastStartElement ( open, forwardParams.m_dataType ) );
					continue;
				} case H245 :: DataType :: e_nullData:
					break;
				default:
					PSYSTEMLOG ( Error, "unknown dataType: " <<
						forwardParams.m_dataType.getTag ( ) );
					continue;
			}
			if ( ! open.hasOptionalField (
				H245 :: OpenLogicalChannel :: e_reverseLogicalChannelParameters ) ) {
				PSYSTEMLOG ( Error, "no forward or reverse params" );
				continue;
			}
			H245 :: OpenLogicalChannel_reverseLogicalChannelParameters & reverseParams =
				open.get_reverseLogicalChannelParameters ( );
			if ( reverseParams.m_dataType.getTag ( ) != H245 :: DataType :: e_audioData ) {
				PSYSTEMLOG ( Error, "unsupported reverse tag: "
					<< reverseParams.m_dataType.getTag ( ) );
				continue;
			}
			if ( ! reverseParams.hasOptionalField (
				H245 :: OpenLogicalChannel_reverseLogicalChannelParameters ::
				e_multiplexParameters ) ) {
				PSYSTEMLOG ( Error, "no multiplex parameters - not supported" );
				continue;
			}
			if ( reverseParams.get_multiplexParameters ( ).getTag ( ) !=
				H245 :: OpenLogicalChannel_reverseLogicalChannelParameters_multiplexParameters ::
				e_h2250LogicalChannelParameters ) {
				PSYSTEMLOG ( Error, "not h2250LogicalChannelParameters - not supported" );
				continue;
			}
			H245 :: H2250LogicalChannelParameters & h225Params =
				reverseParams.get_multiplexParameters ( );
			if ( ! h225Params.hasOptionalField (
				H245 :: H2250LogicalChannelParameters :: e_mediaControlChannel ) ) {
				PSYSTEMLOG ( Error, "no rtcp - not supported" );
				continue;
			}
			if ( ! h225Params.hasOptionalField (
				H245 :: H2250LogicalChannelParameters :: e_mediaChannel ) ) {
				PSYSTEMLOG ( Error, "no rtp - not supported" );
				continue;
			}
			if ( ! checkAddr ( h225Params.get_mediaControlChannel ( ), testIp, testAddr, true ) )
				continue;
			if ( ! checkAddr ( h225Params.get_mediaChannel ( ), testIp, testAddr, true ) )
				continue;
			fsout.push_back ( FastStartElement ( open, reverseParams.m_dataType ) );
		} catch ( std :: exception & e ) {
			PSYSTEMLOG ( Error, "fastStart: can't decode " << fastStart [ i ] << ": " << e.what ( ) );
			return H225 :: ArrayOf_Asn_OctetString ( );
		}
	}
	removeUnsupported ( fsin );
	removeUnsupported ( fsout );
	if ( fsin.empty ( ) || fsout.empty ( ) )
		return H225 :: ArrayOf_Asn_OctetString ( );
	FastStartElementVector tmp = fsin;
	removeReverseUnsupported ( tmp );
	if ( tmp.empty ( ) ) {
		if ( ! direct )
			getRecodesMapIn ( fsin );
		else
			fsin.swap ( tmp );
	} else {
		fsin.swap ( tmp );
		getNoRecodesMapIn ( fsin );
	}
	if ( fsin.empty ( ) )
		return H225 :: ArrayOf_Asn_OctetString ( );
	tmp = fsout;
	removeReverseUnsupported ( tmp );
	if ( tmp.empty ( ) ) {
		if ( ! direct )
			getRecodesMapOut ( fsout );
		else
			fsout.swap ( tmp );
	} else {
		fsout.swap ( tmp );
		getNoRecodesMapOut ( fsout );
	}
	if ( fsout.empty ( ) )
		return H225 :: ArrayOf_Asn_OctetString ( );
	Session * s = getRtpSession ( );
	int localRtcpPort = s -> getLocalAddress ( true, true );
	PIPSocket :: Address localAddr = getLocalAddress ( false );
	H225 :: ArrayOf_Asn_OctetString retFs;
	retFs.reserve ( fsin.size ( ) + fsout.size ( ) );
	for ( unsigned i = 0; i < fsin.size ( ); i ++ ) {
		changeDataTypeAndFramesIn ( fsin [ i ] );
		if ( ! direct ) {
			H245 :: H2250LogicalChannelParameters & h225Params =
				fsin [ i ].open.m_forwardLogicalChannelParameters.m_multiplexParameters;
			H245 :: UnicastAddress & unicastAddr = h225Params.get_mediaControlChannel ( );
			H245 :: UnicastAddress_iPAddress & ip = unicastAddr;
			setIPAddress ( ip, localAddr, localRtcpPort );
		}
		Asn :: ostream os;
		fsin [ i ].open.encode ( os );
		retFs.push_back ( os.str ( ) );
	}
	int localRtpPort = s -> getLocalAddress ( true, false );
	for ( unsigned i = 0; i < fsout.size ( ); i ++ ) {
		changeDataTypeAndFramesOut ( fsout [ i ] );
		if ( ! direct ) {
			H245 :: H2250LogicalChannelParameters & h225Params =
				fsout [ i ].open.get_reverseLogicalChannelParameters ( ).get_multiplexParameters ( );
			H245 :: UnicastAddress & rtcpUnicastAddr = h225Params.get_mediaControlChannel ( );
			H245 :: UnicastAddress_iPAddress & rtcpIp = rtcpUnicastAddr;
			setIPAddress ( rtcpIp, localAddr, localRtcpPort );
			H245 :: UnicastAddress & rtpUnicastAddr = h225Params.get_mediaChannel ( );
			H245 :: UnicastAddress_iPAddress & rtpIp = rtpUnicastAddr;
			setIPAddress ( rtpIp, localAddr, localRtpPort );
		}
		Asn :: ostream os;
		fsout [ i ].open.encode ( os );
		retFs.push_back ( os.str ( ) );
	}
	return retFs;
}

static void getRtpCodec ( const H245 :: AudioCapability & audioData, int & rtpCodec ) {
	rtpCodec = 0;
	switch ( audioData.getTag ( ) ) {
		case H245 :: AudioCapability :: e_g711Alaw64k:
		case H245 :: AudioCapability :: e_g711Ulaw64k:
			rtpCodec = 2;
			break;
		case H245 :: AudioCapability :: e_g711Alaw56k:
		case H245 :: AudioCapability :: e_g711Ulaw56k:
			rtpCodec = 1;
			break;
		case H245 :: AudioCapability :: e_g7231:
			rtpCodec = 4;
			break;
		case H245 :: AudioCapability :: e_g729wAnnexB:
		case H245 :: AudioCapability :: e_g729AnnexAwAnnexB:
		case H245 :: AudioCapability :: e_g729:
		case H245 :: AudioCapability :: e_g729AnnexA:
			rtpCodec = 3;
			break;
	}
}

bool H245Handler :: handleFastStartResponseNew ( H225 :: ArrayOf_Asn_OctetString & fastStart ) {
	PSYSTEMLOG ( Info, "H2425Handler :: handleFastStartResponseNew" );
	FastStartElementVector fsin, fsout;
	for ( std :: size_t i = 0, fsz = fastStart.size ( ); i < fsz; i ++ ) {
		try {
			Asn :: istream is ( fastStart [ i ] );
			H245 :: OpenLogicalChannel open ( is );
			if ( is.hasException ( ) ) {
				PSYSTEMLOG ( Error, "fastStart: can't decode " << fastStart [ i ] << ": "
					<< is.getException ( ) );
				return false;
			}
			H245 :: OpenLogicalChannel_forwardLogicalChannelParameters & forwardParams =
				open.m_forwardLogicalChannelParameters;
			H245 :: UnicastAddress_iPAddress * testIp;
			PIPSocket :: Address testAddr;
			switch ( forwardParams.m_dataType.getTag ( ) ) {
				case H245 :: DataType :: e_audioData: {
					if ( open.hasOptionalField (
						H245 :: OpenLogicalChannel :: e_reverseLogicalChannelParameters ) ) {
						PSYSTEMLOG ( Error, "forward & reverse - not supported" );
						continue;
					}
					if ( forwardParams.m_multiplexParameters.getTag ( ) != H245 ::
						OpenLogicalChannel_forwardLogicalChannelParameters_multiplexParameters
						:: e_h2250LogicalChannelParameters ) {
						PSYSTEMLOG ( Error,
							"not h2250LogicalChannelParameters - not supported" );
						continue;
					}
					H245 :: H2250LogicalChannelParameters & h225Params =
						forwardParams.m_multiplexParameters;
					if ( ! h225Params.hasOptionalField (
						H245 :: H2250LogicalChannelParameters :: e_mediaControlChannel ) ) {
						PSYSTEMLOG ( Error, "no rtcp - not supported" );
						continue;
					}
					if ( ! h225Params.hasOptionalField (
						H245 :: H2250LogicalChannelParameters :: e_mediaChannel ) ) {
						PSYSTEMLOG ( Error, "no rtp - not supported" );
						continue;
					}
					if ( ! checkAddr ( h225Params.get_mediaControlChannel ( ),
						testIp, testAddr, false ) )
						continue;
					if ( ! checkAddr ( h225Params.get_mediaChannel ( ),
						testIp, testAddr, false ) )
						continue;
					fsin.push_back ( FastStartElement ( open, forwardParams.m_dataType ) );
					continue;
				} case H245 :: DataType :: e_nullData:
					break;
				default:
					PSYSTEMLOG ( Error, "unknown dataType: " <<
						forwardParams.m_dataType.getTag ( ) );
					continue;
			}
			if ( ! open.hasOptionalField (
				H245 :: OpenLogicalChannel :: e_reverseLogicalChannelParameters ) ) {
				PSYSTEMLOG ( Error, "no forward or reverse params" );
				continue;
			}
			H245 :: OpenLogicalChannel_reverseLogicalChannelParameters & reverseParams =
				open.get_reverseLogicalChannelParameters ( );
			if ( reverseParams.m_dataType.getTag ( ) != H245 :: DataType :: e_audioData ) {
				PSYSTEMLOG ( Error, "unsupported reverse tag: " <<
					reverseParams.m_dataType.getTag ( ) );
				continue;
			}
			if ( ! reverseParams.hasOptionalField ( H245 ::
				OpenLogicalChannel_reverseLogicalChannelParameters :: e_multiplexParameters ) ) {
				PSYSTEMLOG ( Error, "no multiplexParameters - not supported" );
				continue;
			}
			if ( reverseParams.get_multiplexParameters ( ).getTag ( ) !=
				H245 :: OpenLogicalChannel_reverseLogicalChannelParameters_multiplexParameters ::
				e_h2250LogicalChannelParameters ) {
				PSYSTEMLOG ( Error, "not h2250LogicalChannelParameters - not supported" );
				continue;
			}
			H245 :: H2250LogicalChannelParameters & h225Params =
				reverseParams.get_multiplexParameters ( );
			if ( ! h225Params.hasOptionalField (
				H245 :: H2250LogicalChannelParameters :: e_mediaControlChannel ) ) {
				PSYSTEMLOG ( Error, "no rtcp - not supported" );
				continue;
			}
			if ( ! checkAddr ( h225Params.get_mediaControlChannel ( ), testIp, testAddr, false ) )
				continue;
			h225Params.removeOptionalField ( H245 :: H2250LogicalChannelParameters :: e_mediaChannel );
			fsout.push_back ( FastStartElement ( open, reverseParams.m_dataType ) );
		} catch ( std :: exception & e ) {
			PSYSTEMLOG ( Error, "fastStart: can't decode " << fastStart [ i ] << ": " << e.what ( ) );
			return false;
		}
	}
	if ( fsin.size ( ) != 1 || fsout.size ( ) != 1 )
		return false;
	FastStartElement & ie = fsin [ 0 ];
	FastStartMap :: const_iterator ii = originalFastStartIn.find ( ie.codec.getCodec ( ) );
	if ( ii == originalFastStartIn.end ( ) ) {
		PSYSTEMLOG ( Error, "unknown channel in answer: " << ie.open.m_forwardLogicalChannelNumber );
		return false;
	}
	FastStartElement & oe = fsout [ 0 ];
	FastStartMap :: iterator oi = originalFastStartOut.find ( oe.codec.getCodec ( ) );
	if ( oi == originalFastStartOut.end ( ) ) {
		PSYSTEMLOG ( Error, "unknown codec in answer: " << oe.codec.getCodec ( ) );
		return false;
	}
	codec = ie.codec;
	const FastStartElement & oie = ii -> second;
	Session * s = getRtpSession ( );
	fastStart.clear ( );
	PIPSocket :: Address localAddr = getLocalAddress ( true );
	int localRtcpPort = 0;
	if ( ! direct ) {
		localRtcpPort = s -> getLocalAddress ( false, true );
		int localRtpPort = s -> getLocalAddress ( false, false );
		H245 :: UnicastAddress_iPAddress * ip;
		PIPSocket :: Address addr;
		int port;
		getControlAddr ( ie.open.m_forwardLogicalChannelParameters.m_multiplexParameters, addr, port, ip );
		s -> setSendAddress ( false, true, addr, WORD ( port ), 0, 0, false, "", "" );
		setIPAddress ( * ip, localAddr, localRtcpPort );
		int rtpCodec = 0, rtpFrames = oie.changeFrames;
		bool sendCodec = rtpFrames;
		if ( sendCodec ) {
			getRtpCodec ( ie.open.m_forwardLogicalChannelParameters.m_dataType, rtpCodec );
			sendCodec = rtpCodec;
		}
		ss :: string recodeTo, recodeFrom;
		if ( oie.recodeTo.getCodec ( ) != "unknown" ) {
			recodeFrom = oie.recodeTo.getCodec ( );
			recodeTo = oie.codec.getCodec ( );
		}
		getMediaAddr ( ie.open.m_forwardLogicalChannelParameters.m_multiplexParameters, addr, port, ip );
		differentCodec = isDifferentCodec ( recodeTo, recodeFrom );
		s -> setSendAddress ( false, false, addr, WORD ( port ), rtpCodec, rtpFrames, sendCodec, recodeTo, recodeFrom );
		setIPAddress ( * ip, localAddr, localRtpPort );
	}
	ie.open.m_forwardLogicalChannelParameters.m_dataType = oie.open.m_forwardLogicalChannelParameters.m_dataType;
	Asn :: ostream os;
	ie.open.encode ( os );
	fastStart.push_back ( os.str ( ) );
	FastStartElement & ooe = oi -> second;
	oe.open.get_reverseLogicalChannelParameters ( ).m_dataType =
		ooe.open.get_reverseLogicalChannelParameters ( ).m_dataType;
	if ( ! direct ) {
		if ( ooe.changeFrames )
			setFrames ( oe.open.get_reverseLogicalChannelParameters ( ).m_dataType, ooe.changeFrames );
		H245 :: UnicastAddress_iPAddress * ip;
		PIPSocket :: Address addr;
		int port;
		getControlAddr ( oe.open.get_reverseLogicalChannelParameters ( ).get_multiplexParameters ( ),
			addr, port, ip );
		setIPAddress ( * ip, localAddr, localRtcpPort );
		getControlAddr ( ooe.open.get_reverseLogicalChannelParameters ( ).get_multiplexParameters ( ),
			addr, port, ip );
		s -> setSendAddress ( true, true, addr, WORD ( port ), 0, 0, false, "", "" );
		int rtpCodec = 0, rtpFrames = ooe.changeFrames;
		bool sendCodec = rtpFrames;
		if ( sendCodec ) {
			getRtpCodec ( oe.open.get_reverseLogicalChannelParameters ( ).m_dataType, rtpCodec );
			sendCodec = rtpCodec;
		}
		ss :: string recodeTo, recodeFrom;
		if ( ooe.recodeTo.getCodec ( ) != "unknown" ) {
			recodeFrom = ooe.codec.getCodec ( );
			recodeTo = ooe.recodeTo.getCodec ( );
		}
		getMediaAddr ( ooe.open.get_reverseLogicalChannelParameters ( ).get_multiplexParameters ( ),
			addr, port, ip );
		differentCodec = isDifferentCodec ( recodeTo, recodeFrom );
		s -> setSendAddress ( true, false, addr, WORD ( port ), rtpCodec, rtpFrames, sendCodec, recodeTo, recodeFrom );
	}
	Asn :: ostream os2;
	oe.open.encode ( os2 );
	fastStart.push_back ( os2.str ( ) );
	return true;
}

const CodecInfo & H245Handler :: getCodec ( ) {
	return codec;
}

void H245Handler :: setOutPeer ( int op, bool oa, const CodecInfoSet & oac, bool oatrtdr ) {
	outPeer = op;
	outAllCodecs = oa;
	outAllowedCodecs = oac;
	outAnswerToRTDR = oatrtdr;
}

bool H245Handler :: getCallerBreak ( ) const {
	return callerBreak;
}

bool H245Handler :: getCalledBreak ( ) const {
	return calledBreak;
}

std :: ostream & operator<< ( std :: ostream & os, const RequestElement & e ) {
	return os << "dataType: " << e.dataType << ", recodeFrom: " << e.recodeFrom << ", recodeTo: " << e.recodeTo;
}

RequestElement :: RequestElement ( const H245 :: DataType & d ) :
	dataType ( d ), recodeFrom ( :: getCodec ( d ).getCodec ( ) ), rtpCodec ( 0 ), rtpFrames ( 0 ) { }
