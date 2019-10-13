#pragma implementation
#include "ss.hpp"
#include "allocatable.hpp"
#include "pointer.hpp"
#include "radius.hpp"
#include <ptlib.h>
#include <ptlib/sockets.h>
#include "radiusthread.hpp"
#include "tarifround.hpp"
#include "tarifinfo.hpp"
#include "pricedata.hpp"
#include "requestinfo.hpp"
#include "accountinginfo.hpp"
#include "sqloutthread.hpp"
#include "Log.h"
#include "aftertask.hpp"
#include "ipport.hpp"
#include "icqcontact.hpp"
#include "radgwinfo.hpp"
#include "Conf.hpp"
#include <ptlib/svcproc.h>
#include <stdexcept>
#include "mysql.hpp"

using namespace Radius;
const int recvBufSize = 65000;
RadiusThread :: RadiusThread ( ) : PThread ( 1000, NoAutoDeleteThread, NormalPriority,
	"RadiusThread" ), authSock ( 1812 ), accSock ( 1813 ),
	recvBuf ( new unsigned char [ recvBufSize ], Pointer < unsigned char > :: delArray ) {
	Resume ( );
}

RadiusThread :: ~RadiusThread ( ) { }

void RadiusThread :: Close ( ) {
	authSock.Close ( );
	accSock.Close ( );
}

void RadiusThread :: Main ( ) {
	PSYSTEMLOG ( Info, "RadiusThread::Main()" );
	if ( ! authSock.Listen ( PIPSocket :: Address ( "*" ) ) ) {
		PSYSTEMLOG ( Error, "Listen failed on radius auth socket: " <<
			authSock.GetErrorText ( ) );
		return;
	}
	if ( ! accSock.Listen ( PIPSocket :: Address ( "*" ) ) ) {
		PSYSTEMLOG ( Error, "Listen failed on radius acc socket: " <<
			accSock.GetErrorText ( ) );
		return;
	}
	while ( authSock.IsOpen ( ) && accSock.IsOpen ( ) ) {
		int r = PSocket :: Select ( authSock, accSock, 3000 );
		switch ( r ) {
			case - 1:
				readAuth ( );
				break;
			case - 2:
				readAcc ( );
				break;
			case - 3:
				readAuth ( );
				readAcc ( );
				break;
			case 0:
				break;
			default:
				PSYSTEMLOG ( Info, "Error " << r << " on radius select" );
		}
	}
	PSYSTEMLOG ( Info, "RadiusThread::Main() ended" );
}

void RadiusThread :: readAuth ( ) {
	if ( ! authSock.Read ( recvBuf, recvBufSize ) ) {
		PSYSTEMLOG ( Error, "cant read radius auth socket: " << authSock.GetErrorText ( ) );
		return;
	}
	PIPSocket :: Address addr;
	WORD port;
	authSock.GetLastReceiveAddress ( addr, port );
	RadGWInfo gw;
	conf -> getSecret ( addr, gw );
	if ( ! gw.verifySecret )
		port = 1812;
	try {
		Pointer < Request > r = new Request ( gw.secret,
			static_cast < const char * > ( addr.AsString ( ) ), port, recvBuf,
			authSock.GetLastReadCount ( ) );
		PSYSTEMLOG ( Info, "received auth" );
		radiusLog -> logRadiusMsg ( * r, OpengateLog :: Receiving );
		handleAuth ( r, gw );
	} catch ( std :: exception & e ) {
		PSYSTEMLOG ( Error, "exception: " << e.what ( ) );
	}
}

