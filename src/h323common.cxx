#pragma implementation
#pragma implementation "asn.hpp"
#include "ss.hpp"
#include "allocatable.hpp"
#include <iomanip>
#include <stdexcept>
#include <ptlib.h>
#include <limits>
#include <cstring>
#include "asn.hpp"
#include "h225.hpp"
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/sequenced_index.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/member.hpp>
#include "q931.hpp"
#include "tarifinfo.hpp"
#include "tarifround.hpp"
#include "pricedata.hpp"
#include "sourcedata.hpp"
#include "signallingoptions.hpp"
#include "codecinfo.hpp"
#include "outcarddetails.hpp"
#include "outchoicedetails.hpp"
#include "pointer.hpp"
#include "aftertask.hpp"
#include "moneyspool.hpp"
#include <tr1/functional>
#include "smartguard.hpp"
#include <tr1/memory>
#include "commoncalldetails.hpp"
#include <ptlib/sockets.h>
#include "h323common.hpp"
#include "h323.hpp"
#include "ipport.hpp"
#include "icqcontact.hpp"
#include "radgwinfo.hpp"
#include "Conf.hpp"
#include "rasthread.hpp"
#include "AddrUtils.h"
#include "Log.h"
#include <ptlib/svcproc.h>

ss :: string h225ProtocolID = "0.0.8.2250.0.4"; //!!!!! or .3 ?

CodecInfo getCodec ( const H245 :: DataType & dataType ) {
	switch ( int t = dataType.getTag ( ) ) {
		case H245 :: DataType :: e_audioData: {
			const H245 :: AudioCapability & audioData = dataType;
			return getCodec ( audioData );
		} case H245 :: DataType :: e_data: {
			const H245 :: DataApplicationCapability & data = dataType;
			switch ( int t = data.m_application.getTag ( ) ) {
				case H245 :: DataApplicationCapability_application :: e_t38fax:
					return "t38fax";
				default:
					PSYSTEMLOG ( Error, "unknown application:" << t );
			}
			break;
		} case H245 :: DataType :: e_nullData:
			break;
		default:
			PSYSTEMLOG ( Error, "unknown dataType:" << t );
	}
	return "unknown";
}

CodecInfo getCodec ( const H245 :: AudioCapability & audioData ) {
	switch ( audioData.getTag ( ) ) {
		case H245 :: AudioCapability :: e_nonStandard: {
			const H245 :: NonStandardParameter & nonStandard = audioData;
			const H245 :: NonStandardIdentifier & id = nonStandard.m_nonStandardIdentifier;
			unsigned tag = id.getTag ( );
			if ( tag == H245 :: NonStandardIdentifier :: e_h221NonStandard ) {
				const H245 :: NonStandardIdentifier_h221NonStandard & h221 = id;
				const ss :: string & data = nonStandard.m_data;
				if ( h221.m_t35CountryCode == 181 && h221.m_t35Extension == 0 && h221.m_manufacturerCode == 18 ) {
					if ( data == "ClearChid" )
						return "clear-channel";
					if ( data == "G7231ar" )
						return "g723ar";
					if ( data == "G7231ar53" )
						return "g723ar53";
					if ( data == "G7231ar63" )
						return "g723ar63";
					if ( data == "G726r16" )
						return "g726r16";
					if ( data == "G726r24" )
						return "g726r24";
					if ( data == "G726r32" )
						return "g726r32";
				} else if ( h221.m_t35CountryCode == 9 && h221.m_t35Extension == 0 && h221.m_manufacturerCode == 61 ) {
					if ( ! data.compare ( 0, 5, "Speex" ) )
						return "Speex";
					if ( ! data.compare ( 0, 6, "G.726-" ) )
						return "g726";
					if ( data == "LPC-10" )
						return "LPC-10";
				} else if ( h221.m_t35CountryCode == 181 && h221.m_t35Extension == 0 && h221.m_manufacturerCode == 21324 )
					return "necrosoft";
				PSYSTEMLOG ( Error, "nonStandard string: " << data );
				break;
			} else if ( tag == H245 :: NonStandardIdentifier :: e_object ) {
				const Asn :: ObjectId & obj = id;
				if ( obj.str ( ) == "0.7.43.6.1.4.1.2516" ) {
					const ss :: string & data = nonStandard.m_data;
					if ( ! data.compare ( 0, 4, "VH96" ) )
						return "VH96";
					if ( ! data.compare ( 0, 4, "VH88" ) )
						return "VH88";
					if ( ! data.compare ( 0, 4, "VH80" ) )
						return "VH80";
					if ( ! data.compare ( 0, 4, "VH72" ) )
						return "VH72";
					if ( ! data.compare ( 0, 4, "VH64" ) )
						return "VH64";
					PSYSTEMLOG ( Error, "unknown object data: " << data );
					break;
				}
				PSYSTEMLOG ( Error, "unknown object id: " << obj );
				break;
			}
			PSYSTEMLOG ( Error, "nonStandardIdentifier.tag: " << id.getTag ( ) );
			break;
		} case H245 :: AudioCapability :: e_g711Alaw64k: {
			const H245 :: AudioCapability_g711Alaw64k & c = audioData;
			return CodecInfo ( "g711alaw", c );
		} case H245 :: AudioCapability :: e_g711Alaw56k: {
			const H245 :: AudioCapability_g711Alaw56k & c = audioData;
			return CodecInfo ( "g711alaw", c );
		} case H245 :: AudioCapability :: e_g711Ulaw64k: {
			const H245 :: AudioCapability_g711Ulaw64k & c = audioData;
			return CodecInfo ( "g711ulaw", c );
		} case H245 :: AudioCapability :: e_g711Ulaw56k: {
			const H245 :: AudioCapability_g711Ulaw56k & c = audioData;
			return CodecInfo ( "g711ulaw", c );
		} case H245 :: AudioCapability :: e_g7231: {
			const H245 :: AudioCapability_g7231 & c = audioData;
			return CodecInfo ( "g7231", c.m_maxAl_sduAudioFrames );
		} case H245 :: AudioCapability :: e_g728:
			return "g728";
		case H245 :: AudioCapability :: e_g729wAnnexB: {
			const H245 :: AudioCapability_g729wAnnexB & c = audioData;
			return CodecInfo ( "g729br8", c );
		} case H245 :: AudioCapability :: e_g729AnnexAwAnnexB: {
			const H245 :: AudioCapability_g729AnnexAwAnnexB & c = audioData;
			return CodecInfo ( "g729abr8", c );
		} case H245 :: AudioCapability :: e_g729: {
			const H245 :: AudioCapability_g729 & c = audioData;
			return CodecInfo ( "g729r8", c );
		} case H245 :: AudioCapability :: e_g729AnnexA: {
			const H245 :: AudioCapability_g729AnnexA & c = audioData;
			return CodecInfo ( "g729ar8", c );
		} case H245 :: AudioCapability :: e_gsmEnhancedFullRate:
			return "gsmefr";
		case H245 :: AudioCapability :: e_gsmFullRate:
			return "gsmfr";
		default:
			PSYSTEMLOG ( Error, "audioData.tag: " << audioData.getTag ( ) );
	}
	return "unknown";
}

FastStartElement :: FastStartElement ( const H245 :: OpenLogicalChannel & o, const H245 :: AudioCapability & c,
	int ch ) : open ( o ), codec ( :: getCodec ( c ) ), changeFrames ( ch ), outChangeFrames ( 0 ) { }

