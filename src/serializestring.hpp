#ifndef __SERIALIZESTRING_HPP
#define __SERIALIZESTRING_HPP
#pragma interface

//#include <boost/config.hpp>
//#include <boost/serialization/level.hpp>
//#include <boost/serialization/split_free.hpp>

BOOST_CLASS_IMPLEMENTATION ( ss :: string, boost :: serialization :: object_serializable )

namespace boost {
namespace serialization {

template < typename Archive, typename _CharT, typename _Traits, typename _Alloc >
void save ( Archive & ar, const std :: basic_string < _CharT, _Traits, _Alloc > & s, const unsigned int /*version*/ ) {
	std :: string t ( s.begin ( ), s.end ( ) );
	ar << t;
}

template < typename Archive, typename _CharT, typename _Traits, typename _Alloc >
void load ( Archive & ar, std :: basic_string < _CharT, _Traits, _Alloc > & s, const unsigned int /*version*/ ) {
	std :: string t;
	ar >> t;
	s.assign ( t.begin ( ), t.end ( ) );
}

template < typename Archive, typename _CharT, typename _Traits, typename _Alloc >
void serialize ( Archive & ar, std :: basic_string < _CharT, _Traits, _Alloc > & s, const unsigned int version ) {
	split_free ( ar, s, version );
}

}
} // namespace boost::serialization

#endif
