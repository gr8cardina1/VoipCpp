#include "ss.hpp"
#include "debugstream.hpp"

DebugStream :: DebugStream ( ) { }

DebugStream :: ~DebugStream ( ) { }

void DebugStream :: add ( const ss :: string & s ) {
	str += s;
}
