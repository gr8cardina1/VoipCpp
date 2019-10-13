#ifndef MGCPEPTHREADHANDLER_HPP_
#define MGCPEPTHREADHANDLER_HPP_
#pragma interface

class EpThreadHandler : public Allocatable < __SS_ALLOCATOR > {
protected:
	virtual ~EpThreadHandler ( );
public:
	virtual void detach ( ) = 0;
};

namespace SDP {
	class SessionDescription;
}

class EpOriginateThreadHandler : public EpThreadHandler {
public:
	virtual void sdpAck ( const SDP :: SessionDescription & sdp ) = 0;
	virtual void connected ( ) = 0;
};

#endif /*MGCPEPTHREADHANDLER_HPP_*/
