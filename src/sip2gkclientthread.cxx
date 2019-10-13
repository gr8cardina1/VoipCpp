#pragma implementation
#include "ss.hpp"
#include "allocatable.hpp"
#include <ptlib.h>
#include <ptlib/sockets.h>
#include "basegkclientthread.hpp"
#include "siporiginatehandler.hpp"
#include "sip2gkclientthread.hpp"
#include <boost/optional.hpp>
#include "sip2.hpp"
#include <tr1/functional>
#include "condvar.hpp"
#include "automutex.hpp"
#include <queue>
#include "threadmessagequeue.hpp"
#include "siptransportthread.hpp"
#include <ptlib/svcproc.h>
#include "sqloutthread.hpp"

namespace H323 {
ss :: string globallyUniqueId ( );
}

namespace SIP2 {

class GkClientThread :: Impl : private OriginateHandler, public Allocatable < __SS_ALLOCATOR > {
	typedef std :: tr1 :: function < bool ( ) > Message;
	ThreadMessageQueue < Message > q;
	ss :: string username;
	ss :: string gkName;
	ss :: string ip;
	ss :: string localIp;
	ss :: string pswd;
	ss :: string callId;
	ClientTransactionId id;
	PTime nextRegistration;
	StringVector realms;
	int port;
	int regPeriod;
	unsigned cseq;
	int regTries;
	bool registered;
	bool inprogress;
	void tryingReceived ( ) { }
	void ringingReceived ( const Response & /*r*/ ) { }
	void okReceived ( const Response & r );
	bool okReceivedLocal ( const Response & r );
	void unauthorizedReceived ( const StringVector & realms );
	bool unauthorizedReceivedLocal ( const StringVector & realms );
	void disconnectReceived ( int c, const ResponseMIMEInfo & m );
	bool disconnectReceivedLocal ( int c, const ResponseMIMEInfo & m );
	void onholdReceived ( int /*level*/, int /*port*/, const ss :: string & /*addr*/ ) {
		PSYSTEMLOG ( Error, "onhold in sipgkclient" );
	}
	void dtmfRelayReceived ( const DTMF :: Relay & /*r*/ ) {
		PSYSTEMLOG ( Error, "dtmfrelay in sipgkclient" );
	}
	static bool closeLocal ( ) {
		return false;
	}
	bool updateStateLocal ( const ss :: string & ln, int rs );
	void sendRegister ( );
	bool acceptsDtmfOverride ( ) const {
		return false;
	}
public:
	Impl ( const ss :: string & username, int regPeriod, const ss :: string & ip, const IntVector & localIps, const ss :: string & pswd,
		int port, const ss :: string & gkName );
	~Impl ( ) {
		if ( transportThread )
			transportThread -> detachHandler ( id );
	}
	void main ( );
	void close ( );
	void updateState ( const ss :: string & n, const ss :: string & ln, int rs );
};

static ss :: string getLocalAddress ( ) {
	PIPSocket :: Address addr;
	PIPSocket :: GetHostAddress ( addr );
	return static_cast < const char * > ( addr.AsString ( ) );
}

static ss :: string generateCallidLikeUUID ( ) {
	return toHex ( H323 :: globallyUniqueId ( ) );
}

GkClientThread :: Impl :: Impl ( const ss :: string & username, int regPeriod, const ss :: string & ip, const IntVector & localIps,
	const ss :: string & pswd, int port, const ss :: string & gkName ) : username ( username ), gkName ( gkName ), ip ( ip ), pswd ( pswd ),
	callId ( generateCallidLikeUUID ( ) ), id ( "", SIP2 :: Request :: mRegister ), port ( port ), regPeriod ( regPeriod ), cseq ( 0 ),
	regTries ( 0 ), registered ( false ), inprogress ( false ) {
	int addr = localIps.empty ( ) ? INADDR_ANY : localIps [ 0 ];
	if ( addr == INADDR_ANY )
		localIp = getLocalAddress ( );
	else
		localIp = static_cast < const char * > ( PIPSocket :: Address ( addr ).AsString ( ) );
}

void GkClientThread :: Impl :: main ( ) {
	sendRegister ( );
	while ( true ) {
		Message m;
		if ( q.get ( m ) && ! m ( ) )
			return;
		if ( ! inprogress && PTime ( ) > nextRegistration )
			sendRegister ( );
	}
}

void GkClientThread :: Impl :: close ( ) {
	q.put ( & GkClientThread :: Impl :: closeLocal );
}

void GkClientThread :: Impl :: updateState ( const ss :: string & /*n*/, const ss :: string & ln, int rs ) {
	q.put ( std :: tr1 :: bind ( & GkClientThread :: Impl :: updateStateLocal, this, ln, rs ) );
}

bool GkClientThread :: Impl :: updateStateLocal ( const ss :: string & ln, int rs ) {
	username = ln;
	regPeriod = rs;
	regTries = 0;
	if ( ! inprogress && ! registered )
		sendRegister ( );
	return true;
}

void GkClientThread :: Impl :: sendRegister ( ) {
	inprogress = true;
	id = transportThread -> sendRegister ( ip, username, port, localIp, callId, ++ cseq, regPeriod, realms, pswd, this );
}

void GkClientThread :: Impl :: okReceived ( const Response & r ) {
	q.put ( std :: tr1 :: bind ( & GkClientThread :: Impl :: okReceivedLocal, this, r ) );
}

bool GkClientThread :: Impl :: okReceivedLocal ( const Response & r ) {
	inprogress = false;
	registered = true;
	regTries = 0;
	const ResponseMIMEInfo & m = r.getMime ( );
	const ContactHeader & c = m.getContact ( );
	long expires;
	if ( c.hasExpires ( ) )
		expires = c.getExpires ( );
	else if ( m.hasExpires ( ) )
		expires = m.getExpires ( );
	else
		expires = regPeriod;
	expires -= 10;
	nextRegistration = PTime ( ) + PTimeInterval ( 0, expires );
	return true;
}

void GkClientThread :: Impl :: unauthorizedReceived ( const StringVector & rlms ) {
	q.put ( std :: tr1 :: bind ( & GkClientThread :: Impl :: unauthorizedReceivedLocal, this, rlms ) );
}

bool GkClientThread :: Impl :: unauthorizedReceivedLocal ( const StringVector & rlms ) {
	realms = rlms;
	regTries ++;
	registered = false;
	if ( regTries > 3 ) {
		inprogress = false;
		nextRegistration = PTime ( ) + PTimeInterval ( 0, ( 2l << ( regTries - 4 ) ) * 60 );
		return true;
	}
	sendRegister ( );
	return true;
}

void GkClientThread :: Impl :: disconnectReceived ( int c, const ResponseMIMEInfo & m ) {
	q.put ( std :: tr1 :: bind ( & GkClientThread :: Impl :: disconnectReceivedLocal, this, c, m ) );
}

bool GkClientThread :: Impl :: disconnectReceivedLocal ( int c, const ResponseMIMEInfo & m ) {
	if ( c == Response :: scFailureIntervalTooBrief )
		if ( unsigned e = m.getMinExpires ( ) )
			regPeriod = e;
	regTries ++;
	registered = false;
	if ( regTries > 3 ) {
		inprogress = false;
		nextRegistration = PTime ( ) + PTimeInterval ( 0, ( 2l << ( regTries - 4 ) ) * 60 );
		return true;
	}
	sendRegister ( );
	return true;
}

GkClientThread :: GkClientThread ( const ss :: string & username, int regPeriod, const ss :: string & ip, const IntVector & localIps,
	const ss :: string & pswd, int port, const ss :: string & gkName ) :
	impl ( new Impl ( username, regPeriod, ip, localIps, pswd, port, gkName ) ) {
	SetThreadName ( ( "SIP2 :: GkClient " + gkName ).c_str ( ) );
	ss :: ostringstream os;
	os << "insert into GateKeepersState ( gkName, state, ip, port ) values " <<
		" ( \"" << gkName << "\", \"UNREGISTERED\", \"" << ip << "\", " << port << " )";
	sqlOut -> add ( os.str ( ) );
	PSYSTEMLOG ( Info, "Constructing SIP2 :: GkClientThread for " << gkName );
	Resume ( );
}

GkClientThread :: ~ GkClientThread ( ) {
	delete impl;
}

void GkClientThread :: Main ( ) {
	impl -> main ( );
}

void GkClientThread :: Close ( ) {
	impl -> close ( );
}

void GkClientThread :: updateState ( const ss :: string & n, const ss :: string & ln, int rs ) {
	impl -> updateState ( n, ln, rs );
}

}
