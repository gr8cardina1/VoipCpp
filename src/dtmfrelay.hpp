#ifndef DTMFRELAY_HPP_
#define DTMFRELAY_HPP_
#pragma interface

namespace DTMF {

struct Relay {
	short msecs;
	char signal;
	void printOn ( std :: ostream & os ) const;
	Relay ( ss :: string :: const_iterator b, ss :: string :: const_iterator e );
};

inline std :: ostream & operator<< ( std :: ostream & os, const Relay & r ) {
	r.printOn ( os );
	return os;
}

}

#endif /*DTMFRELAY_HPP_*/