static void calledSocketConnected ( PTCPSocket * sock ) {
	if ( ! sock -> SetOption ( SO_KEEPALIVE, 1 ) )
		PSYSTEMLOG ( Error, "SO_KEEPALIVE: " << sock -> GetErrorNumber ( ) <<
			' ' << sock -> GetErrorText ( ) );
	sock -> SetReadTimeout ( 3000 );
}

bool tryNextChoiceIterationH323 ( CommonCallDetails & common, PTCPSocket * & calledSocket ) {
	PIPSocket :: Address calledIp ( common.getCalledIp ( ).c_str ( ) );
	int calledPort = common.getCalledPort ( );
	if ( common.getIsGk ( ) && ! conf -> getAddrFromGk ( PIPSocket :: Address ( common.getCalledIp ( ).c_str ( ) ),
		common.getSentDigits ( ), calledIp, calledPort ) )
		return false;
	const OutChoiceDetails & choice = * common.curChoice ( );
	if ( choice.getFromNat ( ) ) {
		PTCPSocket tsock;
		tsock.SetReadTimeout ( 7000 );
		if ( ! tsock.Listen ( ) ) {
			PSYSTEMLOG ( Error, "Failed to listen on nat connect socket " <<
				tsock.GetErrorText ( ) );
			return false;
		}
		H225 :: RasMessage msg;
		msg.setTag ( H225 :: RasMessage :: e_infoRequest );
		H225 :: InfoRequest & irq = msg;
		irq.includeOptionalField ( H225 :: InfoRequest :: e_tokens );
		H225 :: ArrayOf_ClearToken & tokens = irq.get_tokens ( );
		tokens.setSize ( 1 );
		PIPSocket :: Address addr;
		WORD port;
		tsock.GetLocalAddress ( addr, port );
		Asn :: ostringstream os;
		os << addr.AsString ( ) << ':' << port;
		tokens [ 0 ].includeOptionalField ( H235 :: ClearToken :: e_challenge );
		tokens [ 0 ].get_challenge ( ) = os.str ( );
		tokens [ 0 ].m_tokenOID = "0.0.0.0";
		if ( ! rasThread -> writeRasReply ( * choice.getSocket ( ),
			AddrUtils :: convertToH225TransportAddr ( calledIp, WORD ( calledPort ) ), msg ) ) {
			PSYSTEMLOG ( Error, "Failed to send nat connect message " << calledIp );
			return false;
		}
		calledSocket = new PTCPSocket ( WORD ( calledPort ) );
		calledSocket -> SetReadTimeout ( 7000 );
		if ( ! calledSocket -> Accept ( tsock ) ) {
			PSYSTEMLOG ( Error, "Failed to accept nat connect " << calledIp <<
				" : " << calledSocket -> GetErrorText ( ) );
			delete calledSocket;
			calledSocket = 0;
			return false;
		}
		calledSocketConnected ( calledSocket );
		return true;
	}
	calledSocket = new PTCPSocket ( WORD ( calledPort ) );
	calledSocket -> SetReadTimeout ( 7000 );
	const IntVector & localIps = choice.getInterfaces ( );
	for ( unsigned i = 0; i < localIps.size ( ); i ++ ) {
		PSYSTEMLOG ( Info, "local ip: " << PIPSocket :: Address ( localIps [ i ] ) );
		if ( ! calledSocket -> Connect ( PIPSocket :: Address ( localIps [ i ] ), calledIp ) )
			continue;
		calledSocketConnected ( calledSocket );
		return true;
	}
	PSYSTEMLOG ( Error, "Failed to connect to call signalling channel at "
		<< calledIp << " : " << calledSocket -> GetErrorText ( ) );
	delete calledSocket;
	calledSocket = 0;
	return false;
}

void setIPAddress ( H245 :: UnicastAddress_iPAddress & ip, const PIPSocket :: Address & addr, int port ) {
	ss :: string t = ip.m_network;
	for ( int i = 0; i < 4; i ++ )
		t [ i ] = addr [ i ];
	ip.m_network = t;
	ip.m_tsapIdentifier = port;
}

void encodeH225IntoQ931 ( const H225 :: H323_UserInformation & uuField, Q931 & mesg ) {
	Asn :: ostream os;
	uuField.encode ( os );
	mesg.setIE ( Q931 :: ieUserUser, os.str ( ) );
}

static void sendMsg ( const ss :: string & stream, PTCPSocket * destination ) {
	destination -> SetWriteTimeout ( 1000 );
	destination -> Write ( stream.data ( ), PINDEX ( stream.size ( ) ) );
	//PSYSTEMLOG ( Info, "SendMsg4: " << stream.c_str() );
}

void sendMesgToDest ( const Q931 & mesg, const H225 :: H323_UserInformation * uuField,
	PTCPSocket * destination, const CommonCallDetails & common ) {
	PIPSocket :: Address peerAddr;
	WORD peerPort;
	if ( ! destination -> GetPeerAddress ( peerAddr, peerPort ) ) {
		PSYSTEMLOG ( Error, "Can't get peer address, not sending: " << destination -> GetErrorText ( ) );
		return;
	}
	Log -> logQ931Msg ( mesg, uuField, OpengateLog :: Sending, peerAddr, peerPort, common.getForceDebug ( ) );
	sendMsg ( mesg.tpkt ( ), destination );
}

ss :: string makeTpkt ( const ss :: string & s ) {
	char tpkt [ 4 ];
	tpkt [ 0 ] = 3;
	tpkt [ 1 ] = 0;
	ss :: string :: size_type packetLength = s.size ( ) + 4;
	if ( packetLength > 65535 )
		throw std :: runtime_error ( "too long string in makeTpkt" );
	tpkt [ 2 ] = char ( packetLength >> 8 );
	tpkt [ 3 ] = char ( packetLength );
	ss :: string t;
	t.reserve ( packetLength );
	t.assign ( tpkt, 4 );
	t += s;
	return t;
}

void sendH245MesgToDest ( const ss :: string & s, const H245 :: MultimediaSystemControlMessage & mesg,
	PTCPSocket * destination, bool forceDebug ) {
// Task: to send the given H.245 message to the given destination
	PIPSocket :: Address peerAddr;
	WORD peerPort;
	destination -> GetPeerAddress ( peerAddr, peerPort );
	Log -> logH245Msg ( mesg, OpengateLog::Sending, peerAddr, peerPort, forceDebug );
	sendMsg ( s, destination );
}

