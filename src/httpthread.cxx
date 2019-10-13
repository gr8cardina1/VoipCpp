#pragma implementation
#include "ss.hpp"
#include "allocatable.hpp"
#include <ptlib.h>
#include "httpthread.hpp"
#include <ptclib/http.h>
#include <ptlib/sockets.h>
#include "aftertask.hpp"
#include "ipport.hpp"
#include "icqcontact.hpp"
#include "radgwinfo.hpp"
#include "Conf.hpp"

#include <ptclib/pxml.h>
#include "ivractiveresource.hpp"
#include <ptlib/svcproc.h>

HTTPThread :: HTTPThread ( PTCPSocket * s, const PHTTPSpace & sp ) :
	PThread ( 1000, AutoDeleteThread, NormalPriority, "HttpThread" ),
	sock ( s ), space ( sp ) {
	conf -> takeHttpCall ( );
	Resume ( );
}

HTTPThread :: ~HTTPThread ( ) {
	conf -> releaseHttpCall ( );
	delete sock;
}

void HTTPThread :: Main ( ) {
#ifdef SO_LINGER
	const linger ling = { 1, 5 };
	sock -> SetOption ( SO_LINGER, & ling, sizeof ( ling ) );
#endif
	sock -> SetReadTimeout ( 10000 );
	PHTTPServer serv ( space );
	if ( ! serv.Open ( sock, false ) ) {
		PSYSTEMLOG ( Error, "http server cant open socket" );
		return;
	}
	serv.SetReadTimeout ( 10000 );
	while ( serv.ProcessCommand ( ) ) {
		PSYSTEMLOG ( Info, "HTTPThread :: Main: ProcessCommand: " );
		if ( conf -> shuttingDown ( ) )
			break;
	}
	PSYSTEMLOG ( Info, "HTTP thread ended" );
}
