#pragma implementation
#include "ss.hpp"
#include "allocatable.hpp"
#include <ptlib.h>
#include "legthread.hpp"

LegThread :: LegThread ( H323Call * c, LegThread * * p, unsigned _id ) :
	PThread ( 1000, AutoDeleteThread, NormalPriority, PString ( PString :: Printf, "Leg N%u", _id ) ),
	call ( c ), id ( _id ), begInited ( false ) {
	* p = this;
}