int RadiusThread :: handleAuthInt ( const Request * r, RequestInfo & ri, const RadGWInfo & gw ) {
	ri.hasStart = gw.hasStart;
	int pNum = 0;
	while ( const AttributeString * proxyState = r -> findString ( aProxyState, pNum ++ ) )
		ri.proxyStates.push_back ( proxyState -> getVal ( ) );
	if ( const AttributeString * userName = r -> findString ( aUserName ) ) {
		ss :: ostringstream os;
		os << std :: atoi ( userName -> getVal ( ).c_str ( ) );
		ri.acctn = os.str ( );
	}
	ri.pass = r -> getUserPassword ( );
	if ( ri.pass.empty ( ) ) {
		PSYSTEMLOG ( Error, "no pass" );
		return - 1;
	} else {
		ss :: ostringstream os;
		os << strtoll ( ri.pass.c_str ( ), 0, 10 );
		ri.pass = os.str ( );
	}
	if ( r -> findCisco ( cH323Ivr, "h323-ivr-out=register:yes" ) )
		ri.registerAni = true;
	if ( r -> findCisco ( cH323Ivr, "h323-ivr-out=service:callback" ) )
		ri.callback = true;
	if ( r -> findCisco ( cH323Ivr, "h323-ivr-out=service:callback1.5" ) )
		ri.callback15 = true;
	if ( r -> findCisco ( cH323Ivr, "h323-ivr-out=service:callback2" ) )
		ri.callback2 = true;
	if ( r -> findCisco ( cH323Ivr, "h323-ivr-out=service:callback3" ) )
		ri.callback3 = true;
	if ( r -> findCisco ( cH323Ivr, "h323-ivr-out=service:callback4" ) )
		ri.callback4 = true;
	if ( r -> findVendorString ( cH323Ivr, "h323-ivr-out=service:smscallback" ) ) {
		ri.smscallback = true;
		ri.callback = true;
	}
	if ( r -> findVendorString ( cH323Ivr, "h323-ivr-out=service:smscallback1" ) )
		ri.smscallback1 = true;
	if ( r -> findVendorString ( cH323Ivr, "h323-ivr-out=service:smscallback2" ) )
		ri.smscallback2 = true;
	if ( r -> findVendorString ( cH323Ivr, "h323-ivr-out=service:smscallback3" ) )
		ri.smscallback3 = true;
	r -> getVendorString ( cH323Ivr, "h323-ivr-out=NUMBER1:", ri.number1 );
	r -> getVendorString ( cH323Ivr, "h323-ivr-out=NUMBER2:", ri.number2 );
	r -> getVendorString ( cH323Ivr, "h323-ivr-out=language:", ri.lang );
	if ( const AttributeCisco * cbTime = r -> findCisco ( cH323Ivr, "h323-ivr-out=callback-time:" ) )
		ri.cbTime = std :: atoi ( cbTime -> getVal ( ).substr ( 27 ).c_str ( ) );
	else if ( const ss :: string * s = r -> findVendorString ( cH323Ivr, "h323-ivr-out=ELAPSED-TIME:" ) )
		ri.cbTime = std :: atoi ( s -> substr ( 26 ).c_str ( ) );
	if ( gw.useAni ) {
		if ( const AttributeCisco * ani = r -> findCisco ( cH323Ivr, "h323-ivr-out=ANI:" ) )
			ri.ani = ani -> getVal ( ).substr ( 17 );
		else if ( const AttributeString * ani = r -> findString ( aCallingStationId ) )
			ri.ani = ani -> getVal ( );
		else
			PSYSTEMLOG ( Error, "no ani" );
	}
	ri.ani = MySQL :: escape ( ri.ani );
	if ( const AttributeInt * serviceType = r -> findInt ( aServiceType ) )
		ri.inet = serviceType -> getVal ( ) == sFramed;
	else
		ri.inet = gw.defaultServiceType == sFramed;
	if ( const AttributeString * calledStationId = r -> findString ( aCalledStationId ) )
		ri.calledStationId = calledStationId -> getVal ( );
	if ( gw.useDnis ) {
		if ( const AttributeCisco * dnis = r -> findCisco ( cH323Ivr, "h323-ivr-out=DNIS:" ) )
			ri.dnis = dnis -> getVal ( ).substr ( 18 );
		else if ( ri.inet )
			ri.dnis = ri.calledStationId;
	} else if ( gw.useAcode )
		r -> getVendorString ( cH323Ivr, "h323-ivr-out=ACCESSCODE:", ri.dnis );
	ri.dnis = MySQL :: escape ( ri.dnis );
	ri.ip = r -> getIp ( );
	if ( const AttributeString * session = r -> findString ( aAcctSessionId ) )
		ri.sessionId = session -> getVal ( );
	if ( const AttributeCisco * h323ConfId = r -> findCisco ( cH323ConfId, "h323-conf-id=" ) )
		ri.h323Conf = h323ConfId -> getVal ( ).substr ( 13 );
	else if ( const AttributeQuintum * h323ConfId = r -> findQuintum ( cH323ConfId, "h323-conf-id=" ) )
		ri.h323Conf = h323ConfId -> getVal ( ).substr ( 13 );
	else {
		if ( ! ri.inet ) {
			PSYSTEMLOG ( Error, "no h323-conf-id" );
			return - 1;
		}
		if ( gw.unameOnly )
			ri.h323Conf = ri.acctn;
		else {
			const AttributeInt * ip = r -> findInt ( aNASIPAddress );
			if ( ! ri.sessionId.empty ( ) && ip ) {
				PIPSocket :: Address addr ( htonl ( ip -> getVal ( ) ) );
				ri.h323Conf = static_cast < const char * > ( addr.AsString ( ) );
				ri.h323Conf += ' ' + ri.sessionId;
			} else {
				PSYSTEMLOG ( Error, "no sessionid or nasipaddress" );
				return - 1;
			}
		}
	}
	if ( ri.sessionId.empty ( ) && ! gw.hasStart )
		ri.sessionId = ri.h323Conf;
	if ( const AttributeCisco * cbn = r -> findCisco ( cH323Ivr, "h323-ivr-out=callback-real-number:" ) )
		ri.cbNumber = cbn -> getVal ( ).substr ( 34 );
	if ( const AttributeCisco * ac = r -> findCisco ( cH323Ivr, "h323-ivr-out=apply-code:" ) )
		ri.applyCode = ac -> getVal ( ).substr ( 24 );
	if ( ! ri.applyCode.empty ( ) )
		return conf -> applyCode ( ri );
	if ( ri.smscallback )
		return conf -> handleSmsCallback ( ri );
	if ( ! ri.inet && ri.calledStationId.empty ( ) )
		return conf -> checkPin ( ri );
	return conf -> getCreditTime ( ri );
}

