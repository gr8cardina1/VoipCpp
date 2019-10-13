#pragma implementation
#include "ss.hpp"
#include "allocatable.hpp"
#include "pointer.hpp"
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
#include "h225.hpp"
#include "latencylimits.hpp"
#include "fakecalldetector.hpp"
#include "h323leg.hpp"
#include "h323common.hpp"
#include "h323.hpp"
#include "AddrUtils.h"
#include "Log.h"
#include "ipport.hpp"
#include "icqcontact.hpp"
#include "radgwinfo.hpp"
#include "Conf.hpp"
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>
#include <ptlib/svcproc.h>
#include <tr1/tuple>
#include <boost/range.hpp>
#include "slprint.hpp"
#include "random.hpp"

bool H323Leg :: initChoice ( ) {
	try {
		sendSetup ( );
	} catch ( std :: logic_error & e ) {
		PSYSTEMLOG ( Error, "exception: " << e.what ( ) );
		return false;
	}
	return true;
}

bool H323Leg :: iteration ( ) {
	int selection;
	if ( h245Sock )
		selection = PSocket :: Select ( * sock, * h245Sock, 3000 );
	else
		selection = PSocket :: Select ( * sock, * sock, 3000 );
	if ( selection > 0 ) {
		PSYSTEMLOG ( Error, "select: " << PChannel :: GetErrorText (
			static_cast < PChannel :: Errors > ( selection ) ) );
		return selection == PChannel :: Interrupted;
	}
	bool availableH225 = ( ! h245Sock && selection < 0 ) || ( h245Sock && ( selection == -1 || selection == - 3 ) );
	bool availableH245 = h245Sock && ( selection == -2 || selection == - 3 );
	if ( availableH225 && ! receiveMesg ( ) ) {
		PSYSTEMLOG ( Info, "receiveMesg failed" );
		return false;
	}
	if ( availableH245 && ! receiveH245Mesg ( ) ) {
		PSYSTEMLOG ( Info, "receiveH245Mesg failed" );
		return false;
	}
	return true;
}

void H323Leg :: calledSocketConnected ( ) {
	PIPSocket :: Address local;
	sock -> GetLocalAddress ( local );
	thread -> calledSocketConnected ( local );
}

bool H323Leg :: tryChoice ( ) {
	if ( tryNextChoiceIterationH323 ( common, sock ) ) {
		calledSocketConnected ( );
		return true;
	}
	return false;
}

bool H323Leg :: ended ( ) const {
	return ! sock -> IsOpen ( );
}

void H323Leg :: addFastStartFromIn ( H225 :: ArrayOf_Asn_OctetString & v, const PIPSocket :: Address & local,
	int port ) {
	int channel = 101;
	for ( RecodeInfoVector :: const_iterator i = inRecodes.begin ( ); i != inRecodes.end ( ); ++ i )
		:: addFastStartFromIn ( v, local, port, i -> codec, channel, false );
}

void H323Leg :: addFastStartFromOut ( H225 :: ArrayOf_Asn_OctetString & v, const PIPSocket :: Address & local,
	int port ) {
	for ( RecodeInfoVector :: const_iterator i = outRecodes.begin ( ); i != outRecodes.end ( ); ++ i )
		:: addFastStartFromOut ( v, local, port, i -> codec, false );
}

void H323Leg :: addFastStart ( H225 :: Setup_UUIE & setup, const PIPSocket :: Address & local ) {
	setup.includeOptionalField ( H225 :: Setup_UUIE :: e_fastStart );
	int port = thread -> getLocalPort ( );
	addFastStartFromIn ( setup.get_fastStart ( ), local, port );
	addFastStartFromOut ( setup.get_fastStart ( ), local, port );
}

static void addCapability ( H245 :: TerminalCapabilitySet_capabilityTable & table, const CodecInfo & codec,
	H245 :: Capability :: Choices capType, int & tableNumber ) {
		PSYSTEMLOG(Info, "addCapability. table size = " << table.size());
	H245 :: CapabilityTableEntry entry;
	entry.m_capabilityTableEntryNumber = tableNumber ++;
	entry.includeOptionalField ( H245 :: CapabilityTableEntry :: e_capability );
	H245 :: Capability & cap = entry.get_capability ( );
	cap.setTag ( capType );
	H245 :: AudioCapability & audioData = cap;
	:: setCodec ( audioData, codec.getCodec ( ) );
	int frames = codec.getFrames ( );
	if ( frames )
		setFrames ( audioData, frames );
	table.push_back ( entry );
}

void H323Leg :: addCaps ( H225 :: ArrayOf_Asn_OctetString & reply ) {
	capsSent = true;
	H245 :: MultimediaSystemControlMessage mesg;
	mesg.setTag ( H245 :: MultimediaSystemControlMessage :: e_request );
	H245 :: RequestMessage & request = mesg;
	request.setTag ( H245 :: RequestMessage :: e_terminalCapabilitySet );
	H245 :: TerminalCapabilitySet & caps = request;
	caps.m_sequenceNumber = 1;
	caps.m_protocolIdentifier = h225ProtocolID;
	caps.includeOptionalField ( H245 :: TerminalCapabilitySet :: e_capabilityTable );
	H245 :: TerminalCapabilitySet_capabilityTable & table = caps.get_capabilityTable ( );
	int tableNumber = 1;
	using boost :: lambda :: _1;
	ss :: for_each ( inRecodes,
		boost :: lambda :: bind ( addCapability, boost :: ref ( table ), & _1 ->* & RecodeInfo :: codec,
		H245 :: Capability :: e_receiveAndTransmitAudioCapability, boost :: ref ( tableNumber ) ) );
//	ss :: for_each ( outRecodes,
//		boost :: lambda :: bind ( addCapability, boost :: ref ( table ), & _1 ->* & RecodeInfo :: codec,
//		H245 :: Capability :: e_transmitAudioCapability, boost :: ref ( tableNumber ) ) );
	caps.includeOptionalField ( H245 :: TerminalCapabilitySet :: e_capabilityDescriptors );
	H245 :: TerminalCapabilitySet_capabilityDescriptors & descrs = caps.get_capabilityDescriptors ( );
	descrs.setSize ( 1 );
	H245 :: CapabilityDescriptor & descr = descrs [ 0 ];
	descr.m_capabilityDescriptorNumber = 1;
	descr.includeOptionalField ( H245 :: CapabilityDescriptor :: e_simultaneousCapabilities );
	H245 :: CapabilityDescriptor_simultaneousCapabilities & capSetArray = descr.get_simultaneousCapabilities ( );
	capSetArray.setSize ( 2 );
	H245 :: AlternativeCapabilitySet & capSet = capSetArray [ 0 ];
	for ( int i = 1; i < tableNumber; i ++ )
		capSet.push_back ( i );

	H245 :: CapabilityTableEntry entry;
	entry.m_capabilityTableEntryNumber = tableNumber ++;
	entry.includeOptionalField ( H245 :: CapabilityTableEntry :: e_capability );
	H245 :: Capability & cap = entry.get_capability ( );
	cap.setTag ( H245 :: Capability :: e_receiveRTPAudioTelephonyEventCapability );
	H245 :: AudioTelephonyEventCapability & dtmfData = cap;
	dtmfData.m_audioTelephoneEvent = "0-15";
	dtmfData.m_dynamicRTPPayloadType = ( common.getTelephoneEventsPayloadType ( ) ? : 101 );
	table.push_back ( entry );
	capSetArray [ 1 ].push_back ( tableNumber - 1 );

	Asn :: ostream os;
	mesg.encode ( os );
	reply.push_back ( os.str ( ) );
}

