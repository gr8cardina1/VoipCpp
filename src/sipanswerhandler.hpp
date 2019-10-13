#ifndef SIPANSWERHANDLER_HPP_
#define SIPANSWERHANDLER_HPP_
#pragma interface

namespace DTMF {
	struct Relay;
}

namespace SIP2 {

class AnswerHandler {
public:
	virtual ~AnswerHandler ( ) { }
	virtual void disconnectReceived ( int c ) = 0;
	virtual void cancelReceived ( ) = 0;
	virtual void onholdReceived ( int level, int port, const ss :: string & ip ) = 0;
	virtual void dtmfRelayReceived ( const DTMF :: Relay & r ) = 0;
};

}

#endif /*SIPANSWERHANDLER_HPP_*/