static ss :: string printCredit ( int c ) {
	if ( c >= 100000000 )
		return "unlimited";
	ss :: ostringstream os;
	os << c;
	return os.str ( );
}

void RadiusThread :: handleAuth ( const Request * r, const RadGWInfo & gw ) {
	RequestInfo ri;
	int ret = handleAuthInt ( r, ri, gw );
	Request answer ( * r, ret ? rAccessReject : rAccessAccept );
	for ( unsigned i = 0; i < ri.proxyStates.size ( ); i ++ )
		answer.append ( new ProxyState ( ri.proxyStates [ i ] ) );
	if ( ! ri.inet ) {
		answer.append ( new H323BillingModel ( "1" ) );
		ss :: ostringstream os;
		os << - ret;
		answer.append ( new H323ReturnCode ( os.str ( ) ) );
		if ( ! gw.sendDuration ) {
			answer.append ( new H323IvrIn ( "options:" + ri.options ) );
			answer.append ( new H323IvrIn ( "language:" + ri.lang ) );
			answer.append ( new H323IvrIn ( "time:" + printCredit ( ri.creditRealTime ) ) );
		} else {
			answer.append ( new H323PreferredLang ( ri.lang ) );
			answer.append ( new H323IvrIn ( printCredit ( ri.creditTime ) ) );
		}
		if ( ret != - 2 ) {
			os.str ( "" );
			os << setprecision ( 2 ) << setiosflags ( ios :: fixed ) << ri.creditAmount * ri.valuteRate;
			answer.append ( new H323CreditAmount ( os.str ( ) ) );
		}
	}
	if ( ! ret ) {
		if ( ! ri.inet ) {
			answer.append ( new H323CreditTime ( printCredit ( ri.creditTime ) ) );
/*
			if ( gw.sendDuration )
				answer.append ( new H323IvrIn ( printCredit ( ri.creditTime ) ) );
			else
				answer.append ( new H323IvrIn ( "time:" + printCredit ( ri.creditRealTime ) ) );
*/
		} else {
			if ( const AttributeInt * p = r -> findInt ( aFramedProtocol ) )
				answer.append ( new FramedProtocol ( p -> getVal ( ) ) );
			if ( r -> findInt ( aServiceType ) ) {
				// callback frag for internet cards is set in Conf::getCreditTime() -
				// when card is configured as callback='y' in database
				if ( ri.callback ) {
					// cisco starts internet dialup-callback when receiving ServiceType=sCallbackFramed;
					// other attributes were not investigated, but cisco needs them to successfully
					// establish PPP interconnection.
					// These AV-pairs were taken from freeradius AuthenticationAck,
					// upon receiving it cisco did callback successfully - so we
					// just mimic freeradius behavior
					answer.append ( new FramedMTU ( 1500 ) );
					answer.append ( new FramedCompression ( fcVJTCPIP ) );
					answer.append ( new ServiceType ( sCallbackFramed ) );
					answer.append ( new IdleTimeout ( 600 ) );
					answer.append ( new PortLimit ( 1 ) );
				} else
					answer.append ( new ServiceType ( sFramed ) );
			}
		}
		answer.append ( new SessionTimeout ( ri.creditTime ) );
		if ( ri.callback && ! ri.inet && ! gw.sendDuration ) {
			answer.append ( new H323IvrIn ( "callback-number:" + ri.cbDialNumber ) );
			answer.append ( new H323IvrIn ( "callback-real-number:" + ri.cbRealNumber ) );
		}
		if ( ! ri.inet && ! gw.sendDuration ) {
			if ( ri.requireCallbackAuth )
				answer.append ( new H323IvrIn ( "require-callback-auth:y" ) );
			else
				answer.append ( new H323IvrIn ( "require-callback-auth:n" ) );
		}
		for ( StringStringMap :: const_iterator i = ri.radiusClass.begin ( );
			i != ri.radiusClass.end ( ); ++ i ) {
			if ( i -> first == "Xedia-DNS-Server" )
				answer.append ( new XediaDNSServer ( i -> second ) );
			else if ( i -> first == "Xedia-Address-Pool" )
				answer.append ( new XediaAddressPool ( i -> second ) );
			else if ( i -> first == "Ascend-Client-Primary-DNS" )
				answer.append ( new AscendClientPrimaryDNS ( i -> second ) );
			else if ( i -> first == "Ascend-Client-Secondary-DNS" )
				answer.append ( new AscendClientSecondaryDNS ( i -> second ) );
			else if ( i -> first == "Framed-IP-Address" )
				answer.append ( new FramedIPAddress ( i -> second ) );
			else if ( i -> first == "Framed-IP-Netmask" )
				answer.append ( new FramedIPNetmask ( i -> second ) );
			else if ( i -> first == "Framed-Pool" )
				answer.append ( new FramedPool ( i -> second ) );
			else if ( i -> first == "Filter-Id" )
				answer.append ( new FilterId ( i -> second ) );
			else if ( i -> first == "Cisco-Assign-IP-Pool" )
				answer.append ( new CiscoAssignIPPool ( i -> second ) );
			else if ( i -> first == "Framed-Protocol" )
				answer.append ( new FramedProtocol ( std :: atoi ( i -> second.c_str ( ) ) ) );
			else
				PSYSTEMLOG ( Error, "unknown radius class parameter " << i -> first );
		}
	}
	ret = answer.print ( reinterpret_cast < char * > ( recvBuf.data ( ) ), recvBufSize );
	authSock.SetSendAddress ( PString ( answer.getIp ( ).c_str ( ) ), WORD ( answer.getPort ( ) ) );
	if ( ! authSock.Write ( recvBuf, ret ) )
		PSYSTEMLOG ( Error, "auth write: " << authSock.GetErrorText ( ) );
	PSYSTEMLOG ( Info, "sent auth reply" );
	radiusLog -> logRadiusMsg ( answer, OpengateLog :: Sending );
}