void H323Leg :: addMasterSlave ( H225 :: ArrayOf_Asn_OctetString & reply,
	const H245 :: MasterSlaveDetermination * incDet ) {
	masterSlaveSent = true;
	H245 :: MultimediaSystemControlMessage mesg;
	mesg.setTag ( H245 :: MultimediaSystemControlMessage :: e_request );
	H245 :: RequestMessage & request = mesg;
	request.setTag ( H245 :: RequestMessage :: e_masterSlaveDetermination );
	H245 :: MasterSlaveDetermination & determination = request;
	const unsigned terminalType = 120;
	determination.m_terminalType = terminalType;
	determinationNumber = Random :: number ( ) % 16777216;
	if ( incDet && terminalType == incDet -> m_terminalType ) {
		while ( determinationNumber == incDet -> m_statusDeterminationNumber ||
			( determinationNumber ^ incDet -> m_statusDeterminationNumber ) == 0x800000 )
			determinationNumber = Random :: number ( ) % 16777216;
	}
	determination.m_statusDeterminationNumber = determinationNumber;
	Asn :: ostream os;
	mesg.encode ( os );
	reply.push_back ( os.str ( ) );
}

void H323Leg :: addH245 ( H225 :: Setup_UUIE & setup ) {
	if ( ! common.curChoice ( ) -> getSigOptions ( ).getPermitTunnelingInSetup ( ) )
		return;
	setup.includeOptionalField ( H225 :: Setup_UUIE :: e_parallelH245Control );
	H225 :: ArrayOf_Asn_OctetString & parallelControl = setup.get_parallelH245Control ( );
	addCaps ( parallelControl );
	addMasterSlave ( parallelControl );
}

void H323Leg :: sendSetup ( ) {
	Q931 q931 ( Q931 :: msgSetup, ref, false );
	q931.setBearerCapabilities ( Q931 :: tcSpeech, 1, Q931 :: csCCITT, Q931 :: uil1G711A );
	q931.setCallingPartyNumber ( common.getCallingDigitsIn ( ) );
	H225 :: H323_UserInformation uuField;
	H225 :: H323_UU_PDU & pdu = uuField.m_h323_uu_pdu;
	H225 :: H323_UU_PDU_h323_message_body & body = pdu.m_h323_message_body;
	body.setTag ( H225 :: H323_UU_PDU_h323_message_body :: e_setup );
	H225 :: Setup_UUIE & setup = body;
	setup.m_protocolIdentifier = h225ProtocolID;
	setup.m_sourceInfo.includeOptionalField ( H225 :: EndpointType :: e_vendor );
	H225 :: H221NonStandard & n = setup.m_sourceInfo.get_vendor ( ).m_vendor;
	n.m_t35CountryCode = 181;
	n.m_t35Extension = 0;
	n.m_manufacturerCode = 18;
	setup.m_sourceInfo.includeOptionalField ( H225 :: EndpointType :: e_gateway );
	H225 :: GatewayInfo & g = setup.m_sourceInfo.get_gateway ( );
	g.includeOptionalField ( H225 :: GatewayInfo :: e_protocol );
	H225 :: ArrayOf_SupportedProtocols & a = g.get_protocol ( );
	a.setSize ( 1 );
	a [ 0 ].setTag ( H225 :: SupportedProtocols :: e_voice );
	H225 :: VoiceCaps & c = a [ 0 ];
	c.includeOptionalField ( H225 :: VoiceCaps :: e_supportedPrefixes );
	setup.m_conferenceGoal.setTag ( H225 :: Setup_UUIE_conferenceGoal :: e_create );
	setup.m_callType.setTag ( H225 :: CallType :: e_pointToPoint );
	setup.m_conferenceID = callId.m_guid;
	setup.includeOptionalField ( H225 :: Setup_UUIE :: e_callIdentifier );
	setup.get_callIdentifier ( ) = callId;
	PIPSocket :: Address sourceAddr;
	WORD sourcePort;
	PIPSocket :: Address destAddr;
	WORD destPort;
	sock -> GetLocalAddress ( sourceAddr, sourcePort );
	sock -> GetPeerAddress ( destAddr, destPort );
	translateSetup ( common, q931, callId, q931, uuField,
		AddrUtils :: convertToH225TransportAddr ( sourceAddr, sourcePort ),
		AddrUtils :: convertToH225TransportAddr ( destAddr, destPort ) );
	if ( ! common.curChoice ( ) -> getSigOptions ( ).getDropFastStart ( ) ) {
		PIPSocket :: Address localAddr = thread -> getLocalIp ( );
		if ( localAddr == INADDR_ANY )
			localAddr = sourceAddr;
		addFastStart ( setup, localAddr );
	}
	pdu.includeOptionalField ( H225 :: H323_UU_PDU :: e_h245Tunneling );
	if ( useTunneling ) {
		pdu.get_h245Tunneling ( ) = true;
		if ( common.curChoice ( ) -> getSigOptions ( ).getPermitTunnelingInSetup ( ) )
			addH245 ( setup );
	} else
		pdu.get_h245Tunneling ( ) = false;
	encodeH225IntoQ931 ( uuField, q931 );
	sendMesgToDest ( q931, & uuField, sock, common );
	int outPeerId = common.curChoice ( ) -> getPeer ( );
	_fakeCallDetector.initialize ( outPeerId, conf -> getMesgLatency ( outPeerId ) );
}

void H323Leg :: sendReleaseComplete ( ) {
	if ( ! rcSent ) {
		:: sendReleaseComplete ( sock, common, callId, ref, true );
		rcSent = true;
	}
}

void H323Leg :: closeChoice ( ) {
	if ( h245Sock ) {
		if ( h245Sock -> IsOpen ( ) )
			h245Sock -> Close ( );
		safeDel ( h245Sock );
	}
	if ( sock ) {
		if ( sock -> IsOpen ( ) ) {
			common.setDisconnectCauseWeak ( Q931 :: cvNormalUnspecified );
			sendReleaseComplete ( );
			sock -> Close ( );
		}
		safeDel ( sock );
	}
}

