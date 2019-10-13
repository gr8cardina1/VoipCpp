#pragma implementation
#include "ss.hpp"
#include "allocatable.hpp"
#include "pointer.hpp"
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/sequenced_index.hpp>
#include <boost/multi_index/ordered_index.hpp>
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
#include "answerlegthread.hpp"
#include "h225.hpp"
#include "calldetails.hpp"
#include "h323common.hpp"
#include <queue>
#include "h323answerleg.hpp"
#include "Log.h"
#include "automutex.hpp"
#include "AddrUtils.h"
#include "rtpstat.hpp"
#include "session.hpp"
#include "ipport.hpp"
#include "icqcontact.hpp"
#include "radgwinfo.hpp"
#include "Conf.hpp"
#include "rtpsession.hpp"
#include "h323call.hpp"
#include <ptlib/svcproc.h>
#include "random.hpp"

void H323AnswerLegThread :: shutDownLeg ( ) {
	if ( h245Sock && h245Sock -> IsOpen ( ) )
		h245Sock -> Close ( );
	if ( sock -> IsOpen ( ) ) {
		sendReleaseComplete ( );
		sock -> Close ( );
	}
}

void H323AnswerLegThread :: sendReleaseComplete ( ) {
	if ( ! rcSent ) {
		:: sendReleaseComplete ( sock, common, details.id, details.ref, false );
		rcSent = true;
	}
}

bool H323AnswerLegThread :: receiveMesg ( ) {
	try {
		Q931 mesg ( readMsg ( sock ) );
		return handleMesg ( mesg );
	} catch ( std :: exception & e ) {
		PSYSTEMLOG ( Error, "unable to receive Q931 message: " << e.what ( ) );
		return false;
	}
}

bool H323AnswerLegThread :: receiveH245Mesg ( ) {
	BYTE tpkt [ 4 ];
	if ( ! h245Sock -> ReadBlock ( tpkt, sizeof ( tpkt ) ) )
		return false;
	if ( tpkt [ 0 ] != 3 ) {
		PSYSTEMLOG ( Error, "Expecting TPKT in H.245" );
		return false;
	}
	int bufferSize = ( ( tpkt [ 2 ] << 8 ) | tpkt [ 3 ] ) - 4;
	Asn :: string buffer ( bufferSize, '\0' );
	if ( ! h245Sock -> Read ( & buffer [ 0 ], bufferSize ) )
		return false;
	PIPSocket :: Address peerAddr;
	WORD peerPort;
	h245Sock -> GetPeerAddress ( peerAddr, peerPort );
	try {
		Asn :: istream is ( buffer );
		H245 :: MultimediaSystemControlMessage mesg ( is );
		Log -> logH245Msg ( mesg, OpengateLog :: Receiving, peerAddr, peerPort, common.getForceDebug ( ) );
		if ( is.hasException ( ) ) {
			PSYSTEMLOG ( Error, "decode exception: " << is.getException ( ) );
			return false;
		}
		if ( answeredCodec.getCodec ( ) == "unknown" ) {
			details.h245Spool.push_back ( buffer );
			return true;
		}
		H225 :: ArrayOf_Asn_OctetString reply;
		if ( ! processH245Control ( reply ) )
			return false;
		if ( ! processH245Message ( mesg, reply ) )
			return false;
		for ( unsigned i = 0; i < reply.size ( ); i ++ ) {
			Asn :: istream is ( reply [ i ] );
			H245 :: MultimediaSystemControlMessage mesg ( is );
			sendH245MesgToDest ( makeTpkt ( reply [ i ] ), mesg, h245Sock, common.getForceDebug ( ) );
		}
		return true;
	} catch ( std :: exception & e ) {
		PSYSTEMLOG ( Error, "decode exception: " << e.what ( ) );
		return false;
	}
}

bool H323AnswerLegThread :: handleMesg ( Q931 & mesg ) {
	mesg.removeIE ( Q931 :: ieDisplay );
	Pointer < H225 :: H323_UserInformation > uuField;
	if ( mesg.hasIE ( Q931 :: ieUserUser ) ) {
		const Asn :: string & s ( mesg.getIE ( Q931 :: ieUserUser ) );
		try {
			Asn :: istream is ( s );
			uuField = new H225 :: H323_UserInformation ( is );
			if ( is.hasException ( ) )
				PSYSTEMLOG ( Error, "decode exception: " << is.getException ( ) );
		} catch ( std :: exception & e ) {
			PSYSTEMLOG ( Error, "decode c++ exception: " << e.what ( ) );
		}
	}
	PIPSocket :: Address peerAddr;
	WORD peerPort;
	sock -> GetPeerAddress ( peerAddr, peerPort );
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
			PSYSTEMLOG ( Error, "H323AnswerLegThread::handleMesg ( " << mesg << " ) - ! HasUU" );
			return false;
		}
	}
	switch ( mesg.getMessageType ( ) ) {
		case Q931 :: msgReleaseComplete:
			return handleRelease ( mesg, uuField );
		case Q931 :: msgFacility:
			return handleFacility ( mesg, * uuField );
		case Q931 :: msgStatusEnquiry:
			sendStatus ( );
			return true;
		case Q931 :: msgStatus:
		case Q931 :: msgSetupAck:
		case Q931 :: msgNotify:
			return true;
		default:
			PSYSTEMLOG ( Error, "Unknown message type: " << mesg.getMessageType ( ) << " !!!!!!!!!!" );
			if ( ! common.getForceDebug ( ) )
				Log -> logQ931Msg ( mesg, uuField, OpengateLog :: Receiving,
					PIPSocket :: Address ( ), 0, true );
			return false;
	}
}