void RadiusThread :: readAcc ( ) {
	if ( ! accSock.Read ( recvBuf, recvBufSize ) ) {
		PSYSTEMLOG ( Error, "cant read radius acc socket: " << accSock.GetErrorText ( ) );
		return;
	}
	PIPSocket :: Address addr;
	WORD port;
	accSock.GetLastReceiveAddress ( addr, port );
	RadGWInfo gw;
	conf -> getSecret ( addr, gw );
	if ( ! gw.verifySecret )
		port = 1813;
	try {
		Pointer < Request > r = new Request ( gw.secret,
			static_cast < const char * > ( addr.AsString ( ) ), port, recvBuf,
			accSock.GetLastReadCount ( ) );
		PSYSTEMLOG ( Info, "received acc from " << addr << ':' << port );
		radiusLog -> logRadiusMsg ( * r, OpengateLog :: Receiving );
		if ( gw.verifySecret && ! r -> accSecretVerified ( ) ) {
			PSYSTEMLOG ( Error, "cannot verify secret" );
			return;
		}
		handleAcc ( r, gw );
	} catch ( std :: exception & e ) {
		PSYSTEMLOG ( Error, "exception: " << e.what ( ) );
	}
}

void RadiusThread :: handleAcc ( const Request * r, const RadGWInfo & gw ) {
	if ( r -> getCode ( ) != rAccountingRequest )
		return;
	Request answer ( * r, rAccountingResponse );
	handleAccInt ( r, answer, gw );
	int l = answer.print ( reinterpret_cast < char * > ( recvBuf.data ( ) ), recvBufSize );
	accSock.SetSendAddress ( PString ( answer.getIp ( ).c_str ( ) ), WORD ( answer.getPort ( ) ) );
	if ( ! accSock.Write ( recvBuf, l ) )
		PSYSTEMLOG ( Error, "acc write: " << accSock.GetErrorText ( ) );
	PSYSTEMLOG ( Info, "sent acc reply" );
	radiusLog -> logRadiusMsg ( answer, OpengateLog :: Sending );
}

