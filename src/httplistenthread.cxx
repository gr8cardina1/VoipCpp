#pragma implementation
#include "ss.hpp"
#include "allocatable.hpp"
#include <ptlib.h>
#include "httplistenthread.hpp"
#include "httpthread.hpp"
#include <ptclib/http.h>
#include <ptlib/sockets.h>
#include "aftertask.hpp"
#include "ipport.hpp"
#include "icqcontact.hpp"
#include "radgwinfo.hpp"
#include "Conf.hpp"
#include "activeresource.hpp"
#include "totalactiveresource.hpp"
#include "callpathresource.hpp"
#include <ptlib/svcproc.h>

#include <ptclib/pxml.h>
#include "ivractiveresource.hpp"


class AdminsAuth : public PHTTPAuthority, public Allocatable < __SS_ALLOCATOR > {
	PCLASSINFO ( AdminsAuth, PHTTPAuthority )
public:
	AdminsAuth (
		const ss :: string & realm // Name space for the username and password.
	);
	virtual PObject * Clone ( ) const;
	virtual PString GetRealm(
		const PHTTPRequest & request // Request information.
	) const;
	virtual BOOL Validate (
		const PHTTPRequest & request, // Request information.
		const PString & authInfo // Authority information string.
	) const;
	virtual BOOL IsActive ( ) const;
protected:
	ss :: string realm;
};

AdminsAuth :: AdminsAuth ( const ss :: string & realm_ ) : realm ( realm_ ) {
	PAssert ( ! realm.empty ( ), "Must have a realm!" );
}

PObject * AdminsAuth :: Clone ( ) const {
	return new AdminsAuth ( realm );
}

BOOL AdminsAuth :: IsActive ( ) const {
	return true;
}

PString AdminsAuth :: GetRealm ( const PHTTPRequest & ) const {
	return realm.c_str ( );
}

BOOL AdminsAuth :: Validate ( const PHTTPRequest &,
	const PString & authInfo ) const {
	PString user, pass;
	DecodeBasicAuthority ( authInfo, user, pass );
	return conf -> checkAdminsPass ( static_cast < const char * > ( user ), static_cast < const char * > ( pass ) );
}

HTTPListenThread :: HTTPListenThread ( ) :
	PThread ( 1000, NoAutoDeleteThread, NormalPriority, "HttpListenThread" ),
	sock ( new PTCPSocket ) {
	while ( ! sock -> Listen ( PIPSocket :: Address ( "*" ), 5,
		3335, PSocket :: CanReuseAddress ) ) {
		PSYSTEMLOG ( Error, "Listen failed on http socket: " << sock -> GetErrorText ( ) );
		if ( conf -> shuttingDown ( ) )
			break;
		Sleep ( 1000 );
	}
	Resume ( );
}

HTTPListenThread :: ~HTTPListenThread ( ) {
	delete sock;
}

void HTTPListenThread :: Close ( ) {
	sock -> Close ( );
}

void HTTPListenThread :: Main ( ) {
	PSYSTEMLOG ( Info, "HTTP Listen Thread :: Main begin" );
	sock -> SetReadTimeout ( 10000 );
	PHTTPSpace space;
	AdminsAuth auth ( "PSBC Internal Resources" );
	PHTTPResource * res = new ActiveResource ( );
	res -> SetAuthority ( auth );
	if ( ! space.AddResource ( res ) ) {
		PSYSTEMLOG ( Error, "cant add http resource" );
		return;
	}
	res = new ActiveResourceCSV ( );
	res -> SetAuthority ( auth );
	if ( ! space.AddResource ( res ) ) {
		PSYSTEMLOG ( Error, "cant add http resource" );
		return;
	}
	res = new TotalActiveResource ( );
	res -> SetAuthority ( auth );
	if ( ! space.AddResource ( res ) ) {
		PSYSTEMLOG ( Error, "cant add http resource" );
		return;
	}
	res = new CallPathResource ( );
	res -> SetAuthority ( auth );
	if ( ! space.AddResource ( res ) ) {
		PSYSTEMLOG ( Error, "cant add http resource" );
		return;
	}

	res = new IVRActiveResource ( );
	res -> SetAuthority ( auth );
	if ( ! space.AddResource ( res ) ) {
		PSYSTEMLOG ( Error, "cant add ivr resource" );
		return;
	}

	res = new IVRExpectedTimeResource ( );
	res -> SetAuthority ( auth );
	if ( ! space.AddResource ( res ) ) {
		PSYSTEMLOG ( Error, "cant add ivr resource" );
		return;
	}

	res = new IVRLanguageResource ( );
	res -> SetAuthority ( auth );
	if ( ! space.AddResource ( res ) ) {
		PSYSTEMLOG ( Error, "cant add ivr resource" );
		return;
	}

	PSYSTEMLOG ( Info, "HTTPThread :: Main: After add resource." );
	while ( sock -> IsOpen ( ) ) {
		if ( conf -> shuttingDown ( ) )
			break;
		PTCPSocket * httpChannel = new PTCPSocket;
		if ( httpChannel -> Accept ( * sock ) ) {
			PSYSTEMLOG ( Info, "New incoming connection to http port. this = " << this );
			new HTTPThread ( httpChannel, space );
		} else {
			// Whoops, error in accepting call channel
			if ( httpChannel -> GetErrorCode ( ) != PChannel :: Timeout &&
				httpChannel -> GetErrorCode ( ) != PChannel :: Interrupted )
				PSYSTEMLOG ( Error, "Error in accepting http connection: " <<
					httpChannel -> GetErrorText ( ) );
			delete httpChannel;
		}
	}
	PSYSTEMLOG ( Info, "HTTP listen thread ended" );
}
