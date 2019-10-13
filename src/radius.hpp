#ifndef __RADIUS_HPP
#define __RADIUS_HPP
#pragma interface

namespace Radius {

enum AttributeType {
	aUserName = 1,
	aUserPassword = 2,
	aCHAPPassword = 3,
	aNASIPAddress = 4,
	aNASPort = 5,
	aServiceType = 6,
	aFramedProtocol = 7,
	aFramedIPAddress = 8,
	aFramedIPNetmask = 9,
	aFramedRouting = 10,
	aFilterId = 11,
	aFramedMTU = 12,
	aFramedCompression = 13,
	aLoginIPHost = 14,
	aLoginService = 15,
	aLoginTCPPort = 16,
	aReplyMessage = 18,
	aCallbackNumber = 19,
	aCallbackId =  20,
	aFramedRoute = 22,
	aFramedIPXNetwork = 23,
	aState = 24,
	aClass = 25,
	aVendor = 26,
	aSessionTimeout = 27,
	aIdleTimeout = 28,
	aTerminationAction = 29,
	aCalledStationId = 30,
	aCallingStationId = 31,
	aNASIdentifier = 32,
	aProxyState = 33,
	aLoginLATService = 34,
	aLoginLATNode = 35,
	aLoginLATGroup = 36,
	aFramedAppletTalkLink = 37,
	aFramedAppleTalkNetwork = 38,
	aFramedAppleTalkZone = 39,
	aAcctStatusType = 40,
	aAcctDelayTime = 41,
	aAcctInputOctets = 42,
	aAcctOutputOctets = 43,
	aAcctSessionId = 44,
	aAcctAuthentic = 45,
	aAcctSessionTime = 46,
	aAcctInputPackets = 47,
	aAcctOutputPackets = 48,
	aAcctTerminateCause = 49,
	aAcctMultiSessionId = 50,
	aAcctLinkCount = 51,
	aAcctInputGigawords = 52,
	aAcctOutputGigawords = 53,
	aEventTimestamp = 55,
	aEgressVLANID = 56,
	aIngressFilters = 57,
	aEgressVLANName = 58,
	aUserPriorityTable = 59,
	aCHAPChallenge = 60,
	aNASPortType = 61,
	aPortLimit = 62,
	aLoginLATPort = 63,
	aTunnelType = 64,
	aTunnelMediumType = 65,
	aTunnelClientEndpoint = 66,
	aTunnelServerEndpoint = 67,
	aAcctTunnelConnection = 68,
	aTunnelPassword = 69,
	aARAPPassword = 70,
	aARAPFeatures = 71,
	aARAPZoneAccess = 72,
	aARAPSecurity = 73,
	aARAPSecurityData = 74,
	aPasswordRetry = 75,
	aPrompt = 76,
	aConnectInfo = 77,
	aConfigurationToken = 78,
	aEAPMessage = 79,
	aMessageAuthenticator = 80,
	aTunnelPrivateGroupID = 81,
	aTunnelAssignmentID = 82,
	aTunnelPreference = 83,
	aARAPChallengeResponse = 84,
	aAcctInterimInterval = 85,
	aAcctTunnelPacketsLost = 86,
	aNASPortId = 87,
	aFramedPool = 88,
	aCUI = 89,
	aTunnelClientAuthID = 90,
	aTunnelServerAuthID = 91,
	aNASFilterRule = 92,
	aOriginatingLineInfo = 94,
	aNASIPv6Address = 95,
	aFramedInterfaceId = 96,
	aFramedIPv6Prefix = 97,
	aLoginIPv6Host = 98,
	aFramedIPv6Route = 99,
	aFramedIPv6Pool = 100,
	aErrorCause = 101,
	aEAPKeyName = 102,
	aDigestResponse = 103,
	aDigestRealm = 104,
	aDigestNonce = 105,
	aDigestResponseAuth = 106,
	aDigestNextnonce = 107,
	aDigestMethod = 108,
	aDigestURI = 109,
	aDigestQop = 110,
	aDigestAlgorithm = 111,
	aDigestEntityBodyHash = 112,
	aDigestCNonce = 113,
	aDigestNonceCount = 114,
	aDigestUsername = 115,
	aDigestOpaque = 116,
	aDigestAuthParam = 117,
	aDigestAKAAuts = 118,
	aDigestDomain = 119,
	aDigestStale = 120,
	aDigestHA1 = 121,
	aSIPAOR = 122,
	aDelegatedIPv6Prefix = 123,
	aMIP6FeatureVector = 124,
	aMIP6HomeLinkPrefix = 125,
	aUnassigned = 126,
	aExperimentalUse = 192,
	aImplementationSpecific = 224,
	aModifiedANumber = 224,
	aReserved = 241
};

enum VendorId {
	vCisco = 9,
	vAscend = 529,
	vXedia = 838,
	vQuintum = 6618
};

enum CiscoAttributeType {
	cH323Ivr = 1,
	cH323IncomingConfId = 1,
	cH323RemoteId = 1,
	cCiscoNASPort = 2,
	cH323RemoteAddress = 23,
	cH323ConfId = 24,
	cH323SetupTime = 25,
	cH323CallOrigin = 26,
	cH323CallType = 27,
	cH323ConnectTime = 28,
	cH323DisconnectTime = 29,
	cH323DisconnectCause = 30,
	cH323VoiceQuality = 31,
	cH323GWId = 33,
	cH323CreditAmount = 101,
	cH323CreditTime = 102,
	cH323ReturnCode = 103,
	cH323PreferredLang = 107,
	cH323BillingModel = 109,
	cCiscoAssignIPPool = 218
};

enum QuintumAttributeType {
	qH323Ivr = cH323Ivr,
	qCiscoNASPort = cCiscoNASPort,
	qH323IncomingConfId = cH323IncomingConfId,
	qH323RemoteAddress = cH323RemoteAddress,
	qH323ConfId = cH323ConfId,
	qH323SetupTime = cH323SetupTime,
	qH323CallOrigin = cH323CallOrigin,
	qH323CallType = cH323CallType,
	qH323ConnectTime = cH323ConnectTime,
	qH323DisconnectTime = cH323DisconnectTime,
	qH323DisconnectCause = cH323DisconnectCause,
	qH323VoiceQuality = cH323VoiceQuality,
	qH323GWId = cH323GWId
};

enum XediaAttributeType {
	xXediaDNSServer = 1,
	xXediaAddressPool = 3
};

enum AscendAttributeType {
	aAscendClientPrimaryDNS = 135,
	aAscendClientSecondaryDNS = 136
};

enum ServiceTypeValue {
	sLogin = 1,
	sFramed = 2,
	sCallbackLogin = 3,
	sCallbackFramed = 4,
	sOutbound = 5,
	sAdministrative = 6,
	sNASPrompt = 7,
	sAuthenticateOnly = 8,
	sCallbackNASPrompt = 9,
	sCallCheck = 10,
	sCallbackAdministrative = 11,
	sVoice = 12,
	sFax = 13,
	sModemRelay = 14,
	sIAPPRegister = 15,
	sIAPPAPCheck = 16,
	sAuthorizeOnly = 17
};

enum FramedProtocolValue {
	fPPP = 1,
	fSLIP = 2,
	fARAP = 3,
	fGandalf = 4,
	fXylogics = 5,
	fX75Synchronous = 6,
	fGPRSPDPContext = 7
};

enum FramedCompressionValue {
	fcNone = 0,
	fcVJTCPIP = 1,
	fcIPX = 2,
	fcStacLZS = 3
};

enum AcctStatusTypeValue {
	sStart = 1,
	sStop = 2,
	sInterimUpdate = 3,
	sAccountingOn = 7,
	sAccountingOff = 8,
	sTunnelStart = 9,
	sTunnelStop = 10,
	sTunnelReject = 11,
	sTunnelLinkStart = 12,
	sTunnelLinkStop = 13,
	sTunnelLinkReject = 14,
	sFailed = 15
};

enum AcctAuthenticValue {
	aaRADIUS = 1,
	aaLocal = 2,
	aaRemote = 3,
	aaDiameter = 4
};

enum AcctTerminateCauseValue {
	atUserRequest = 1,
	atLostCarrier = 2,
	atLostService = 3,
	atIdleTimeout = 4,
	atSessionTimeout = 5,
	atAdminReset = 6,
	atAdminReboot = 7,
	atPortError = 8,
	atNASError = 9,
	atNASRequest = 10,
	atNASReboot = 11,
	atPortUnneeded = 12,
	atPortPreempted = 13,
	atPortSuspended = 14,
	atServiceUnavailable = 15,
	atCallback = 16,
	atUserError = 17,
	atHostRequest = 18,
	atSupplicantRestart = 19,
	atReauthenticationFailure = 20,
	atPortReinitialized = 21,
	atPortAdministrativelyDisabled = 22
};

enum NASPortTypeValue {
	npAsync = 0,
	npSync = 1,
	npISDNSync = 2,
	npISDNAsyncV120 = 3,
	npISDNAsyncV110 = 4,
	npVirtual = 5,
	npPIAFS = 6,
	npHDLCClearChannel = 7,
	npX25 = 8,
	npX75 = 9,
	npG3Fax = 10,
	npSDSL = 11,
	npADSLCAP = 12,
	npADSLDMT = 13,
	npIDSL = 14,
	npEthernet = 15,
	npxDSL = 16,
	npCable = 17,
	npWirelessOther = 18,
	npIEEE80211 = 19,
	npTokenRing = 20,
	npFDDI = 21,
	npCDMA2000 = 22,
	npUMTS = 23,
	np1XEV = 24,
	npIAPP = 25,
	npFTTP = 26,
	npIEEE80216 = 27,
	npIEEE80220 = 28,
	npIEEE80222 = 29,
	npPPPoA = 30,
	npPPPoEoA = 31,
	npPPPoEoE = 32,
	npPPPoEoVLAN = 33,
	npPPPoEoQinQ = 34,
	npxPON = 35
};

enum PacketType {
	rAccessRequest = 1,
	rAccessAccept = 2,
	rAccessReject = 3,
	rAccountingRequest = 4,
	rAccountingResponse = 5,
	rAccountingStatus = 6,
	rPasswordRequest = 7,
	rPasswordAck = 8,
	rPasswordReject = 9,
	rAccountingMessage = 10,
	rAccessChallenge = 11,
	rStatusServer = 12,
	rStatusClient = 13,
	rResourceFreeRequest = 21,
	rResourceFreeResponse = 22,
	rResourceQueryRequest = 23,
	rResourceQueryResponse = 24,
	rAlternateResourceReclaimRequest = 25,
	rNASRebootRequest = 26,
	rNASRebootResponse = 27,
	rNextPasscode = 29,
	rNewPin = 30,
	rTerminateSession = 31,
	rPasswordExpired = 32,
	rEventRequest = 33,
	rEventResponse = 34,
	rDisconnectRequest = 40,
	rDisconnectACK = 41,
	rDisconnectNAK = 42,
	rCoARequest = 43,
	rCoAACK = 44,
	rCoANAK = 45,
	rIPAddressAllocate = 50,
	rIPAddressRelease = 51,
	rUnassigned = 52,
	rExperimentalUse = 250,
	rReserved = 254
};

class Attribute : public Allocatable < __SS_ALLOCATOR > {
	ss :: string name;
	int attribute;
	virtual int printVal ( char * buf, int size ) const = 0;
	virtual void printVal ( std :: ostream & os ) const = 0;
public:
	Attribute ( const ss :: string & n, int attr );
	static Attribute * load ( const unsigned char * buf, int maxl, int & l );
	int print ( char * buf, int size ) const;
	void print ( std :: ostream & os ) const;
	virtual ~Attribute ( );
	const ss :: string & getName ( ) const;
	int getAttr ( ) const;
};

class AttributeString : public Attribute {
	ss :: string value;
public:
	AttributeString ( const ss :: string & n, int attr, const ss :: string & val );
	int printVal ( char * buf, int size ) const;
	void printVal ( std :: ostream & os ) const;
	const ss :: string & getVal ( ) const;
};

class UserName : public AttributeString {
public:
	UserName ( const ss :: string & val );
};

class UserPassword : public AttributeString {
	using AttributeString :: printVal;
public:
	UserPassword ( const ss :: string & val );
	void printVal ( std :: ostream & os ) const;
};

class FilterId : public AttributeString {
public:
	FilterId ( const ss :: string & val );
};

class CalledStationId : public AttributeString {
public:
	CalledStationId ( const ss :: string & val );
};

class CallingStationId : public AttributeString {
public:
	CallingStationId ( const ss :: string & val );
};

class ModifiedANumber : public AttributeString {
public:
	ModifiedANumber ( const ss :: string & val );
};

class ProxyState : public AttributeString {
public:
	ProxyState ( const ss :: string & val );
};

class NASIdentifier : public AttributeString {
public:
	NASIdentifier ( const ss :: string & val );
};

class AcctSessionId : public AttributeString {
public:
	AcctSessionId ( const ss :: string & val );
};

class AcctMultiSessionId : public AttributeString {
public:
	AcctMultiSessionId ( const ss :: string & val );
};

class ConnectInfo : public AttributeString {
public:
	ConnectInfo ( const ss :: string & val );
};

class FramedPool : public AttributeString {
public:
	FramedPool ( const ss :: string & val );
};

class AttributeInt : public Attribute {
	int value;
public:
	AttributeInt ( const ss :: string & n, int attr, int val );
	int printVal ( char * buf, int size ) const;
	void printVal ( std :: ostream & os ) const;
	int getVal ( ) const;
};

class AttributeAddr : public AttributeInt {
	using AttributeInt :: printVal;
public:
	AttributeAddr ( const ss :: string & n, int attr, int val );
	AttributeAddr ( const ss :: string & n, int attr, const ss :: string & val );
	void printVal ( std :: ostream & os ) const;
};

class NASIPAddress : public AttributeInt {
public:
	NASIPAddress ( int val );
};

class NASPort : public AttributeInt {
public:
	NASPort ( int val );
};

class NASPortType : public AttributeInt {
	using AttributeInt :: printVal;
public:
	NASPortType ( int val );
	void printVal ( std :: ostream & os ) const;
};

class SessionTimeout : public AttributeInt {
public:
	SessionTimeout ( int val );
};

class AcctStatusType : public AttributeInt {
	using AttributeInt :: printVal;
public:
	AcctStatusType ( int val );
	void printVal ( std :: ostream & os ) const;
};

class AcctDelayTime : public AttributeInt {
public:
	AcctDelayTime ( int val );
};

class IdleTimeout : public AttributeInt {
public:
	IdleTimeout ( int val );
};

class FramedMTU : public AttributeInt {
public:
	FramedMTU ( int val );
};

class FramedCompression : public AttributeInt {
	using AttributeInt :: printVal;
public:
	FramedCompression ( int val );
	void printVal ( std :: ostream & os ) const;
};

class PortLimit : public AttributeInt {
public:
	PortLimit ( int val );
};

class ServiceType : public AttributeInt {
	using AttributeInt :: printVal;
public:
	ServiceType ( int val );
	void printVal ( std :: ostream & os ) const;
};


class FramedProtocol : public AttributeInt {
	using AttributeInt :: printVal;
public:
	FramedProtocol ( int val );
	void printVal ( std :: ostream & os ) const;
};

class FramedIPAddress : public AttributeAddr {
public:
	FramedIPAddress ( int val );
	FramedIPAddress ( const ss :: string & val );
};

class FramedIPNetmask : public AttributeAddr {
public:
	FramedIPNetmask ( int val );
	FramedIPNetmask ( const ss :: string & val );
};

class AcctSessionTime : public AttributeInt {
public:
	AcctSessionTime ( int val );
};

class AcctInputOctets : public AttributeInt {
public:
	AcctInputOctets ( int val );
};

class AcctOutputOctets : public AttributeInt {
public:
	AcctOutputOctets ( int val );
};

class AcctAuthentic : public AttributeInt {
	using AttributeInt :: printVal;
public:
	AcctAuthentic ( int val );
	void printVal ( std :: ostream & os ) const;
};

class AcctInputPackets : public AttributeInt {
public:
	AcctInputPackets ( int val );
};

class AcctOutputPackets : public AttributeInt {
public:
	AcctOutputPackets ( int val );
};

class AcctTerminateCause : public AttributeInt {
	using AttributeInt :: printVal;
public:
	AcctTerminateCause ( int val );
	void printVal ( std :: ostream & os ) const;
};

class AcctLinkCount : public AttributeInt {
public:
	AcctLinkCount ( int val );
};

class EventTimestamp : public AttributeInt {
public:
	EventTimestamp ( int val );
};

class AttributeVendor : public Attribute {
	int vendorId;
	int vendorType;
public:
	AttributeVendor ( const ss :: string & n, int id, int type );
	static Attribute * load ( const unsigned char * buf, int l );
	int printVal ( char * buf, int size ) const;
	virtual int printVendorVal ( char * buf, int size ) const = 0;
	void printVal ( std :: ostream & os ) const;
	virtual void printVendorVal ( std :: ostream & os ) const = 0;
	int getVendor ( ) const;
	int getType ( ) const;
};

class AttributeVendorString : public AttributeVendor {
	ss :: string value;
public:
	AttributeVendorString ( const ss :: string & n, int id, int type, const ss :: string & v );
	int printVendorVal ( char * buf, int size ) const;
	void printVendorVal ( std :: ostream & os ) const;
	const ss :: string & getVal ( ) const;
};

class AttributeVendorInt : public AttributeVendor {
	int value;
public:
	AttributeVendorInt ( const ss :: string & n, int id, int type, int v );
	int printVendorVal ( char * buf, int size ) const;
	void printVendorVal ( std :: ostream & os ) const;
	int getVal ( ) const;
};

class AttributeVendorAddr : public AttributeVendorInt {
	using AttributeVendorInt :: printVendorVal;
public:
	AttributeVendorAddr ( const ss :: string & n, int id, int type, int v );
	AttributeVendorAddr ( const ss :: string & n, int id, int type, const ss :: string & v );
	void printVendorVal ( std :: ostream & os ) const;
};

class CiscoNASPort : public AttributeVendorString {
public:
	CiscoNASPort ( const ss :: string & val );
};

class QuintumNASPort : public AttributeVendorString {
public:
	QuintumNASPort ( const ss :: string & val );
};

class CiscoAssignIPPool : public AttributeVendorAddr {
public:
	CiscoAssignIPPool ( int val );
	CiscoAssignIPPool ( const ss :: string & val );
};

class AttributeCisco : public AttributeVendorString {
public:
	AttributeCisco ( const ss :: string & n, int type, const ss :: string & val );
	AttributeCisco ( const ss :: string & n, int type, int val );
};

class H323GWId : public AttributeCisco {
public:
	H323GWId ( const ss :: string & val );
};

class H323ConfId : public AttributeCisco {
public:
	H323ConfId ( const ss :: string & val );
};

class H323IncomingConfId : public AttributeCisco {
public:
	H323IncomingConfId ( const ss :: string & val );
};

class H323RemoteId : public AttributeCisco {
public:
	H323RemoteId ( const ss :: string & val );
};

class H323CallOrigin : public AttributeCisco {
public:
	H323CallOrigin ( const ss :: string & val );
};

class H323CallType : public AttributeCisco {
public:
	H323CallType ( const ss :: string & val );
};

class H323SetupTime : public AttributeCisco {
public:
	H323SetupTime ( const ss :: string & val );
};

class H323ConnectTime : public AttributeCisco {
public:
	H323ConnectTime ( const ss :: string & val );
};

class H323DisconnectTime : public AttributeCisco {
public:
	H323DisconnectTime ( const ss :: string & val );
};

class H323DisconnectCause : public AttributeCisco {
public:
	H323DisconnectCause ( const ss :: string & val );
};

class H323VoiceQuality : public AttributeCisco {
public:
	H323VoiceQuality ( const ss :: string & val );
};

class H323RemoteAddress : public AttributeCisco {
public:
	H323RemoteAddress ( const ss :: string & val );
};

class H323ReturnCode : public AttributeCisco {
public:
	H323ReturnCode ( const ss :: string & val );
};

class H323PreferredLang : public AttributeCisco {
public:
	H323PreferredLang ( const ss :: string & val );
};

class H323BillingModel : public AttributeCisco {
public:
	H323BillingModel ( const ss :: string & val );
};

class H323IvrIn : public AttributeCisco {
public:
	H323IvrIn ( const ss :: string & val );
};

class H323IvrOut : public AttributeCisco {
public:
	H323IvrOut ( const ss :: string & val );
};

class H323CreditAmount : public AttributeCisco {
public:
	H323CreditAmount ( const ss :: string & val );
};

class H323CreditTime : public AttributeCisco {
public:
	H323CreditTime ( const ss :: string & val );
};

class AttributeQuintum : public AttributeVendorString {
public:
	AttributeQuintum ( const ss :: string & n, int type, const ss :: string & val );
};

class QH323ConfId : public AttributeQuintum {
public:
	QH323ConfId ( const ss :: string & val );
};

class QH323IncomingConfId : public AttributeQuintum {
public:
	QH323IncomingConfId ( const ss :: string & val );
};

class QH323CallOrigin : public AttributeQuintum {
public:
	QH323CallOrigin ( const ss :: string & val );
};

class QH323CallType : public AttributeQuintum {
public:
	QH323CallType ( const ss :: string & val );
};

class QH323SetupTime : public AttributeQuintum {
public:
	QH323SetupTime ( const ss :: string & val );
};

class QH323ConnectTime : public AttributeQuintum {
public:
	QH323ConnectTime ( const ss :: string & val );
};

class QH323DisconnectTime : public AttributeQuintum {
public:
	QH323DisconnectTime ( const ss :: string & val );
};

class QH323DisconnectCause : public AttributeQuintum {
public:
	QH323DisconnectCause ( const ss :: string & val );
};

class QH323VoiceQuality : public AttributeQuintum {
public:
	QH323VoiceQuality ( const ss :: string & val );
};

class QH323RemoteAddress : public AttributeQuintum {
public:
	QH323RemoteAddress ( const ss :: string & val );
};

class QH323GWId : public AttributeQuintum {
public:
	QH323GWId ( const ss :: string & val );
};

class QH323IvrOut : public AttributeQuintum {
public:
	QH323IvrOut ( const ss :: string & val );
};

class XediaDNSServer : public AttributeVendorAddr {
public:
	XediaDNSServer ( int val );
	XediaDNSServer ( const ss :: string & val );
};

class XediaAddressPool : public AttributeVendorString {
public:
	XediaAddressPool ( const ss :: string & val );
};

class AscendClientPrimaryDNS : public AttributeVendorAddr {
public:
	AscendClientPrimaryDNS ( int val );
	AscendClientPrimaryDNS ( const ss :: string & val );
};

class AscendClientSecondaryDNS : public AttributeVendorAddr {
public:
	AscendClientSecondaryDNS ( int val );
	AscendClientSecondaryDNS ( const ss :: string & val );
};

class Request : public Allocatable < __SS_ALLOCATOR > {
	ss :: string secret, ip;
	int port, id, code;
	enum { vecLen = 16 };
	unsigned char vec [ vecLen ], digest [ vecLen ];
	typedef std :: list < Pointer < Attribute >, __SS_ALLOCATOR < Pointer < Attribute > > > AttributesList;
	AttributesList attributes;
public:
	Request ( const ss :: string & s, PacketType c, const ss :: string & ip, int port );
	Request ( const ss :: string & s, const ss :: string & aip, int p, unsigned char * buf, int l );
	Request ( const Request & r, PacketType c );
	void append ( Pointer < Attribute > a );
	int print ( char * buf, int size ) ;
	void print ( std :: ostream & os ) const;
	const Attribute * find ( int a ) const;
	const AttributeString * findString ( int a, int num = 0 ) const;
	const AttributeInt * findInt ( int a ) const;
	const AttributeCisco * findCisco ( int a, const char * n = "", int skip = 0 ) const;
	const AttributeQuintum * findQuintum ( int a, const char * n = "" ) const;
	const AttributeVendorString * findVendorString ( int v, int a ) const;
	const ss :: string * findVendorString ( int a, const char * n = "" ) const;
	bool getVendorString ( int a, const char * n, ss :: string & s ) const;
	ss :: string getUserPassword ( ) const;
	const ss :: string & getIp ( ) const;
	int getPort ( ) const;
	ss :: string getCodeName ( ) const;
	bool accSecretVerified ( ) const;
	int getCode ( ) const;
};

inline std :: ostream & operator<< ( std :: ostream & os, const Request & r ) {
	r.print ( os );
	return os;
}

}

#endif
