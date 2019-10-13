#ifndef PRINTHEX_HPP_
#define PRINTHEX_HPP_
#pragma interface

template < class T > void printHex ( std :: ostream & os, const T & s ) {
	typedef unsigned char uchar;
	std :: ios_base :: fmtflags f = os.flags ( );
	for ( unsigned i = 0; i < s.size ( ); i ++ ) {
		if ( i )
			os << ' ';
		os << std :: setw ( 2 ) << std :: setfill ( '0' ) << std :: hex << unsigned ( uchar ( s [ i ] ) );
	}
	os.flags ( f );
}

#endif /*PRINTHEX_HPP_*/
