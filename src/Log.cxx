#pragma implementation
#include "ss.hpp"
#include "allocatable.hpp"
#include <ptlib.h>
#include <ptlib/sockets.h>
#include "Log.h"
#include "aftertask.hpp"
#include "ipport.hpp"
#include "icqcontact.hpp"
#include "radgwinfo.hpp"
#include "Conf.hpp"
#include <stdexcept>
#include <limits>
#include <cstring>
#include "asn.hpp"
#include "h225.hpp"
#include "AddrUtils.h"
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/sequenced_index.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/member.hpp>
#include "q931.hpp"
#include "ixcudpsocket.hpp"
#include <boost/multi_index/mem_fun.hpp>
#include "sip.hpp"
#include <boost/optional.hpp>
#include "sip2.hpp"
#include "pointer.hpp"
#include "radius.hpp"
#include <ptlib/svcproc.h>
#include "mgcp.hpp"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

int OpengateLog :: getLogLevel ( ) const {
	if ( conf -> getLogLevel ( ) == 1 )
		return OpengateLog :: None;
	return OpengateLog :: Trace;
}

void OpengateLog :: open ( ) {
	logMutex.Wait ( );
	logFile.Open ( logFilename, PFile :: ReadWrite );
	logFile.SetPosition ( 0, PFile :: End );
	logFile.imbue ( std :: locale :: classic ( ) );
}

void OpengateLog :: close ( ) {
	logFile.Close ( );
	logMutex.Signal ( );
}

OpengateLog :: OpengateLog ( const ss :: string & nm ) {
	PTime now;
	PString dateStamp = now.AsString ( "yyyy/MM/dd hh:mm:ss" );
	const char * startupMsg = "Starting logging: ";
	logFilename = nm.c_str ( );

	// Check to see if logging has been requested
	// If none then do not open log file

	open ( );
	logFile << startupMsg;
	logFile << dateStamp;
	logFile.WriteLine ( "" ); //Cheating really want to flush buffer...
	close ( );
}

OpengateLog :: ~OpengateLog ( ) { }

void OpengateLog :: writeHeader ( ) {
	PTime now;
	PString dateStamp = now.AsString ( "yyyy/MM/dd hh:mm:ss.uuu" );
	open ( );
	logFile << dateStamp << ' ';
}

void OpengateLog :: writeHeader ( Direction dir ) {
	writeHeader ( );
	if ( dir == Sending )
		logFile << "Sent ";
	else
		logFile << "Recv ";
}

void OpengateLog :: writeHeader ( Direction dir, const PIPSocket :: Address & address, WORD port ) {
	writeHeader ( dir );
	logFile << address << ':' << port << "\t";
}

void OpengateLog :: writeHeader ( Direction dir, const ss :: string & address, WORD port ) {
	writeHeader ( dir );
	logFile << address << ':' << port << "\t";
}

void OpengateLog :: writeHeader ( Direction dir, const H225 :: TransportAddress & addr ) {
	if ( ( addr.getTag ( ) == H225 :: TransportAddress :: e_ipAddress ) ) {
		const H225 :: TransportAddress_ipAddress & ipAddr = addr;
		PIPSocket :: Address address;
		WORD port;
		AddrUtils :: convertToIPAddress ( ipAddr, address, port );
		writeHeader ( dir, address, port );
	} else
		writeHeader ( dir );
}

void OpengateLog :: writeTrailer ( ) {
	logFile.WriteLine ( "" ); //adding CRLF and insuring buffer is flushed.
	close ( );
}


//Log PString style messages to the Log file

void OpengateLog :: logNote ( const ss :: string & note ) {
	if ( getLogLevel ( ) > OpengateLog :: None ) {
		writeHeader ( );
		logFile << note;
		writeTrailer ( );
	}
}

// Log H225_RasMessage style messages to the RAS log file

void OpengateLog :: logRasMsg ( const H225 :: RasMessage & msg, Direction dir,
	const H225 :: TransportAddress & addr ) {
	if ( getLogLevel ( ) > OpengateLog :: None ) {
		writeHeader ( dir, addr );
		if ( getLogLevel ( ) == OpengateLog :: Info ) {
			logFile << "H.225 ";
			msg.printTagName ( logFile );
		} else {
			logFile.WriteLine ( "H.225" );
			logFile << setprecision ( 0 ) << msg;
		}
		writeTrailer ( );
	}
}

