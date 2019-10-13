#ifndef MGCPCONF_HPP_
#define MGCPCONF_HPP_
#pragma interface

struct MgcpClassInfo {
	unsigned short maxDatagram;
	bool sBearerInformation : 1;
	bool sRestartMethod : 1;
	bool sRestartDelay : 1;
	bool sReasonCode : 1;
	bool sPackageList : 1;
	bool sMaxMGCPDatagram : 1;
	bool sPersistentEvents : 1;
	bool sNotificationState : 1;
	MgcpClassInfo ( ) : maxDatagram ( 0 ), sBearerInformation ( false ), sRestartMethod ( false ),
		sRestartDelay ( false ), sReasonCode ( false ), sPackageList ( false ), sMaxMGCPDatagram ( false ),
		sPersistentEvents ( false ), sNotificationState ( false ) { }
};

struct MgcpEndpointInfo {
	ss :: string name;
	unsigned inPeer;
};

class MgcpEndpointInfoVector : public std :: vector < MgcpEndpointInfo, __SS_ALLOCATOR < MgcpEndpointInfo > > { };

struct MgcpGatewayInfo {
	ss :: string name;
	ss :: string ip;
	MgcpEndpointInfoVector eps;
	MgcpClassInfo cls;
};

class MgcpGatewayInfoVector : public std :: vector < MgcpGatewayInfo, __SS_ALLOCATOR < MgcpGatewayInfo > > { };

#endif /*MGCPCONF_HPP_*/
