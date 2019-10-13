#pragma implementation
#include "ss.hpp"
#include "allocatable.hpp"
#include "pointer.hpp"
#include "radius.hpp"
#include <tr1/cstdint>
#include "md5.hpp"
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iomanip>
#include <stdexcept>
#include <sys/socket.h>
#include <cstring>
#include <boost/io/ios_state.hpp>

template < typename _CharT, typename _Traits, typename _Alloc > static bool hasPrefix (
	const std :: basic_string < _CharT, _Traits, _Alloc > & s, const _CharT * p ) {
	typedef std :: basic_string < _CharT, _Traits, _Alloc > str_type;
	typedef typename str_type :: size_type size_type;
	size_type sz = s.size ( );
	const _CharT * d = s.data ( );
	for ( size_type i = 0; i < sz; i ++ ) {
		_CharT c = p [ i ];
		if ( c == _CharT ( ) )
			return true;
		if ( ! _Traits :: eq ( c, d [ i ] ) )
			return false;
	}
	return p [ sz ] == _CharT ( );
}

namespace Radius {

Attribute * Attribute :: load ( const unsigned char * buf, int maxl, int & l ) {
	if ( maxl < 2 )
		return 0;
	l = buf [ 1 ];
	if ( l > maxl || l < 2 )
		return 0;
	if ( buf [ 0 ] == aVendor )
		return AttributeVendor :: load ( buf, l );
	int ival = 0;
	ss :: string sval ( reinterpret_cast < const char * > ( buf ) + 2, l - 2 );
	if ( l >= 6 )
		ival = buf [ 2 ] * 65536 * 256 + buf [ 3 ] * 65536 + buf [ 4 ] * 256 + buf [ 5 ];
	switch ( buf [ 0 ] ) {
		case aUserName:
			return new UserName ( sval );
		case aUserPassword:
			return new UserPassword ( sval );
		case aNASIPAddress:
			return new NASIPAddress ( ival );
		case aNASPort:
			return new NASPort ( ival );
		case aServiceType:
			return new ServiceType ( ival );
		case aFramedProtocol:
			return new FramedProtocol ( ival );
		case aFramedIPAddress:
			return new FramedIPAddress ( ival );
		case aFilterId:
			return new FilterId ( sval );
		case aFramedIPNetmask:
			return new FramedIPNetmask ( ival );
		case aSessionTimeout:
			return new SessionTimeout ( ival );
		case aCalledStationId:
			return new CalledStationId ( sval );
		case aCallingStationId:
			return new CallingStationId ( sval );
		case aModifiedANumber:
			return new ModifiedANumber ( sval );
		case aProxyState:
			return new ProxyState ( sval );
		case aNASIdentifier:
			return new NASIdentifier ( sval );
		case aAcctStatusType:
			return new AcctStatusType ( ival );
		case aAcctDelayTime:
			return new AcctDelayTime ( ival );
		case aAcctInputOctets:
			return new AcctInputOctets ( ival );
		case aAcctOutputOctets:
			return new AcctOutputOctets ( ival );
		case aAcctSessionId:
			return new AcctSessionId ( sval );
		case aAcctAuthentic:
			return new AcctAuthentic ( ival );
		case aAcctSessionTime:
			return new AcctSessionTime ( ival );
		case aAcctInputPackets:
			return new AcctInputPackets ( ival );
		case aAcctOutputPackets:
			return new AcctOutputPackets ( ival );
		case aAcctTerminateCause:
			return new AcctTerminateCause ( ival );
		case aAcctMultiSessionId:
			return new AcctMultiSessionId ( sval );
		case aAcctLinkCount:
			return new AcctLinkCount ( ival );
		case aEventTimestamp:
			return new EventTimestamp ( ival );
		case aNASPortType:
			return new NASPortType ( ival );
		case aConnectInfo:
			return new ConnectInfo ( sval );
		case aFramedPool:
			return new FramedPool ( sval );
	}
	ss :: ostringstream os;
	os << "unknown " << int ( buf [ 0 ] );
	if ( l == 6 )
		return new AttributeInt ( os.str ( ), buf [ 0 ], ival );
	return new AttributeString ( os.str ( ), buf [ 0 ], sval );
}

int Attribute :: print ( char * buf, int size ) const {
	if ( size < 2 )
		return 0;
	size -= 2;
	* buf = char ( attribute );
	int l = printVal ( buf + 2, size );
	if ( l < 0 )
		return 0;
	l += 2;
	buf [ 1 ] = char ( l );
	return l;
}

void Attribute :: print ( std :: ostream & os ) const {
	os << attribute << ':' << name << ':';
	printVal ( os );
}

Attribute :: Attribute ( const ss :: string & n, int attr ) : name ( n ),
	attribute ( attr ) { }

Attribute :: ~Attribute ( ) { }

int Attribute :: getAttr ( ) const {
	return attribute;
}

const ss :: string & Attribute :: getName ( ) const {
	return name;
}

AttributeString :: AttributeString ( const ss :: string & n, int attr, const ss :: string & val ) :
	Attribute ( n, attr ), value ( val ) { }

int AttributeString :: printVal ( char * buf, int size ) const {
	ss :: string :: size_type sz = value.size ( );
	if ( ss :: string :: size_type ( size ) < sz || sz > 253 )
		return 0;
	std :: memcpy ( buf, value.data ( ), sz );
	return int ( sz );
}

void AttributeString :: printVal ( std :: ostream & os ) const {
	os << value;
}

const ss :: string & AttributeString :: getVal ( ) const {
	return value;
}

UserName :: UserName ( const ss :: string & val ) : AttributeString ( "UserName", aUserName, val ) { }

UserPassword :: UserPassword ( const ss :: string & val ) : AttributeString ( "UserPassword", aUserPassword, val ) { }

void UserPassword :: printVal ( std :: ostream & os ) const {
	os << "****";
}

FramedPool :: FramedPool ( const ss :: string & val ) : AttributeString ( "FramedPool", aFramedPool, val ) { }

FilterId :: FilterId ( const ss :: string & val ) : AttributeString ( "FilterId", aFilterId, val ) { }

CalledStationId :: CalledStationId ( const ss :: string & val ) :
	AttributeString ( "CalledStationId", aCalledStationId, val ) { }

CallingStationId :: CallingStationId ( const ss :: string & val ) :
	AttributeString ( "CallingStationId", aCallingStationId, val ) { }

NASIdentifier :: NASIdentifier ( const ss :: string & val ) : AttributeString ( "NASIdentifier", aNASIdentifier, val ) { }

ProxyState :: ProxyState ( const ss :: string & val ) : AttributeString ( "ProxyState", aProxyState, val ) { }

AcctSessionId :: AcctSessionId ( const ss :: string & val ) : AttributeString ( "AcctSessionId", aAcctSessionId, val ) { }

AcctMultiSessionId :: AcctMultiSessionId ( const ss :: string & val ) :
	AttributeString ( "AcctMultiSessionId", aAcctMultiSessionId, val ) { }

ConnectInfo :: ConnectInfo ( const ss :: string & val ) : AttributeString ( "ConnectInfo", aConnectInfo, val ) { }

ModifiedANumber :: ModifiedANumber ( const ss :: string & val ) :
	AttributeString ( "ModifiedANumber", aModifiedANumber, val ) { }

AttributeInt :: AttributeInt ( const ss :: string & n, int attr, int val ) :
	Attribute ( n, attr ), value ( val ) { }

int AttributeInt :: printVal ( char * buf, int size ) const {
	if ( size < 4 )
		return 0;
	buf [ 0 ] = char ( value >> 24 );
	buf [ 1 ] = char ( value >> 16 );
	buf [ 2 ] = char ( value >> 8 );
	buf [ 3 ] = char ( value );
	return 4;
}

void AttributeInt :: printVal ( std :: ostream & os ) const {
	os << value;
}

int AttributeInt :: getVal ( ) const {
	return value;
}

AttributeAddr :: AttributeAddr ( const ss :: string & n, int attr, int val ) : AttributeInt ( n, attr, val ) { }

static std :: tr1 :: uint32_t inetAddr ( const ss :: string & a ) {
	in_addr i;
	if ( inet_pton ( AF_INET, a.c_str ( ), & i ) <= 0 )
		throw std :: runtime_error ( std :: string ( "bad address " ).append ( a.begin ( ), a.end ( ) ) );
	return i.s_addr;
}

AttributeAddr :: AttributeAddr ( const ss :: string & n, int attr, const ss :: string & val ) :
	AttributeInt ( n, attr, ntohl ( inetAddr ( val ) ) ) { }

void AttributeAddr :: printVal ( std :: ostream & os ) const {
	struct in_addr sa;
	sa.s_addr = htonl ( getVal ( ) );
	char buf [ INET_ADDRSTRLEN ];
	os << inet_ntop ( AF_INET, & sa, buf, INET_ADDRSTRLEN );
}

NASIPAddress :: NASIPAddress ( int val ) : AttributeInt ( "NASIPAddress", aNASIPAddress, val ) { }

NASPort :: NASPort ( int val ) : AttributeInt ( "NASPort", aNASPort, val ) { }

ServiceType :: ServiceType ( int val ) : AttributeInt ( "ServiceType", aServiceType, val ) { }

void ServiceType :: printVal ( std :: ostream & os ) const {
	switch ( ServiceTypeValue ( getVal ( ) ) ) {
		case sLogin:
			os << "Login";
			return;
		case sFramed:
			os << "Framed";
			return;
		case sCallbackLogin:
			os << "CallbackLogin";
			return;
		case sCallbackFramed:
			os << "CallbackFramed";
			return;
		case sOutbound:
			os << "Outbound";
			return;
		case sAdministrative:
			os << "Administrative";
			return;
		case sNASPrompt:
			os << "NASPrompt";
			return;
		case sAuthenticateOnly:
			os << "AuthenticateOnly";
			return;
		case sCallbackNASPrompt:
			os << "CallbackNASPrompt";
			return;
		case sCallCheck:
			os << "CallCheck";
			return;
		case sCallbackAdministrative:
			os << "CallbackAdministrative";
			return;
		case sVoice:
			os << "Voice";
			return;
		case sFax:
			os << "Fax";
			return;
		case sModemRelay:
			os << "ModemRelay";
			return;
		case sIAPPRegister:
			os << "IAPPRegister";
			return;
		case sIAPPAPCheck:
			os << "IAPPAPCheck";
			return;
		case sAuthorizeOnly:
			os << "AuthorizeOnly";
			return;
	}
	AttributeInt :: printVal ( os );
}

FramedProtocol :: FramedProtocol ( int val ) :  AttributeInt ( "FramedProtocol", aFramedProtocol, val ) { }

void FramedProtocol :: printVal ( std :: ostream & os ) const {
	switch ( FramedProtocolValue ( getVal ( ) ) ) {
		case fPPP:
			os << "PPP";
			return;
		case fSLIP:
			os << "SLIP";
			return;
		case fARAP:
			os << "ARAP";
			return;
		case fGandalf:
			os << "Gandalf";
			return;
		case fXylogics:
			os << "Xylogics";
			return;
		case fX75Synchronous:
			os << "X75Synchronous";
			return;
		case fGPRSPDPContext:
			os << "GPRSPDPContext";
			return;
	}
	AttributeInt :: printVal ( os );
}

FramedIPAddress :: FramedIPAddress ( int val ) : AttributeAddr ( "FramedIPAddress", aFramedIPAddress, val ) { }

FramedIPAddress :: FramedIPAddress ( const ss :: string & val ) : AttributeAddr ( "FramedIPAddress", aFramedIPAddress, val ) { }

FramedIPNetmask :: FramedIPNetmask ( int val ) : AttributeAddr ( "FramedIPNetmask", aFramedIPNetmask, val ) { }

FramedIPNetmask :: FramedIPNetmask ( const ss :: string & val ) : AttributeAddr ( "FramedIPNetmask", aFramedIPNetmask, val ) { }

FramedMTU :: FramedMTU ( int val ) : AttributeInt ( "FramedMTU", aFramedMTU, val ) { }

FramedCompression :: FramedCompression ( int val ) : AttributeInt ( "FramedCompression", aFramedCompression, val ) { }

void FramedCompression :: printVal ( std :: ostream & os ) const {
	switch ( FramedCompressionValue ( getVal ( ) ) ) {
		case fcNone:
			os << "None";
			return;
		case fcVJTCPIP:
			os << "VJTCPIP";
			return;
		case fcIPX:
			os << "IPX";
			return;
		case fcStacLZS:
			os << "StacLZS";
			return;
	}
	AttributeInt :: printVal ( os );
}

SessionTimeout :: SessionTimeout ( int val ) : AttributeInt ( "SessionTimeout", aSessionTimeout, val ) { }

IdleTimeout :: IdleTimeout ( int val ) : AttributeInt ( "IdleTimeout", aIdleTimeout, val ) { }

AcctStatusType :: AcctStatusType ( int val ) : AttributeInt ( "AcctStatusType", aAcctStatusType, val ) { }

void AcctStatusType :: printVal ( std :: ostream & os ) const {
	switch ( AcctStatusTypeValue ( getVal ( ) ) ) {
		case sStart:
			os << "Start";
			return;
		case sStop:
			os << "Stop";
			return;
		case sInterimUpdate:
			os << "InterimUpdate";
			return;
		case sAccountingOn:
			os << "AccountingOn";
			return;
		case sAccountingOff:
			os << "AccountingOff";
			return;
		case sTunnelStart:
			os << "TunnelStart";
			return;
		case sTunnelStop:
			os << "TunnelStop";
			return;
		case sTunnelReject:
			os << "TunnelReject";
			return;
		case sTunnelLinkStart:
			os << "TunnelLinkStart";
			return;
		case sTunnelLinkStop:
			os << "TunnelLinkStop";
			return;
		case sTunnelLinkReject:
			os << "TunnelLinkReject";
			return;
		case sFailed:
			os << "Failed";
			return;
	}
	AttributeInt :: printVal ( os );
}

AcctDelayTime :: AcctDelayTime ( int val ) : AttributeInt ( "AcctDelayTime", aAcctDelayTime, val ) { }

AcctInputOctets :: AcctInputOctets ( int val ) : AttributeInt ( "AcctInputOctets", aAcctInputOctets, val ) { }

AcctOutputOctets :: AcctOutputOctets ( int val ) : AttributeInt ( "AcctOutputOctets", aAcctOutputOctets, val ) { }

AcctAuthentic :: AcctAuthentic ( int val ) : AttributeInt ( "AcctAuthentic", aAcctAuthentic, val ) { }

void AcctAuthentic :: printVal ( std :: ostream & os ) const {
	switch ( AcctAuthenticValue ( getVal ( ) ) ) {
		case aaRADIUS:
			os << "RADIUS";
			return;
		case aaLocal:
			os << "Local";
			return;
		case aaRemote:
			os << "Remote";
			return;
		case aaDiameter:
			os << "Diameter";
			return;
	}
	AttributeInt :: printVal ( os );
}

AcctSessionTime :: AcctSessionTime ( int val ) : AttributeInt ( "AcctSessionTime", aAcctSessionTime, val ) { }

AcctInputPackets :: AcctInputPackets ( int val ) : AttributeInt ( "AcctInputPackets", aAcctInputPackets, val ) { }

AcctOutputPackets :: AcctOutputPackets ( int val ) : AttributeInt ( "AcctOutputPackets", aAcctOutputPackets, val ) { }

AcctTerminateCause :: AcctTerminateCause ( int val ) : AttributeInt ( "AcctTerminateCause", aAcctTerminateCause, val ) { }

void AcctTerminateCause :: printVal ( std :: ostream & os ) const {
	switch ( AcctTerminateCauseValue ( getVal ( ) ) ) {
		case atUserRequest:
			os << "UserRequest";
			return;
		case atLostCarrier:
			os << "LostCarrier";
			return;
		case atLostService:
			os << "LostService";
			return;
		case atIdleTimeout:
			os << "IdleTimeout";
			return;
		case atSessionTimeout:
			os << "SessionTimeout";
			return;
		case atAdminReset:
			os << "AdminReset";
			return;
		case atAdminReboot:
			os << "AdminReboot";
			return;
		case atPortError:
			os << "PortError";
			return;
		case atNASError:
			os << "NASError";
			return;
		case atNASRequest:
			os << "NASRequest";
			return;
		case atNASReboot:
			os << "NASReboot";
			return;
		case atPortUnneeded:
			os << "PortUnneeded";
			return;
		case atPortPreempted:
			os << "PortPreempted";
			return;
		case atPortSuspended:
			os << "PortSuspended";
			return;
		case atServiceUnavailable:
			os << "ServiceUnavailable";
			return;
		case atCallback:
			os << "Callback";
			return;
		case atUserError:
			os << "UserError";
			return;
		case atHostRequest:
			os << "HostRequest";
			return;
		case atSupplicantRestart:
			os << "SupplicantRestart";
			return;
		case atReauthenticationFailure:
			os << "ReauthenticationFailure";
			return;
		case atPortReinitialized:
			os << "PortReinitialized";
			return;
		case atPortAdministrativelyDisabled:
			os << "PortAdministrativelyDisabled";
			return;
	}
	AttributeInt :: printVal ( os );
}

AcctLinkCount :: AcctLinkCount ( int val ) : AttributeInt ( "AcctLinkCount", aAcctMultiSessionId, val ) { }

EventTimestamp :: EventTimestamp ( int val ) : AttributeInt ( "EventTimestamp", aEventTimestamp, val ) { }

NASPortType :: NASPortType ( int val ) : AttributeInt ( "NASPortType", aNASPortType, val ) { }

void NASPortType :: printVal ( std :: ostream & os ) const {
	switch ( NASPortTypeValue ( getVal ( ) ) ) {
		case npAsync:
			os << "Async";
			return;
		case npSync:
			os << "Sync";
			return;
		case npISDNSync:
			os << "ISDNSync";
			return;
		case npISDNAsyncV120:
			os << "ISDNAsyncV120";
			return;
		case npISDNAsyncV110:
			os << "ISDNAsyncV110";
			return;
		case npVirtual:
			os << "Virtual";
			return;
		case npPIAFS:
			os << "PIAFS";
			return;
		case npHDLCClearChannel:
			os << "HDLCClearChannel";
			return;
		case npX25:
			os << "X25";
			return;
		case npX75:
			os << "X75";
			return;
		case npG3Fax:
			os << "G3Fax";
			return;
		case npSDSL:
			os << "SDSL";
			return;
		case npADSLCAP:
			os << "ADSLCAP";
			return;
		case npADSLDMT:
			os << "ADSLDMT";
			return;
		case npIDSL:
			os << "IDSL";
			return;
		case npEthernet:
			os << "Ethernet";
			return;
		case npxDSL:
			os << "xDSL";
			return;
		case npCable:
			os << "Cable";
			return;
		case npWirelessOther:
			os << "WirelessOther";
			return;
		case npIEEE80211:
			os << "IEEE80211";
			return;
		case npTokenRing:
			os << "TokenRing";
			return;
		case npFDDI:
			os << "FDDI";
			return;
		case npCDMA2000:
			os << "CDMA2000";
			return;
		case npUMTS:
			os << "UMTS";
			return;
		case np1XEV:
			os << "1XEV";
			return;
		case npIAPP:
			os << "IAPP";
			return;
		case npFTTP:
			os << "FTTP";
			return;
		case npIEEE80216:
			os << "IEEE80216";
			return;
		case npIEEE80220:
			os << "IEEE80220";
			return;
		case npIEEE80222:
			os << "IEEE80222";
			return;
		case npPPPoA:
			os << "PPPoA";
			return;
		case npPPPoEoA:
			os << "PPPoEoA";
			return;
		case npPPPoEoE:
			os << "PPPoEoE";
			return;
		case npPPPoEoVLAN:
			os << "PPPoEoVLAN";
			return;
		case npPPPoEoQinQ:
			os << "PPPoEoQinQ";
			return;
		case npxPON:
			os << "xPON";
			return;
	}
	AttributeInt :: printVal ( os );
}

PortLimit :: PortLimit ( int val ) : AttributeInt ( "PortLimit", aPortLimit, val ) { }

static ss :: string decodeValue ( const ss :: string & s ) {
	ss :: string :: size_type pos = s.find ( '=' );
	if ( pos == ss :: string :: npos )
		return s;
	return s.substr ( pos + 1 );
}

Attribute * AttributeVendor :: load ( const unsigned char * buf, int l ) {
	if ( l < 8 )
		return 0;
	if ( buf [ 7 ] != l - 6 )
		return 0;
	int vid = buf [ 2 ] * 65536 * 256 + buf [ 3 ] * 65536 + buf [ 4 ] * 256 + buf [ 5 ];
	int vt = buf [ 6 ];
	ss :: string val ( reinterpret_cast < const char * > ( buf ) + 8, l - 8 );
	int ival = 0;
	if ( l == 12 )
		ival = buf [ 8 ] * 65536 * 256 + buf [ 9 ] * 65536 + buf [ 10 ] * 256 + buf [ 11 ];
#define DO_VSA(cname,name) \
	if ( val.find ( name "=", 0, sizeof ( name ) ) == ss :: string :: npos ) \
		break; \
	return new cname ( decodeValue ( val ) )
#define TRY_VSA(cname,name) \
	if ( ! val.find ( name "=", 0, sizeof ( name ) ) ) \
		return new cname ( decodeValue ( val ) )
	if ( vid == vQuintum ) {
		switch ( QuintumAttributeType ( vt ) ) {
			case qH323Ivr:
				TRY_VSA ( QH323IncomingConfId, "h323-incoming-conf-id" );
				DO_VSA ( QH323IvrOut, "h323-ivr-out" );
			case qCiscoNASPort:
				return new QuintumNASPort ( val );
			case qH323RemoteAddress:
				DO_VSA ( QH323RemoteAddress, "h323-remote-address" );
			case qH323ConfId:
				DO_VSA ( QH323ConfId, "h323-conf-id" );
			case qH323SetupTime:
				DO_VSA ( QH323SetupTime, "h323-setup-time" );
			case qH323CallOrigin:
				DO_VSA ( QH323CallOrigin, "h323-call-origin" );
			case qH323CallType:
				DO_VSA ( QH323CallType, "h323-call-type" );
			case qH323ConnectTime:
				DO_VSA ( QH323ConnectTime, "h323-connect-time" );
			case qH323DisconnectTime:
				DO_VSA ( QH323DisconnectTime, "h323-disconnect-time" );
			case qH323DisconnectCause:
				DO_VSA ( QH323DisconnectCause, "h323-disconnect-cause" );
			case qH323VoiceQuality:
				DO_VSA ( QH323VoiceQuality, "h323-voice-quality" );
			case qH323GWId:
				DO_VSA ( QH323GWId, "h323-gw-id" );
		}
		ss :: ostringstream os;
		os << "quintum " << vt;
		return new AttributeQuintum ( os.str ( ), vt, val );
	}
	if ( vid == vXedia ) {
		switch ( XediaAttributeType ( vt ) ) {
			case xXediaDNSServer:
				return new XediaDNSServer ( ival );
			case xXediaAddressPool:
				return new XediaAddressPool ( val );
		}
		ss :: ostringstream os;
		os << "xedia " << vt;
		return new AttributeVendorString ( os.str ( ), vXedia, vt, val );
	}
	if ( vid == vAscend ) {
		switch ( AscendAttributeType ( vt ) ) {
			case aAscendClientPrimaryDNS:
				return new AscendClientPrimaryDNS ( ival );
			case aAscendClientSecondaryDNS:
				return new AscendClientSecondaryDNS ( ival );
		}
		ss :: ostringstream os;
		os << "ascend " << vt;
		return new AttributeVendorString ( os.str ( ), vAscend, vt, val );
	}
	if ( vid != vCisco ) {
		ss :: ostringstream os;
		os << "vendor " << vid << ':' << vt;
		return new AttributeVendorString ( os.str ( ), vid, vt, val );
	}
	switch ( CiscoAttributeType ( vt ) ) {
		case cH323Ivr:
			TRY_VSA ( H323IncomingConfId, "h323-incoming-conf-id" );
			TRY_VSA ( H323IvrIn, "h323-ivr-in" );
			TRY_VSA ( H323RemoteId, "h323-remote-id" );
			DO_VSA ( H323IvrOut, "h323-ivr-out" );
		case cCiscoNASPort:
			return new CiscoNASPort ( val );
		case cH323RemoteAddress:
			DO_VSA ( H323RemoteAddress, "h323-remote-address" );
		case cH323ConfId:
			DO_VSA ( H323ConfId, "h323-conf-id" );
		case cH323SetupTime:
			DO_VSA ( H323SetupTime, "h323-setup-time" );
		case cH323CallOrigin:
			DO_VSA ( H323CallOrigin, "h323-call-origin" );
		case cH323CallType:
			DO_VSA ( H323CallType, "h323-call-type" );
		case cH323ConnectTime:
			DO_VSA ( H323ConnectTime, "h323-connect-time" );
		case cH323DisconnectTime:
			DO_VSA ( H323DisconnectTime, "h323-disconnect-time" );
		case cH323DisconnectCause:
			DO_VSA ( H323DisconnectCause, "h323-disconnect-cause" );
		case cH323VoiceQuality:
			DO_VSA ( H323VoiceQuality, "h323-voice-quality" );
		case cH323GWId:
			DO_VSA ( H323GWId, "h323-gw-id" );
		case cH323CreditAmount:
			DO_VSA ( H323CreditAmount, "h323-credit-amount" );
		case cH323CreditTime:
			DO_VSA ( H323CreditTime, "h323-credit-time" );
		case cH323ReturnCode:
			DO_VSA ( H323ReturnCode, "h323-return-code" );
		case cH323PreferredLang:
			DO_VSA ( H323PreferredLang, "h323-preferred-lang" );
		case cH323BillingModel:
			DO_VSA ( H323BillingModel, "h323-billing-model" );
		case cCiscoAssignIPPool:
			return new CiscoAssignIPPool ( ival );
	}
#undef DO_VSA
#undef TRY_VSA
	ss :: ostringstream os;
	os << "cisco " << vt;
	return new AttributeCisco ( os.str ( ), vt, val );
}

AttributeVendor :: AttributeVendor ( const ss :: string & n, int id, int type ) :
	Attribute ( n, aVendor ), vendorId ( id ), vendorType ( type ) { }

int AttributeVendor :: printVal ( char * buf, int size ) const {
	size -= 5;
	if ( size < 0 )
		return 0;
	buf [ 0 ] = char ( vendorId >> 24 );
	buf [ 1 ] = char ( vendorId >> 16 );
	buf [ 2 ] = char ( vendorId >> 8 );
	buf [ 3 ] = char ( vendorId );
	buf [ 4 ] = char ( vendorType );
	int r = printVendorVal ( buf + 5, size );
	if ( r > 0 )
		return 5 + r;
	return r;
}

void AttributeVendor :: printVal ( std :: ostream & os ) const {
	os << vendorId << ':' << vendorType << ':';
	printVendorVal ( os );
}

int AttributeVendor :: getVendor ( ) const {
	return vendorId;
}

int AttributeVendor :: getType ( ) const {
	return vendorType;
}

AttributeVendorString :: AttributeVendorString ( const ss :: string & n, int id, int type, const ss :: string & val ) :
	AttributeVendor ( n, id, type ), value ( val ) { }

int AttributeVendorString :: printVendorVal ( char * buf, int size ) const {
	ss :: string :: size_type sz = value.size ( );
	if ( ss :: string :: size_type ( size ) <= sz || sz > 253 )
		return 0;
	buf [ 0 ] = char ( sz + 2 );
	std :: memcpy ( buf + 1, value.data ( ), sz );
	return int ( sz + 1 );
}

void AttributeVendorString :: printVendorVal ( std :: ostream & os ) const {
	os << value;
}

const ss :: string & AttributeVendorString :: getVal ( ) const {
	return value;
}

AttributeVendorInt :: AttributeVendorInt ( const ss :: string & n, int id, int type, int val ) :
	AttributeVendor ( n, id, type ), value ( val ) {
}

int AttributeVendorInt :: printVendorVal ( char * buf, int size ) const {
	if ( size < 5 )
		return 0;
	buf [ 0 ] = 6;
	buf [ 1 ] = char ( value >> 24 );
	buf [ 2 ] = char ( value >> 16 );
	buf [ 3 ] = char ( value >> 8 );
	buf [ 4 ] = char ( value );
	return 5;
}

void AttributeVendorInt :: printVendorVal ( std :: ostream & os ) const {
	os << value;
}

int AttributeVendorInt :: getVal ( ) const {
	return value;
}

AttributeVendorAddr :: AttributeVendorAddr ( const ss :: string & n, int id, int type, int v ) :
	AttributeVendorInt ( n, id, type, v ) { }

AttributeVendorAddr :: AttributeVendorAddr ( const ss :: string & n, int id, int type, const ss :: string & v ) :
	AttributeVendorInt ( n, id, type, ntohl ( inetAddr ( v ) ) ) { }

void AttributeVendorAddr :: printVendorVal ( std :: ostream & os ) const {
	struct in_addr sa;
	sa.s_addr = htonl ( getVal ( ) );
	char buf [ INET_ADDRSTRLEN ];
	os << inet_ntop ( AF_INET, & sa, buf, INET_ADDRSTRLEN );
}

CiscoNASPort :: CiscoNASPort ( const ss :: string & val ) :
	AttributeVendorString ( "CiscoNASPort", vCisco, cCiscoNASPort, val ) { }

QuintumNASPort :: QuintumNASPort ( const ss :: string & val ) :
	AttributeVendorString ( "QuintumNASPort", vQuintum, qCiscoNASPort, val ) { }

CiscoAssignIPPool :: CiscoAssignIPPool ( int val ) :
	AttributeVendorAddr ( "CiscoAssignIPPool", vCisco, cCiscoAssignIPPool, val ) { }

CiscoAssignIPPool :: CiscoAssignIPPool ( const ss :: string & val ) :
	AttributeVendorAddr ( "CiscoAssignIPPool", vCisco, cCiscoAssignIPPool, val ) { }

AttributeCisco :: AttributeCisco ( const ss :: string & n, int type, const ss :: string & val ) :
	AttributeVendorString ( n, vCisco, type, ( n + '=' ) += val ) { }

static ss :: string intToString ( int n ) {
	ss :: ostringstream os;
	os << n;
	return os.str ( );
}

AttributeCisco :: AttributeCisco ( const ss :: string & n, int type, int val ) :
	AttributeVendorString ( n, vCisco, type, ( n + '=' ) += intToString ( val ) ) { }

H323IvrIn :: H323IvrIn ( const ss :: string & val ) : AttributeCisco ( "h323-ivr-in", cH323Ivr, val ) { }

H323IvrOut :: H323IvrOut ( const ss :: string & val ) : AttributeCisco ( "h323-ivr-out", cH323Ivr, val ) { }

H323IncomingConfId :: H323IncomingConfId ( const ss :: string & val ) :
	AttributeCisco ( "h323-incoming-conf-id", cH323IncomingConfId, val ) { }

H323RemoteId :: H323RemoteId ( const ss :: string & val ) : AttributeCisco ( "h323-remote-id", cH323RemoteId, val ) { }

H323RemoteAddress :: H323RemoteAddress ( const ss :: string & val ) :
	AttributeCisco ( "h323-remote-address", cH323RemoteAddress, val ) { }

H323ConfId :: H323ConfId ( const ss :: string & val ) : AttributeCisco ( "h323-conf-id", cH323ConfId, val ) { }

H323SetupTime :: H323SetupTime ( const ss :: string & val ) : AttributeCisco ( "h323-setup-time", cH323SetupTime, val ) { }

H323CallOrigin :: H323CallOrigin ( const ss :: string & val ) : AttributeCisco ( "h323-call-origin", cH323CallOrigin, val ) { }

H323CallType :: H323CallType ( const ss :: string & val ) : AttributeCisco ( "h323-call-type", cH323CallType, val ) { }

H323ConnectTime :: H323ConnectTime ( const ss :: string & val ) :
	AttributeCisco ( "h323-connect-time", cH323ConnectTime, val ) { }

H323DisconnectTime :: H323DisconnectTime ( const ss :: string & val ) :
	AttributeCisco ( "h323-disconnect-time", cH323DisconnectTime, val ) { }

H323DisconnectCause :: H323DisconnectCause ( const ss :: string & val ) :
	AttributeCisco ( "h323-disconnect-cause", cH323DisconnectCause, val ) { }

H323VoiceQuality :: H323VoiceQuality ( const ss :: string & val ) :
	AttributeCisco ( "h323-voice-quality", cH323VoiceQuality, val ) { }

H323GWId :: H323GWId ( const ss :: string & val ) : AttributeCisco ( "h323-gw-id", cH323GWId, val ) { }

H323CreditAmount :: H323CreditAmount ( const ss :: string & val ) :
	AttributeCisco ( "h323-credit-amount", cH323CreditAmount, val ) { }

H323CreditTime :: H323CreditTime ( const ss :: string & val ) :
	AttributeCisco ( "h323-credit-time", cH323CreditTime, val ) { }

H323ReturnCode :: H323ReturnCode ( const ss :: string & val ) : AttributeCisco ( "h323-return-code", cH323ReturnCode, val ) { }

H323PreferredLang :: H323PreferredLang ( const ss :: string & val ) :
	AttributeCisco ( "h323-preferred-lang", cH323PreferredLang, val ) { }

H323BillingModel :: H323BillingModel ( const ss :: string & val ) :
	AttributeCisco ( "h323-billing-model", cH323BillingModel, val ) { }

AttributeQuintum :: AttributeQuintum ( const ss :: string & n, int type, const ss :: string & val ) :
	AttributeVendorString ( n, vQuintum, type, ( n + '=' ) += val ) { }

QH323IvrOut :: QH323IvrOut ( const ss :: string & val ) : AttributeQuintum ( "h323-ivr-out", qH323Ivr, val ) { }

QH323IncomingConfId :: QH323IncomingConfId ( const ss :: string & val ) :
	AttributeQuintum ( "h323-incoming-conf-id", qH323IncomingConfId, val ) { }

QH323RemoteAddress :: QH323RemoteAddress ( const ss :: string & val ) :
	AttributeQuintum ( "h323-remote-address", qH323RemoteAddress, val ) { }

QH323ConfId :: QH323ConfId ( const ss :: string & val ) : AttributeQuintum ( "h323-conf-id", qH323ConfId, val ) { }

QH323SetupTime :: QH323SetupTime ( const ss :: string & val ) : AttributeQuintum ( "h323-setup-time", qH323SetupTime, val ) { }

QH323CallOrigin :: QH323CallOrigin ( const ss :: string & val ) :
	AttributeQuintum ( "h323-call-origin", qH323CallOrigin, val ) { }

QH323CallType :: QH323CallType ( const ss :: string & val ) : AttributeQuintum ( "h323-call-type", qH323CallType, val ) { }

QH323ConnectTime :: QH323ConnectTime ( const ss :: string & val ) :
	AttributeQuintum ( "h323-connect-time", qH323ConnectTime, val ) { }

QH323DisconnectTime :: QH323DisconnectTime ( const ss :: string & val ) :
	AttributeQuintum ( "h323-disconnect-time", qH323DisconnectTime, val ) { }

QH323DisconnectCause :: QH323DisconnectCause ( const ss :: string & val ) :
	AttributeQuintum ( "h323-disconnect-cause", qH323DisconnectCause, val ) { }

QH323VoiceQuality :: QH323VoiceQuality ( const ss :: string & val ) :
	AttributeQuintum ( "h323-voice-quality", qH323VoiceQuality, val ) { }

QH323GWId :: QH323GWId ( const ss :: string & val ) : AttributeQuintum ( "h323-gw-id", qH323GWId, val ) { }

XediaDNSServer :: XediaDNSServer ( int val ) : AttributeVendorAddr ( "XediaDNSServer", vXedia, xXediaDNSServer, val ) { }

XediaDNSServer :: XediaDNSServer ( const ss :: string & val ) :
	AttributeVendorAddr ( "XediaDNSServer", vXedia, xXediaDNSServer, val ) { }

XediaAddressPool :: XediaAddressPool ( const ss :: string & val ) :
	AttributeVendorString ( "XediaAddressPool", vXedia, xXediaAddressPool, val ) { }

AscendClientPrimaryDNS :: AscendClientPrimaryDNS ( int val ) :
	AttributeVendorAddr ( "AscendClientPrimaryDNS", vAscend, aAscendClientPrimaryDNS, val ) { }

AscendClientPrimaryDNS :: AscendClientPrimaryDNS ( const ss :: string & val ) :
	AttributeVendorAddr ( "AscendClientPrimaryDNS", vAscend, aAscendClientPrimaryDNS, val ) { }

AscendClientSecondaryDNS :: AscendClientSecondaryDNS ( int val ) :
	AttributeVendorAddr ( "AscendClientSecondaryDNS", vAscend, aAscendClientSecondaryDNS, val ) { }

AscendClientSecondaryDNS :: AscendClientSecondaryDNS ( const ss :: string & val ) :
	AttributeVendorAddr ( "AscendClientSecondaryDNS", vAscend, aAscendClientSecondaryDNS, val ) { }

Request :: Request ( const ss :: string & s, PacketType c, const ss :: string & ip, int port ) :
	secret ( s ), ip ( ip ), port ( port ), code ( c ) {
	static unsigned char aid = 0;
	id = __sync_fetch_and_add ( & aid, 1 );
}

Request :: Request ( const ss :: string & s, const ss :: string & aip, int p, unsigned char * buf, int l ) :
	secret ( s ), ip ( aip ), port ( p ) {
	struct Header {
		unsigned char code;
		unsigned char id;
		unsigned short len;
		unsigned char vec [ vecLen ];
		unsigned char data [ ];
	};
	if ( l < int ( sizeof ( Header ) ) )
		throw std :: runtime_error ( "request too short" );
	Header * h = reinterpret_cast < Header * > ( buf );
	int tl = ntohs ( h -> len );
	if ( l < tl )
		throw std :: runtime_error ( "request too short" );
	if ( l > tl )
		l = tl;
	id = h -> id;
	code = h -> code;
	std :: memcpy ( vec, h -> vec, vecLen );
	if ( code == rAccessRequest ) {
		MD5 ctx;
		ctx.processBytes ( secret.data ( ), secret.size ( ) );
		ctx.processBytes ( vec, vecLen );
		ss :: string t = ctx.finish ( );
		std :: memcpy ( digest, t.data ( ), 16 );
	}
	if ( code == rAccountingRequest ) {
		std :: memset ( h -> vec, 0, vecLen );
		MD5 ctx;
		ctx.processBytes ( buf, l );
		ctx.processBytes ( secret.data ( ), secret.size ( ) );
		ss :: string t = ctx.finish ( );
		std :: memcpy ( digest, t.data ( ), 16 );
	}
	int maxl = int ( l - sizeof ( Header ) );
	buf += sizeof ( Header );
	while ( maxl > 0 ) {
		Attribute * a = Attribute :: load ( buf, maxl, l );
		if ( ! a )
			throw std :: runtime_error ( "cant decode attributes" );
		append ( a );
		buf += l;
		maxl -= l;
	}
	if ( maxl ) {
		std :: ostringstream os;
		os << "maxl: " << maxl;
		throw std :: runtime_error ( os.str ( ) );
	}
}

Request :: Request ( const Request & r, PacketType c ) : secret ( r.secret ), ip ( r.ip ), port ( r.port ),
	id ( r.id ), code ( c ) {
	std :: memcpy ( vec, r.vec, vecLen );
	std :: memcpy ( digest, r.digest, vecLen );
}

void Request :: append ( Pointer < Attribute > a ) {
	attributes.push_back ( a );
}

int Request :: print ( char * buf, int size ) {
	size -= vecLen + 4;
	if ( size < 0 )
		return 0;
	char * ptr = buf;
	* ptr ++ = char ( code );
	* ptr ++ = char ( id );
	ptr += 2;
	if ( code == rAccountingRequest )
		std :: memset ( ptr, 0, vecLen );
	else
		std :: memcpy ( ptr, vec, vecLen );
	ptr += vecLen;
	for ( AttributesList :: const_iterator i = attributes.begin ( ); i != attributes.end ( ); i ++ ) {
		int l = ( * i ) -> print ( ptr, size );
		if ( l <= 0 )
			break;
		ptr += l;
		size -= l;
	}
	int l = int ( ptr - buf );
	buf [ 2 ] = char ( l / 256 );
	buf [ 3 ] = char ( l & 255 );
	MD5 ctx;
	ctx.processBytes ( buf, l );
	ctx.processBytes ( secret.data ( ), secret.size ( ) );
	ss :: string t = ctx.finish ( );
	std :: memcpy ( buf + 4, t.data ( ), vecLen );
	std :: memcpy ( digest, buf + 4, vecLen );
	return l;
}

static void printCode ( std :: ostream & os, PacketType code ) {
	switch ( code ) {
		case rAccessRequest:
			os << "AccessRequest";
			return;
		case rAccessAccept:
			os << "AccessAccept";
			return;
		case rAccessReject:
			os << "AccessReject";
			return;
		case rAccountingRequest:
			os << "AccountingRequest";
			return;
		case rAccountingResponse:
			os << "AccountingResponse";
			return;
		case rAccountingStatus:
			os << "AccountingStatus";
			return;
		case rPasswordRequest:
			os << "PasswordRequest";
			return;
		case rPasswordAck:
			os << "PasswordAck";
			return;
		case rPasswordReject:
			os << "PasswordReject";
			return;
		case rAccountingMessage:
			os << "AccountingMessage";
			return;
		case rAccessChallenge:
			os << "AccessChallenge";
			return;
		case rStatusServer:
			os << "StatusServer";
			return;
		case rStatusClient:
			os << "StatusClient";
			return;
		case rResourceFreeRequest:
			os << "ResourceFreeRequest";
			return;
		case rResourceFreeResponse:
			os << "ResourceFreeResponse";
			return;
		case rResourceQueryRequest:
			os << "ResourceQueryRequest";
			return;
		case rResourceQueryResponse:
			os << "ResourceQueryResponse";
			return;
		case rAlternateResourceReclaimRequest:
			os << "AlternateResourceReclaimRequest";
			return;
		case rNASRebootRequest:
			os << "NASRebootRequest";
			return;
		case rNASRebootResponse:
			os << "NASRebootResponse";
			return;
		case rNextPasscode:
			os << "NextPasscode";
			return;
		case rNewPin:
			os << "NewPin";
			return;
		case rTerminateSession:
			os << "TerminateSession";
			return;
		case rPasswordExpired:
			os << "PasswordExpired";
			return;
		case rEventRequest:
			os << "EventRequest";
			return;
		case rEventResponse:
			os << "EventResponse";
			return;
		case rDisconnectRequest:
			os << "DisconnectRequest";
			return;
		case rDisconnectACK:
			os << "DisconnectACK";
			return;
		case rDisconnectNAK:
			os << "DisconnectNAK";
			return;
		case rCoARequest:
			os << "CoARequest";
			return;
		case rCoAACK:
			os << "CoAACK";
			return;
		case rCoANAK:
			os << "CoANAK";
			return;
		case rIPAddressAllocate:
			os << "IPAddressAllocate";
			return;
		case rIPAddressRelease:
			os << "IPAddressRelease";
			return;
		case rUnassigned:
		case rExperimentalUse:
		case rReserved:
			break;
	}
	os << code;
}

void Request :: print ( std :: ostream & os ) const {
	os << "id: " << id << "\ncode: ";
	printCode ( os, PacketType ( code ) );
	boost :: io :: ios_flags_saver fls ( os );
	boost :: io :: ios_fill_saver fis ( os );
	os << "\nip: " << ip << "\nport: " << port << "\nauthenticator: " << std :: hex << std :: setfill ( '0' );
	for ( int i = 0; i < vecLen; i ++ )
		os << std :: setw ( 2 ) << unsigned ( vec [ i ] );
	os << "\ndigest: ";
	for ( int i = 0; i < vecLen; i ++ )
		os << std :: setw ( 2 ) << unsigned ( digest [ i ] );
	os << std :: endl;
	fis.restore ( );
	fls.restore ( );
	for ( AttributesList :: const_iterator i = attributes.begin ( ); i != attributes.end ( ); i ++ ) {
		( * i ) -> print ( os );
		os << "\n";
	}
}

const Attribute * Request :: find ( int a ) const {
	for ( AttributesList :: const_iterator i = attributes.begin ( ); i != attributes.end ( ); i ++ ) {
		if ( ( * i ) -> getAttr ( ) == a )
			return * i;
	}
	return 0;
}

const AttributeString * Request :: findString ( int a, int num ) const {
	for ( AttributesList :: const_iterator i = attributes.begin ( ); i != attributes.end ( ); i ++ ) {
		if ( ( * i ) -> getAttr ( ) == a && ( num -- == 0 ) )
			return dynamic_cast < const AttributeString * > ( static_cast < const Attribute * > ( * i ) );
	}
	return 0;
}

const AttributeInt * Request :: findInt ( int a ) const {
	for ( AttributesList :: const_iterator i = attributes.begin ( ); i != attributes.end ( ); i ++ ) {
		if ( ( * i ) -> getAttr ( ) == a )
			return dynamic_cast < const AttributeInt * > ( static_cast < const Attribute * > ( * i ) );
	}
	return 0;
}

const AttributeCisco * Request :: findCisco ( int a, const char * n, int skip ) const {
	for ( AttributesList :: const_iterator i = attributes.begin ( ); i != attributes.end ( ); i ++ ) {
		if ( ( * i ) -> getAttr ( ) != aVendor )
			continue;
		const AttributeCisco * v = dynamic_cast < const AttributeCisco * > ( static_cast < const Attribute * > ( * i ) );
		if ( ! v || v -> getVendor ( ) != vCisco || v -> getType ( ) != a )
			continue;
		if ( hasPrefix ( v -> getVal ( ), n ) && skip -- == 0 )
			return v;
	}
	return 0;
}

const AttributeQuintum * Request :: findQuintum ( int a, const char * n ) const {
	for ( AttributesList :: const_iterator i = attributes.begin ( ); i != attributes.end ( ); i ++ ) {
		if ( ( * i ) -> getAttr ( ) != aVendor )
			continue;
		const AttributeQuintum * v = dynamic_cast < const AttributeQuintum * > ( static_cast < const Attribute * > ( * i ) );
		if ( ! v || v -> getVendor ( ) != vQuintum || v -> getType ( ) != a )
			continue;
		if ( hasPrefix ( v -> getVal ( ), n ) )
			return v;
	}
	return 0;
}

const AttributeVendorString * Request :: findVendorString ( int ve, int a ) const {
	for ( AttributesList :: const_iterator i = attributes.begin ( ); i != attributes.end ( ); i ++ ) {
		if ( ( * i ) -> getAttr ( ) != aVendor )
			continue;
		const AttributeVendorString * v = dynamic_cast < const AttributeVendorString * > ( static_cast < const Attribute * > ( * i ) );
		if ( v && v -> getVendor ( ) == ve && v -> getType ( ) == a )
			return v;
	}
	return 0;
}

const ss :: string * Request :: findVendorString ( int a, const char * n ) const {
	if ( const AttributeCisco * c = findCisco ( a, n ) )
		return & c -> getVal ( );
	if ( const AttributeQuintum * q = findQuintum ( a, n ) )
		return & q -> getVal ( );
	return 0;
}

bool Request :: getVendorString ( int a, const char * n, ss :: string & s ) const {
	if ( const ss :: string * t = findVendorString ( a, n ) ) {
		s.assign ( * t, std :: strlen ( n ), ss :: string :: npos );
		return true;
	}
	s.clear ( );
	return false;
}

ss :: string Request :: getUserPassword ( ) const {
	if ( const AttributeString * a = findString ( aUserPassword ) ) {
		const ss :: string & p = a -> getVal ( );
		ss :: string t ( p.size ( ), ' ' );
		for ( unsigned i = 0; i < p.size ( ); i ++ )
			t [ i ] = char ( p [ i ] ^ digest [ i % vecLen ] );
		return t;
	}
	return ss :: string ( );
}

const ss :: string & Request :: getIp ( ) const {
	return ip;
}

int Request :: getPort ( ) const {
	return port;
}

bool Request :: accSecretVerified ( ) const {
	return ! memcmp ( vec, digest, vecLen );
}

ss :: string Request :: getCodeName ( ) const {
	switch ( PacketType ( code ) ) {
		case rAccessRequest:
			return "AccessRequest";
		case rAccessAccept:
			return "AccessAccept";
		case rAccessReject:
			return "AccessReject";
		case rAccountingRequest:
			return "AccountingRequest";
		case rAccountingResponse:
			return "AccountingResponse";
		case rAccountingStatus:
			return "AccountingStatus";
		case rPasswordRequest:
			return "PasswordRequest";
		case rPasswordAck:
			return "PasswordAck";
		case rPasswordReject:
			return "PasswordReject";
		case rAccountingMessage:
			return "AccountingMessage";
		case rAccessChallenge:
			return "AccessChallenge";
		case rStatusServer:
			return "StatusServer";
		case rStatusClient:
			return "StatusClient";
		case rResourceFreeRequest:
			return "ResourceFreeRequest";
		case rResourceFreeResponse:
			return "ResourceFreeResponse";
		case rResourceQueryRequest:
			return "ResourceQueryRequest";
		case rResourceQueryResponse:
			return "ResourceQueryResponse";
		case rAlternateResourceReclaimRequest:
			return "AlternateResourceReclaimRequest";
		case rNASRebootRequest:
			return "NASRebootRequest";
		case rNASRebootResponse:
			return "NASRebootResponse";
		case rNextPasscode:
			return "NextPasscode";
		case rNewPin:
			return "NewPin";
		case rTerminateSession:
			return "TerminateSession";
		case rPasswordExpired:
			return "PasswordExpired";
		case rEventRequest:
			return "EventRequest";
		case rEventResponse:
			return "EventResponse";
		case rDisconnectRequest:
			return "DisconnectRequest";
		case rDisconnectACK:
			return "DisconnectACK";
		case rDisconnectNAK:
			return "DisconnectNAK";
		case rCoARequest:
			return "CoARequest";
		case rCoAACK:
			return "CoAACK";
		case rCoANAK:
			return "CoANAK";
		case rIPAddressAllocate:
			return "IPAddressAllocate";
		case rIPAddressRelease:
			return "IPAddressRelease";
		case rUnassigned:
		case rExperimentalUse:
		case rReserved:
			break;
	}
	ss :: ostringstream os;
	os << code;
	return os.str ( );
}

int Request :: getCode ( ) const {
	return code;
}

}
