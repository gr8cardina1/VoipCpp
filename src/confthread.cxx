#pragma implementation
#include <ptlib.h>
#include "ss.hpp"
#include "allocatable.hpp"
#include "confthread.hpp"
#include <ptlib/sockets.h>
#include "aftertask.hpp"
#include "ipport.hpp"
#include "icqcontact.hpp"
#include "radgwinfo.hpp"
#include "Conf.hpp"
#include <ptlib/svcproc.h>

ConfThread :: ~ConfThread ( ) { }

ConfThread :: ConfThread ( ) :
	PThread ( 1000, NoAutoDeleteThread, LowestPriority, "ConfThread" ) {
	Resume ( );
}

void ConfThread :: Main ( ) {
	while ( true ) {
		Sleep ( 10000 );
		try {
			conf -> reloadConf ( );
		} catch ( std :: exception & e ) {
			PSYSTEMLOG ( Error, "ConfThread::Main exception: " << e.what ( ) );
			PSYSTEMLOG ( Error, "shutting down" );
			conf -> setShuttingDown ( );
		}
		if ( conf -> shuttingDown ( ) )
			break;
	}
	PSYSTEMLOG ( Info, "ConfThread ended" );
}