template < class T > void OpengateLog :: logFastStart ( const T & uuie ) {
	if ( ! uuie.hasOptionalField ( T :: e_fastStart ) )
		return;
	const H225 :: ArrayOf_Asn_OctetString & fs = uuie.get_fastStart ( );
	for ( unsigned i = 0; i < fs.size ( ); i ++ ) {
		try {
			Asn :: istream is ( fs [ i ] );
			H245 :: OpenLogicalChannel openlc ( is );
			if ( is.hasException ( ) ) {
				PSYSTEMLOG ( Error, "fastStart: can't decode" << fs [ i ] << ": " << is.getException ( ) );
//				return;
			}
			logFile.WriteLine ( "" );
			logFile.WriteLine ( "FastStart:" );
			logFile << setprecision ( 0 ) << openlc;
		} catch ( std :: exception & e ) {
			PSYSTEMLOG ( Error, "fastStart: can't decode" << fs [ i ] << ": " << e.what ( ) );
		}
	}
}

void OpengateLog :: logQ931Msg ( const Q931 & msg, const H225 :: H323_UserInformation * uuField,
	Direction dir, const PIPSocket :: Address & address, WORD port, bool forceDebug ) {
	if ( ! forceDebug && getLogLevel ( ) <= OpengateLog :: None )
		return;
	writeHeader ( dir, address, port );
	logQ931MsgInt ( msg, uuField, forceDebug );
	writeTrailer ( );
}

void OpengateLog :: LogSIPMsg ( SIP :: PDU & msg, Direction Dir,
	const PIPSocket :: Address & Address, WORD Port, bool forceDebug ) {
	if ( ! forceDebug && getLogLevel ( ) <= OpengateLog :: None )
		return;
	writeHeader ( Dir, Address, Port );
	LogSIPMsgInt ( msg, forceDebug );
	writeTrailer ( );
}

void OpengateLog :: LogSIP2Msg ( const SIP2 :: Request & msg, Direction Dir,
	const PIPSocket :: Address & Address, WORD Port, bool forceDebug ) {
	if ( ! forceDebug && getLogLevel ( ) <= OpengateLog :: None )
		return;
	writeHeader ( Dir, Address, Port );
	LogSIP2MsgInt ( msg, forceDebug );
	writeTrailer ( );
}

void OpengateLog :: LogSIP2Msg ( const SIP2 :: Response & msg, Direction Dir,
	const PIPSocket :: Address & Address, WORD Port, bool forceDebug ) {
	if ( ! forceDebug && getLogLevel ( ) <= OpengateLog :: None )
		return;
	writeHeader ( Dir, Address, Port );
	LogSIP2MsgInt ( msg, forceDebug );
	writeTrailer ( );
}

void OpengateLog :: logMGCPMsg ( const MGCP :: PDU & msg, Direction dir, const ss :: string & address,
	WORD port, bool forceDebug ) {
	logMGCPMsg ( msg, dir, PIPSocket :: Address ( address.c_str ( ) ), port, forceDebug );
}

void OpengateLog :: logMGCPMsg ( const MGCP :: PDU & msg, Direction dir, const PIPSocket :: Address & address,
	WORD port, bool forceDebug ) {
	if ( ! forceDebug && getLogLevel ( ) <= OpengateLog :: None )
		return;
	writeHeader ( dir, address, port );
	logMGCPMsgInt ( msg, forceDebug );
	writeTrailer ( );
}

void OpengateLog :: logProgressIndicator ( const Q931 & mesg ) {
	Q931 :: ProgressDescription descr;
	Q931 :: CodingStandard codingStandard;
	Q931 :: Location location;
	for ( int i = 0; mesg.getProgressIndicator ( descr, i, & codingStandard, & location ); i ++ ) {
		logFile.WriteLine ( "Progress-Indicator:" );
		logFile.WriteLine ( "{" );
		logFile << "  progressDescription = " << Q931 :: getProgressDescriptionName ( descr );
		logFile.WriteLine ( "" );
		logFile << "  codingStandard = " << Q931 :: getCodingStandardName ( codingStandard );
		logFile.WriteLine ( "" );
		logFile << "  location = " << Q931 :: getLocationName ( location );
		logFile.WriteLine ( "" );
		logFile.WriteLine ( "}" );
	}
}

void OpengateLog :: logBearerCapability ( const Q931 & mesg ) {
	Q931 :: InformationTransferCapability capability;
	unsigned transferRate = 0;
	Q931 :: CodingStandard codingStandard = Q931 :: csCCITT;
	Q931 :: UserInfoLayer1 userInfoLayer1 = Q931 :: uil1H221;
	if ( ! mesg.getBearerCapabilities ( capability, transferRate, & codingStandard, & userInfoLayer1 ) )
		return;
	logFile.WriteLine ( "Bearer-Capability:" );
	logFile.WriteLine ( "{" );
	logFile << "  informationTransferCapability = " << Q931 :: getInformationTransferCapabilityName ( capability );
	logFile.WriteLine ( "" );
	logFile << "  transferRate = " << transferRate * 64;
	logFile.WriteLine ( "" );
	logFile << "  codingStandard = " << Q931 :: getCodingStandardName ( codingStandard );
	logFile.WriteLine ( "" );
	logFile << "  userInfoLayer1 = " << Q931 :: getUil1Name ( userInfoLayer1 );
	logFile.WriteLine ( "" );
	logFile.WriteLine ( "}" );
}

