#ifndef MGCPEP_HPP_
#define MGCPEP_HPP_
#pragma interface

class Gw;

class CodecInfo;

class EpOriginateThreadHandler;
class OriginateCallArg;

class EpStateHandler;
//vnesti vnutr Ep ?

class MgcpClassInfo;

class Ep : public Allocatable < __SS_ALLOCATOR >, boost :: noncopyable {
	ss :: string name;
	Gw * gw;
	EpStateHandler * handler;
	unsigned inPeer;
	bool sBearerInformation, sRestartMethod, sRestartDelay, sReasonCode, sPackageList, sMaxMGCPDatagram,
		sPersistentEvents, sNotificationState;
	friend class EpStartHandler;
public:
	Ep ( const ss :: string & n, Gw * g, unsigned ip, const MgcpClassInfo & cl );
	~Ep ( );
	const ss :: string & getName ( ) const {
		return name;
	}
	const ss :: string & getGwName ( ) const;
	const ss :: string & getIp ( ) const;
	unsigned short getPort ( ) const;
	unsigned getInPeer ( ) const {
		return inPeer;
	}
	void handleRestart ( MGCP :: PduVector & ret );
	void handleRequest ( MGCP :: PduVector & ret, const MGCP :: PDU & pdu, const PIPSocket :: Address & ia );
	void handleResponse ( MGCP :: PduVector & ret, const MGCP :: PDU & pdu );
	void init ( MGCP :: PduVector & ret );
	void transactionTimedOut ( MGCP :: PduVector & ret, const MGCP :: PDU & pdu );
	void changeHandler ( EpStateHandler * h );
	void detachThreadHandler ( MGCP :: PduVector & ret, bool ok );
	bool originateCall ( const OriginateCallArg & a );
	void sendRinging ( MGCP :: PduVector & ret, int localRtpPort, const PIPSocket :: Address & localAddr,
		const CodecInfo & inCodec );
	void sendOk ( MGCP :: PduVector & ret, int localRtpPort, const PIPSocket :: Address & localAddr,
		const CodecInfo & inCodec, unsigned telephoneEventsPayloadType );
	void reloadConf ( unsigned ip, const MgcpClassInfo & cl );
};


#endif /*MGCPEP_HPP_*/