void H323AnswerLegThread :: handleH245Setup ( const H225 :: TransportAddress & addr ) {
	if ( h245Sock )
		return;
	if ( common.getFromNat ( ) )
		return;
	useTunneling = false;
	if ( addr.getTag ( ) != H225 :: TransportAddress :: e_ipAddress )
		return;
	const H225 :: TransportAddress_ipAddress & ip = addr;
	WORD port;
	PIPSocket :: Address ipAddr, localAddr;
	AddrUtils :: convertToIPAddress ( ip, ipAddr, port );
	if ( port == 0 )
		return;
	h245Sock = new PTCPSocket ( port );
	h245Sock -> SetReadTimeout ( 3000 );
	h245Sock -> SetWriteTimeout ( 3000 );
	sock -> GetLocalAddress ( localAddr );
	if ( h245Sock -> Connect ( localAddr, ipAddr ) )
		return;
	h245Sock -> Close ( );
	safeDel ( h245Sock );
}

template < typename T > void H323AnswerLegThread :: handleH245Setup ( const T & mesg ) {
	if ( ! mesg.hasOptionalField ( T :: e_h245Address ) )
		return;
	const H225 :: TransportAddress & addr = mesg.get_h245Address ( );
	handleH245Setup ( addr );
}

bool H323AnswerLegThread :: handleSetup ( const Q931 & mesg, const H225 :: H323_UserInformation & uuField ) {
	PSYSTEMLOG(Info, "H323AnswerLegThread :: handleSetup");

	const H225 :: H323_UU_PDU & pdu = uuField.m_h323_uu_pdu;
	const H225 :: H323_UU_PDU_h323_message_body & body = pdu.m_h323_message_body;
	if ( body.getTag ( ) != H225 :: H323_UU_PDU_h323_message_body :: e_setup ) {
		PSYSTEMLOG ( Error, "bad setup tag: " << body.getTag ( ) );
		return false;
	}
	details.setup = mesg;
	details.uuField = uuField;
	const H225 :: Setup_UUIE & setup = body;
	const H225 :: EndpointType & sourceInfo = setup.m_sourceInfo;
	if ( sourceInfo.hasOptionalField ( H225 :: EndpointType :: e_vendor ) ) {
		const H225 :: VendorIdentifier & vendorId = sourceInfo.get_vendor ( );
		if ( vendorId.hasOptionalField ( H225 :: VendorIdentifier :: e_productId ) ) {
			if ( static_cast < const Asn :: string & > ( vendorId.get_productId ( ) ) == "VoIP" )
				sourceIsVoIP = true;
		}
	}
	if ( sourceIsVoIP )
		PSYSTEMLOG ( Info, "source is VoIP !!!!!!!!!!" );
	if ( ! common.getSigOptions ( ).dropTunneling ( ) &&
		pdu.hasOptionalField ( H225 :: H323_UU_PDU :: e_h245Tunneling ) )
		useTunneling = pdu.get_h245Tunneling ( );
	if ( useTunneling && setup.hasOptionalField ( H225 :: Setup_UUIE :: e_parallelH245Control ) )
		details.h245Spool = setup.get_parallelH245Control ( );
	if ( setup.hasOptionalField ( H225 :: Setup_UUIE :: e_fastStart ) && ! setup.get_fastStart ( ).empty ( ) )
		return true;
	if ( useTunneling && pdu.hasOptionalField ( H225 :: H323_UU_PDU :: e_h245Control ) ) {
		details.h245Spool = pdu.get_h245Control ( );
		return true;
	}
	handleH245Setup ( setup );
	return true;
}

void H323AnswerLegThread :: checkStartH245 ( H225 :: ArrayOf_Asn_OctetString & reply ) {
	if ( ! common.getSigOptions ( ).startH245 ( ) || ! reply.empty ( ) )
		return;
	H245 :: MultimediaSystemControlMessage answer;
	Asn :: ostream os;
	const unsigned terminalType = 120;
	answer.setTag ( H245 :: MultimediaSystemControlMessage :: e_request );
	H245 :: RequestMessage & request = answer;
	request.setTag ( H245 :: RequestMessage :: e_masterSlaveDetermination );
	H245 :: MasterSlaveDetermination & determination = request;
	determination.m_terminalType = terminalType;
	determination.m_statusDeterminationNumber = determinationNumber;
	answer.encode ( os );
	reply.push_back ( os.str ( ) );
	masterSlaveSent = true;
	static Asn :: ObjectId pid ( "0.0.8.2250.0.4" );
	if ( encodeCapabilitySet ( 1, pid, reply ) )
		capSetSent = true;
}

void H323AnswerLegThread :: handleFastStartAndH245 ( Q931 & /*mesg*/, H225 :: H323_UserInformation & uuField,
	H225 :: ArrayOf_Asn_OctetString & fs, int localRtpPort, PIPSocket :: Address localAddr,
	const CodecInfo & inCodec, const CodecInfo & outCodec ) {
	H225 :: H323_UU_PDU & pdu = uuField.m_h323_uu_pdu;
	setTunneling ( pdu );
	PSYSTEMLOG ( Info, "handleFastStartAndH245 with addr " << localAddr );
	if ( localAddr == INADDR_ANY )
		sock -> GetLocalAddress ( localAddr );
	int channel = 101;
	addFastStartFromIn ( fs, localAddr, localRtpPort, inCodec, channel, true );
	addFastStartFromOut ( fs, localAddr, localRtpPort, outCodec, true );
	if ( useTunneling ) {
		pdu.includeOptionalField ( H225 :: H323_UU_PDU :: e_h245Control );
		if ( ! processH245Control ( pdu.get_h245Control ( ) ) ) {
			shutDown ( Q931 :: cvNormalUnspecified );
			return;
		}
		checkStartH245 ( pdu.get_h245Control ( ) );
		if ( pdu.get_h245Control ( ).empty ( ) )
			pdu.removeOptionalField ( H225 :: H323_UU_PDU :: e_h245Control );
	} else if ( h245Sock ) {
		H225 :: ArrayOf_Asn_OctetString reply;
		if ( ! processH245Control ( reply ) ) {
			shutDown ( Q931 :: cvNormalUnspecified );
			return;
		}
		checkStartH245 ( reply );
		for ( unsigned i = 0; i < reply.size ( ); i ++ ) {
			Asn :: istream is ( reply [ i ] );
			H245 :: MultimediaSystemControlMessage mesg ( is );
			sendH245MesgToDest ( makeTpkt ( reply [ i ] ), mesg, h245Sock, common.getForceDebug ( ) );
		}
	}
}