void OpengateLog :: logSignal ( const Q931 & mesg ) {
	Q931 :: SignalInfo sig;
	if ( ! mesg.getSignal ( sig ) )
		return;
	logFile << "Signal: " << Q931 :: getSignalName ( sig );
	logFile.WriteLine ( "" );
}

void OpengateLog :: logNotificationIndicator ( const Q931 & mesg ) {
	Q931 :: NotificationDescription nd;
	if ( ! mesg.getNotificationDescription ( nd ) )
		return;
	logFile << "NotificationDescription: " << Q931 :: getNotificationDescriptionName ( nd );
	logFile.WriteLine ( "" );
}

void OpengateLog :: logQ931MsgInt ( const Q931 & msg, const H225 :: H323_UserInformation * uuField, bool forceDebug ) {
	if ( ! forceDebug && getLogLevel ( ) == OpengateLog :: Info ) {
		logFile << "Q.931 " << msg.getMessageTypeName ( );
		return;
	}
	logFile.WriteLine ( "Q.931" );
	logFile << setprecision ( 0 ) << msg;
	logFile.WriteLine ( "" );
	logBearerCapability ( msg );
	logProgressIndicator ( msg );
	logSignal ( msg );
	logNotificationIndicator ( msg );
	if ( ! uuField )
		return;
	logFile.WriteLine ( "UserUserField:" );
	logFile << setprecision ( 0 ) << * uuField;
	const H225 :: H323_UU_PDU & UU_PDU = uuField -> m_h323_uu_pdu;
	if ( UU_PDU.hasOptionalField ( H225 :: H323_UU_PDU :: e_h245Control ) ) {
		for ( unsigned i = 0; i < UU_PDU.get_h245Control ( ).size ( ); i ++ ) {
			Asn :: istream is ( UU_PDU.get_h245Control ( ) [ i ] );
			try {
				H245 :: MultimediaSystemControlMessage h245Mesg ( is );
				logFile.WriteLine ( "" );
				logFile.WriteLine ( "Tunneled H.245:" );
				logFile << setprecision ( 0 ) << h245Mesg;
			} catch ( ... ) { }
		}
	}
	const H225 :: H323_UU_PDU_h323_message_body & body = UU_PDU.m_h323_message_body;
	switch ( body.getTag ( ) ) {
		case H225 :: H323_UU_PDU_h323_message_body :: e_callProceeding: {
			const H225 :: CallProceeding_UUIE & callProceeding = body;
			logFastStart ( callProceeding );
			break;
		} case H225 :: H323_UU_PDU_h323_message_body :: e_connect: {
			const H225 :: Connect_UUIE & connect = body;
			logFastStart ( connect );
			break;
		} case H225 :: H323_UU_PDU_h323_message_body :: e_alerting: {
			const H225 :: Alerting_UUIE & alerting = body;
			logFastStart ( alerting );
			break;
		} case H225 :: H323_UU_PDU_h323_message_body :: e_setup: {
			const H225 :: Setup_UUIE & setup = body;
			logFastStart ( setup );
			if ( setup.hasOptionalField ( H225 :: Setup_UUIE :: e_parallelH245Control ) ) {
				for ( unsigned i = 0; i < setup.get_parallelH245Control ( ).size ( ); i ++ ) {
					Asn :: istream is ( setup.get_parallelH245Control ( ) [ i ] );
					try {
						H245 :: MultimediaSystemControlMessage h245Mesg ( is );
						logFile.WriteLine ( "" );
						logFile.WriteLine ( "Parallel H.245:" );
						logFile << setprecision ( 0 ) << h245Mesg;
					} catch ( ... ) { }
				}
			}
			break;
		} case H225 :: H323_UU_PDU_h323_message_body :: e_facility: {
			const H225 :: Facility_UUIE & facility = body;
			logFastStart ( facility );
			break;
		} case H225 :: H323_UU_PDU_h323_message_body :: e_progress: {
			const H225 :: Progress_UUIE & progress = body;
			logFastStart ( progress );
		}
	}
}

