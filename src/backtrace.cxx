#include <cstdlib>
#include <dlfcn.h>
#ifdef linux
#include <execinfo.h>
#endif
#include <cxxabi.h>
#include "ss.hpp"
#include <boost/noncopyable.hpp>
#include <tr1/functional>
#include "scopeguard.hpp"
#include "backtrace.hpp"
#include <ptlib.h>
#include "automutex.hpp"
#include <stdexcept>

struct FrameInfo {
	const char * fname;
	ss :: string sname;
	const char * saddr;
	bool valid;
	explicit FrameInfo ( const void * p ) : valid ( false ) {
		Dl_info info;
		if ( ! dladdr ( p, & info ) )
			throw std :: runtime_error ( dlerror ( ) );
		saddr = static_cast < const char * > ( info.dli_saddr );
		fname = info.dli_fname;
		if ( ! info.dli_sname )
			return;
		int status;
		char * d = abi :: __cxa_demangle ( info.dli_sname, 0, 0, & status );
		if ( ! d ) {
			sname = info.dli_sname;
			valid = true;
			return;
		}
		ScopeGuard g = makeGuard ( std :: tr1 :: bind ( std :: free, d ) );
		sname = d;
		valid = sname != "PThread::PX_ThreadStart(void*)" && sname != "PServiceProcess::_main(void*)";
	}
	bool validName ( ) const {
		return valid;
	}
};

static const FrameInfo & getFrameInfo ( const void * p ) {
	static PMutex mut;
	typedef std :: map < const void *, FrameInfo, std :: less < const void * >,
		__SS_ALLOCATOR < std :: pair < const void * const, FrameInfo > > > FrameMap;
	static FrameMap fmap;
	AutoMutex am ( mut );
	FrameMap :: const_iterator i = fmap.find ( p );
	if ( i != fmap.end ( ) )
		return i -> second;
	FrameInfo f ( p );
	return fmap.insert ( std :: make_pair ( p, f ) ).first -> second;
}

bool printFrame ( std :: ostream & os, const void * p ) {
	try {
		const FrameInfo & f = getFrameInfo ( p );
		os << p << ':' << f.fname << ':' << f.sname;
		unsigned long dif;
		if ( p >= f.saddr ) {
			dif = static_cast < const char * > ( p ) - f.saddr;
			os << '+';
		} else {
			dif = f.saddr - static_cast < const char * > ( p );
			os << '-';
		}
		os << "0x" << std :: hex << dif << std :: endl;
		return f.validName ( );
	} catch ( std :: exception & e ) {
		os << e.what ( ) << std :: endl;
		return false;
	}
}

void printBacktrace ( std :: ostream & os ) {
#ifdef linux
	const int size = 20;
	void * addrs [ size ];
	int i = backtrace ( addrs, size );
	for ( int j = 0; j < i; j ++ )
		printFrame ( os, addrs [ j ] );
#else
	const void * p;

#define TRACEFRAME(i) \
	p = __builtin_return_address ( i ); \
	if ( p == ( const void * ) 1 ) \
		return; \
	if ( ! printFrame ( os, p ) ) \
		return;

	TRACEFRAME(0)
	TRACEFRAME(1)
	TRACEFRAME(2)
	TRACEFRAME(3)
	TRACEFRAME(4)
	TRACEFRAME(5)
	TRACEFRAME(6)
	TRACEFRAME(7)
	TRACEFRAME(8)
	TRACEFRAME(9)
	TRACEFRAME(10)
	TRACEFRAME(11)
	TRACEFRAME(12)
	TRACEFRAME(13)
	TRACEFRAME(14)
	TRACEFRAME(15)
	TRACEFRAME(16)
	TRACEFRAME(17)
	TRACEFRAME(18)
	TRACEFRAME(19)
#undef TRACEFRAME
#endif
}

TraceVector getBacktrace ( ) {
	TraceVector v;
#ifdef linux
	const int size = 20;
	void * addrs [ size ];
	int i = backtrace ( addrs, size );
	v.reserve ( i );
	for ( int j = 0; j < i; j ++ )
		v.push_back ( addrs [ j ] );
#else
	const void * p;

#define TRACEFRAME(i) \
	p = __builtin_return_address ( i ); \
	if ( p == ( const void * ) 1 ) \
		return v; \
	v.push_back ( p ); \
	try { \
		const FrameInfo & f = getFrameInfo ( p ); \
		if ( ! f.validName ( ) ) \
			return v; \
	} catch ( std :: exception & e ) { \
		return v; \
	}

	TRACEFRAME(0)
	TRACEFRAME(1)
	TRACEFRAME(2)
	TRACEFRAME(3)
	TRACEFRAME(4)
	TRACEFRAME(5)
	TRACEFRAME(6)
	TRACEFRAME(7)
	TRACEFRAME(8)
	TRACEFRAME(9)
	TRACEFRAME(10)
	TRACEFRAME(11)
	TRACEFRAME(12)
	TRACEFRAME(13)
	TRACEFRAME(14)
	TRACEFRAME(15)
	TRACEFRAME(16)
	TRACEFRAME(17)
	TRACEFRAME(18)
	TRACEFRAME(19)
#undef TRACEFRAME
#endif
	return v;
}

ss :: string printBacktrace ( ) {
	ss :: ostringstream os;
	printBacktrace ( os );
	return os.str ( );
}

void printBacktrace ( std :: ostream & os, const TraceVector & v ) {
	for ( unsigned i = 0; i < v.size ( ) && printFrame ( os, v [ i ] ); ++ i )
		;
}