bool H323AnswerLegThread :: handleRelease ( const Q931 & mesg, const H225 :: H323_UserInformation * uuField ) {
	Q931 :: CauseValues cause = getCause ( mesg, uuField );
	PSYSTEMLOG ( Info, "get release complete 0x" << std :: hex << cause );
	common.setDisconnectCause ( cause );
	sendReleaseComplete ( );
	conf -> replaceReleaseCompleteCause ( common, true );
	PSYSTEMLOG ( Info, "H323AnswerLegThread: initialized disconnectCause to " <<
		common.getDisconnectCause ( ) );
	call -> released ( common.getDisconnectCause ( ) );
	return false;
}

bool H323AnswerLegThread :: handleFacility ( const Q931 & mesg, const H225 :: H323_UserInformation & origUuField ) {
	const H225 :: H323_UU_PDU & origPdu = origUuField.m_h323_uu_pdu;
	if ( origPdu.hasOptionalField ( H225 :: H323_UU_PDU :: e_h245Control ) )
		appendBack ( details.h245Spool, origPdu.get_h245Control ( ) );
	const H225 :: H323_UU_PDU_h323_message_body & body = origPdu.m_h323_message_body;
	switch ( body.getTag ( ) ) {
		case H225 :: H323_UU_PDU_h323_message_body :: e_facility: {
			const H225 :: Facility_UUIE & fac = body;
			switch ( fac.m_reason.getTag ( ) ) {
				case H225 :: FacilityReason :: e_startH245:
					if ( sourceIsVoIP )
						return true;
				case H225 :: FacilityReason :: e_undefinedReason:
				case H225 :: FacilityReason :: e_transportedInformation:
					break;
				default:
					PSYSTEMLOG ( Error, "unknown facility reason: " << mesg <<
						origUuField << "!!!!!!!!!!!!!!!!" );
			}
			handleH245Setup ( fac );
		} case H225 :: H323_UU_PDU_h323_message_body :: e_empty:
			break;
		default:
			PSYSTEMLOG ( Error, "Bad body tag for Facility: " << body.getTag ( ) );
			return false;
	}
	if ( details.h245Spool.empty ( ) || answeredCodec.getCodec ( ) == "unknown" )
		return true;
	if ( h245Sock ) {
		H225 :: ArrayOf_Asn_OctetString reply;
		if ( ! processH245Control ( reply ) )
			return false;
		for ( unsigned i = 0; i < reply.size ( ); i ++ ) {
			Asn :: istream is ( reply [ i ] );
			H245 :: MultimediaSystemControlMessage mesg ( is );
			sendH245MesgToDest ( makeTpkt ( reply [ i ] ), mesg, h245Sock, common.getForceDebug ( ) );
		}
		return true;
	}
	if ( ! useTunneling )
		return true;
	Q931 facility ( Q931 :: msgFacility, mesg.getCallReference ( ), true );
	H225 :: H323_UserInformation uuField;
	H225 :: H323_UU_PDU & pdu = uuField.m_h323_uu_pdu;
	pdu.m_h323_message_body.setTag ( H225 :: H323_UU_PDU_h323_message_body :: e_empty );
	pdu.includeOptionalField ( H225 :: H323_UU_PDU :: e_h245Tunneling );
	pdu.get_h245Tunneling ( ) = true;
	pdu.includeOptionalField ( H225 :: H323_UU_PDU :: e_h245Control );
	if ( ! processH245Control ( pdu.get_h245Control ( ) ) )
		return false;
	if ( pdu.get_h245Control ( ).empty ( ) )
		return true;
	encodeH225IntoQ931 ( uuField, facility );
	sendMesgToDest ( facility, & uuField, sock, common );
	return true;
}

bool H323AnswerLegThread :: processH245Control ( H225 :: ArrayOf_Asn_OctetString & reply ) {
	for ( unsigned i = 0; i < details.h245Spool.size ( ); i ++ ) {
		try {
			Asn :: istream is ( details.h245Spool [ i ] );
			H245 :: MultimediaSystemControlMessage h245Mesg ( is );
			if ( is.hasException ( ) ) {
				PSYSTEMLOG ( Error, "tunneled h245: can't decode" << details.h245Spool [ i ] <<
					": " << is.getException ( ) );
				return false;
			}
			if ( ! processH245Message ( h245Mesg, reply ) )
				return false;
		} catch ( std :: exception & e ) {
			PSYSTEMLOG ( Error, "tunneled h245: can't decode control [ "
				<< i << " ] : " << e.what ( ) );
			return false;
		}
	}
	details.h245Spool.clear ( );
	return true;
}

bool H323AnswerLegThread :: processH245Message ( const H245 :: MultimediaSystemControlMessage & mesg,
	H225 :: ArrayOf_Asn_OctetString & reply ) {
	switch ( mesg.getTag ( ) ) {
		case H245 :: MultimediaSystemControlMessage :: e_request:
			return handleRequest ( mesg, reply );
		case H245 :: MultimediaSystemControlMessage :: e_response:
			return handleResponse ( mesg, reply );
		case H245 :: MultimediaSystemControlMessage :: e_indication:
			return handleIndication ( mesg, reply );
		default :
			PSYSTEMLOG ( Error, "Unknown H245 message type not handled : " << mesg.getTag ( ) );
			return false;
	}
}