void translateSetup ( const CommonCallDetails & common, const Q931 & orig, const H225 :: CallIdentifier & id,
	Q931 & mesg, H225 :: H323_UserInformation & uuField, const H225 :: TransportAddress & sourceCallSigAddr,
	const H225 :: TransportAddress & destCallSigAddr ) {
	H225 :: H323_UU_PDU & pdu = uuField.m_h323_uu_pdu;
	H225 :: H323_UU_PDU_h323_message_body & body = pdu.m_h323_message_body;
	H225 :: Setup_UUIE & setup = body;
	setup.includeOptionalField ( H225 :: Setup_UUIE :: e_canOverlapSend );
	setup.get_canOverlapSend ( ) = false;
	setup.includeOptionalField ( H225 :: Setup_UUIE :: e_sourceCallSignalAddress );
	setup.get_sourceCallSignalAddress ( ) = sourceCallSigAddr;
	ss :: string sentDigits = common.getSentDigits ( );
	mesg.setCalledPartyNumber ( sentDigits );
	if ( common.getSigOptions ( ).addDestinationAddress ( ) ) {
		setup.includeOptionalField ( H225 :: Setup_UUIE :: e_destinationAddress );
		setup.get_destinationAddress ( ).setSize ( 1 );
		H323 :: setAliasAddress ( sentDigits, setup.get_destinationAddress ( ) [ 0 ] );
		setup.includeOptionalField ( H225 :: Setup_UUIE :: e_destCallSignalAddress );
		setup.get_destCallSignalAddress ( ) = destCallSigAddr;
	}
	if ( ! common.getCallingDigits ( ).empty ( ) ) {
		if ( ! common.getCallingDigits ( ) [ 0 ] || common.getCallingDigits ( ) == "-" ) {
			setup.removeOptionalField ( H225 :: Setup_UUIE :: e_sourceAddress );
			mesg.removeIE ( Q931 :: ieCallingPartyNumber );
		} else {
			setup.includeOptionalField ( H225 :: Setup_UUIE :: e_sourceAddress );
			setup.get_sourceAddress ( ).setSize ( 1 );
			H323 :: setAliasAddress ( common.getCallingDigits ( ), setup.get_sourceAddress ( ) [ 0 ] );
			mesg.setCallingPartyNumber ( common.getCallingDigits ( ) );
		}
	}
	setup.removeOptionalField ( H225 :: Setup_UUIE :: e_h245Address );

	if ( common.getSigOptions ( ).dropNonStandard ( ) )
		dropNonStandard ( uuField );
	handleCallId ( setup, id );
	dropTokens ( setup );
	const OutCardDetails & ocd = common.curChoice ( ) -> getOutCardDetails ( );
	int remotePrice = ocd.getRedirect ( ) ? ocd.getPrice ( ) : common.getSource ( ).price;
	if ( remotePrice && common.getOutRemotePrice ( ) ) {
		pdu.includeOptionalField ( H225 :: H323_UU_PDU :: e_nonStandardControl );
		pdu.get_nonStandardControl ( ).setSize ( 1 );
		H225 :: NonStandardParameter & parm = pdu.get_nonStandardControl ( ) [ 0 ];
		H225 :: NonStandardIdentifier & id = parm.m_nonStandardIdentifier;
		id.setTag ( H225 :: NonStandardIdentifier :: e_h221NonStandard );
		H225 :: H221NonStandard & h221 = id;
		h221.m_t35CountryCode = 181;
		h221.m_t35Extension = 0;
		h221.m_manufacturerCode = 666;
		Asn :: ostringstream os;
		os << "p " << remotePrice << ' ' << ( ocd.getRedirect ( ) ? ocd.getCode ( ) : common.getSource ( ).code );
		parm.m_data = os.str ( );
	}
	handleMultipleCalls ( setup );
	conf -> translateProtoClass ( common.curChoice ( ) -> getPeer ( ), orig, mesg );
}

void sendReleaseComplete ( PTCPSocket * sock, CommonCallDetails & common, const H225 :: CallIdentifier & id, int ref,
	bool fromCaller ) {
//	common.setDisconnectCauseWeak ( Q931 :: cvNoRouteToDestination );
	common.setDisconnectCauseWeak ( conf -> getDefaultDisconnectCause ( )
		/*Q931 :: cvNoCircuitChannelAvailable*/ );
	Q931 rc ( Q931 :: msgReleaseComplete, ref, fromCaller );
	rc.setCause ( Q931 :: CauseValues ( common.getDisconnectCause ( ) ) );
	H225 :: H323_UserInformation uuField;
	H225 :: H323_UU_PDU & pdu = uuField.m_h323_uu_pdu;
	pdu.m_h323_message_body.setTag ( H225 :: H323_UU_PDU_h323_message_body :: e_releaseComplete );
	H225 :: ReleaseComplete_UUIE & complete = pdu.m_h323_message_body;
	complete.m_protocolIdentifier = h225ProtocolID;
	complete.get_callIdentifier ( ) = id;
	encodeH225IntoQ931 ( uuField, rc );
	sendMesgToDest ( rc, & uuField, sock, common );
}

void dropNonStandard ( H225 :: H323_UserInformation & uuField ) {
	H225 :: H323_UU_PDU & pdu = uuField.m_h323_uu_pdu;
	pdu.removeOptionalField ( H225 :: H323_UU_PDU :: e_nonStandardControl );
	pdu.removeOptionalField ( H225 :: H323_UU_PDU :: e_nonStandardData );
	pdu.removeOptionalField ( H225 :: H323_UU_PDU :: e_tunnelledSignallingMessage );
	uuField.removeOptionalField ( H225 :: H323_UserInformation :: e_user_data );
}

bool admissiblePort ( int /*port*/ ) {
	return true; //port > 1024;
}

static void getAddr ( H245 :: UnicastAddress & unicastAddr, PIPSocket :: Address & addr, int & port,
	H245 :: UnicastAddress_iPAddress * & ip ) {
	ip = & static_cast < H245 :: UnicastAddress_iPAddress & > ( unicastAddr );
	const ss :: string & nw = ip -> m_network;
	addr = PIPSocket :: Address ( nw [ 0 ], nw [ 1 ], nw [ 2 ], nw [ 3 ] );
	port = ip -> m_tsapIdentifier;
}

void getControlAddr ( H245 :: H2250LogicalChannelParameters & h225p, PIPSocket :: Address & addr,
	int & port, H245 :: UnicastAddress_iPAddress * & ip ) {
	if ( ! h225p.hasOptionalField ( H245 :: H2250LogicalChannelParameters :: e_mediaControlChannel ) )
		throw std :: runtime_error ( "no mediaControlChannel" );
	getAddr ( h225p.get_mediaControlChannel ( ), addr, port, ip );
}

void getMediaAddr ( H245 :: H2250LogicalChannelParameters & h225p, PIPSocket :: Address & addr,
	int & port, H245 :: UnicastAddress_iPAddress * & ip ) {
	if ( ! h225p.hasOptionalField ( H245 :: H2250LogicalChannelParameters :: e_mediaChannel ) )
		throw std :: runtime_error ( "no mediaChannel" );
	getAddr ( h225p.get_mediaChannel ( ), addr, port, ip );
}

