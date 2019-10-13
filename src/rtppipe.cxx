#pragma implementation
#include <ptlib.h>
#include <ptlib/pipechan.h>
#include "ss.hpp"
#include "allocatable.hpp"
#include "rtppipe.hpp"
#include "automutex.hpp"
#include "rtpstat.hpp"
#include "rtprequest.hpp"
#include <ptlib/svcproc.h>

RTPPipe :: RTPPipe ( ) : pipe ( "./psbc_rtp", PPipeChannel :: ReadWrite, false, true ) { }

RTPPipe :: ~RTPPipe ( ) {
	pipe.Kill ( );
}

bool RTPPipe :: send ( RTPRequest & r, int retCount ) {
	AutoMutex am ( mut );
	if ( ! pipe.Write ( & r, sizeof ( r ) ) ) {
		PSYSTEMLOG ( Error, "can't write request: " <<
			pipe.GetErrorText ( ) );
		return false;
	}
	bool ret = true;
	retCount = int ( retCount * sizeof ( r.data [ 0 ] ) );
	char * p = reinterpret_cast < char * > ( r.data );
	while ( retCount > 0 && ! pipe.ReadBlock ( p, retCount ) ) {
		int lastReadCount = pipe.GetLastReadCount ( );
		PSYSTEMLOG ( Error, "can't read request: " << pipe.GetErrorText ( ) << ", got " << lastReadCount
			<< " bytes, trying again" );
		p += lastReadCount;
		retCount -= lastReadCount;
	}
	PString err;
	if ( pipe.ReadStandardError ( err ) )
		PSYSTEMLOG ( Error, "rtpslave error: " << err );
	return ret;
}