bool H323AnswerLegThread :: handleResponse ( const H245 :: ResponseMessage & mesg,
	H225 :: ArrayOf_Asn_OctetString & /*reply*/ ) {
	switch ( mesg.getTag ( ) ) {
		case H245 :: ResponseMessage :: e_terminalCapabilitySetAck:
			return true;
		case H245 :: ResponseMessage :: e_masterSlaveDeterminationAck:
			return true;
		case H245 :: ResponseMessage :: e_masterSlaveDeterminationReject:
			return true;
		case H245 :: ResponseMessage :: e_openLogicalChannelAck:
			return true;
		default :
			PSYSTEMLOG ( Error, "unknown response tag: " << mesg.getTag ( ) );
			return false;
	}
}

bool H323AnswerLegThread :: handleRequest ( const H245 :: RequestMessage & mesg,
	H225 :: ArrayOf_Asn_OctetString & reply ) {
	switch ( mesg.getTag ( ) ) {
		case H245 :: RequestMessage :: e_terminalCapabilitySet:
			return handleCapabilitySet ( mesg, reply );
		case H245 :: RequestMessage :: e_masterSlaveDetermination:
			return handleMasterSlaveDetermination ( mesg, reply );
		case H245 :: RequestMessage :: e_roundTripDelayRequest:
			return handleRTDR ( mesg, reply );
		case H245 :: RequestMessage :: e_openLogicalChannel:
			return handleOpenLogicalChannel ( mesg, reply );
		case H245 :: RequestMessage :: e_closeLogicalChannel:
			return handleCloseLogicalChannel ( mesg, reply );
		case H245 :: RequestMessage :: e_requestMode:
			return handleRequestMode ( mesg, reply );
		default:
			PSYSTEMLOG ( Error, "unknown request tag: " << mesg.getTag ( ) );
			return false;
	}
}

bool H323AnswerLegThread :: handleIndication ( const H245 :: IndicationMessage & mesg,
	H225 :: ArrayOf_Asn_OctetString & reply ) {
	switch ( mesg.getTag ( ) ) {
		case H245 :: IndicationMessage :: e_masterSlaveDeterminationRelease:
			return handleMasterSlaveDeterminationRelease ( mesg, reply );
		case H245 :: IndicationMessage :: e_userInput:
			return handleUserInput ( mesg, reply );
		case H245 :: IndicationMessage :: e_terminalCapabilitySetRelease:
			return handleTerminalCapabilitySetRelease ( mesg, reply );
		default:
			PSYSTEMLOG ( Error, "unknown indication tag: " << mesg.getTag ( ) );
			return false;
	}
}

bool H323AnswerLegThread :: handleMasterSlaveDeterminationRelease (
	const H245 :: MasterSlaveDeterminationRelease & /*mesg*/, H225 :: ArrayOf_Asn_OctetString & /*reply*/ ) {
	PSYSTEMLOG ( Error, "master/slave determination released - something went wrong" );
	return true;
}

bool H323AnswerLegThread :: handleUserInput ( const H245 :: UserInputIndication & /*mesg*/,
	H225 :: ArrayOf_Asn_OctetString & /*reply*/ ) {
	PSYSTEMLOG ( Error, "user input transfer not supported" );
	return true;
}

bool H323AnswerLegThread :: handleTerminalCapabilitySetRelease (
	const H245 :: TerminalCapabilitySetRelease & /*mesg*/, H225 :: ArrayOf_Asn_OctetString & /*reply*/ ) {
	PSYSTEMLOG ( Error, "capability set released - something went wrong" );
	return true;
}

bool H323AnswerLegThread :: handleRTDR ( const H245 :: RoundTripDelayRequest & mesg,
	H225 :: ArrayOf_Asn_OctetString & reply ) {
	H245 :: MultimediaSystemControlMessage answer;
	answer.setTag ( H245 :: MultimediaSystemControlMessage :: e_response );
	H245 :: ResponseMessage & response = answer;
	response.setTag ( H245 :: ResponseMessage :: e_roundTripDelayResponse );
	H245 :: RoundTripDelayResponse & rtdr = response;
	rtdr.m_sequenceNumber = mesg.m_sequenceNumber;
	Asn :: ostream os;
	answer.encode ( os );
	reply.push_back ( os.str ( ) );
	return true;
}

bool H323AnswerLegThread :: handleMasterSlaveDetermination ( const H245 :: MasterSlaveDetermination & mesg,
	H225 :: ArrayOf_Asn_OctetString & reply ) {
	H245 :: MultimediaSystemControlMessage answer;
	Asn :: ostream os;
	const unsigned terminalType = 120;
	if ( ! masterSlaveSent ) {
		answer.setTag ( H245 :: MultimediaSystemControlMessage :: e_request );
		H245 :: RequestMessage & request = answer;
		request.setTag ( H245 :: RequestMessage :: e_masterSlaveDetermination );
		H245 :: MasterSlaveDetermination & determination = request;
		determination.m_terminalType = terminalType;
		if ( terminalType == mesg.m_terminalType )
			while ( determinationNumber == mesg.m_statusDeterminationNumber ||
				( determinationNumber ^ mesg.m_statusDeterminationNumber ) == 0x800000 )
				determinationNumber = Random :: number ( ) % 16777216;
		determination.m_statusDeterminationNumber = determinationNumber;
		answer.encode ( os );
		reply.push_back ( os.str ( ) );
		os.clear ( );
	} else
		masterSlaveSent = false;
	answer.setTag ( H245 :: MultimediaSystemControlMessage :: e_response );
	H245 :: ResponseMessage & response = answer;
	response.setTag ( H245 :: ResponseMessage :: e_masterSlaveDeterminationAck );
	H245 :: MasterSlaveDeterminationAck & ack = response;
	if ( mesg.m_terminalType < terminalType )
		ack.m_decision.setTag ( H245 :: MasterSlaveDeterminationAck_decision :: e_slave );
	else if ( mesg.m_terminalType > terminalType )
		ack.m_decision.setTag ( H245 :: MasterSlaveDeterminationAck_decision :: e_master );
	else if ( mesg.m_statusDeterminationNumber < determinationNumber )
		ack.m_decision.setTag ( H245 :: MasterSlaveDeterminationAck_decision :: e_master );
	else
		ack.m_decision.setTag ( H245 :: MasterSlaveDeterminationAck_decision :: e_slave );
	answer.encode ( os );
	reply.push_back ( os.str ( ) );
	return true;
}

