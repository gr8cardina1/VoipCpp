#ifndef DEBUGSTREAM_HPP_
#define DEBUGSTREAM_HPP_

class DebugStream {
	ss :: string str;
public:
	DebugStream ( );
	virtual ~DebugStream ( );
	virtual void add ( const ss :: string & s );
};

template < typename T > DebugStream & operator<< ( DebugStream & d, const T & t ) {
	ss :: ostringstream os;
	os << t;
	d.add ( os.str ( ) );
	return d;
}

#endif /*DEBUGSTREAM_HPP_*/
