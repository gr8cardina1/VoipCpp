#ifndef __UNKNOWNDIGIT_HPP
#define __UNKNOWNDIGIT_HPP
#pragma interface

class UnknownDigit : public std :: range_error {
public:
	UnknownDigit ( char c ) : std :: range_error ( std :: string ( "unknown digit " ) + c ) { }
};
#endif