bool H323AnswerLegThread :: encodeCapabilitySet ( unsigned seqNum, const Asn :: ObjectId & pid,
	H225 :: ArrayOf_Asn_OctetString & reply ) {
	H245 :: MultimediaSystemControlMessage answer;
	answer.setTag ( H245 :: MultimediaSystemControlMessage :: e_request );
	H245 :: RequestMessage & request = answer;
	request.setTag ( H245 :: RequestMessage :: e_terminalCapabilitySet );
	H245 :: TerminalCapabilitySet & caps = request;
	caps.m_sequenceNumber = seqNum;
	caps.m_protocolIdentifier = pid;
	caps.includeOptionalField ( H245 :: TerminalCapabilitySet :: e_capabilityTable );
	H245 :: TerminalCapabilitySet_capabilityTable & table = caps.get_capabilityTable ( );
	table.setSize ( 2 );
	H245 :: CapabilityTableEntry & entry = table [ 0 ];
	entry.m_capabilityTableEntryNumber = 1;
	entry.includeOptionalField ( H245 :: CapabilityTableEntry :: e_capability );
	H245 :: Capability & cap = entry.get_capability ( );
	cap.setTag ( H245 :: Capability :: e_receiveAudioCapability );
	H245 :: AudioCapability & audioData = cap;

	if ( ! :: setCodec ( audioData, answeredCodec.getCodec ( ) ) )
		return false;
	if ( answeredCodec.getFrames ( ) )
		setFrames ( audioData, answeredCodec.getFrames ( ) );
	caps.includeOptionalField ( H245 :: TerminalCapabilitySet :: e_capabilityDescriptors );
	H245 :: TerminalCapabilitySet_capabilityDescriptors & descrs = caps.get_capabilityDescriptors ( );
	descrs.setSize ( 1 );
	H245 :: CapabilityDescriptor & descr = descrs [ 0 ];
	descr.m_capabilityDescriptorNumber = 1;
	descr.includeOptionalField ( H245 :: CapabilityDescriptor :: e_simultaneousCapabilities );
	H245 :: CapabilityDescriptor_simultaneousCapabilities & capSetArray =
		descr.get_simultaneousCapabilities ( );
	capSetArray.setSize ( 1 );
	H245 :: AlternativeCapabilitySet & capSet = capSetArray [ 0 ];
	capSet.setSize ( 1 );
	capSet [ 0 ] = 1;
	H245 :: CapabilityTableEntry & entry2 = table [ 1 ];
	entry2.m_capabilityTableEntryNumber = 2;
	entry2.includeOptionalField ( H245 :: CapabilityTableEntry :: e_capability );
	H245 :: Capability & cap2 = entry2.get_capability ( );
	cap2.setTag ( H245 :: Capability :: e_receiveRTPAudioTelephonyEventCapability );
	H245 :: AudioTelephonyEventCapability & dtmfData = cap2;
	dtmfData.m_audioTelephoneEvent = "0-15";
	if ( details.common.getTelephoneEventsPayloadType ( ) == 0 )
		dtmfData.m_dynamicRTPPayloadType = 101;
	else
		dtmfData.m_dynamicRTPPayloadType = details.common.getTelephoneEventsPayloadType ( );
	Asn :: ostream os;
	answer.encode ( os );
	reply.push_back ( os.str ( ) );
	return true;
}

bool H323AnswerLegThread :: handleCapabilitySet ( const H245 :: TerminalCapabilitySet & mesg,
	H225 :: ArrayOf_Asn_OctetString & reply ) {
	H245 :: MultimediaSystemControlMessage answer;
	answer.setTag ( H245 :: MultimediaSystemControlMessage :: e_response );
	H245 :: ResponseMessage & response = answer;
	response.setTag ( H245 :: ResponseMessage :: e_terminalCapabilitySetAck );
	H245 :: TerminalCapabilitySetAck & ack = response;
	ack.m_sequenceNumber = mesg.m_sequenceNumber;
	Asn :: ostream os;
	answer.encode ( os );
	reply.push_back ( os.str ( ) );
	if ( capSetSent ) {
		return true;
		capSetSent = false;
	}
	return encodeCapabilitySet ( mesg.m_sequenceNumber, mesg.m_protocolIdentifier, reply );
}

