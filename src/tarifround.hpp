#ifndef __TARIFROUND_HPP
#define __TARIFROUND_HPP
#pragma interface

namespace boost {
	namespace serialization {
		class access;
	}
}

class TarifRound {
	friend class boost :: serialization :: access;
	unsigned char rmin, reach, rfree;
	template < class Archive > void serialize ( Archive & ar, const unsigned int /*version*/ ) {
		ar & rmin;
		ar & reach;
		ar & rfree;
	}
public:
	TarifRound ( int rm = 0, int re = 1, int rf = 0 );
	int round ( int secs ) const;
	int roundNoFree ( int secs ) const;
	int roundDown ( int secs ) const;
};
#endif