Q931 :: CauseValues getCause ( const Q931 & mesg, const H225 :: H323_UserInformation * uuField ) {
	Q931 :: CauseValues cause = mesg.getCause ( );
	if ( cause != Q931 :: cvError || ! uuField )
		return cause;
	const H225 :: H323_UU_PDU & pdu = uuField -> m_h323_uu_pdu;
	const H225 :: H323_UU_PDU_h323_message_body & body = pdu.m_h323_message_body;
	if ( body.getTag ( ) != H225 :: H323_UU_PDU_h323_message_body :: e_releaseComplete )
		return cause;
	const H225 :: ReleaseComplete_UUIE & release = body;
	if ( ! release.hasOptionalField ( H225 :: ReleaseComplete_UUIE :: e_reason ) )
		return cause;
	switch ( release.get_reason ( ).getTag ( ) ) {
		case H225 :: ReleaseCompleteReason :: e_noBandwidth:
			return conf -> getDefaultDisconnectCause(); //Q931 :: cvNoCircuitChannelAvailable; // ?????????
		case H225 :: ReleaseCompleteReason :: e_gatekeeperResources:
		case H225 :: ReleaseCompleteReason :: e_newConnectionNeeded:
			return Q931 :: cvResourceUnavailable;
		case H225 :: ReleaseCompleteReason :: e_unreachableDestination:
			return Q931 :: cvNoRouteToDestination;
		case H225 :: ReleaseCompleteReason :: e_destinationRejection:
		case H225 :: ReleaseCompleteReason :: e_facilityCallDeflection:
			return Q931 :: cvNormalCallClearing;
		case H225 :: ReleaseCompleteReason :: e_invalidRevision:
			return Q931 :: cvIncompatibleDestination;
		case H225 :: ReleaseCompleteReason :: e_noPermission:
			return Q931 :: cvInterworkingUnspecified;
		case H225 :: ReleaseCompleteReason :: e_unreachableGatekeeper:
			return Q931 :: cvNetworkOutOfOrder;
		case H225 :: ReleaseCompleteReason :: e_gatewayResources:
			return Q931 :: cvCongestion;
		case H225 :: ReleaseCompleteReason :: e_badFormatAddress:
			return Q931 :: cvInvalidNumberFormat;
		case H225 :: ReleaseCompleteReason :: e_adaptiveBusy:
			return Q931 :: cvTemporaryFailure;
		case H225 :: ReleaseCompleteReason :: e_inConf:
			return Q931 :: cvUserBusy;
		case H225 :: ReleaseCompleteReason :: e_undefinedReason:
		case H225 :: ReleaseCompleteReason :: e_replaceWithConferenceInvite:
		case H225 :: ReleaseCompleteReason :: e_securityDenied:
		case H225 :: ReleaseCompleteReason :: e_callerNotRegistered:
		case H225 :: ReleaseCompleteReason :: e_genericDataReason:
		case H225 :: ReleaseCompleteReason :: e_neededFeatureNotSupported:
			return Q931 :: cvNormalUnspecified;
		case H225 :: ReleaseCompleteReason :: e_calledPartyNotRegistered:
			return Q931 :: cvSubscriberAbsent;
		case H225 :: ReleaseCompleteReason :: e_nonStandardReason:
		case H225 :: ReleaseCompleteReason :: e_tunnelledSignallingRejected:
			return Q931 :: cvNonStandardReason;
	}
	return cause;
}

Q931 readMsg ( PTCPSocket * socket ) {
	char c = 0;
	if ( ! socket -> ReadBlock ( & c, 1 ) ) {
		std :: ostringstream os;
		os << "Can't read: " << socket -> GetErrorText ( );
		throw std :: runtime_error ( os.str ( ) );
	}
	if ( c != 3 )
		throw std :: runtime_error ( "Invalid call signalling message, not a TKPT." );
	unsigned char header [ 3 ];
	if ( ! socket -> ReadBlock ( header, sizeof ( header ) ) )
		throw std :: runtime_error ( "Failed to read TPKT header" );
	int bufferSize = ( ( header [ 1 ] << 8 ) | header [ 2 ] ) - 4;
	if ( bufferSize <= 0 )
		throw std :: runtime_error ( "Invalid bufferSize" );
	ss :: string buffer ( bufferSize, '\0' );
	if ( socket -> ReadBlock ( ( void * ) & buffer [ 0 ], bufferSize ) ) {
//		sqlOut -> add ( "insert into Q931Dump values ( 0, '" + MySQL :: escape ( buffer ) + "' )" );
//		char* textBuffer = new char[bufferSize*4];
//
//		for( int i = 0; i < bufferSize; ++ i )
//		{
//			sprintf(textBuffer, "%02x..", (unsigned char)buffer [ i ] );
//		}
//		sprintf(textBuffer, ".....%d\n", bufferSize );
//
//
//		PSYSTEMLOG(Info, "Q931 readMsg ( PTCPSocket * socket ): " << textBuffer);
//		delete textBuffer;
		return Q931 ( buffer );
	}
	throw std :: runtime_error ( "Can't read TPKT body" );
}

void getInPrice ( const H225 :: H323_UU_PDU & pdu, CommonCallDetails & common ) {
	if ( ! pdu.hasOptionalField ( H225 :: H323_UU_PDU :: e_nonStandardControl ) ||
		pdu.get_nonStandardControl ( ).empty ( ) )
		return;
	const H225 :: NonStandardParameter & parm = pdu.get_nonStandardControl ( ) [ 0 ];
	const H225 :: NonStandardIdentifier & id = parm.m_nonStandardIdentifier;
	if ( id.getTag ( ) != H225 :: NonStandardIdentifier :: e_h221NonStandard )
		return;
	const H225 :: H221NonStandard & h221 = id;
	if ( h221.m_t35CountryCode != 181 || h221.m_t35Extension != 0 || h221.m_manufacturerCode != 666 )
		return;
	const ss :: string & d = parm.m_data;
	if ( d.compare ( 0, 2, "p " ) )
		return;
	ss :: istringstream is ( d );
	ss :: string p;
	is >> p >> common.source ( ).price >> common.source ( ).code;
}

bool getCallID ( const H225 :: Setup_UUIE & setup, H225 :: CallIdentifier & id ) {
	if ( ! setup.hasOptionalField ( H225 :: Setup_UUIE :: e_callIdentifier ) )
		return false;
	id = setup.get_callIdentifier ( );
	return true;
}

void setFrames ( H245 :: AudioCapability & audioData, int f ) {
	switch ( int t = audioData.getTag ( ) ) {
		case H245 :: AudioCapability :: e_nonStandard:
			break;
		case H245 :: AudioCapability :: e_g711Alaw64k: {
			H245 :: AudioCapability_g711Alaw64k & c = audioData;
			c = f;
			break;
		} case H245 :: AudioCapability :: e_g711Alaw56k: {
			H245 :: AudioCapability_g711Alaw56k & c = audioData;
			c = f;
			break;
		} case H245 :: AudioCapability :: e_g711Ulaw64k: {
			H245 :: AudioCapability_g711Ulaw64k & c = audioData;
			c = f;
			break;
		} case H245 :: AudioCapability :: e_g711Ulaw56k: {
			H245 :: AudioCapability_g711Ulaw56k & c = audioData;
			c = f;
			break;
		} case H245 :: AudioCapability :: e_g7231: {
			H245 :: AudioCapability_g7231 & c = audioData;
			c.m_maxAl_sduAudioFrames = f;
			break;
		} case H245 :: AudioCapability :: e_g729wAnnexB: {
			H245 :: AudioCapability_g729wAnnexB & c = audioData;
			c = f;
			break;
		} case H245 :: AudioCapability :: e_g729AnnexAwAnnexB: {
			H245 :: AudioCapability_g729AnnexAwAnnexB & c = audioData;
			c = f;
			break;
		} case H245 :: AudioCapability :: e_g729: {
			H245 :: AudioCapability_g729 & c = audioData;
			c = f;
			break;
		} case H245 :: AudioCapability :: e_g729AnnexA: {
			H245 :: AudioCapability_g729AnnexA & c = audioData;
			c = f;
			break;
		} default:
			PSYSTEMLOG ( Error, "unsupported tag in setFrames: " << t );
	}
}

