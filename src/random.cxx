#include "ss.hpp"
#include <ptlib.h>
#include "automutex.hpp"
#include <boost/random/mersenne_twister.hpp>
namespace Random {

class random_device : private boost :: noncopyable {
public:
	typedef unsigned int result_type;
	static const bool has_fixed_range = true;
	static const result_type min_value = boost :: integer_traits < result_type > :: const_min;
	static const result_type max_value = boost :: integer_traits < result_type > :: const_max;
	explicit random_device ( const ss :: string & token = default_token );
	~random_device ( );
	unsigned operator() ( );
private:
	static const char * const default_token;
	class Impl;
	Impl * impl;
};

const char * const random_device :: default_token = "/dev/urandom";

class random_device :: Impl {
	PFile f;
public:
	Impl ( const ss :: string & token ) : f ( token.c_str ( ), PFile :: ReadOnly ) { }
	unsigned read ( ) {
		unsigned r;
		f.Read ( & r, sizeof ( r ) );
		return r;
	}
};

random_device :: random_device ( const ss :: string & token ) : impl ( new Impl ( token ) ) { }

random_device :: ~random_device ( ) {
	delete impl;
}

unsigned random_device :: operator() ( ) {
	return impl -> read ( );
}

unsigned number ( ) {
	static random_device rd;
	static boost :: mt19937 mt ( rd );
	static PMutex mut;
	AutoMutex am ( mut );
	return mt ( );
}

}