bool H323AnswerLegThread :: handleOpenLogicalChannel ( const H245 :: OpenLogicalChannel & mesg,
	H225 :: ArrayOf_Asn_OctetString & reply ) {
	if ( mesg.hasOptionalField ( H245 :: OpenLogicalChannel :: e_reverseLogicalChannelParameters ) ) {
		PSYSTEMLOG ( Error, "open with reverse params - unsupported" );
		return false;
	}
	const H245 :: OpenLogicalChannel_forwardLogicalChannelParameters & oldForwardParams =
		mesg.m_forwardLogicalChannelParameters;
//	if ( forwardParams.m_dataType.getTag ( ) != H245 :: DataType :: e_audioData ) {
//		PSYSTEMLOG ( Error, "unsupported dataType tag: " << forwardParams.m_dataType.getTag ( ) );
//		return false;
//	}
	const H245 :: OpenLogicalChannel_forwardLogicalChannelParameters_multiplexParameters & multiParams =
		oldForwardParams.m_multiplexParameters;
	if ( multiParams.getTag ( ) !=
		H245 :: OpenLogicalChannel_forwardLogicalChannelParameters_multiplexParameters ::
		e_h2250LogicalChannelParameters ) {
		PSYSTEMLOG ( Error, "unsupported multiParams tag: " << multiParams.getTag ( ) );
		return false;
	}
	const H245 :: H2250LogicalChannelParameters & oldH225Params = multiParams;
	if ( oldH225Params.hasOptionalField ( H245 :: H2250LogicalChannelParameters :: e_mediaChannel ) ) {
		PSYSTEMLOG ( Error, "open with mediaChannel - unsupported" );
		return false;
	}
	if ( ! oldH225Params.hasOptionalField ( H245 :: H2250LogicalChannelParameters :: e_mediaControlChannel ) ) {
		PSYSTEMLOG ( Error, "no mediaControlChannel - not supported" );
		return false;
	}
	const H245 :: TransportAddress & addr = oldH225Params.get_mediaControlChannel ( );
	const H245 :: UnicastAddress_iPAddress * ip;
	PIPSocket :: Address ipAddr;
	if ( ! checkAddr ( addr, ip, ipAddr ) )
		return false;
	CodecInfo c = :: getCodec ( oldForwardParams.m_dataType );
	call -> setSendAddress ( this, ipAddr, ip -> m_tsapIdentifier & ~ 1, c, c, c, c );
	H245 :: MultimediaSystemControlMessage answer;
	answer.setTag ( H245 :: MultimediaSystemControlMessage :: e_response );
	H245 :: ResponseMessage & response = answer;
	response.setTag ( H245 :: ResponseMessage :: e_openLogicalChannelAck );
	H245 :: OpenLogicalChannelAck & ack = response;
	ack.m_forwardLogicalChannelNumber = mesg.m_forwardLogicalChannelNumber;
	ack.includeOptionalField ( H245 :: OpenLogicalChannelAck :: e_forwardMultiplexAckParameters );
	H245 :: OpenLogicalChannelAck_forwardMultiplexAckParameters & forwardParams =
		ack.get_forwardMultiplexAckParameters ( );
	forwardParams.setTag ( H245 :: OpenLogicalChannelAck_forwardMultiplexAckParameters ::
		e_h2250LogicalChannelAckParameters );
	H245 :: H2250LogicalChannelAckParameters & h225Params = forwardParams;
	h225Params.includeOptionalField ( H245 :: H2250LogicalChannelAckParameters :: e_sessionID );
	h225Params.get_sessionID ( ) = 1;
	PIPSocket :: Address localAddr;
	sock -> GetLocalAddress ( localAddr );
	int localPort = call -> getLocalPort ( this );
	h225Params.includeOptionalField ( H245 :: H2250LogicalChannelAckParameters :: e_mediaControlChannel );
	H245 :: TransportAddress & t1 = h225Params.get_mediaControlChannel ( );
	t1.setTag ( H245 :: TransportAddress :: e_unicastAddress );
	H245 :: UnicastAddress & u1 = t1;
	u1.setTag ( H245 :: UnicastAddress :: e_iPAddress );
	setIPAddress ( u1, localAddr, localPort + 1 );
	h225Params.includeOptionalField ( H245 :: H2250LogicalChannelAckParameters :: e_mediaChannel );
	H245 :: TransportAddress & t2 = h225Params.get_mediaChannel ( );
	t2.setTag ( H245 :: TransportAddress :: e_unicastAddress );
	H245 :: UnicastAddress & u2 = t2;
	u2.setTag ( H245 :: UnicastAddress :: e_iPAddress );
	setIPAddress ( u2, localAddr, localPort );
	Asn :: ostream os;
	answer.encode ( os );
	reply.push_back ( os.str ( ) );
	os.clear ( );

	answer.setTag ( H245 :: MultimediaSystemControlMessage :: e_request );
	H245 :: RequestMessage & request = answer;
	request.setTag ( H245 :: RequestMessage :: e_openLogicalChannel );
	H245 :: OpenLogicalChannel & olc = request;
	olc.m_forwardLogicalChannelNumber = 1;
	H245 :: OpenLogicalChannel_forwardLogicalChannelParameters & forwardParams2 =
		olc.m_forwardLogicalChannelParameters;
	forwardParams2.m_dataType = oldForwardParams.m_dataType;
	H245 :: OpenLogicalChannel_forwardLogicalChannelParameters_multiplexParameters & multiParams2 =
		forwardParams2.m_multiplexParameters;
	multiParams2.setTag ( H245 :: OpenLogicalChannel_forwardLogicalChannelParameters_multiplexParameters
		:: e_h2250LogicalChannelParameters );
	H245 :: H2250LogicalChannelParameters & h225Params2 = multiParams2;
	h225Params2.m_sessionID = 1;
	h225Params2.includeOptionalField ( H245 :: H2250LogicalChannelParameters :: e_mediaControlChannel );
	H245 :: TransportAddress & t3 = h225Params2.get_mediaControlChannel ( );
	t3.setTag ( H245 :: TransportAddress :: e_unicastAddress );
	H245 :: UnicastAddress & u3 = t3;
	u3.setTag ( H245 :: UnicastAddress :: e_iPAddress );
	setIPAddress ( u3, localAddr, localPort + 1 );
	answer.encode ( os );
	reply.push_back ( os.str ( ) );
	return true;
}