bool setCodec ( H245 :: AudioCapability & audioData, const ss :: string & c ) {
	if ( c == "g729r8" ) {
		audioData.setTag ( H245 :: AudioCapability :: e_g729 );
		H245 :: AudioCapability_g729 & c = audioData;
		c = 2;
		return true;
	}
	if ( c == "g729abr8" ) {
		audioData.setTag ( H245 :: AudioCapability :: e_g729AnnexAwAnnexB );
		H245 :: AudioCapability_g729AnnexAwAnnexB & c = audioData;
		c = 2;
		return true;
	}
	if ( c == "g729br8" ) {
		audioData.setTag ( H245 :: AudioCapability :: e_g729wAnnexB );
		H245 :: AudioCapability_g729wAnnexB & c = audioData;
		c = 2;
		return true;
	}
	if ( c == "g729ar8" ) {
		audioData.setTag ( H245 :: AudioCapability :: e_g729AnnexA );
		H245 :: AudioCapability_g729AnnexA & c = audioData;
		c = 2;
		return true;
	}
	if ( c == "g711alaw" ) {
		audioData.setTag ( H245 :: AudioCapability :: e_g711Alaw64k );
		return true;
	}
	if ( c == "g711ulaw" ) {
		audioData.setTag ( H245 :: AudioCapability :: e_g711Ulaw64k );
		return true;
	}
	if ( c == "g7231" ) {
		audioData.setTag ( H245 :: AudioCapability :: e_g7231 );
		H245 :: AudioCapability_g7231 & c = audioData;
		c.m_silenceSuppression = false;
		return true;
	}
	if ( c == "g723ar" ) {
		audioData.setTag ( H245 :: AudioCapability :: e_nonStandard );
		H245 :: NonStandardParameter & nonStandard = audioData;
		H245 :: NonStandardIdentifier & id = nonStandard.m_nonStandardIdentifier;
		id.setTag ( H245 :: NonStandardIdentifier :: e_h221NonStandard );
		H245 :: NonStandardIdentifier_h221NonStandard & h221 = id;
		h221.m_t35CountryCode = 181;
		h221.m_t35Extension = 0;
		h221.m_manufacturerCode = 18;
		nonStandard.m_data = "G7231ar";
		return true;
	}
	if ( c == "g723ar53" ) {
		audioData.setTag ( H245 :: AudioCapability :: e_nonStandard );
		H245 :: NonStandardParameter & nonStandard = audioData;
		H245 :: NonStandardIdentifier & id = nonStandard.m_nonStandardIdentifier;
		id.setTag ( H245 :: NonStandardIdentifier :: e_h221NonStandard );
		H245 :: NonStandardIdentifier_h221NonStandard & h221 = id;
		h221.m_t35CountryCode = 181;
		h221.m_t35Extension = 0;
		h221.m_manufacturerCode = 18;
		nonStandard.m_data = "G7231ar53";
		return true;
	}
	if ( c == "g723ar63" ) {
		audioData.setTag ( H245 :: AudioCapability :: e_nonStandard );
		H245 :: NonStandardParameter & nonStandard = audioData;
		H245 :: NonStandardIdentifier & id = nonStandard.m_nonStandardIdentifier;
		id.setTag ( H245 :: NonStandardIdentifier :: e_h221NonStandard );
		H245 :: NonStandardIdentifier_h221NonStandard & h221 = id;
		h221.m_t35CountryCode = 181;
		h221.m_t35Extension = 0;
		h221.m_manufacturerCode = 18;
		nonStandard.m_data = "G7231ar63";
		return true;
	}
	if ( c == "g726" ) {
		audioData.setTag ( H245 :: AudioCapability :: e_nonStandard );
		H245 :: NonStandardParameter & nonStandard = audioData;
		H245 :: NonStandardIdentifier & id = nonStandard.m_nonStandardIdentifier;
		id.setTag ( H245 :: NonStandardIdentifier :: e_h221NonStandard );
		H245 :: NonStandardIdentifier_h221NonStandard & h221 = id;
		h221.m_t35CountryCode = 9;
		h221.m_t35Extension = 0;
		h221.m_manufacturerCode = 61;
		nonStandard.m_data = "G.726-32";
		return true;
	}
	if ( c == "g726r16" ) {
		audioData.setTag ( H245 :: AudioCapability :: e_nonStandard );
		H245 :: NonStandardParameter & nonStandard = audioData;
		H245 :: NonStandardIdentifier & id = nonStandard.m_nonStandardIdentifier;
		id.setTag ( H245 :: NonStandardIdentifier :: e_h221NonStandard );
		H245 :: NonStandardIdentifier_h221NonStandard & h221 = id;
		h221.m_t35CountryCode = 181;
		h221.m_t35Extension = 0;
		h221.m_manufacturerCode = 18;
		nonStandard.m_data = "G726r16";
		return true;
	}
	if ( c == "g726r24" ) {
		audioData.setTag ( H245 :: AudioCapability :: e_nonStandard );
		H245 :: NonStandardParameter & nonStandard = audioData;
		H245 :: NonStandardIdentifier & id = nonStandard.m_nonStandardIdentifier;
		id.setTag ( H245 :: NonStandardIdentifier :: e_h221NonStandard );
		H245 :: NonStandardIdentifier_h221NonStandard & h221 = id;
		h221.m_t35CountryCode = 181;
		h221.m_t35Extension = 0;
		h221.m_manufacturerCode = 18;
		nonStandard.m_data = "G726r24";
		return true;
	}
	if ( c == "g726r32" ) {
		audioData.setTag ( H245 :: AudioCapability :: e_nonStandard );
		H245 :: NonStandardParameter & nonStandard = audioData;
		H245 :: NonStandardIdentifier & id = nonStandard.m_nonStandardIdentifier;
		id.setTag ( H245 :: NonStandardIdentifier :: e_h221NonStandard );
		H245 :: NonStandardIdentifier_h221NonStandard & h221 = id;
		h221.m_t35CountryCode = 181;
		h221.m_t35Extension = 0;
		h221.m_manufacturerCode = 18;
		nonStandard.m_data = "G726r32";
		return true;
	}
	if ( c == "gsmfr" ) {
		audioData.setTag ( H245 :: AudioCapability :: e_gsmFullRate );
		return true;
	}
	if ( c == "gsmefr" ) {
		audioData.setTag ( H245 :: AudioCapability :: e_gsmEnhancedFullRate );
		return true;
	}
/*
	if ( c == "telephone-event" )
	{
		if ( pTelephonEvent )
			*pTelephonEvent = "";
		return true;
	}
*/
	PSYSTEMLOG ( Error, "unsupported codec in setCodec: " << c );
	audioData.setTag ( H245 :: AudioCapability :: e_g729 );
	return false;
}

void addFastStartFromIn ( H225 :: ArrayOf_Asn_OctetString & v, const PIPSocket :: Address & local,
	int port, const CodecInfo & inCodec, int & channel, bool answer ) {
	H245 :: OpenLogicalChannel fromin;
	fromin.m_forwardLogicalChannelNumber = channel ++;
	H245 :: OpenLogicalChannel_forwardLogicalChannelParameters & f = fromin.m_forwardLogicalChannelParameters;
	H245 :: DataType & d = f.m_dataType;
	d.setTag ( H245 :: DataType :: e_audioData );
	H245 :: AudioCapability & a = d;
	if ( ! setCodec ( a, inCodec.getCodec ( ) ) )
		return;
	if ( int frames = inCodec.getFrames ( ) )
		setFrames ( a, frames );
	H245 :: OpenLogicalChannel_forwardLogicalChannelParameters_multiplexParameters & m = f.m_multiplexParameters;
	m.setTag ( H245 :: OpenLogicalChannel_forwardLogicalChannelParameters_multiplexParameters :: e_h2250LogicalChannelParameters );
	H245 :: H2250LogicalChannelParameters & l = m;
	l.m_sessionID = 1;
	l.includeOptionalField ( H245 :: H2250LogicalChannelParameters :: e_mediaControlChannel );
	H245 :: TransportAddress & t = l.get_mediaControlChannel ( );
	t.setTag ( H245 :: TransportAddress :: e_unicastAddress );
	H245 :: UnicastAddress & u = t;
	u.setTag ( H245 :: UnicastAddress :: e_iPAddress );
	setIPAddress ( u, local, port + 1 );
	if ( answer ) {
		l.includeOptionalField ( H245 :: H2250LogicalChannelParameters :: e_mediaChannel );
		H245 :: TransportAddress & t2 = l.get_mediaChannel ( );
		t2.setTag ( H245 :: TransportAddress :: e_unicastAddress );
		H245 :: UnicastAddress & u2 = t2;
		u2.setTag ( H245 :: UnicastAddress :: e_iPAddress );
		setIPAddress ( u2, local, port );
	}
	Asn :: ostream os;
	fromin.encode ( os );
	v.push_back ( os.str ( ) );
}

