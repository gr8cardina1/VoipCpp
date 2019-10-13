#ifndef SIPORIGINATEHANDLER_HPP_
#define SIPORIGINATEHANDLER_HPP_
#pragma interface

namespace DTMF {
	struct Relay;
}

namespace SIP2 {

class Response;
class ResponseMIMEInfo;

class OriginateHandler {
protected:
	virtual ~OriginateHandler ( ) { }
public:
	virtual void tryingReceived ( ) = 0;
	virtual void ringingReceived ( const Response & r ) = 0;
	virtual void okReceived ( const Response & r ) = 0;
	virtual void disconnectReceived ( int c, const ResponseMIMEInfo & m ) = 0;
	virtual void unauthorizedReceived ( const StringVector & realms ) = 0;
	virtual void onholdReceived ( int level, int port, const ss :: string & ip ) = 0;
	virtual void dtmfRelayReceived ( const DTMF :: Relay & r ) = 0;
	virtual bool acceptsDtmfOverride ( ) const = 0;
};

}

#endif /*SIPORIGINATEHANDLER_HPP_*/
