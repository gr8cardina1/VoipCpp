#pragma implementation
#include <ptlib.h>
#include "ss.hpp"
#include "allocatable.hpp"
#include "endthread.hpp"
#include "aftertask.hpp"
#include <ptlib/sockets.h>
#include "ipport.hpp"
#include "icqcontact.hpp"
#include "radgwinfo.hpp"
#include "Conf.hpp"
#include <ptlib/svcproc.h>

EndThread :: ~EndThread ( ) { }

EndThread :: EndThread ( ) :
	PThread ( 1000, NoAutoDeleteThread, HighestPriority, "EndThread" ) {
	Resume ( );
}

void EndThread :: Main ( ) {
	while ( true ) {
		Sleep ( 1000 );
		try {
			conf -> balanceCards ( );
		} catch ( std :: exception & e ) {
			PSYSTEMLOG ( Error, "EndThread::Main exception: " << e.what ( ) );
		}
		if ( conf -> shuttingDown ( ) )
			break;
	}
	PSYSTEMLOG ( Info, "EndThread ended" );
}