void addFastStartFromOut ( H225 :: ArrayOf_Asn_OctetString & v, const PIPSocket :: Address & local,
	int port, const CodecInfo & outCodec, bool answer ) {
	H245 :: OpenLogicalChannel fromout;
	fromout.m_forwardLogicalChannelNumber = 1;
	H245 :: OpenLogicalChannel_forwardLogicalChannelParameters & f = fromout.m_forwardLogicalChannelParameters;
	f.m_dataType.setTag ( H245 :: DataType :: e_nullData );
	f.m_multiplexParameters.setTag ( H245 :: OpenLogicalChannel_forwardLogicalChannelParameters_multiplexParameters :: e_none );
	fromout.includeOptionalField ( H245 :: OpenLogicalChannel :: e_reverseLogicalChannelParameters );
	H245 :: OpenLogicalChannel_reverseLogicalChannelParameters & r = fromout.get_reverseLogicalChannelParameters ( );
	H245 :: DataType & d = r.m_dataType;
	d.setTag ( H245 :: DataType :: e_audioData );
	H245 :: AudioCapability & a = d;
	if ( ! setCodec ( a, outCodec.getCodec ( ) ) )
		return;
	if ( int frames = outCodec.getFrames ( ) )
		setFrames ( a, frames );
	r.includeOptionalField ( H245 :: OpenLogicalChannel_reverseLogicalChannelParameters :: e_multiplexParameters );
	H245 :: OpenLogicalChannel_reverseLogicalChannelParameters_multiplexParameters & m = r.get_multiplexParameters ( );
	m.setTag ( H245 :: OpenLogicalChannel_reverseLogicalChannelParameters_multiplexParameters :: e_h2250LogicalChannelParameters );
	H245 :: H2250LogicalChannelParameters & l = m;
	l.m_sessionID = 1;
	l.includeOptionalField ( H245 :: H2250LogicalChannelParameters :: e_mediaControlChannel );
	H245 :: TransportAddress & t = l.get_mediaControlChannel ( );
	t.setTag ( H245 :: TransportAddress :: e_unicastAddress );
	H245 :: UnicastAddress & u = t;
	u.setTag ( H245 :: UnicastAddress :: e_iPAddress );
	setIPAddress ( u, local, port + 1 );
	if ( ! answer ) {
		l.includeOptionalField ( H245 :: H2250LogicalChannelParameters :: e_mediaChannel );
		H245 :: TransportAddress & t2 = l.get_mediaChannel ( );
		t2.setTag ( H245 :: TransportAddress :: e_unicastAddress );
		H245 :: UnicastAddress & u2 = t2;
		u2.setTag ( H245 :: UnicastAddress :: e_iPAddress );
		setIPAddress ( u2, local, port );
	}
	Asn :: ostream os;
	fromout.encode ( os );
	v.push_back ( os.str ( ) );
}

void appendBack ( H225 :: ArrayOf_Asn_OctetString & to, const H225 :: ArrayOf_Asn_OctetString & from ) {
	for ( unsigned i = 0; i < from.size ( ); i ++ )
		to.push_back ( from [ i ] );
}

static unsigned short int mySeqNumber = 1; // Sequence number for messages we send....

static void buildLRQ ( PUDPSocket * replySocket, const PIPSocket :: Address & myAddr,
	const H225 :: ArrayOf_AliasAddress & destAlias, const ss :: string & eid, H225 :: RasMessage & mesg ) {
	mesg.setTag ( H225 :: RasMessage :: e_locationRequest );
	H225 :: LocationRequest & lrq = mesg;
	PIPSocket :: Address replyAddr;
	WORD replyPort;

	replySocket -> GetLocalAddress ( replyAddr, replyPort );
	if ( replyAddr == INADDR_ANY )
		replyAddr = myAddr;
	lrq.m_requestSeqNum = mySeqNumber ++;
	lrq.m_destinationInfo = destAlias;
	lrq.m_replyAddress = AddrUtils :: convertToH225TransportAddr ( replyAddr, replyPort );
	lrq.includeOptionalField ( H225 :: LocationRequest :: e_sourceInfo );
	lrq.get_sourceInfo ( ).setSize ( 1 );
	ss :: string cname = conf -> getName ( );
	H323 :: setAliasAddress ( cname, lrq.get_sourceInfo ( ) [ 0 ], H225 :: AliasAddress :: e_h323_ID );
	lrq.includeOptionalField ( H225 :: LocationRequest :: e_endpointIdentifier );
	lrq.get_endpointIdentifier ( ) = eid.empty ( ) ? cname : eid;
	lrq.includeOptionalField ( H225 :: LocationRequest :: e_gatekeeperIdentifier );
	lrq.get_gatekeeperIdentifier ( ) = "GK_TEST";
}

static const WORD rasPort = 1719;

static bool sendLRQ ( PUDPSocket * socket, const PIPSocket :: Address & addr,
	const PIPSocket :: Address & gatekeeperAddr, const H225 :: ArrayOf_AliasAddress & destAlias,
	const ss :: string & eid = ss :: string ( ) ) {
	socket -> Connect ( gatekeeperAddr );
	H225 :: RasMessage lrq;
	buildLRQ ( socket, addr, destAlias, eid, lrq );
	H225 :: TransportAddress dest = AddrUtils :: convertToH225TransportAddr ( gatekeeperAddr, rasPort );
	Log -> logNote ( "Sending Location Request" );
	Log -> logRasMsg ( lrq, OpengateLog :: Sending, dest );
	Asn :: ostream os;
	lrq.encode ( os );
	return socket -> Write ( os.str ( ).data ( ), PINDEX ( os.str ( ).size ( ) ) );
}

static bool decodeLCF ( const H225 :: RasMessage & mesg, H225 :: TransportAddress & destAddr ) {
	switch ( mesg.getTag ( ) ) {
		case H225 :: RasMessage :: e_locationConfirm:
			break;
		case H225 :: RasMessage :: e_locationReject:
			PSYSTEMLOG ( Info, "got lrj" );
			return false;
		default:
			PSYSTEMLOG ( Error, "DecodeLCF: invalid tag: " << mesg.getTag ( ) );
			return false;
	}
	const H225 :: LocationConfirm & lcf = mesg;
	destAddr = lcf.m_callSignalAddress;
	return true;
}

