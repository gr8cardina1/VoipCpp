#pragma implementation
#include "ss.hpp"
#include "allocatable.hpp"
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/sequenced_index.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/member.hpp>
#include <ptlib.h>
#include "callbuffer.hpp"
#include <ptlib/svcproc.h>

CallBuffer :: CallBuffer ( int s, const IntSet & cs ) : secs ( s ), codes ( cs ) { }

int CallBuffer :: getCode ( const ss :: string & digits ) {
	PTime now;
	for ( CallVector :: iterator i = calls.begin ( ); i != calls.end ( ); ) {
		if ( ( now - i -> when ).GetSeconds ( ) <= secs )
			break;
		PSYSTEMLOG ( Info, "removing digits " << i -> digits << " from buffer" );
		i = calls.erase ( i );
	}
	typedef CallVector :: index < Call > :: type ByDigits;
	const ByDigits & byDigits = calls.get < Call > ( );
	ByDigits :: const_iterator i = byDigits.find ( digits );
	if ( i == byDigits.end ( ) ) {
		PSYSTEMLOG ( Info, "digits " << digits << " not in buffer" );
		return 0;
	}
	PSYSTEMLOG ( Info, "digits " << digits << " code " << i -> code );
	return i -> code;
}

void CallBuffer :: addCall ( const ss :: string & digits, int code ) {
	typedef CallVector :: index < Call > :: type ByDigits;
	ByDigits & byDigits = calls.get < Call > ( );
	byDigits.erase ( digits );
	if ( calls.push_back ( Call ( digits, code ) ).second )
		PSYSTEMLOG ( Info, "set code " << code << " for digits " << digits );
	else
		PSYSTEMLOG ( Error, "internal error: cant insert call" );
}