bool H323AnswerLegThread :: handleCloseLogicalChannel ( const H245 :: CloseLogicalChannel & mesg,
	H225 :: ArrayOf_Asn_OctetString & reply ) {
	H245 :: MultimediaSystemControlMessage answer;
	answer.setTag ( H245 :: MultimediaSystemControlMessage :: e_response );
	H245 :: ResponseMessage & response = answer;
	response.setTag ( H245 :: ResponseMessage :: e_closeLogicalChannelAck );
	H245 :: CloseLogicalChannelAck & ack = response;
	ack.m_forwardLogicalChannelNumber = mesg.m_forwardLogicalChannelNumber;
	Asn :: ostream os;
	answer.encode ( os );
	reply.push_back ( os.str ( ) );
	return true;
}

bool H323AnswerLegThread :: handleRequestMode ( const H245 :: RequestMode & mesg,
	H225 :: ArrayOf_Asn_OctetString & reply ) {
	PSYSTEMLOG ( Info, "(storm): H323Leg.handleRequestMode" );
	H245 :: MultimediaSystemControlMessage answer;
	answer.setTag ( H245 :: MultimediaSystemControlMessage :: e_response );
	H245 :: ResponseMessage & response = answer;
	response.setTag ( H245 :: ResponseMessage :: e_requestModeAck );
	H245 :: RequestModeAck & ack = response;
	ack.m_sequenceNumber = mesg.m_sequenceNumber;
	H245 :: RequestModeAck_response & resp = ack.m_response;
	resp.setTag ( H245 :: RequestModeAck_response :: e_willTransmitMostPreferredMode );
	Asn :: ostream os;
	answer.encode ( os );
	reply.push_back ( os.str ( ) );
	return true;
}

void H323AnswerLegThread :: setTunneling ( H225 :: H323_UU_PDU & pdu ) {
	pdu.includeOptionalField ( H225 :: H323_UU_PDU :: e_h245Tunneling );
	pdu.get_h245Tunneling ( ) = useTunneling;
}

void H323AnswerLegThread :: sendProceeding ( ) {
	Q931 mesg ( Q931 :: msgCallProceeding, details.ref, true );
	H225 :: H323_UserInformation uuField;
	H225 :: H323_UU_PDU & pdu = uuField.m_h323_uu_pdu;
	setTunneling ( pdu );
	pdu.m_h323_message_body.setTag ( H225 :: H323_UU_PDU_h323_message_body :: e_callProceeding );
	H225 :: CallProceeding_UUIE & proceeding = pdu.m_h323_message_body;
	proceeding.m_protocolIdentifier = h225ProtocolID;
	proceeding.get_callIdentifier ( ) = details.id;
	encodeH225IntoQ931 ( uuField, mesg );
	sendMesgToDest ( mesg, & uuField, sock, common );
}

void H323AnswerLegThread :: sendWithFacility ( Q931 & mesg, H225 :: H323_UserInformation & uuField ) {
	H225 :: H323_UU_PDU & pdu = uuField.m_h323_uu_pdu;
	setTunneling ( pdu );
	if ( ! common.getSigOptions ( ).useFacility ( ) || ! pdu.hasOptionalField ( H225 :: H323_UU_PDU :: e_h245Control ) ) {
		encodeH225IntoQ931 ( uuField, mesg );
		sendMesgToDest ( mesg, & uuField, sock, common );
		return;
	}
	Q931 fac ( Q931 :: msgFacility, details.ref, true );
	H225 :: H323_UserInformation fUuField;
	H225 :: H323_UU_PDU & fPdu = fUuField.m_h323_uu_pdu;
	fPdu.m_h323_message_body.setTag ( H225 :: H323_UU_PDU_h323_message_body :: e_empty );
	fPdu.includeOptionalField ( H225 :: H323_UU_PDU :: e_h245Control );
	fPdu.get_h245Control ( ) = pdu.get_h245Control ( );
	pdu.removeOptionalField ( H225 :: H323_UU_PDU :: e_h245Control );
	setTunneling ( fPdu );
	encodeH225IntoQ931 ( uuField, mesg );
	sendMesgToDest ( mesg, & uuField, sock, common );
	encodeH225IntoQ931 ( fUuField, fac );
	sendMesgToDest ( fac, & uuField, sock, common );
}

void H323AnswerLegThread :: sendAlerting ( int localRtpPort, const PIPSocket :: Address & localAddr,
	const CodecInfo & inCodec, const CodecInfo & outCodec ) {
	Q931 mesg ( Q931 :: msgAlerting, details.ref, true );
	H225 :: H323_UserInformation uuField;
	H225 :: H323_UU_PDU & pdu = uuField.m_h323_uu_pdu;
	pdu.m_h323_message_body.setTag ( H225 :: H323_UU_PDU_h323_message_body :: e_alerting );
	H225 :: Alerting_UUIE & alerting = pdu.m_h323_message_body;
	alerting.m_protocolIdentifier = h225ProtocolID;
	alerting.get_callIdentifier ( ) = details.id;
	alerting.includeOptionalField ( H225 :: Alerting_UUIE :: e_fastStart );
	if ( inCodec.getCodec ( ) != "unknown" )
		handleFastStartAndH245 ( mesg, uuField, alerting.get_fastStart ( ), localRtpPort, localAddr,
			inCodec, outCodec );
	if ( alerting.get_fastStart ( ).empty ( ) )
		alerting.removeOptionalField ( H225 :: Alerting_UUIE :: e_fastStart );
	sendWithFacility ( mesg, uuField );
}