void OpengateLog :: LogSIPMsgInt ( SIP :: PDU & msg, bool forceDebug ) {
	if ( ! forceDebug && getLogLevel ( ) == OpengateLog :: Info ) {
		logFile << "SIP ";
		switch ( msg.getMethod ( ) ) {
			case SIP :: PDU :: mInvite:
				logFile << "INVITE";
				break;
			case SIP :: PDU :: mAck:
				logFile << "ACK";
				break;
			case SIP :: PDU :: mOptions:
				logFile << "OPTIONS";
				break;
			case SIP :: PDU :: mBye:
				logFile << "BYE";
				break;
			case SIP :: PDU :: mCancel:
				logFile << "CANCEL";
				break;
			case SIP :: PDU :: mRegister:
				logFile << "REGISTER";
				break;
			case SIP :: PDU :: mRefer:
				logFile << "REFER";
				break;
			case SIP :: PDU :: mMessage:
				logFile << "MESSAGE";
				break;
			case SIP :: PDU :: mNotify:
				logFile << "NOTIFY";
				break;
			case SIP :: PDU :: mSubscribe:
				logFile << "SUBSCRIBE";
				break;
			case SIP :: PDU :: mInfo:
				logFile << "INFO";
				break;
			case SIP :: PDU :: mNum:
				break;
		}
		return;
	}
	logFile.WriteLine ( "SIP" );
	logFile << msg;
	logFile.WriteLine ( "" );
}

void OpengateLog :: LogSIP2MsgInt ( const SIP2 :: Request & msg, bool forceDebug ) {
	if ( ! forceDebug && getLogLevel ( ) == OpengateLog :: Info ) {
		logFile << "SIP " << SIP2 :: methodName ( msg.getMethod ( ) );
		return;
	}
	logFile.WriteLine ( "SIP" );
	logFile << msg;
	logFile.WriteLine ( "" );
}

void OpengateLog :: LogSIP2MsgInt ( const SIP2 :: Response & msg, bool forceDebug ) {
	if ( ! forceDebug && getLogLevel ( ) == OpengateLog :: Info ) {
		logFile << "SIP " << msg.getStatusCode ( ) << ' ' << msg.getReasonPhrase ( );
		return;
	}
	logFile.WriteLine ( "SIP" );
	logFile << msg;
	logFile.WriteLine ( "" );
}

void OpengateLog :: logMGCPMsgInt ( const MGCP :: PDU & msg, bool forceDebug ) {
	if ( ! forceDebug && getLogLevel ( ) == OpengateLog :: Info ) {
		logFile << "MGCP ";
		MGCP :: PDU :: ReturnCodes rc = msg.getCode ( );
		if ( rc != MGCP :: PDU :: rcInvalid )
			logFile << rc << ' ' << msg.getComment ( );
		else
			logFile << msg.getVerbName ( );
		return;
	}
	logFile.WriteLine ( "MGCP" );
	logFile << msg;
	logFile.WriteLine ( "" );
}

void OpengateLog :: logH245Msg ( const H245 :: MultimediaSystemControlMessage & msg,
	Direction dir, const PIPSocket :: Address & address, WORD port, bool forceDebug ) {
	if ( ! forceDebug && getLogLevel ( ) <= OpengateLog :: None )
		return;
	writeHeader ( dir, address, port );
	if ( forceDebug || getLogLevel ( ) >= OpengateLog :: Info ) {
		logFile.WriteLine ( "H.245" );
		logFile << setprecision ( 0 ) << msg;
		writeTrailer ( );
		return;
	}
	logFile << "H.245 ";
	msg.printTagName ( logFile );
	switch ( msg.getTag ( ) ) {
		case H245 :: MultimediaSystemControlMessage :: e_request: {
			const H245 :: RequestMessage & request = msg;
			logFile << ' ';
			request.printTagName ( logFile );
			break;
		} case H245 :: MultimediaSystemControlMessage :: e_response: {
			const H245 :: ResponseMessage & response = msg;
			logFile << ' ';
			response.printTagName ( logFile );
			break;
		} case H245 :: MultimediaSystemControlMessage :: e_command: {
			const H245 :: CommandMessage & command = msg;
			logFile << ' ';
			command.printTagName ( logFile );
			break;
		} case H245 :: MultimediaSystemControlMessage :: e_indication: {
			const H245 :: IndicationMessage & indication = msg;
			logFile << ' ';
			indication.printTagName ( logFile );
			break;
		} default: // Do nothing
			break;
	}
	writeTrailer ( );
}

void OpengateLog :: logRadiusMsg ( const Radius :: Request & msg,
	Direction dir, bool forceDebug ) {
	if ( ! forceDebug && getLogLevel ( ) <= OpengateLog :: None )
		return;
	writeHeader ( dir, msg.getIp ( ), WORD ( msg.getPort ( ) ) );
	if ( forceDebug || getLogLevel ( ) >= OpengateLog :: Info ) {
		logFile.WriteLine ( "RADIUS" );
		logFile << setprecision ( 0 ) << msg;
	} else
		logFile << "RADIUS " << msg.getCodeName ( );
	writeTrailer ( );
}

OpengateLog * Log = 0, * radiusLog = 0;
