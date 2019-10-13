#ifndef MGCPGW_HPP_
#define MGCPGW_HPP_
#pragma interface

namespace MGCP {
	typedef std :: vector < MGCP :: PDU, __SS_ALLOCATOR < MGCP :: PDU > > PduVector;
}

class CodecInfo;
class EpOriginateThreadHandler;
class OriginateCallArg;
class MgcpEndpointInfoVector;
class MgcpClassInfo;
class MgcpGatewayInfo;

class Gw : public Allocatable < __SS_ALLOCATOR >, boost :: noncopyable {
	class Impl;
	Impl * impl;
public:
	Gw ( const ss :: string & n, const ss :: string & a, const MgcpEndpointInfoVector & e, unsigned short p,
		const MgcpClassInfo & cls );
	~Gw ( );
	const ss :: string & getName ( ) const;
	const ss :: string & getIp ( ) const;
	unsigned short getPort ( ) const;
	unsigned short getMaxDatagram ( ) const;
	void handleRequest ( MGCP :: PduVector & ret, const MGCP :: PDU & pdu, const PIPSocket :: Address & ia );
	void init ( MGCP :: PduVector & ret ) const;
	void handleResponse ( MGCP :: PduVector & ret, const MGCP :: PDU & pdu, const ss :: string & ep );
	void transactionTimedOut ( MGCP :: PduVector & ret, const MGCP :: PDU & pdu );
	void detachThreadHandler ( MGCP :: PduVector & ret, const ss :: string & ep, bool ok );
	bool originateCall ( const ss :: string & ep, const OriginateCallArg & a );
	void sendRinging ( MGCP :: PduVector & ret, const ss :: string & ep, int localRtpPort,
		const PIPSocket :: Address & localAddr, const CodecInfo & inCodec );
	void sendOk ( MGCP :: PduVector & ret, const ss :: string & ep, int localRtpPort,
		const PIPSocket :: Address & localAddr, const CodecInfo & inCodec, unsigned telephoneEventsPayloadType );
	void reloadConf ( MGCP :: PduVector & ret, const MgcpGatewayInfo & gi );
};

#endif /*MGCPGW_HPP_*/
