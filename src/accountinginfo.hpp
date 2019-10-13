#ifndef __ACCOUNTINGINFO_HPP
#define __ACCOUNTINGINFO_HPP
#pragma interface

struct AccountingInfo {
	AccountingInfo ( ) : nasPortType ( 0 ), acctStatusType ( 0 ), serviceType ( 0 ),
		acctInputOctets ( 0 ), acctOutputOctets ( 0 ), acctSessionTime ( 0 ),
		acctDelayTime ( 0 ), sessionTimeout ( 0 ) { }
	int nasPortType;
	ss :: string userName;
	ss :: string calledStationId;
	ss :: string callingStationId;
	ss :: string modifiedANumber;
	int acctStatusType;
	int serviceType;
	ss :: string acctSessionId;
	int acctInputOctets;
	int acctOutputOctets;
	int acctSessionTime;
	int acctDelayTime;
	ss :: string nasIpAddress;
	ss :: string ciscoNasPort;
	ss :: string h323ConfId;
	ss :: string h323IncomingConfId;
	ss :: string h323GwId;
	ss :: string h323CallOrigin;
	ss :: string h323CallType;
	ss :: string h323SetupTime;
	ss :: string h323ConnectTime;
	ss :: string h323DisconnectTime;
	ss :: string h323DisconnectCause;
	ss :: string h323VoiceQuality;
	ss :: string h323RemoteAddress;
	int sessionTimeout;
};
#endif