void H323AnswerLegThread :: sendConnect ( int localRtpPort, const PIPSocket :: Address & localAddr,
	const CodecInfo & inCodec, const CodecInfo & outCodec ) {
	PSYSTEMLOG ( Info, "sendConnect with addr " << localAddr );
	Q931 mesg ( Q931 :: msgConnect, details.ref, true );
	mesg.setBearerCapabilities ( Q931 :: tcSpeech, 1, Q931 :: csCCITT, Q931 :: uil1G711A );
	H225 :: H323_UserInformation uuField;
	H225 :: H323_UU_PDU & pdu = uuField.m_h323_uu_pdu;
	pdu.m_h323_message_body.setTag ( H225 :: H323_UU_PDU_h323_message_body :: e_connect );
	H225 :: Connect_UUIE & connect = pdu.m_h323_message_body;
	connect.m_protocolIdentifier = h225ProtocolID;
	connect.get_callIdentifier ( ) = details.id;
	connect.m_conferenceID = details.id.m_guid;
	connect.includeOptionalField ( H225 :: Connect_UUIE :: e_fastStart );
	handleFastStartAndH245 ( mesg, uuField, connect.get_fastStart ( ), localRtpPort, localAddr, inCodec, outCodec );
	if ( connect.get_fastStart ( ).empty ( ) )
		connect.removeOptionalField ( H225 :: Connect_UUIE :: e_fastStart );
	sendWithFacility ( mesg, uuField );
}

void H323AnswerLegThread :: peerAlertedLeg ( int localRtpPort, const PIPSocket :: Address & localAddr,
	const CodecInfo & inCodec, const CodecInfo & outCodec ) {
	answeredCodec = inCodec;
	sendAlerting ( localRtpPort, localAddr, inCodec, outCodec );
}

void H323AnswerLegThread :: peerConnectedLeg ( int localRtpPort, const PIPSocket :: Address & localAddr,
	const CodecInfo & inCodec, const CodecInfo & outCodec ) {
	answeredCodec = inCodec;
	sendConnect ( localRtpPort, localAddr, inCodec, outCodec );
}

bool H323AnswerLegThread :: checkAddr ( const H245 :: TransportAddress & addr,
	const H245 :: UnicastAddress_iPAddress * & ip, PIPSocket :: Address & ipAddr ) {
	if ( addr.getTag ( ) != H245 :: TransportAddress :: e_unicastAddress ) {
		PSYSTEMLOG ( Error, "RTP Addr isn't e_unicastAddress" );
		return false;
	}
	const H245 :: UnicastAddress & unicastAddr = addr;
	if ( unicastAddr.getTag ( ) != H245 :: UnicastAddress :: e_iPAddress ) {
		PSYSTEMLOG ( Error, "RTP Addr isn't e_iPAddress" );
		return false;
	}
	ip = & static_cast < const H245 :: UnicastAddress_iPAddress & > ( unicastAddr );
	const Asn :: string & nw = ip -> m_network;
	ipAddr = PIPSocket :: Address ( nw [ 0 ], nw [ 1 ], nw [ 2 ], nw [ 3 ] );
	if ( ! admissibleIP ( ipAddr ) ) {
		PSYSTEMLOG ( Error, "RTP Addr isn't admissible" );
		return false;
	}
	if ( ! admissiblePort ( ip -> m_tsapIdentifier ) ) {
		PSYSTEMLOG ( Error, "RTP Port " << ip -> m_tsapIdentifier << " isn't admissible" );
		return false;
	}
	return true;
}

void H323AnswerLegThread :: init ( ) {
	if ( sock -> IsOpen ( ) )
		sendProceeding ( );
}

bool H323AnswerLegThread :: ended ( ) const {
	return ! sock -> IsOpen ( );
}

void H323AnswerLegThread :: checkPeerMessages ( ) {
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

bool H323AnswerLegThread :: iteration ( ) {
	int selection;
	if ( h245Sock )
		selection = PSocket :: Select ( * sock, * h245Sock, 3000 );
	else
		selection = PSocket :: Select ( * sock, * sock, 3000 );
	if ( selection > 0 && selection != PChannel :: Interrupted ) {
		PSYSTEMLOG ( Error, "select: " << PChannel :: GetErrorText (
			static_cast < PChannel :: Errors > ( selection ) ) );
		return false;
	}
	checkPeerMessages ( );
	bool availableH225 = ( ! h245Sock && selection < 0 ) ||
		( h245Sock && ( selection == -1 || selection == - 3 ) );
	bool availableH245 = h245Sock && ( selection == -2 || selection == - 3 );
	if ( availableH225 && ! receiveMesg ( ) )
		return false;
	if ( availableH245 && ! receiveH245Mesg ( ) )
		return false;
	return true;
}

H323AnswerLegThread :: H323AnswerLegThread ( H323Call * c, LegThread * * p, PTCPSocket * s, unsigned i,
	const CallDetails & d ) : AnswerLegThread ( c, p, i, details.common ),
	details ( d ), sock ( s ), h245Sock ( 0 ), determinationNumber ( Random :: number ( ) % 16777216 ),
	sourceIsVoIP ( false ), useTunneling ( false ), rcSent ( false ), masterSlaveSent ( false ), capSetSent ( false ) {
	if ( ! handleSetup ( details.setup, details.uuField ) )
		shutDown ( Q931 :: cvNormalUnspecified );
	Resume ( );
}

H323AnswerLegThread :: ~H323AnswerLegThread ( ) {
	safeDel ( sock );
	safeDel ( h245Sock );
}

void H323AnswerLegThread :: sendStatus ( ) {
	Q931 mesg ( Q931 :: msgStatus, details.ref, true );
	H225 :: H323_UserInformation uuField;
	H225 :: H323_UU_PDU & pdu = uuField.m_h323_uu_pdu;
	pdu.m_h323_message_body.setTag ( H225 :: H323_UU_PDU_h323_message_body :: e_status );
	H225 :: Status_UUIE & status = pdu.m_h323_message_body;
	status.m_protocolIdentifier = h225ProtocolID;
	status.m_callIdentifier = details.id;
	encodeH225IntoQ931 ( uuField, mesg );
	sendMesgToDest ( mesg, & uuField, sock, common );
}

void H323AnswerLegThread :: putMessage ( const PeerMessage & m ) {
	AutoMutex am ( qm );
	peerQueue.push ( m );
	PXAbortBlock ( );
}
