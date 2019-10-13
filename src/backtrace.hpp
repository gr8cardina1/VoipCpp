#ifndef BACKTRACE_HPP_
#define BACKTRACE_HPP_
#pragma interface

void printBacktrace ( std :: ostream & os );
ss :: string printBacktrace ( );
typedef std :: vector < const void *, __SS_ALLOCATOR < const void * > > TraceVector;
TraceVector getBacktrace ( );
void printBacktrace ( std :: ostream & os, const TraceVector & v );
#endif /*BACKTRACE_HPP_*/
