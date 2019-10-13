#ifndef SIPTRANSPORTTHREAD_HPP_
#define SIPTRANSPORTTHREAD_HPP_
#pragma interface

namespace SDP {
	class SessionDescription;
}

namespace DTMF {
	struct Relay;
}

class CodecInfo;

namespace SIP2 {

class ClientTransactionId;
class ServerTransactionId;
class OriginateHandler;
class AnswerHandler;

class TransportThread : public PThread, public Allocatable < __SS_ALLOCATOR > {
public:
	class Impl;
private:
	Impl * impl;
public:
	TransportThread ( );
	~TransportThread ( );
	void Main ( );
	void Close ( );
	bool sendMessage ( const ss :: string & host, int port, const ss :: string & msg );
	bool sendMessage ( Request & r );
	bool sendMessage ( const Response & r );
	void detachHandler ( const ClientTransactionId & id );
	void detachHandler ( const ServerTransactionId & id, int c );
	ClientTransactionId originateCall ( const ss :: string & host, const ss :: string & user,
		int port, const ss :: string & localHost, const ss :: string & localUser,
		const ss :: string & callId, unsigned maxForwards, unsigned cseq, const StringVector & realms,
		const ss :: string & remotePartyID, SDP :: SessionDescription * sdp, OriginateHandler * h,
		const ss :: string & passertid, const ss :: string & privacy );
	ClientTransactionId sendRegister ( const ss :: string & host, const ss :: string & user, int port,
		const ss :: string & localHost, const ss :: string & callId, unsigned cseq, unsigned expires, const StringVector & realms,
		const ss :: string & pass, OriginateHandler * h );
	void sendRinging ( const ServerTransactionId & id, int localRtpPort,
		const PIPSocket :: Address & localAddr, const CodecInfo & inCodec );
	void sendOk ( const ServerTransactionId & id, int localRtpPort,
		const PIPSocket :: Address & localAddr, const CodecInfo & inCodec, unsigned telephoneEventsPayloadType );
	ClientTransactionId sendOnhold ( const ClientTransactionId & inviteId, int level, OriginateHandler * h );
	ClientTransactionId sendOnhold ( const ServerTransactionId & inviteId, int level, AnswerHandler * h );
	ClientTransactionId sendDtmf ( const ClientTransactionId & inviteId, const DTMF :: Relay & r );
	ClientTransactionId sendDtmf ( const ServerTransactionId & inviteId, const DTMF :: Relay & r );
};

extern TransportThread * transportThread;

}

#endif /*SIPTRANSPORTTHREAD_HPP_*/
