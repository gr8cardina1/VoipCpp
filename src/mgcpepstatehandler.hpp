#ifndef MGCPEPSTATEHANDLER_HPP_
#define MGCPEPSTATEHANDLER_HPP_
#pragma interface

class Ep;
class CodecInfo;
class OriginateCallArg;

class EpStateHandler : public Allocatable < __SS_ALLOCATOR >, boost :: noncopyable {
protected:
	Ep * ep;
public:
	EpStateHandler ( Ep * e ) : ep ( e ) { }
	virtual ~EpStateHandler ( ) { }
	virtual void handleRequest ( MGCP :: PduVector & ret, const MGCP :: PDU & pdu, const PIPSocket :: Address & ia );
	virtual void handleResponse ( MGCP :: PduVector & ret, const MGCP :: PDU & pdu );
	virtual void transactionTimedOut ( MGCP :: PduVector & ret, const MGCP :: PDU & pdu ) = 0;
	virtual void sendRinging ( MGCP :: PduVector & ret, int localRtpPort, const PIPSocket :: Address & localAddr,
		const CodecInfo & inCodec );
	virtual void sendOk ( MGCP :: PduVector & ret, int localRtpPort, const PIPSocket :: Address & localAddr,
		const CodecInfo & inCodec, unsigned telephoneEventsPayloadType );
	virtual bool originateCall ( const OriginateCallArg & /*a*/ ) {
		return false;
	}
};

#endif /*MGCPEPSTATEHANDLER_HPP_*/