void H323Leg :: disableTunneling ( ) {
	if ( ! useTunneling )
		return;
	useTunneling = masterSlaveSent = capsSent = false;
}

bool H323Leg :: handleH245 ( const H225 :: H323_UU_PDU & origPdu ) {
	if ( ! origPdu.hasOptionalField ( H225 :: H323_UU_PDU :: e_provisionalRespToH245Tunneling ) &&
		( ! origPdu.hasOptionalField ( H225 :: H323_UU_PDU :: e_h245Tunneling ) ||
		! origPdu.get_h245Tunneling ( ) ) )
		disableTunneling ( );
	if ( ! origPdu.hasOptionalField ( H225 :: H323_UU_PDU :: e_h245Control ) )
		return true;
	const H225 :: ArrayOf_Asn_OctetString & control = origPdu.get_h245Control ( );
	Q931 facility ( Q931 :: msgFacility, ref, false );
	H225 :: H323_UserInformation uuField;
	H225 :: H323_UU_PDU & pdu = uuField.m_h323_uu_pdu;
	pdu.m_h323_message_body.setTag ( H225 :: H323_UU_PDU_h323_message_body :: e_empty );
	pdu.includeOptionalField ( H225 :: H323_UU_PDU :: e_h245Tunneling );
	pdu.get_h245Tunneling ( ) = true;
	pdu.includeOptionalField ( H225 :: H323_UU_PDU :: e_h245Control );
/// For DTMF handling.

	PSYSTEMLOG(Info, "H323Leg :: handleH245.sends message:\n" << facility);

	H225 :: ArrayOf_Asn_OctetString & reply = pdu.get_h245Control ( );
		PSYSTEMLOG(Info, "H323Leg :: handleH245.1.");
	for ( unsigned i = 0; i < control.size ( ); i ++ ) {
		PSYSTEMLOG(Info, "H323Leg :: handleH245.2 size:" << control.size ( ));
		try {
			Asn :: istream is ( control [ i ] );
		PSYSTEMLOG(Info, "H323Leg :: handleH245.3");
			H245 :: MultimediaSystemControlMessage mesg ( is );
		PSYSTEMLOG(Info, "H323Leg :: handleH245.3");
			if ( is.hasException ( ) ) {
				PSYSTEMLOG ( Error, "tunneled h245: can't decode" << control [ i ] << ": " << is.getException ( ) );
				return false;
			}
		PSYSTEMLOG(Info, "H323Leg :: handleH245.4");
			if ( ! processH245Message ( mesg, reply ) )
				return false;
		PSYSTEMLOG(Info, "H323Leg :: handleH245.5");
		} catch ( std :: exception & e ) {
			PSYSTEMLOG ( Error, "tunneled h245: can't decode control [ " << i << " ] : " << e.what ( ) );
			return false;
		}
	}
	if ( pdu.get_h245Control ( ).empty ( ) )
		return true;
	encodeH225IntoQ931 ( uuField, facility );
	PSYSTEMLOG(Info, "H323Leg :: handleH245.sends message:\n" << facility);
	sendMesgToDest ( facility, & uuField, sock, common );
	return true;
}

