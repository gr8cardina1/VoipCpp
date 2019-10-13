#ifndef SMARTGUARD_HPP_
#define SMARTGUARD_HPP_
#pragma interface

#include "nonaliasable.hpp"
//otlichniy variant, no gcc ne moget ego soptimizirovat tak ge kak vtoroy
class SmartGuard : public Allocatable < __SS_ALLOCATOR>, boost :: noncopyable, nonaliasable {
	std :: tr1 :: function < void ( ) > f;
public:
	template < typename T > SmartGuard ( const T & t ) : f ( t ) { }
	void dismiss ( ) {
		f = 0;
	}
	~SmartGuard ( ) {
		if ( f ) {
			try {
				f ( );
			} catch ( ... ) { }
		}
	}
};

#endif /*SMARTGUARD_HPP_*/
