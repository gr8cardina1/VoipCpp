#ifndef __SS_HPP
#define __SS_HPP
#pragma interface
#define BOOST_SPIRIT_USE_LIST_FOR_TREES
//#define BOOST_SPIRIT_USE_BOOST_ALLOCATOR_FOR_TREES
#include <ext/mt_allocator.h>
#include <string>
#include <sstream>
#include <vector>
#include <list>
#include <set>
#include <map>
#include <boost/noncopyable.hpp>

#ifndef linux
namespace std {
using :: strtoull;
}
#endif

namespace ss {

template < typename T > class mt_alloc : public __gnu_cxx :: __mt_alloc < T > {
public:
	mt_alloc ( const __gnu_cxx :: __mt_alloc < T > & a ) : __gnu_cxx :: __mt_alloc < T > ( a ) { }
	mt_alloc ( ) { }
};

#define __SS_ALLOCATOR ss :: mt_alloc

typedef std :: basic_string < char, std :: char_traits < char >, __SS_ALLOCATOR < char > > string;
typedef std :: basic_ostringstream < char, std :: char_traits < char >, __SS_ALLOCATOR < char > > ostringstream;
typedef std :: basic_istringstream < char, std :: char_traits < char >, __SS_ALLOCATOR < char > > istringstream;

}

typedef std :: vector < int, __SS_ALLOCATOR < int > > IntVector;
typedef std :: vector < ss :: string, __SS_ALLOCATOR < ss :: string > > StringVector;

typedef std :: list < ss :: string, __SS_ALLOCATOR < ss :: string > > StringList;

typedef std :: map < int, int, std :: less < int >,
	__SS_ALLOCATOR < std :: pair < const int, int > > > IntIntMap;
typedef std :: map < int, IntIntMap, std :: less < int >,
	__SS_ALLOCATOR < std :: pair < const int, IntIntMap > > > IntIntIntMap;
typedef std :: map < int, double, std :: less < int >,
	__SS_ALLOCATOR < std :: pair < const int, double > > > IntDoubleMap;
typedef std :: map < ss :: string, ss :: string, std :: less < ss :: string >,
	__SS_ALLOCATOR < std :: pair < const ss :: string, ss :: string > > > StringStringMap;
typedef std :: map < ss :: string, int, std :: less < ss :: string >,
	__SS_ALLOCATOR < std :: pair < const ss :: string, int > > > StringIntMap;
typedef std :: map < ss :: string, unsigned, std :: less < ss :: string >,
	__SS_ALLOCATOR < std :: pair < const ss :: string, unsigned > > > StringUnsignedMap;
typedef std :: map < ss :: string, bool, std :: less < ss :: string >,
	__SS_ALLOCATOR < std :: pair < const ss :: string, bool > > > StringBoolMap;

typedef std :: multimap < ss :: string, int, std :: less < ss :: string >,
	__SS_ALLOCATOR < std :: pair < const ss :: string, int > > > StringIntMultiMap;
typedef std :: multimap < ss :: string, ss :: string, std :: less < ss :: string >,
	__SS_ALLOCATOR < std :: pair < const ss :: string, ss :: string > > > StringStringMultiMap;
typedef std :: map < int, ss :: string, std :: less < int >,
	__SS_ALLOCATOR < std :: pair < const int, ss :: string > > > IntStringMap;
typedef std :: multimap < int, ss :: string, std :: less < int >,
	__SS_ALLOCATOR < std :: pair < const int, ss :: string > > > IntStringMultiMap;

typedef std :: set < ss :: string, std :: less < ss :: string >, __SS_ALLOCATOR < ss :: string > > StringSet;
typedef std :: set < int, std :: less < int >, __SS_ALLOCATOR < int > > IntSet;

typedef std :: multiset < ss :: string, std :: less < ss :: string >,
	__SS_ALLOCATOR < ss :: string > > StringMultiSet;

typedef std :: map < int, StringStringMap, std :: less < int >,
	__SS_ALLOCATOR < std :: pair < const int, StringStringMap > > > IntStringStringMap;

typedef std :: map < ss :: string, StringSet, std :: less < ss :: string >,
	__SS_ALLOCATOR < std :: pair < const ss :: string, StringSet > > > StringStringSetMap;

#endif