static bool receiveLCF ( PUDPSocket & socket, H225 :: TransportAddress & destAddr ) {
	static const int bufferSize = 4096;
	char buffer [ bufferSize ];
	// We need to set the read timeout to a sensible value.....
	PTimeInterval timeout ( 0, 3 ); // Try 3 seconds, should read from config
	socket.SetReadTimeout ( timeout );

	PIPSocket :: Address addr;
	WORD port;
	if ( ! socket.ReadFrom ( buffer, bufferSize, addr, port ) ) {
		PSYSTEMLOG ( Error, "ReceiveLCF: can't read: " << socket.GetErrorText ( ) );
		return false;
	}
	try {
		Asn :: istream is ( ss :: string ( buffer, socket.GetLastReadCount ( ) ) );
		H225 :: RasMessage reply ( is );
		if ( is.hasException ( ) )
			PSYSTEMLOG ( Error, "ReceiveLCF: can't decode message: " << is.getException ( ) );
		H225 :: TransportAddress sender = AddrUtils :: convertToH225TransportAddr ( addr, port );
		Log -> logNote ( "Received a reply to the Location Request" );
		Log -> logRasMsg ( reply, OpengateLog :: Receiving, sender );
		return decodeLCF ( reply, destAddr );
	} catch ( std :: exception & e ) {
		PSYSTEMLOG ( Error, "ReceiveLCF: can't decode message: " << e.what ( ) );
		return false;
	}
}

bool getAddrFromGK ( const PIPSocket :: Address & gk, const ss :: string & digits, PIPSocket :: Address & ip, int & port ) {
	PUDPSocket socket ( rasPort );
//	bool first = true;
	PIPSocket :: Address myAddr;
	PIPSocket :: GetHostAddress ( myAddr );
	H225 :: ArrayOf_AliasAddress destAlias;
	destAlias.setSize ( 1 );
	H323 :: setAliasAddress ( digits, destAlias [ 0 ] );
	if ( ! sendLRQ ( & socket, myAddr, gk, destAlias ) ) {
		PSYSTEMLOG ( Error, "SendLRQ: " << socket.GetErrorText ( ) );
		return false;
	}
	int selection = PSocket :: Select ( socket, socket, 3000 );
	if ( selection > 0 ) {
		PSYSTEMLOG ( Error, "gk select: " << PChannel :: GetErrorText ( PChannel :: Errors ( selection ) ) );
		return false;
	}
	if ( selection == 0 ) {
		PSYSTEMLOG ( Error, "gk select: timeout" );
		return false;
	}
	H225 :: TransportAddress destAddr;
	if ( ! receiveLCF ( socket, destAddr ) ) {
		return false;
/*                if ( ! first )
                        return false;
                first = false;
                int rasId = 0;
                if ( ! sendRRQ ( & socket, myAddr, MyCall.gkCalled, rasId ) ) {
                        conf -> unLockRas ( );
                        PSYSTEMLOG ( Error, "SendRRQ: " << socket.GetErrorText ( ) );
                        return false;
                }
                ss :: string eid;
                conf -> waitRrq ( rasId, eid );
                if ( eid.empty ( ) )
                        return false;


                if ( ! sendLRQ ( & socket, myAddr, MyCall.gkCalled, destAlias, eid ) ) {
                        PSYSTEMLOG ( Error, "SendLRQ: " << socket.GetErrorText ( ) );
                        return false;
                }
                selection = PSocket :: Select ( socket, socket, 3000 );
                if ( selection > 0 ) {
                        PSYSTEMLOG ( Error, "gk select: " << PChannel :: GetErrorText (
                                PChannel :: Errors ( selection ) ) );
                        return false;
                }
                if ( selection == 0 ) {
                        PSYSTEMLOG ( Error, "gk select: timeout" );
                        return false;
                }
                if ( ! receiveLCF ( socket, destAddr ) )
                        return false;
                if ( ! sendARQ ( & socket, myAddr, MyCall.gkCalled, MyCall.ref, MyCall.confId, MyCall.Id, destAlias, eid, rasId ) ) {
                        PSYSTEMLOG ( Error, "SendARQ: " << socket.GetErrorText ( ) );
                        conf -> unLockRas ( );
                        return false;
                }
                if ( ! conf -> waitArq ( rasId, destAddr ) )
                        return false;
*/	}
	if ( destAddr.getTag ( ) != H225 :: TransportAddress :: e_ipAddress ) {
		PSYSTEMLOG ( Error, "gk returned address which is not an IP address" );
		return false;
	}
	H225 :: TransportAddress_ipAddress & destIP = destAddr;
	WORD p;
	AddrUtils :: convertToIPAddress ( destIP, ip, p );
	port = p;
	return true;
}

static void getIncomingCodecsFromH245Mesg ( const ss :: string & s, CodecInfoVector & incomingCodecs ) {
	try {
		Asn :: istream is ( s );
		H245 :: MultimediaSystemControlMessage mesg ( is );
		if ( is.hasException ( ) ) {
			PSYSTEMLOG ( Error, "tunneled h245: can't decode" << s << ": " << is.getException ( ) );
			return;
		}
		if ( mesg.getTag ( ) != H245 :: MultimediaSystemControlMessage :: e_request )
			return;
		const H245 :: RequestMessage & request = mesg;
		if ( request.getTag ( ) != H245 :: RequestMessage :: e_terminalCapabilitySet )
			return;
		const H245 :: TerminalCapabilitySet & caps = request;
		if ( ! caps.hasOptionalField ( H245 :: TerminalCapabilitySet :: e_capabilityTable ) )
			return;
		const H245 :: TerminalCapabilitySet_capabilityTable & table = caps.get_capabilityTable ( );
		StringSet frominset, toinset;
		for ( std :: size_t i = 0; i < table.size ( ); i ++ ) {
			const H245 :: CapabilityTableEntry & entry = table [ i ];
			if ( ! entry.hasOptionalField ( H245 :: CapabilityTableEntry :: e_capability ) )
				continue;
			const H245 :: Capability & cap = entry.get_capability ( );
			switch ( cap.getTag ( ) ) {
				case H245 :: Capability :: e_receiveAudioCapability:
				case H245 :: Capability :: e_transmitAudioCapability:
				case H245 :: Capability :: e_receiveAndTransmitAudioCapability: {
					CodecInfo c = :: getCodec ( cap );
					if ( c.getCodec ( ) != "unknown" ) {
						incomingCodecs.c.push_back ( c );
						switch ( cap.getTag ( ) ) {
							case H245 :: Capability :: e_receiveAudioCapability:
								toinset.insert ( c.getCodec ( ) );
								break;
							case H245 :: Capability :: e_transmitAudioCapability:
								frominset.insert ( c.getCodec ( ) );
								break;
							case H245 :: Capability :: e_receiveAndTransmitAudioCapability:
								toinset.insert ( c.getCodec ( ) );
								frominset.insert ( c.getCodec ( ) );
								break;
						}
					}
				}
			}
		}
		if ( ! frominset.empty ( ) ) {
			StringList diff;
			std :: set_symmetric_difference ( frominset.begin ( ), frominset.end ( ), toinset.begin ( ),
				toinset.end ( ), std :: back_inserter ( diff ) );
			for ( StringList :: const_iterator i = diff.begin ( ); i != diff.end ( ); ++ i )
				 incomingCodecs.c.get < Codec > ( ).erase ( * i );
		}
	} catch ( std :: exception & e ) {
		PSYSTEMLOG ( Error, "tunneled h245: can't decode " << s << ": " << e.what ( ) );
			return;
	}
}