bool H323Leg :: receiveH245Mesg ( ) {
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
	PSYSTEMLOG ( Info, "Received H245 message" );
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
		H225 :: ArrayOf_Asn_OctetString reply;
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

bool H323Leg :: processH245Message ( const H245 :: MultimediaSystemControlMessage & mesg,
	H225 :: ArrayOf_Asn_OctetString & reply ) {
	PSYSTEMLOG ( Info, "H323Leg :: processH245Message : " << mesg );
	switch ( mesg.getTag ( ) ) {
		case H245 :: MultimediaSystemControlMessage :: e_request:
	PSYSTEMLOG ( Info, "H323Leg :: processH245Message e_request" );
			return handleRequest ( mesg, reply );
		case H245 :: MultimediaSystemControlMessage :: e_response:
	PSYSTEMLOG ( Info, "H323Leg :: processH245Message e_response" );
			return handleResponse ( mesg, reply );
		case H245 :: MultimediaSystemControlMessage :: e_command:
	PSYSTEMLOG ( Info, "H323Leg :: processH245Message e_command" );
			return handleCommand ( mesg, reply );
		case H245 :: MultimediaSystemControlMessage :: e_indication:
	PSYSTEMLOG ( Info, "H323Leg :: processH245Message e_indication" );
			return handleIndication ( mesg, reply );
		default :
			PSYSTEMLOG ( Error, "Unknown H245 message type not handled : " << mesg.getTag ( ) );
			return false;
	}
}

bool H323Leg :: handleIndication ( const H245 :: IndicationMessage & mesg,
	H225 :: ArrayOf_Asn_OctetString & reply ) {
	switch ( mesg.getTag ( ) ) {
		case H245 :: IndicationMessage :: e_masterSlaveDeterminationRelease:
			return handleMasterSlaveDeterminationRelease ( mesg, reply );
		case H245 :: IndicationMessage :: e_userInput:
			return handleUserInput ( mesg, reply );
		case H245 :: IndicationMessage :: e_terminalCapabilitySetRelease:
			return handleTerminalCapabilitySetRelease ( mesg, reply );
		default :
			PSYSTEMLOG ( Error, "unknown indication tag: " << mesg.getTag ( ) );
			return false;
	}
}

bool H323Leg :: handleMasterSlaveDeterminationRelease (
	const H245 :: MasterSlaveDeterminationRelease & /*mesg*/, H225 :: ArrayOf_Asn_OctetString & /*reply*/ ) {
	PSYSTEMLOG ( Error, "master/slave determination released - something went wrong" );
	return true;
}

bool H323Leg :: handleUserInput ( const H245 :: UserInputIndication & /*mesg*/,
	H225 :: ArrayOf_Asn_OctetString & /*reply*/ ) {
	PSYSTEMLOG ( Error, "user input transfer not supported" );
	return true;
}

bool H323Leg :: handleTerminalCapabilitySetRelease (
	const H245 :: TerminalCapabilitySetRelease & /*mesg*/, H225 :: ArrayOf_Asn_OctetString & /*reply*/ ) {
	PSYSTEMLOG ( Error, "capability set released - something went wrong" );
	return true;
}

bool H323Leg :: handleEndSession ( const H245 :: EndSessionCommand & mesg,
	H225 :: ArrayOf_Asn_OctetString & /*reply*/ ) {
	switch ( mesg.getTag ( ) ) {
		case H245 :: EndSessionCommand :: e_disconnect:
			PSYSTEMLOG ( Info, "endSession disconnect" );
			if ( thread -> isGoodCall ( ) ) {
				PSYSTEMLOG ( Info, "Normal call clearing" );
				thread -> released ( Q931 :: cvNormalCallClearing );
				return false;
			}
			return false;
		default:
			PSYSTEMLOG ( Error, "unknown endSession tag: " << mesg.getTag ( ) );
			return false;
	}
}

bool H323Leg :: handleCommand ( const H245 :: CommandMessage & mesg,
	H225 :: ArrayOf_Asn_OctetString & reply ) {
	switch ( mesg.getTag ( ) ) {
		case H245 :: CommandMessage :: e_endSessionCommand:
			return handleEndSession ( mesg, reply );
		default :
			PSYSTEMLOG ( Error, "unknown command tag: " << mesg.getTag ( ) );
			return false;
	}
}

bool H323Leg :: handleResponse ( const H245 :: ResponseMessage & mesg,
	H225 :: ArrayOf_Asn_OctetString & /*reply*/ ) {
	switch ( mesg.getTag ( ) ) {
		case H245 :: ResponseMessage :: e_terminalCapabilitySetAck:
			return true;
		case H245 :: ResponseMessage :: e_masterSlaveDeterminationAck:
			return true;
		case H245 :: ResponseMessage :: e_masterSlaveDeterminationReject:
			return true;
		case H245 :: ResponseMessage :: e_openLogicalChannelAck:
			return handleOpenLogicalChannelAck ( mesg );
		default :
			PSYSTEMLOG ( Error, "unknown response tag: " << mesg.getTag ( ) );
			return false;
	}
}

bool H323Leg :: handleOpenLogicalChannelAck ( const H245 :: OpenLogicalChannelAck & msg ) {
	if ( ! msg.hasOptionalField ( H245 :: OpenLogicalChannelAck :: e_forwardMultiplexAckParameters ) ) {
		PSYSTEMLOG ( Error, "no forwardMultiplexAckParameters - not supported" );
		return false;
	}
	const H245 :: OpenLogicalChannelAck_forwardMultiplexAckParameters & ackparam =
		msg.get_forwardMultiplexAckParameters ( );
	if ( ackparam.getTag ( ) != H245 :: OpenLogicalChannelAck_forwardMultiplexAckParameters ::
		e_h2250LogicalChannelAckParameters ) {
		PSYSTEMLOG ( Error, "unsupported ackparams tag: " << ackparam.getTag ( ) );
		return false;
	}
	const H245 :: H2250LogicalChannelAckParameters & h225Params = ackparam;
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
	const H245 :: TransportAddress & controlAddr = h225Params.get_mediaControlChannel ( );
	const H245 :: UnicastAddress_iPAddress * controlIp;
	PIPSocket :: Address controlIpAddr;
	if ( ! checkAddr ( controlAddr, controlIp, controlIpAddr ) )
		return false;
	const H245 :: TransportAddress & mediaAddr = h225Params.get_mediaChannel ( );
	const H245 :: UnicastAddress_iPAddress * mediaIp;
	PIPSocket :: Address mediaIpAddr;
	if ( ! checkAddr ( mediaAddr, mediaIp, mediaIpAddr ) )
		return false;
	thread -> setCodec ( inRecodes.front ( ).backCodec );
	thread -> setSendAddress ( mediaIpAddr, mediaIp -> m_tsapIdentifier, inRecodes.front ( ).backCodec,
		outRecodes.front ( ).backCodec, inRecodes.front ( ).codec, outRecodes.front ( ).codec );
	return true;
}

bool H323Leg :: handleRequest ( const H245 :: RequestMessage & mesg,
	H225 :: ArrayOf_Asn_OctetString & reply ) {
	PSYSTEMLOG ( Info, "H323Leg :: handleRequest.");
	switch ( mesg.getTag ( ) ) {
		case H245 :: RequestMessage :: e_terminalCapabilitySet:
	PSYSTEMLOG ( Info, "H323Leg :: handleRequest: e_terminalCapabilitySet" );
			return handleCapabilitySet ( mesg, reply );
		case H245 :: RequestMessage :: e_masterSlaveDetermination:
	PSYSTEMLOG ( Info, "H323Leg :: handleRequest: e_masterSlaveDetermination" );
			return handleMasterSlaveDetermination ( mesg, reply );
		case H245 :: RequestMessage :: e_roundTripDelayRequest:
	PSYSTEMLOG ( Info, "H323Leg :: handleRequest: e_roundTripDelayRequest" );
			return handleRTDR ( mesg, reply );
		case H245 :: RequestMessage :: e_openLogicalChannel:
	PSYSTEMLOG ( Info, "H323Leg :: handleRequest: e_openLogicalChannel" );
			return handleOpenLogicalChannel ( mesg, reply );
		case H245 :: RequestMessage :: e_closeLogicalChannel:
	PSYSTEMLOG ( Info, "H323Leg :: handleRequest: e_closeLogicalChannel" );
			return handleCloseLogicalChannel ( mesg, reply );
		case H245 :: RequestMessage :: e_requestMode:
	PSYSTEMLOG ( Info, "H323Leg :: handleRequest: e_requestMode" );
			return handleRequestMode ( mesg, reply );
		default:
			PSYSTEMLOG ( Error, "unknown request tag: " << mesg.getTag ( ) );
			return false;
	}
}

bool H323Leg :: handleCloseLogicalChannel ( const H245 :: CloseLogicalChannel & mesg,
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

bool H323Leg :: handleOpenLogicalChannel ( const H245 :: OpenLogicalChannel & mesg,
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
	if ( multiParams.getTag ( ) != H245 :: OpenLogicalChannel_forwardLogicalChannelParameters_multiplexParameters
		:: e_h2250LogicalChannelParameters ) {
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
//	call -> setSendAddress ( this, ipAddr, ip -> m_tsapIdentifier & ~ 1, c, c );
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
	int localPort = thread -> getLocalPort ( );
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
	return true;
}

bool H323Leg :: handleRTDR ( const H245 :: RoundTripDelayRequest & mesg,
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

std :: pair < bool, bool > H323Leg :: getMasterSlaveDecision ( const H245 :: MasterSlaveDetermination & mesg ) const {
	const unsigned terminalType = 120;
	if ( mesg.m_terminalType < terminalType )
		return std :: make_pair ( true, true );
	if ( mesg.m_terminalType > terminalType )
		return std :: make_pair ( true, false );
	unsigned md = ( mesg.m_statusDeterminationNumber - determinationNumber ) & 0xffffff;
	return std :: make_pair ( md && md != 0x800000, md < 0x800000 );
}

bool H323Leg :: handleMasterSlaveDetermination ( const H245 :: MasterSlaveDetermination & mesg,
	H225 :: ArrayOf_Asn_OctetString & reply ) {
	bool sendAgain = false;
	if ( ! masterSlaveSent )
		addMasterSlave ( reply, & mesg );
	else
		sendAgain = true;
	H245 :: MultimediaSystemControlMessage answer;
	answer.setTag ( H245 :: MultimediaSystemControlMessage :: e_response );
	H245 :: ResponseMessage & response = answer;
	bool determined, localMaster;
	std :: tr1 :: tie ( determined, localMaster ) = getMasterSlaveDecision ( mesg );
	if ( __builtin_expect ( determined, true ) ) {
		response.setTag ( H245 :: ResponseMessage :: e_masterSlaveDeterminationAck );
		H245 :: MasterSlaveDeterminationAck & ack = response;
		ack.m_decision.setTag ( localMaster ? H245 :: MasterSlaveDeterminationAck_decision :: e_slave :
			H245 :: MasterSlaveDeterminationAck_decision :: e_master );
	} else {
		response.setTag ( H245 :: ResponseMessage :: e_masterSlaveDeterminationReject );
		H245 :: MasterSlaveDeterminationReject & reject = response;
		reject.m_cause.setTag ( H245 :: MasterSlaveDeterminationReject_cause :: e_identicalNumbers );
		masterSlaveSent = false;
	}
	Asn :: ostream os;
	answer.encode ( os );
	reply.push_back ( os.str ( ) );
	if ( ! __builtin_expect ( determined, true ) && sendAgain )
		addMasterSlave ( reply );
	return true;
}

bool H323Leg :: handleCapabilitySet ( const H245 :: TerminalCapabilitySet & mesg,
	H225 :: ArrayOf_Asn_OctetString & reply ) {
	PSYSTEMLOG ( Info, "H323Leg :: handleRequest: e_terminalCapabilitySet" );
	if ( ! capsSent )
		addCaps ( reply );
	H245 :: MultimediaSystemControlMessage answer;
	answer.setTag ( H245 :: MultimediaSystemControlMessage :: e_response );
	H245 :: ResponseMessage & response = answer;
	response.setTag ( H245 :: ResponseMessage :: e_terminalCapabilitySetAck );
	H245 :: TerminalCapabilitySetAck & ack = response;
	ack.m_sequenceNumber = mesg.m_sequenceNumber;
	Asn :: ostream os;
	answer.encode ( os );
	reply.push_back ( os.str ( ) );
	return true;
}

bool H323Leg :: handleRequestMode ( const H245 :: RequestMode & mesg,
	H225 :: ArrayOf_Asn_OctetString & reply ) {
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

bool H323Leg :: checkAddr ( H245 :: TransportAddress & addr, H245 :: UnicastAddress_iPAddress * & ip,
	PIPSocket :: Address & ipAddr ) {
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
	if ( ! thread -> admissibleIP ( ipAddr ) ) {
		PSYSTEMLOG ( Error, "RTP Addr isn't admissible" );
		return false;
	}
	if ( ! admissiblePort ( ip -> m_tsapIdentifier ) ) {
		PSYSTEMLOG ( Error, "RTP Port " << ip -> m_tsapIdentifier << " isn't admissible" );
		return false;
	}
	return true;
}

bool H323Leg :: checkAddr ( const H245 :: TransportAddress & addr, const H245 :: UnicastAddress_iPAddress * & ip,
	PIPSocket :: Address & ipAddr ) {
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
	if ( ! thread -> admissibleIP ( ipAddr ) ) {
		PSYSTEMLOG ( Error, "RTP Addr isn't admissible" );
		return false;
	}
	if ( ! admissiblePort ( ip -> m_tsapIdentifier ) ) {
		PSYSTEMLOG ( Error, "RTP Port " << ip -> m_tsapIdentifier << " isn't admissible" );
		return false;
	}
	return true;
}

bool H323Leg :: handleFastStartResponse2 ( const H225 :: ArrayOf_Asn_OctetString & v ) {
	FastStartElementVector fsin, fsout;
	PSYSTEMLOG(Info, "H323Leg :: handleFastStartResponse2");
	for ( unsigned i = 0; i < v.size ( ); i ++ ) {
		try {
			Asn :: istream is ( v [ i ] );
			PSYSTEMLOG ( Info, "1a. v.size() = " << v.size() << "; s[ " << v [ i ].size() <<
				" ] " << static_cast < const ss :: string & > ( v [ i ] ) );
			H245 :: OpenLogicalChannel open ( is );
			if ( is.hasException ( ) ) {
				PSYSTEMLOG ( Error, "1fastStart: can't decode " << v [ i ] << ": " << is.getException ( ) );
				return false;
			}
			H245 :: OpenLogicalChannel_forwardLogicalChannelParameters & forwardParams =
				open.m_forwardLogicalChannelParameters;
			H245 :: UnicastAddress_iPAddress * testIp;
			PIPSocket :: Address testAddr;
			switch ( forwardParams.m_dataType.getTag ( ) ) {
				case H245 :: DataType :: e_audioData: {
					if ( open.hasOptionalField ( H245 :: OpenLogicalChannel :: e_reverseLogicalChannelParameters ) ) {
						PSYSTEMLOG ( Error, "forward & reverse - not supported" );
						continue;
					}
					if ( forwardParams.m_multiplexParameters.getTag ( ) !=
						H245 :: OpenLogicalChannel_forwardLogicalChannelParameters_multiplexParameters
						:: e_h2250LogicalChannelParameters ) {
						PSYSTEMLOG ( Error, "not h2250LogicalChannelParameters - not supported" );
						continue;
					}
					H245 :: H2250LogicalChannelParameters & h225Params = forwardParams.m_multiplexParameters;
					if ( ! h225Params.hasOptionalField ( H245 :: H2250LogicalChannelParameters :: e_mediaControlChannel ) ) {
						PSYSTEMLOG ( Error, "no rtcp - not supported" );
						continue;
					}
					if ( ! h225Params.hasOptionalField ( H245 :: H2250LogicalChannelParameters :: e_mediaChannel ) ) {
						PSYSTEMLOG ( Error, "no rtp - not supported" );
						continue;
					}
					if ( ! checkAddr ( h225Params.get_mediaControlChannel ( ), testIp, testAddr) )
						continue;
					if ( ! checkAddr ( h225Params.get_mediaChannel ( ), testIp, testAddr ) )
						continue;
					fsin.push_back ( FastStartElement ( open, forwardParams.m_dataType ) );
					continue;
				} case H245 :: DataType :: e_nullData:
					break;
				default:
					PSYSTEMLOG ( Error, "unknown dataType: " << forwardParams.m_dataType.getTag ( ) );
					continue;
			}
			if ( ! open.hasOptionalField ( H245 :: OpenLogicalChannel :: e_reverseLogicalChannelParameters ) ) {
				PSYSTEMLOG ( Error, "no forward or reverse params" );
				continue;
			}
			H245 :: OpenLogicalChannel_reverseLogicalChannelParameters & reverseParams =
				open.get_reverseLogicalChannelParameters ( );
			if ( reverseParams.m_dataType.getTag ( ) != H245 :: DataType :: e_audioData ) {
				PSYSTEMLOG ( Error, "unsupported reverse tag: " << reverseParams.m_dataType.getTag ( ) );
				continue;
			}
			if ( ! reverseParams.hasOptionalField ( H245 :: OpenLogicalChannel_reverseLogicalChannelParameters
				:: e_multiplexParameters ) ) {
				PSYSTEMLOG ( Error, "no multiplexParameters - not supported" );
				continue;
			}
			if ( reverseParams.get_multiplexParameters ( ).getTag ( ) !=
				H245 :: OpenLogicalChannel_reverseLogicalChannelParameters_multiplexParameters ::
				e_h2250LogicalChannelParameters ) {
				PSYSTEMLOG ( Error, "not h2250LogicalChannelParameters - not supported" );
				continue;
			}
			H245 :: H2250LogicalChannelParameters & h225Params = reverseParams.get_multiplexParameters ( );
			if ( ! h225Params.hasOptionalField ( H245 :: H2250LogicalChannelParameters :: e_mediaControlChannel ) ) {
				PSYSTEMLOG ( Error, "no rtcp - not supported" );
				continue;
			}
			if ( ! checkAddr ( h225Params.get_mediaControlChannel ( ), testIp, testAddr ) )
				continue;
			h225Params.removeOptionalField ( H245 :: H2250LogicalChannelParameters :: e_mediaChannel );
			fsout.push_back ( FastStartElement ( open, reverseParams.m_dataType ) );
		} catch ( std :: exception & e ) {
			PSYSTEMLOG ( Error, "2fastStart: can't decode " << v [ i ] << ": " << e.what ( ) );
			return false;
		}
	}
	if ( fsin.size ( ) != 1 || fsout.size ( ) != 1 )
	{
		PSYSTEMLOG(Error, "3fastStart: fsin.size ( ) != 1 || fsout.size ( ) != 1 - " << fsin.size ( ) << ' ' << fsout.size ( ) );
		return false;
	}
	FastStartElement & ie = fsin [ 0 ], & oe = fsout [ 0 ];
	CodecInfo inCodec = ie.codec, inChangedCodec = ie.codec, outCodec = oe.codec, outChangedCodec = oe.codec;
	typedef RecodeInfoVector :: index < Codec > :: type ByCodec;
	ByCodec & inByCodec = inRecodes.get < Codec > ( );
	ByCodec :: const_iterator i = inByCodec.find ( inCodec );
	if ( i == inByCodec.end ( ) )
		PSYSTEMLOG ( Error, "inByCodec not found: " << inCodec );
	else
		inCodec = i -> backCodec;
	ByCodec & outByCodec = outRecodes.get < Codec > ( );
	i = outByCodec.find ( outCodec );
	if ( i == outByCodec.end ( ) )
		PSYSTEMLOG ( Error, "outByCodec not found: " << outCodec );
	else
		outCodec = i -> backCodec;
	thread -> setCodec ( inCodec );
	H245 :: UnicastAddress_iPAddress * ip;
	PIPSocket :: Address addr;
	int port;
	getMediaAddr ( ie.open.m_forwardLogicalChannelParameters.m_multiplexParameters, addr, port, ip );
	thread -> setSendAddress ( addr, port, inCodec, outCodec, inChangedCodec, outChangedCodec );
	return true;
}

template < class T > bool H323Leg :: handleFastStartResponse ( T & uuie ) {
	if ( uuie.hasOptionalField ( T :: e_fastStart ) ) {
		PSYSTEMLOG(Info, "T :: e_fastStart" );
		fastStartAnswered = true;
		return handleFastStartResponse2 ( uuie.get_fastStart ( ) );
	}
	if ( uuie.hasOptionalField ( T :: e_fastConnectRefused ) )
		fastConnectRefused = true;
	return true;
}

template < class T > bool H323Leg :: handleH245Setup ( const T & uuie ) {
	if ( h245Sock )
		return true;
	if ( ! uuie.hasOptionalField ( T :: e_h245Address ) )
		return true;
	if ( common.curChoice ( ) -> getFromNat ( ) ) {
		PSYSTEMLOG ( Error, "handleH245Setup - fromNat - failed" );
		return false;
	}
	disableTunneling ( );
	const H225 :: TransportAddress & addr = uuie.get_h245Address ( );
	if ( addr.getTag ( ) != H225 :: TransportAddress :: e_ipAddress ) {
		PSYSTEMLOG ( Error, "handleH245Setup - not an ip address" );
		return false;
	}
	const H225 :: TransportAddress_ipAddress & ip = addr;
	WORD port;
	PIPSocket :: Address ipAddr, localAddr;
	AddrUtils :: convertToIPAddress ( ip, ipAddr, port );
	if ( port == 0 ) {
		PSYSTEMLOG ( Error, "handleH245Setup - zero port" );
		return false;
	}
	h245Sock = new PTCPSocket ( port );
	h245Sock -> SetReadTimeout ( 3000 );
	h245Sock -> SetWriteTimeout ( 3000 );
	sock -> GetLocalAddress ( localAddr );
	if ( h245Sock -> Connect ( localAddr, ipAddr ) )
		return true;
	PSYSTEMLOG ( Error, "handleH245Setup - connect: " << h245Sock -> GetErrorText ( ) );
	h245Sock -> Close ( );
	safeDel ( h245Sock );
	return false;
}

bool H323Leg :: handleProceeding ( Q931 & mesg, H225 :: H323_UserInformation & uuField ) {
	PSYSTEMLOG(Info, "H323Leg :: handleProceeding");
	H225 :: H323_UU_PDU & pdu = uuField.m_h323_uu_pdu;
	H225 :: H323_UU_PDU_h323_message_body & body = pdu.m_h323_message_body;
	if ( body.getTag ( ) != H225 :: H323_UU_PDU_h323_message_body :: e_callProceeding ) {
		PSYSTEMLOG ( Error, "Bad body tag for CallProceeding: " << body.getTag ( ) );
		return false;
	}
	if ( common.curChoice ( ) -> getSigOptions ( ).getReplaceCallProceedingWithAlerting ( ) ) {
		makeAlertingFromCallProceeding ( mesg, body );
		return handleAlerting ( mesg, uuField );
	}
	if ( ! handleH245 ( pdu ) )
		return false;
	H225 :: CallProceeding_UUIE & proc = body;
	if ( ! handleH245Setup ( proc ) )
		return false;
	return handleFastStartResponse ( proc );
}

class Alerter {
	OriginateLegThread * thread;
public:
	Alerter ( OriginateLegThread * t ) : thread ( t ) { }
	~Alerter ( ) {
		thread -> alerted ( );
	}
};

bool H323Leg :: handleAlerting ( Q931 & /*mesg*/, H225 :: H323_UserInformation & uuField ) {
	Alerter a ( thread );
	PSYSTEMLOG(Info, "H323Leg :: handleAlerting");
	H225 :: H323_UU_PDU & pdu = uuField.m_h323_uu_pdu;
	if ( ! handleH245 ( pdu ) )
		return false;
	H225 :: H323_UU_PDU_h323_message_body & body = pdu.m_h323_message_body;
	if ( body.getTag ( ) != H225 :: H323_UU_PDU_h323_message_body :: e_alerting ) {
		PSYSTEMLOG ( Error, "Bad body tag for Alerting: " << body.getTag ( ) );
		return false;
	}
	H225 :: Alerting_UUIE & alert = body;
	if ( ! handleH245Setup ( alert ) )
		return false;
	if ( ! handleFastStartResponse ( alert ) )
		return false;
	if ( fastConnectRefused )
		initSlowStart ( );
	return true;
}

bool H323Leg :: initSlowStart ( H225 :: ArrayOf_Asn_OctetString & reply ) {
	PIPSocket :: Address localAddr;
	WORD sourcePort;
	sock -> GetLocalAddress ( localAddr, sourcePort );
	int localPort = thread -> getLocalPort ( );

	H245 :: MultimediaSystemControlMessage mesg;
	mesg.setTag ( H245 :: MultimediaSystemControlMessage :: e_request );
	H245 :: RequestMessage & request = mesg;
	request.setTag ( H245 :: RequestMessage :: e_openLogicalChannel );
	H245 :: OpenLogicalChannel & olc = request;
	olc.m_forwardLogicalChannelNumber = 1;
	H245 :: OpenLogicalChannel_forwardLogicalChannelParameters & forwardParams =
		olc.m_forwardLogicalChannelParameters;
	forwardParams.m_dataType.setTag ( H245 :: DataType :: e_audioData );
	H245 :: AudioCapability & audioData = forwardParams.m_dataType;
	PSYSTEMLOG( Info, "H323Leg :: initSlowStart inRecodes.size() = " << inRecodes.size() );
	if(false == inRecodes.empty() )
	{
		:: setCodec ( audioData, inRecodes.front ( ).codec.getCodec ( ) );
		if ( inRecodes.front ( ).codec.getFrames ( ) )
			setFrames ( audioData, inRecodes.front ( ).codec.getFrames ( ) );
	}
	else
	{
		return false;
	}

	H245 :: OpenLogicalChannel_forwardLogicalChannelParameters_multiplexParameters & multiParams =
		forwardParams.m_multiplexParameters;
	multiParams.setTag ( H245 :: OpenLogicalChannel_forwardLogicalChannelParameters_multiplexParameters
		:: e_h2250LogicalChannelParameters );
	H245 :: H2250LogicalChannelParameters & h225Params = multiParams;
	h225Params.m_sessionID = 1;
	h225Params.includeOptionalField ( H245 :: H2250LogicalChannelParameters :: e_mediaControlChannel );
	H245 :: TransportAddress & t = h225Params.get_mediaControlChannel ( );
	t.setTag ( H245 :: TransportAddress :: e_unicastAddress );
	H245 :: UnicastAddress & u = t;
	u.setTag ( H245 :: UnicastAddress :: e_iPAddress );
	setIPAddress ( u, localAddr, localPort + 1 );
	Asn :: ostream os;
	mesg.encode ( os );
	reply.push_back ( os.str ( ) );
	return true;
}

void H323Leg :: initSlowStart ( ) {
	if ( slowStartInited )
		return;
	if ( useTunneling ) {
		slowStartInited = true;
		Q931 facility ( Q931 :: msgFacility, ref, false );
		H225 :: H323_UserInformation uuField;
		H225 :: H323_UU_PDU & pdu = uuField.m_h323_uu_pdu;
		pdu.m_h323_message_body.setTag ( H225 :: H323_UU_PDU_h323_message_body :: e_empty );
		pdu.includeOptionalField ( H225 :: H323_UU_PDU :: e_h245Tunneling );
		pdu.get_h245Tunneling ( ) = true;
		pdu.includeOptionalField ( H225 :: H323_UU_PDU :: e_h245Control );
		H225 :: ArrayOf_Asn_OctetString & reply = pdu.get_h245Control ( );
		if ( ! capsSent )
			addCaps ( reply );
		if ( ! masterSlaveSent )
			addMasterSlave ( reply );
		if( initSlowStart ( reply ) )
		{
			encodeH225IntoQ931 ( uuField, facility );
			sendMesgToDest ( facility, & uuField, sock, common );
		}
//		else
//			PSYSTEMLOG(Error, "");

		return;
	}
	if ( ! h245Sock )
		return;
	slowStartInited = true;
	H225 :: ArrayOf_Asn_OctetString reply;
	if ( ! capsSent )
		addCaps ( reply );
	if ( ! masterSlaveSent )
		addMasterSlave ( reply );
	if(false == initSlowStart ( reply ) )
	{
//			PSYSTEMLOG(Error, "");
		return;
	}

	for ( unsigned i = 0; i < reply.size ( ); i ++ ) {
		Asn :: istream is ( reply [ i ] );
		H245 :: MultimediaSystemControlMessage mesg ( is );
		sendH245MesgToDest ( makeTpkt ( reply [ i ] ), mesg, h245Sock, common.getForceDebug ( ) );
	}
}

class Connecter {
	OriginateLegThread * thread;
public:
	Connecter ( OriginateLegThread * t ) : thread ( t ) { }
	~Connecter ( ) {
		thread -> connected ( );
	}
};

bool H323Leg :: handleConnect ( Q931 & /*mesg*/, H225 :: H323_UserInformation & uuField ) {
	Connecter c ( thread );
	PSYSTEMLOG(Info, "H323Leg :: handleConnect");

	H225 :: H323_UU_PDU & pdu = uuField.m_h323_uu_pdu;
	if ( ! handleH245 ( pdu ) )
	{
		PSYSTEMLOG( Info, "H323Leg :: handleConnect: ! handleH245." );
		return false;
	}
	H225 :: H323_UU_PDU_h323_message_body & body = pdu.m_h323_message_body;
	if ( body.getTag ( ) != H225 :: H323_UU_PDU_h323_message_body :: e_connect ) {
		PSYSTEMLOG ( Error, "Bad body tag for Connect: " << body.getTag ( ) );
		return false;
	}
	H225 :: Connect_UUIE & conn = body;
	if ( ! handleH245Setup ( conn ) )
	{
		PSYSTEMLOG( Info, "H323Leg :: handleConnect: ! handleH245Setup." );
		return false;
	}
	if ( ! handleFastStartResponse ( conn ) )
	{
		PSYSTEMLOG( Info, "H323Leg :: handleConnect: ! handleFastStartResponse." );
		return false;
	}
	if ( fastConnectRefused || ! fastStartAnswered )
		initSlowStart ( );
	return true;
}

bool H323Leg :: handleRelease ( const Q931 & mesg, const H225 :: H323_UserInformation * uuField ) {
	Q931 :: CauseValues cause = getCause ( mesg, uuField );
	common.setDisconnectCause ( cause );
	sendReleaseComplete ( );
	thread -> released ( cause );
	return false;
}

bool H323Leg :: handleFacility ( const Q931 & /*mesg*/, const H225 :: H323_UserInformation & uuField ) {
	PSYSTEMLOG(Info, "H323Leg :: handleFacility");

	const H225 :: H323_UU_PDU & pdu = uuField.m_h323_uu_pdu;
	if ( ! handleH245 ( pdu ) )
		return false;
	const H225 :: H323_UU_PDU_h323_message_body & body = pdu.m_h323_message_body;
	switch ( body.getTag ( ) ) {
		case H225 :: H323_UU_PDU_h323_message_body :: e_facility: {
			const H225 :: Facility_UUIE & fac = body;
			if ( ! handleH245Setup ( fac ) )
				return false;
			return handleFastStartResponse ( fac );
		} case H225 :: H323_UU_PDU_h323_message_body :: e_empty:
			return true;
		default:
			PSYSTEMLOG ( Error, "Bad body tag for Facility: " << body.getTag ( ) );
			return false;
	}
}

bool H323Leg :: handleProgress ( const Q931 & /*mesg*/, const H225 :: H323_UserInformation & uuField ) {
	PSYSTEMLOG(Info, "H323Leg :: handleProgress");

	const H225 :: H323_UU_PDU & pdu = uuField.m_h323_uu_pdu;
	if ( ! handleH245 ( pdu ) )
		return false;
	const H225 :: H323_UU_PDU_h323_message_body & body = pdu.m_h323_message_body;
	if ( body.getTag ( ) != H225 :: H323_UU_PDU_h323_message_body :: e_progress ) {
		PSYSTEMLOG ( Error, "Bad body tag for Progress: " << body.getTag ( ) );
		return false;
	}
	const H225 :: Progress_UUIE & prog = body;
	if ( ! handleH245Setup ( prog ) )
		return false;
	return handleFastStartResponse ( prog );
}

bool H323Leg :: handleMesg ( Q931 & mesg ) {
	thread -> answered ( );
	PSYSTEMLOG(Info, "H323Leg :: handleMesg: " << mesg);

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
				PSYSTEMLOG ( Error, "H323LegThread::handleMesg ( " << mesg << " ) - ! HasUU" );
			return false;
		}
	}
	_fakeCallDetector.registerMesg ( mesg.getMessageType ( ) );
	switch ( mesg.getMessageType ( ) ) {
		case Q931 :: msgCallProceeding:
			return handleProceeding ( mesg, * uuField );
		case Q931 :: msgAlerting:
			return handleAlerting ( mesg, * uuField );
		case Q931 :: msgConnect:
			return handleConnect ( mesg, * uuField );
		case Q931 :: msgReleaseComplete:
			return handleRelease ( mesg, uuField );
		case Q931 :: msgFacility:
			return handleFacility ( mesg, * uuField );
		case Q931 :: msgProgress:
			return handleProgress ( mesg, * uuField );
		case Q931 :: msgStatusEnquiry:
			sendStatus ( );
			return true;
		case Q931 :: msgStatus:
		case Q931 :: msgSetupAck:
		case Q931 :: msgNotify:
		case Q931 :: msgInformation:
			return true;
		default:
			PSYSTEMLOG ( Error, "Unknown message type: " << mesg.getMessageType ( ) << " !!!!!!!!!!!!!!!!!!!!!" );
			if ( ! common.getForceDebug ( ) )
				Log -> logQ931Msg ( mesg, uuField, OpengateLog :: Receiving, PIPSocket :: Address ( ), 0, true );
			return false;
	}
}

bool H323Leg :: receiveMesg ( ) {
	try {
		PSYSTEMLOG(Info, "H323Leg :: receiveMesg");
		Q931 mesg ( readMsg ( sock ) );
//		PSYSTEMLOG(Info, "H323Leg :: receiveMesg: " << mesg);
		return handleMesg ( mesg );
	} catch ( std :: exception & e ) {
		PSYSTEMLOG ( Error, "unable to receive Q931 message: " << e.what ( ) );
		return false;
	}
}

H323Leg :: H323Leg ( OriginateLegThread * t, CommonCallDetails & c, const RecodeInfoVector & inCodecs,
	const RecodeInfoVector & outCodecs, unsigned r ) : Leg ( t, c ), sock ( 0 ),
	h245Sock ( 0 ), ref ( r ), useTunneling ( ! c.curChoice ( ) -> getSigOptions ( ).getDropTunneling ( ) ),
	fastConnectRefused ( false ), fastStartAnswered ( false ), capsSent ( false ), inRecodes ( inCodecs ),
	outRecodes ( outCodecs ), slowStartInited ( false ), rcSent ( false ) {
	ss :: string s = H323 :: globallyUniqueId ( );
	callId.m_guid = s;
	common.setConfId ( s );
	thread -> setPrintableCallId ( H323 :: printableCallId ( s ) );
}

H323Leg :: ~H323Leg ( ) {
	safeDel ( sock );
}

void H323Leg :: sendStatus ( ) {
	Q931 mesg ( Q931 :: msgStatus, ref, false );
	H225 :: H323_UserInformation uuField;
	H225 :: H323_UU_PDU & pdu = uuField.m_h323_uu_pdu;
	pdu.m_h323_message_body.setTag ( H225 :: H323_UU_PDU_h323_message_body :: e_status );
	H225 :: Status_UUIE & status = pdu.m_h323_message_body;
	status.m_protocolIdentifier = h225ProtocolID;
	status.m_callIdentifier = callId;
	encodeH225IntoQ931 ( uuField, mesg );
	sendMesgToDest ( mesg, & uuField, sock, common );
}

void H323Leg :: rtpTimeoutDetected ( ) {
	_fakeCallDetector.registerRTPtimeout ( );
}

void H323Leg :: wakeUp ( ) {
	thread -> PXAbortBlock ( );
}