void RadiusThread :: handleAccInt ( const Request * r, Request & answer, const RadGWInfo & gw ) {
	AccountingInfo ai;
	if ( const AttributeInt * a = r -> findInt ( aNASPortType ) )
		ai.nasPortType = a -> getVal ( );
	if ( const AttributeString * a = r -> findString ( aUserName ) )
		ai.userName = a -> getVal ( );
	if ( const AttributeString * a = r -> findString ( aCalledStationId ) )
		ai.calledStationId = a -> getVal ( );
	if ( const AttributeString * a = r -> findString ( aCallingStationId ) )
		ai.callingStationId = a -> getVal ( );
	if ( const AttributeInt * a = r -> findInt ( aAcctStatusType ) )
		ai.acctStatusType = a -> getVal ( );
	if ( const AttributeInt * a = r -> findInt ( aServiceType ) )
		ai.serviceType = a -> getVal ( );
	else
		ai.serviceType = gw.defaultServiceType;
	if ( const AttributeString * a = r -> findString ( aAcctSessionId ) )
		ai.acctSessionId = a -> getVal ( );
	if ( const AttributeInt * a = r -> findInt ( aAcctInputOctets ) )
		ai.acctInputOctets = a -> getVal ( );
	if ( const AttributeInt * a = r -> findInt ( aAcctOutputOctets ) )
		ai.acctOutputOctets = a -> getVal ( );
	if ( const AttributeInt * a = r -> findInt ( aAcctSessionTime ) )
		ai.acctSessionTime = a -> getVal ( );
	if ( const AttributeInt * a = r -> findInt ( aAcctDelayTime ) )
		ai.acctDelayTime = a -> getVal ( );
	if ( const AttributeInt * a = r -> findInt ( aNASIPAddress ) )
		ai.nasIpAddress = static_cast < const char * > (
			PIPSocket :: Address ( htonl ( a -> getVal ( ) ) ).AsString ( ) );
	if ( const AttributeVendorString * a = r -> findVendorString ( 9, cCiscoNASPort ) )
		ai.ciscoNasPort = a -> getVal ( );
	else if ( const AttributeVendorString * a = r -> findVendorString ( 6618, cCiscoNASPort ) )
		ai.ciscoNasPort = a -> getVal ( );
	if ( const ss :: string * s = r -> findVendorString ( cH323ConfId, "h323-conf-id=" ) )
		ai.h323ConfId = s -> substr ( 13 );
	if ( const ss :: string * s = r -> findVendorString ( cH323IncomingConfId, "h323-incoming-conf-id=" ) )
		ai.h323IncomingConfId = s -> substr ( 22 );
	if ( const ss :: string * s = r -> findVendorString ( cH323GWId, "h323-gw-id=" ) )
		ai.h323GwId = s -> substr ( 11 );
	if ( const ss :: string * s = r -> findVendorString ( cH323CallOrigin, "h323-call-origin=" ) )
		ai.h323CallOrigin = s -> substr ( 17 );
	if ( const ss :: string * s = r -> findVendorString ( cH323CallType, "h323-call-type=" ) )
		ai.h323CallType = s -> substr ( 15 );
	if ( const ss :: string * s = r -> findVendorString ( cH323SetupTime, "h323-setup-time=" ) )
		ai.h323SetupTime = s -> substr ( 16 );
	if ( const ss :: string * s = r -> findVendorString ( cH323ConnectTime, "h323-connect-time=" ) )
		ai.h323ConnectTime = s -> substr ( 18 );
	if ( const ss :: string * s = r -> findVendorString ( cH323DisconnectTime, "h323-disconnect-time=" ) )
		ai.h323DisconnectTime = s -> substr ( 21 );
	if ( const ss :: string * s = r -> findVendorString ( cH323DisconnectCause, "h323-disconnect-cause=" ) )
		ai.h323DisconnectCause = s -> substr ( 22 );
	if ( const ss :: string * s = r -> findVendorString ( cH323VoiceQuality, "h323-voice-quality=" ) )
		ai.h323VoiceQuality = s -> substr ( 19 );
	if ( const ss :: string * s = r -> findVendorString ( cH323RemoteAddress, "h323-remote-address=" ) )
		ai.h323RemoteAddress = s -> substr ( 20 );
	ss :: string confId = ai.h323ConfId;
	if ( ai.serviceType == sFramed ) {
		if ( gw.unameOnly )
			ai.h323ConfId = ai.userName;
		else {
			if ( ai.nasIpAddress.empty ( ) || ai.acctSessionId.empty ( ) )
				PSYSTEMLOG ( Error, "no nasipaddress or sesionid" );
			ai.h323ConfId = ai.nasIpAddress + ' ' + ai.acctSessionId;
		}
	} else 	if ( ! ai.h323IncomingConfId.empty ( ) )
		ai.h323ConfId = ai.h323IncomingConfId;
	if ( ! gw.hasStart )
		ai.acctSessionId = ai.h323ConfId;
	conf -> handleAccounting ( ai );
	if ( const AttributeString * a = r -> findString ( aAcctSessionId ) )
		ai.acctSessionId = a -> getVal ( );
	if ( ai.sessionTimeout )
		answer.append ( new SessionTimeout ( ai.sessionTimeout ) );
	ss :: ostringstream os;
	
	os << "insert into radacct values ( 0, '" << ai.nasIpAddress << "', '" << ai.ciscoNasPort <<
		"', " << ai.nasPortType << ", '" << ai.userName << "', '" << ai.calledStationId <<
		"', '" << ai.callingStationId << "', '-||-||-', " << ai.acctStatusType << ", " << ai.serviceType <<
		", '" << ai.h323GwId << "', '" << confId << "', '" << ai.h323CallOrigin <<
		"', '" << ai.h323CallType << "', '" << ai.h323SetupTime << "', '" << ai.h323ConnectTime
		<< "', '" << ai.h323DisconnectTime << "', '" << ai.h323DisconnectCause << "', '" <<
		ai.h323VoiceQuality << "', '" << ai.h323RemoteAddress << "', '" << ai.acctSessionId <<
		"', " << ai.acctInputOctets << ", " << ai.acctOutputOctets << ", " <<
		ai.acctSessionTime << ", " << ai.acctDelayTime << ", now(), 0, '' )";
	sqlOut -> add ( os.str ( ) );
	return;
}