void getIncomingCodecsFromSetup ( const H225 :: H323_UU_PDU & pdu, CodecInfoVector & incomingCodecs, PIPSocket :: Address & addr, int & port ) {
	const H225 :: H323_UU_PDU_h323_message_body & body = pdu.m_h323_message_body;
	if ( body.getTag ( ) != H225 :: H323_UU_PDU_h323_message_body :: e_setup )
		return;
	const H225 :: Setup_UUIE & setup = body;
	if ( setup.hasOptionalField ( H225 :: Setup_UUIE :: e_fastStart ) ) {
		const H225 :: ArrayOf_Asn_OctetString & fs = setup.get_fastStart ( );
		StringSet frominset, toinset;
		bool addrSet = false;
		for ( std :: size_t i = 0; i < fs.size ( ); i ++ ) {
			try {
				Asn :: istream is ( fs [ i ] );
				H245 :: OpenLogicalChannel open ( is );
				if ( is.hasException ( ) ) {
					PSYSTEMLOG ( Error, "fastStart: can't decode " << fs [ i ] << ": " <<
						is.getException ( ) );
					return;
				}
				H245 :: OpenLogicalChannel_forwardLogicalChannelParameters & forwardParams =
					open.m_forwardLogicalChannelParameters;
				CodecInfo c = getCodec ( forwardParams.m_dataType );
				if ( c.getCodec ( ) != "unknown" ) {
					incomingCodecs.c.push_back ( c );
					frominset.insert ( c.getCodec ( ) );
				}
				if ( ! open.hasOptionalField ( H245 :: OpenLogicalChannel :: e_reverseLogicalChannelParameters ) )
					continue;
				H245 :: OpenLogicalChannel_reverseLogicalChannelParameters & reverseParams =
					open.get_reverseLogicalChannelParameters ( );
				if ( ! addrSet && reverseParams.hasOptionalField ( H245 :: OpenLogicalChannel_reverseLogicalChannelParameters :: e_multiplexParameters ) ) {
					H245 :: UnicastAddress_iPAddress * ip;
					getMediaAddr ( reverseParams.get_multiplexParameters ( ), addr, port, ip );
					addrSet = true;
				}
				c = getCodec ( reverseParams.m_dataType );
				if ( c.getCodec ( ) != "unknown" ) {
					incomingCodecs.c.push_back ( c );
					toinset.insert ( c.getCodec ( ) );
				}
			} catch ( std :: exception & e ) {
				PSYSTEMLOG ( Error, "fastStart: can't decode " << fs [ i ] << ": " << e.what ( ) );
				break;
			}
		}
		StringList diff;
		std :: set_symmetric_difference ( frominset.begin ( ), frominset.end ( ), toinset.begin ( ), toinset.end ( ),
			std :: back_inserter ( diff ) );
		for ( StringList :: const_iterator i = diff.begin ( ); i != diff.end ( ); ++ i )
			 incomingCodecs.c.get < Codec > ( ).erase ( * i );
		return;
	}
/*	if ( setup.hasOptionalField ( H225 :: Setup_UUIE :: e_parallelH245Control ) ) {
		const H225 :: ArrayOf_Asn_OctetString & parallelControl = setup.get_parallelH245Control ( );
		for ( std :: size_t i = 0; i < parallelControl.size ( ); i ++ )
			getIncomingCodecsFromH245Mesg ( parallelControl [ i ], incomingCodecs );
	}*/
	if ( pdu.hasOptionalField ( H225 :: H323_UU_PDU :: e_h245Control ) ) {
		const H225 :: ArrayOf_Asn_OctetString & control = pdu.get_h245Control ( );
		for ( std :: size_t i = 0; i < control.size ( ); i ++ )
			getIncomingCodecsFromH245Mesg ( control [ i ], incomingCodecs );
	}
}

int getRtpCodec ( H245 :: AudioCapability & audioData ) {
	switch ( int t = audioData.getTag ( ) ) {
		case H245 :: AudioCapability :: e_g711Alaw64k:
			return 2;
		case H245 :: AudioCapability :: e_g711Alaw56k:
			return 1;
		case H245 :: AudioCapability :: e_g711Ulaw64k:
			return 2;
		case H245 :: AudioCapability :: e_g711Ulaw56k:
			return 1;
		case H245 :: AudioCapability :: e_g7231:
			return 4;
		case H245 :: AudioCapability :: e_g729wAnnexB:
			return 3;
		case H245 :: AudioCapability :: e_g729AnnexAwAnnexB:
			return 3;
		case H245 :: AudioCapability :: e_g729:
			return 3;
		case H245 :: AudioCapability :: e_g729AnnexA:
			return 3;
		default:
			PSYSTEMLOG ( Error, "unsupported tag in getRtpCodec: " << t );
			return 0;
	}
}

int getRtpCodec ( const CodecInfo & c ) {
	const ss :: string & s = c.getCodec ( );
	if ( s == "g723ar" || s == "g723ar53" || s == "g723ar63" || s == "g7231" )
		return 4;
	if ( s == "g711alaw" || s == "g711ulaw" )
		return 2;
	if ( s == "g729br8" || s == "g729abr8" || s == "g729r8" || s == "g729ar8" )
		return 3;
	return 0;
}

void changeDataTypeAndFramesIn ( FastStartElement & e ) {
	ss :: string c = e.recodeTo.getCodec ( );
	H245 :: AudioCapability & audioData = e.open.m_forwardLogicalChannelParameters.m_dataType;
	if ( c != "unknown" )
		setCodec ( audioData, c );
	if ( e.changeFrames )
		setFrames ( audioData, e.changeFrames );
	e.codec = :: getCodec ( audioData );
	return;
}

void changeDataTypeAndFramesOut ( FastStartElement & e ) {
	ss :: string c = e.recodeTo.getCodec ( );
	H245 :: AudioCapability & audioData = e.open.get_reverseLogicalChannelParameters ( ).m_dataType;
	if ( c != "unknown" )
		setCodec ( audioData, c );
	if ( e.outChangeFrames )
		setFrames ( audioData, e.outChangeFrames );
	e.codec = :: getCodec ( audioData );
}

void makeAlertingFromCallProceeding ( Q931 & mesg, H225 :: H323_UU_PDU_h323_message_body & body ) {
	H225 :: CallProceeding_UUIE & proc = body;
	PSYSTEMLOG ( Info, "replacing callproceeding with alerting" );
	H225 :: Alerting_UUIE alerting;
	if ( proc.hasOptionalField ( H225 :: CallProceeding_UUIE :: e_h245Address ) ) {
		alerting.includeOptionalField ( H225 :: Alerting_UUIE :: e_h245Address );
		alerting.get_h245Address ( ) = proc.get_h245Address ( );
	}
	if ( proc.hasOptionalField ( H225 :: CallProceeding_UUIE :: e_callIdentifier ) ) {
		alerting.includeOptionalField ( H225 :: Alerting_UUIE :: e_callIdentifier );
		alerting.get_callIdentifier ( ) = proc.get_callIdentifier ( );
	}
	if ( proc.hasOptionalField ( H225 :: CallProceeding_UUIE :: e_fastStart ) ) {
		alerting.includeOptionalField ( H225 :: Alerting_UUIE :: e_fastStart );
		alerting.get_fastStart ( ) = proc.get_fastStart ( );
	}
	if ( proc.hasOptionalField ( H225 :: CallProceeding_UUIE :: e_fastConnectRefused ) ) {
		alerting.includeOptionalField ( H225 :: Alerting_UUIE :: e_fastConnectRefused );
		alerting.get_fastConnectRefused ( ) = proc.get_fastConnectRefused ( );
	}
	alerting.m_protocolIdentifier = proc.m_protocolIdentifier;
	alerting.m_destinationInfo = proc.m_destinationInfo;
	body.setTag ( H225 :: H323_UU_PDU_h323_message_body :: e_alerting );
	H225 :: Alerting_UUIE & t = body;
	t = alerting;
	mesg.setMessageType ( Q931 :: msgAlerting );
}
