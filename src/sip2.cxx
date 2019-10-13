#pragma implementation
#include "ss.hpp"
#include "allocatable.hpp"
#include <ptlib.h>
#include <boost/optional.hpp>
#include "sip2.hpp"
#include <boost/spirit/include/classic_parse_tree.hpp>
#include "rightparser.hpp"
#include "sipgrammars.hpp"
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/sequenced_index.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/mem_fun.hpp>
#include <boost/multi_index/member.hpp>
#include "sdp.hpp"
#include <iomanip>
#include <ptlib/sockets.h>
#include "siptransportthread.hpp"
#include "pointer.hpp"
#include <tr1/cstdint>
#include "md5.hpp"
#include "boost/algorithm/string/predicate.hpp"
#include <boost/io/ios_state.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "random.hpp"
#include <bitset>
#include <boost/utility/in_place_factory.hpp>
#include <boost/variant.hpp>
#include "dtmfrelay.hpp"
#include <boost/iterator/transform_iterator.hpp>

namespace boost {
namespace date_time {

	template < > const char time_formats < char > :: iso_time_format_specifier [ 18 ] =
		{ '%', 'Y', '%', 'm', '%', 'd', 'T', '%', 'H', '%', 'M', '%', 'S', '%', 'F', '%','q' };

	template < > const char time_formats < char > :: iso_time_format_extended_specifier [ 22 ] =
		{ '%', 'Y', '-', '%', 'm', '-', '%', 'd', ' ', '%', 'H', ':', '%', 'M', ':', '%', 'S', '%', 'F', '%', 'Q' };

	template < > const char time_formats < char > :: default_time_duration_format [ 11 ] =
		{ '%', 'H', ':', '%', 'M', ':', '%', 'S', '%', 'F' };

	template < > const char time_formats < char > :: default_time_format [ 23 ] =
		{ '%', 'Y', '-', '%', 'b', '-', '%', 'd', ' ', '%', 'H', ':', '%', 'M', ':', '%', 'S', '%', 'F', ' ', '%', 'z' };

	template < > const char time_formats < char > :: default_time_input_format [ 24 ] =
		{ '%', 'Y', '-', '%', 'b', '-', '%', 'd', ' ', '%', 'H', ':', '%', 'M', ':',
		'%', 'S', '%', 'F', ' ', '%', 'Z', 'P' };

	template < > const char time_formats < char > :: duration_sign_negative_only [ 3 ] = { '%', '-' };

	template < > const char time_formats < char > :: seconds_with_fractional_seconds_format [ 3 ] = { '%', 's' };

	template < > const char time_formats < char > :: fractional_seconds_format [ 3 ] = { '%', 'f' };

	template < > const char time_formats < char > :: fractional_seconds_or_none_format [ 3 ] = { '%', 'F' };

	template < > const char time_formats < char > :: seconds_format [ 3 ] = { '%', 'S' };

	template < > const char time_formats < char > :: posix_zone_string_format [ 4 ] = { '%', 'Z', 'P' };

	template < > const char time_formats < char > :: zone_name_format [ 3 ] = { '%', 'Z' };

	template < > const char time_formats < char > :: zone_abbrev_format [ 3 ] = { '%', 'z' };

	template < > const char time_formats < char > :: zone_iso_extended_format [ 3 ] = { '%', 'Q' };

	template < > const char time_formats < char > :: zone_iso_format [ 3 ] = { '%', 'q' };

	template < > const char * time_input_facet < posix_time :: ptime, char,
		std :: istreambuf_iterator < char, std :: char_traits < char> > > :: iso_time_format_specifier =
			time_formats < char > :: iso_time_format_specifier;

	template < > const char * time_facet < posix_time :: ptime, char,
		std :: ostreambuf_iterator < char, std :: char_traits < char> > > :: iso_time_format_specifier =
		time_formats < char > :: iso_time_format_specifier;

	template < > const char * time_input_facet < posix_time :: ptime, char,
		std :: istreambuf_iterator < char, std :: char_traits < char > > > :: iso_time_format_extended_specifier =
		time_formats < char > :: iso_time_format_extended_specifier;

	template < > const char * time_facet < posix_time :: ptime, char,
		std :: ostreambuf_iterator < char, std :: char_traits < char > > > :: iso_time_format_extended_specifier =
		time_formats < char > :: iso_time_format_extended_specifier;

	template < > const char date_facet < gregorian :: date, char,
		std :: ostreambuf_iterator < char, std :: char_traits < char > > > :: iso_format_specifier [ 7 ] =
		{ '%', 'Y', '%', 'm', '%', 'd' };

	template < > const char date_input_facet < gregorian :: date, char,
		std :: istreambuf_iterator < char, std :: char_traits < char > > > :: iso_format_specifier [ 7 ] =
		{ '%', 'Y', '%', 'm', '%', 'd' };

	template < > const char date_facet < gregorian :: date, char,
		std :: ostreambuf_iterator < char, std :: char_traits < char > > > :: iso_format_extended_specifier [ 9 ] =
		{ '%', 'Y', '-', '%', 'm', '-', '%', 'd' };

	template < > const char date_input_facet < gregorian :: date, char,
		std :: istreambuf_iterator < char, std :: char_traits < char > > > :: iso_format_extended_specifier [ 9 ] =
		{ '%', 'Y', '-', '%', 'm', '-', '%', 'd' };

	template < > const char date_facet < gregorian :: date, char,
		std :: ostreambuf_iterator < char, std :: char_traits < char > > > :: default_date_format [ 9 ] =
		{ '%', 'Y', '-', '%', 'b', '-', '%', 'd'};

	template < > const char date_facet < gregorian :: date, char,
		std :: ostreambuf_iterator < char, std :: char_traits < char > > > :: short_month_format [ 3 ] = { '%', 'b' };

	template < > const char date_input_facet < gregorian :: date, char,
		std :: istreambuf_iterator < char, std :: char_traits < char > > > :: short_month_format [ 3 ] = { '%', 'b' };

	template < > const char date_facet < gregorian :: date, char,
		std :: ostreambuf_iterator < char, std :: char_traits < char > > > :: short_weekday_format [ 3 ] = { '%', 'a' };

	template < > const char date_input_facet < gregorian :: date, char,
		std :: istreambuf_iterator < char, std :: char_traits < char > > > :: short_weekday_format [ 3 ] = { '%', 'a' };

	template < > const char date_input_facet < gregorian :: date, char,
		std :: istreambuf_iterator < char, std :: char_traits < char > > > :: four_digit_year_format [ 3 ] = { '%', 'Y' };

	template < > const char * time_facet < posix_time :: ptime, char,
		std :: ostreambuf_iterator < char, std :: char_traits < char > > > :: default_time_duration_format =
		time_formats < char > :: default_time_duration_format;

	template < > const char * time_facet < posix_time :: ptime, char,
		std :: ostreambuf_iterator < char, std :: char_traits < char > > > :: default_time_format =
		time_formats < char > :: default_time_format;

	template < > const char * time_input_facet < posix_time :: ptime, char,
		std :: istreambuf_iterator < char, std :: char_traits < char > > > :: default_time_duration_format =
		time_formats < char > :: default_time_duration_format;

	template < > const char * time_facet < posix_time :: ptime, char,
		std :: ostreambuf_iterator < char, std :: char_traits < char > > > :: duration_sign_negative_only =
		time_formats < char > :: duration_sign_negative_only;

	template < > const char * time_input_facet < posix_time :: ptime, char,
		std :: istreambuf_iterator < char, std :: char_traits < char > > > :: default_time_input_format =
		time_formats < char > :: default_time_input_format;

	template < > const char * time_facet < posix_time :: ptime, char,
		std :: ostreambuf_iterator < char, std :: char_traits < char > > > :: seconds_with_fractional_seconds_format =
		time_formats < char > :: seconds_with_fractional_seconds_format;

	template < > const char * time_facet < posix_time :: ptime, char,
		std :: ostreambuf_iterator < char, std :: char_traits < char > > > :: seconds_format =
		time_formats < char > :: seconds_format;

	template < > const char * time_facet < posix_time :: ptime, char,
		std :: ostreambuf_iterator < char, std :: char_traits < char > > > :: posix_zone_string_format =
		time_formats < char > :: posix_zone_string_format;

	template < > const char * time_facet < posix_time :: ptime, char,
		std :: ostreambuf_iterator < char, std :: char_traits < char > > > :: zone_name_format =
		time_formats < char > :: zone_name_format;

	template < > const char * time_facet < posix_time :: ptime, char,
		std :: ostreambuf_iterator < char, std :: char_traits < char > > > :: zone_abbrev_format =
		time_formats < char > :: zone_abbrev_format;

	template < > const char * time_facet < posix_time :: ptime, char,
		std :: ostreambuf_iterator < char, std :: char_traits < char > > > :: zone_iso_extended_format =
		time_formats < char > :: zone_iso_extended_format;

	template < > const char * time_facet < posix_time :: ptime, char,
		std :: ostreambuf_iterator < char, std :: char_traits < char > > > :: zone_iso_format =
		time_formats < char > :: zone_iso_format;

	template < > const char * time_facet < posix_time :: ptime, char,
		std :: ostreambuf_iterator < char, std :: char_traits < char > > > :: fractional_seconds_format =
		time_formats < char > :: fractional_seconds_format;

	template < > const char * time_facet < posix_time :: ptime, char,
		std :: ostreambuf_iterator < char, std :: char_traits < char > > > :: fractional_seconds_or_none_format =
		time_formats < char > :: fractional_seconds_or_none_format;

	template < > const char date_facet < gregorian :: date, char,
		std :: ostreambuf_iterator < char, std :: char_traits < char > > > :: long_weekday_format [ 3 ] = { '%', 'A' };

	template < > const char date_facet < gregorian :: date, char,
		std :: ostreambuf_iterator < char, std :: char_traits < char > > > :: long_month_format [ 3 ] = { '%', 'B' };

	template < > const char special_values_formatter < char,
		std :: ostreambuf_iterator < char, std :: char_traits < char > > > :: default_special_value_names [ 3 ] [ 17 ] = {
			{ 'n', 'o', 't', '-', 'a', '-', 'd', 'a', 't', 'e', '-', 't', 'i', 'm', 'e'},
			{ '-','i','n','f','i','n','i','t','y'},
			{ '+', 'i', 'n', 'f', 'i', 'n', 'i', 't', 'y' }
		};

	template < > const char special_values_parser < gregorian :: date, char > :: max_date_time_string [ 18 ] =
		{ 'm', 'a', 'x', 'i', 'm', 'u', 'm', '-', 'd', 'a', 't', 'e', '-', 't', 'i', 'm', 'e' };

	template < > const char special_values_parser < gregorian :: date, char > :: min_date_time_string [ 18 ] =
		{ 'm', 'i', 'n', 'i', 'm', 'u', 'm', '-', 'd', 'a', 't', 'e', '-', 't', 'i', 'm', 'e' };

	template < > const char special_values_parser < gregorian :: date, char > :: pos_inf_string [ 10 ] =
		{ '+', 'i', 'n', 'f', 'i', 'n', 'i', 't', 'y' };

	template < > const char special_values_parser < gregorian :: date, char > :: neg_inf_string [ 10 ] =
		{ '-', 'i', 'n', 'f', 'i', 'n', 'i', 't', 'y' };

	template < > const char special_values_parser < gregorian :: date, char > :: nadt_string [ 16 ] =
		{ 'n', 'o', 't', '-', 'a', '-', 'd', 'a', 't', 'e', '-', 't', 'i', 'm', 'e' };

	template < > const char date_generator_parser < gregorian :: date, char > :: of_string [ 3 ] = { 'o', 'f' };

	template < > const char date_generator_formatter < gregorian :: date, char,
		std :: ostreambuf_iterator < char, std :: char_traits < char > > > :: of_string [ 3 ] = { 'o', 'f' };

	template < > const char date_generator_parser < gregorian :: date, char > :: after_string [ 6 ] =
		{ 'a', 'f', 't', 'e', 'r' };

	template < > const char date_generator_formatter < gregorian :: date, char,
		std :: ostreambuf_iterator < char, std :: char_traits < char > > > :: after_string [ 6 ] =
		{ 'a', 'f', 't', 'e', 'r' };

	template < > const char date_generator_parser < gregorian :: date, char > :: before_string [ 8 ] =
		{ 'b', 'e', 'f', 'o', 'r', 'e' };

	template < > const char date_generator_formatter < gregorian :: date, char,
		std :: ostreambuf_iterator < char, std :: char_traits < char > > > :: before_string [ 8 ] =
		{ 'b', 'e', 'f', 'o', 'r', 'e' };

	template < > const char date_generator_parser < gregorian :: date, char > :: last_string [ 5 ] =
		{ 'l', 'a', 's', 't' };

	template < > const char date_generator_formatter < gregorian :: date, char,
		std :: ostreambuf_iterator < char, std :: char_traits < char > > > :: last_string [ 5 ] =
		{ 'l', 'a', 's', 't' };

	template < > const char date_generator_parser < gregorian :: date, char > :: fifth_string [ 6 ] =
		{ 'f', 'i', 'f', 't', 'h' };

	template < > const char date_generator_formatter < gregorian :: date, char,
		std :: ostreambuf_iterator < char, std :: char_traits < char > > > :: fifth_string [ 6 ] =
		{ 'f', 'i', 'f', 't', 'h' };

	template < > const char date_generator_parser < gregorian :: date, char > :: fourth_string [ 7 ] =
		{ 'f', 'o', 'u', 'r', 't', 'h' };

	template < > const char date_generator_formatter < gregorian :: date, char,
		std :: ostreambuf_iterator < char, std :: char_traits < char > > > :: fourth_string [ 7 ] =
		{ 'f', 'o', 'u', 'r', 't', 'h' };

	template < > const char date_generator_parser < gregorian :: date, char > :: third_string [ 6 ] =
		{ 't', 'h', 'i', 'r', 'd' };

	template < > const char date_generator_formatter < gregorian :: date, char,
		std :: ostreambuf_iterator < char, std :: char_traits < char > > > :: third_string [ 6 ] =
		{ 't', 'h', 'i', 'r', 'd' };

	template < > const char date_generator_parser < gregorian :: date, char > :: second_string [ 7 ] =
		{ 's', 'e', 'c', 'o', 'n', 'd' };

	template < > const char date_generator_formatter < gregorian :: date, char,
		std :: ostreambuf_iterator < char, std :: char_traits < char > > > :: second_string [ 7 ] =
		{ 's', 'e', 'c', 'o', 'n', 'd' };

	template < > const char date_generator_parser < gregorian :: date, char > :: first_string [ 6 ] =
		{ 'f', 'i', 'r', 's', 't' };

	template < > const char date_generator_formatter < gregorian :: date, char,
		std :: ostreambuf_iterator < char, std :: char_traits < char > > > :: first_string [ 6 ] =
		{ 'f', 'i', 'r', 's', 't' };

	template < > const char period_parser < gregorian :: date, char > :: default_period_closed_range_end_delimeter [ 2 ] =
		{ ']' };

	template < > const char period_formatter < char, std :: ostreambuf_iterator < char, std :: char_traits < char > > >
		:: default_period_closed_range_end_delimeter [ 2 ] = { ']' };

	template < > const char period_parser < gregorian :: date, char > :: default_period_open_range_end_delimeter [ 2 ] =
		{ ')' };

	template < > const char period_formatter < char, std :: ostreambuf_iterator < char, std :: char_traits < char > > >
		:: default_period_open_range_end_delimeter [ 2 ] = { ')' };

	template < > const char period_parser < gregorian :: date, char > :: default_period_start_delimeter [ 2 ] = { '[' };

	template < > const char period_formatter < char, std :: ostreambuf_iterator < char, std :: char_traits < char > > >
		:: default_period_start_delimeter [ 2 ] = { '[' };

	template < > const char period_parser < gregorian :: date, char > :: default_period_separator [ 2 ] = { '/' };

	template < > const char period_formatter < char, std :: ostreambuf_iterator < char, std :: char_traits < char > > >
		:: default_period_separator [ 2 ] = { '/' };

}
}

namespace H323 {
ss :: string globallyUniqueId ( );
}

namespace SIP2 {

class URI :: Impl : public Allocatable < __SS_ALLOCATOR > {
	ss :: string user;
	ss :: string password;
	ss :: string host;
	unsigned port;
	StringStringMap parameters;
	StringStringMap headers;
public:
	Impl ( ) : port ( 0 ) { }
	void setUser ( const ss :: string & u ) {
		user = u;
	}
	void setUser ( const ss :: string :: const_iterator & b, const ss :: string :: const_iterator & e ) {
		user.assign ( b, e );
	}
	const ss :: string & getUser ( ) const {
		return user;
	}
	void setPassword ( const ss :: string & p ) {
		password = p;
	}
	void setPassword ( const ss :: string :: const_iterator & b, const ss :: string :: const_iterator & e ) {
		password.assign ( b, e );
	}
	void setHost ( const ss :: string & h ) {
		host = h;
	}
	void setHost ( const ss :: string :: const_iterator & b, const ss :: string :: const_iterator & e ) {
		host.assign ( b, e );
	}
	const ss :: string & getHost ( ) const {
		return host;
	}
	void setPort ( unsigned p ) {
		port = p;
	}
	unsigned getPort ( ) const {
		return port;
	}
	void setParameter ( const ss :: string & pname, const ss :: string & pval ) {
		parameters [ pname ] = pval;
	}
	bool hasParameter ( const ss :: string & n ) const {
		return parameters.count ( n );
	}
	void delParameter ( const ss :: string & pname ) {
		parameters.erase ( pname );
	}
	void addHeader ( const ss :: string hname, const ss :: string & hval ) {
		headers [ hname ] = hval;
	}
	void clearHeaders ( ) {
		headers.clear ( );
	}
	void printOn ( std :: ostream & os ) const;
};

static char toHex ( unsigned i ) {
	if ( i < 10 )
		return char ( i + '0' );
	return char ( i + 'a' - 10 );
}

static void escape ( std :: ostream & os, const ss :: string & s, const std :: bitset < 256 > & allowedChars ) {
	for ( ss :: string :: const_iterator i = s.begin ( ); i != s.end ( ); ++ i ) {
		unsigned char c = * i;
		if ( allowedChars.test ( c ) )
			os << c;
		else
			os << '%' << toHex ( c / 16 ) << toHex ( c % 16 );
	}
}

static std :: bitset < 256 > initAlnumChars ( const char * t ) {
	std :: bitset < 256 > s;
	for ( char c = '0'; c <= '9'; c ++ )
		s.set ( c );
	for ( char c = 'A'; c <= 'Z'; c ++ )
		s.set ( c );
	for ( char c = 'a'; c <= 'z'; c ++ )
		s.set ( c );
	while ( * t )
		s.set ( * t ++ );
	return s;
}

static std :: bitset < 256 > initUserChars ( ) {
	return initAlnumChars ( "&=+$,:?/-_.!~*'()" );
}

static std :: bitset < 256 > userChars = initUserChars ( );

static std :: bitset < 256 > initPasswordChars ( ) {
	return initAlnumChars ( "-_.!~*'()""&=+$," );
}

static std :: bitset < 256 > passwordChars = initPasswordChars ( );

static std :: bitset < 256 > initParamChars ( ) {
	return initAlnumChars ( "-_.!~*'()""[]/:&+$" );
}

static std :: bitset < 256 > paramChars = initParamChars ( );

static std :: bitset < 256 > initHeaderChars ( ) {
	return initAlnumChars ( "-_.!~*'()""[]/?:+$" );
}

static std :: bitset < 256 > headerChars = initHeaderChars ( );

void URI :: Impl :: printOn ( std :: ostream & os ) const {
	os << "sip:";
	if ( ! user.empty ( ) ) {
		escape ( os, user, userChars );
		if ( ! password.empty ( ) )
			escape ( os << ':', password, passwordChars );
		os << '@';
	}
	os << host;
	if ( port )
		os << ':' << port;
	for ( StringStringMap :: const_iterator i = parameters.begin ( ); i != parameters.end ( ); ++ i ) {
		escape ( os << ';', i -> first, paramChars );
		if ( ! i -> second.empty ( ) )
			escape ( os << '=', i -> second, paramChars );
	}
	bool first = true;
	for ( StringStringMap :: const_iterator i = headers.begin ( ); i != headers.end ( ); ++ i ) {
		if ( first ) {
			os << '?';
			first = false;
		} else
			os << '&';
		escape ( os, i -> first, headerChars );
		escape ( os << '=', i -> second, headerChars );
	}
}

URI :: URI ( ) : impl ( new Impl ) { }

URI :: ~URI ( ) {
	delete impl;
}

URI :: URI ( const URI & u ) : impl ( new Impl ( * u.impl ) ) { }

static unsigned fromHex ( char c ) {
	if ( c <= '9' )
		return c - '0';
	if ( c <= 'F' )
		return c - 'A' + 10;
	return c - 'a' + 10;
}

static ss :: string unEscape ( ss :: string :: const_iterator b, ss :: string :: const_iterator e ) {
	ss :: string s;
	for ( ; b != e; ++ b ) {
		char c = * b;
		if ( c != '%' ) {
			s.push_back ( c );
			continue;
		}
		c = * ++ b;
		char c2 = * ++ b;
		s.push_back ( char ( fromHex ( c ) * 16 + fromHex ( c2 ) ) );
	}
	return s;
}

static void parseUri ( URI & u, RightParser :: iter_t i ) {
	RightParser :: iter_t j = i -> children.begin ( );
	if ( j -> value.id ( ).to_long ( ) == uri_grammar_user_id ) {
		u.setUser ( unEscape ( j -> value.begin ( ), j -> value.end ( ) ) );
		++ j;
	}
	if ( j -> value.id ( ).to_long ( ) == uri_grammar_password_id ) {
		u.setPassword ( unEscape ( j -> value.begin ( ), j -> value.end ( ) ) );
		++ j;
	}
	if ( j -> value.id ( ).to_long ( ) == uri_grammar_host_id ) {
		u.setHost ( j -> value.begin ( ), j -> value.end ( ) );
		++ j;
	}
	if ( j != i -> children.end ( ) && j -> value.id ( ).to_long ( ) == uri_grammar_port_id ) {
		u.setPort ( std :: atoi ( ss :: string ( j -> value.begin ( ), j -> value.end ( ) ).c_str ( ) ) );
		++ j;
	}
	while ( j != i -> children.end ( ) && j -> value.id ( ).to_long ( ) == uri_grammar_uriParameter_id ) {
		RightParser :: iter_t k = j -> children.begin ( );
		ss :: string pname ( unEscape ( k -> value.begin ( ), k -> value.end ( ) ) );
		boost :: algorithm :: to_lower ( pname );
		ss :: string pvalue;
		if ( ++ k != j -> children.end ( ) ) {
			pvalue = unEscape ( k -> value.begin ( ), k -> value.end ( ) );
			boost :: algorithm :: to_lower ( pvalue );
		}
		u.setParameter ( pname, pvalue );
		++ j;
	}
	while ( j != i -> children.end ( ) && j -> value.id ( ).to_long ( ) == uri_grammar_header_id ) {
		RightParser :: iter_t k = j -> children.begin ( );
		ss :: string hname ( unEscape ( k -> value.begin ( ), k -> value.end ( ) ) );
		boost :: algorithm :: to_lower ( hname );
		ss :: string hvalue;
		if ( ++ k != j -> children.end ( ) )
			hvalue = unEscape ( k -> value.begin ( ), k -> value.end ( ) );
		u.addHeader ( hname, hvalue );
		++ j;
	}
}

URI :: URI ( const ss :: string & s ) : impl ( new Impl ) {
	try {
		RightParser :: result_t info = parseUri ( s );
		if ( ! info.full )
			throw std :: runtime_error ( std :: string ( "error parsing uri: " ).append ( s.begin ( ), s.end ( ) ) );
		RightParser :: iter_t nu = info.trees.begin ( );
		parseUri ( * this, nu );
	} catch ( ... ) {
		delete impl;
		throw;
	}
}

void URI :: printOn ( std :: ostream & os ) const {
	impl -> printOn ( os );
}

void URI :: setUser ( const ss :: string & u ) {
	impl -> setUser ( u );
}

void URI :: setUser ( const ss :: string :: const_iterator & b, const ss :: string :: const_iterator & e ) {
	impl -> setUser ( b, e );
}

const ss :: string & URI :: getUser ( ) const {
	return impl -> getUser ( );
}

void URI :: setPassword ( const ss :: string & p ) {
	impl -> setPassword ( p );
}

void URI :: setPassword ( const ss :: string :: const_iterator & b, const ss :: string :: const_iterator & e ) {
	impl -> setPassword ( b, e );
}

void URI :: setHost ( const ss :: string & h ) {
	impl -> setHost ( h );
}

void URI :: setHost ( const ss :: string :: const_iterator & b, const ss :: string :: const_iterator & e ) {
	impl -> setHost ( b, e );
}

const ss :: string & URI :: getHost ( ) const {
	return impl -> getHost ( );
}

void URI :: setPort ( unsigned p ) {
	impl -> setPort ( p );
}

unsigned URI :: getPort ( ) const {
	return impl -> getPort ( );
}

void URI :: setParameter ( const ss :: string & pname, const ss :: string & pval ) {
	impl -> setParameter ( pname, pval );
}

bool URI :: hasParameter ( const ss :: string & n ) const {
	return impl -> hasParameter ( n );
}

void URI :: delParameter ( const ss :: string & pname ) {
	impl -> delParameter ( pname );
}

void URI :: addHeader ( const ss :: string hname, const ss :: string & hval ) {
	impl -> addHeader ( hname, hval );
}

void URI :: clearHeaders ( ) {
	impl -> clearHeaders ( );
}

class RouteHeader :: Impl : public Allocatable < __SS_ALLOCATOR > {
	URI uri;
	ss :: string displayName;
	StringStringMap unknownParameters;
public:
	Impl ( ) { }
	void printOn ( std :: ostream & os ) const;
	void setUri ( const URI & u ) {
		uri = u;
	}
	const URI & getUri ( ) const {
		return uri;
	}
	void setDisplayName ( const ss :: string & n ) {
		displayName = n;
	}
	void setUnknownParameter ( const ss :: string & pname, const ss :: string & pvalue ) {
		unknownParameters [ pname ] = pvalue;
	}
};

void RouteHeader :: Impl :: printOn ( std :: ostream & os ) const {
	if ( ! displayName.empty ( ) )
		os << displayName << ' ';
	os << '<' << uri << '>';
	for ( StringStringMap :: const_iterator i = unknownParameters.begin ( ); i != unknownParameters.end ( ); ++ i ) {
		os << ';' << i -> first;
		if ( ! i -> second.empty ( ) )
			os << '=' << i -> second;
	}
}

RouteHeader :: RouteHeader ( ) : impl ( new Impl ) { }

RouteHeader :: ~RouteHeader ( ) {
	delete impl;
}

RouteHeader :: RouteHeader ( const RouteHeader & n ) : impl ( new Impl ( * n.impl ) ) { }

void RouteHeader :: printOn ( std :: ostream & os ) const {
	impl -> printOn ( os );
}

const URI & RouteHeader :: getUri ( ) const {
	return impl -> getUri ( );
}

void RouteHeader :: setUri ( const URI & u ) {
	impl -> setUri ( u );
}

void RouteHeader :: setDisplayName ( const ss :: string & n ) {
	impl -> setDisplayName ( n );
}

void RouteHeader :: setUnknownParameter ( const ss :: string & pname, const ss :: string & pvalue ) {
	impl -> setUnknownParameter ( pname, pvalue );
}

class FromToHeader :: Impl : public Allocatable < __SS_ALLOCATOR > {
	URI uri;
	ss :: string displayName;
	ss :: string tag;
	StringStringMap unknownParameters;
	void cleanUri ( );
public:
	explicit Impl ( const ss :: string & s );
	Impl ( ) { }
	void printOn ( std :: ostream & os ) const;
	void setUri ( const URI & u ) {
		uri = u;
		cleanUri ( );
	}
	const URI & getUri ( ) const {
		return uri;
	}
	void setDisplayName ( const ss :: string & n ) {
		displayName = n;
	}
	void setTag ( const ss :: string & t ) {
		tag = t;
	}
	const ss :: string & getTag ( ) const {
		return tag;
	}
	void setUnknownParameter ( const ss :: string & pname, const ss :: string & pvalue ) {
		unknownParameters [ pname ] = pvalue;
	}
};

void FromToHeader :: Impl :: cleanUri ( ) {
	uri.setPort ( 0 );
	uri.delParameter ( "lr" );
	uri.delParameter ( "transport" );
	uri.delParameter ( "ttl" );
	uri.delParameter ( "maddr" );
	uri.clearHeaders ( );
}

FromToHeader :: Impl :: Impl ( const ss :: string & s ) {
	RightParser :: result_t info = parseFromToContactHeader ( s );
	if ( ! info.full )
		throw std :: runtime_error ( std :: string ( "error parsing fromtoheader: " ).append ( s.begin ( ), s.end ( ) ) );
	RightParser :: iter_t nu = info.trees.begin ( );
	RightParser :: iter_t i = nu -> children.begin ( );
	if ( i -> value.id ( ).to_long ( ) != fromtocontactheader_grammar_uri_id ) {
		displayName.assign ( i -> value.begin ( ), i -> value.end ( ) );
		++ i;
	}
	parseUri ( uri, i );
	cleanUri ( );
	while ( ++ i != nu -> children.end ( ) ) {
		RightParser :: iter_t j = i -> children.begin ( );
		ss :: string pname ( j -> value.begin ( ), j -> value.end ( ) );
		boost :: algorithm :: to_lower ( pname );
		ss :: string pvalue;
		if ( ++ j != i -> children.end ( ) )
			pvalue.assign ( j -> value.begin ( ), j -> value.end ( ) );
		if ( pname == "tag" )
			tag.swap ( pvalue );
		else
			unknownParameters [ pname ].swap ( pvalue );
	}
}

void FromToHeader :: Impl :: printOn ( std :: ostream & os ) const {
	if ( ! displayName.empty ( ) )
		os << displayName << ' ';
	os << '<' << uri << '>';
	if ( ! tag.empty ( ) )
		os << ";tag=" << tag;
	for ( StringStringMap :: const_iterator i = unknownParameters.begin ( ); i != unknownParameters.end ( ); ++ i ) {
		os << ';' << i -> first;
		if ( ! i -> second.empty ( ) )
			os << '=' << i -> second;
	}
}

FromToHeader :: FromToHeader ( ) : impl ( new Impl ) { }

FromToHeader :: FromToHeader ( const ss :: string & s ) : impl ( new Impl ( s ) ) { }

FromToHeader :: ~FromToHeader ( ) {
	delete impl;
}

FromToHeader :: FromToHeader ( const FromToHeader & n ) : impl ( new Impl ( * n.impl ) ) { }

void FromToHeader :: printOn ( std :: ostream & os ) const {
	impl -> printOn ( os );
}

const URI & FromToHeader :: getUri ( ) const {
	return impl -> getUri ( );
}

void FromToHeader :: setUri ( const URI & u ) {
	impl -> setUri ( u );
}

void FromToHeader :: setDisplayName ( const ss :: string & n ) {
	impl -> setDisplayName ( n );
}

void FromToHeader :: setTag ( const ss :: string & t ) {
	impl -> setTag ( t );
}

const ss :: string & FromToHeader :: getTag ( ) const {
	return impl -> getTag ( );
}

void FromToHeader :: setUnknownParameter ( const ss :: string & pname, const ss :: string & pvalue ) {
	impl -> setUnknownParameter ( pname, pvalue );
}

class ContactHeader :: Impl : public Allocatable < __SS_ALLOCATOR > {
	URI uri;
	ss :: string displayName;
	ss :: string q;
	boost :: optional < unsigned > expires;
	StringStringMap unknownParameters;
public:
	explicit Impl ( const ss :: string & s );
	Impl ( ) { }
	void printOn ( std :: ostream & os ) const;
	void setUri ( const URI & u ) {
		uri = u;
	}
	const URI & getUri ( ) const {
		return uri;
	}
	void setDisplayName ( const ss :: string & n ) {
		displayName = n;
	}
	void setQ ( const ss :: string & t ) {
		q = t;
	}
	void setExpires ( unsigned e ) {
		expires = e;
	}
	bool hasExpires ( ) const {
		return expires;
	}
	unsigned getExpires ( ) const {
		return * expires;
	}
	void setUnknownParameter ( const ss :: string & pname, const ss :: string & pvalue ) {
		unknownParameters [ pname ] = pvalue;
	}
};

ContactHeader :: Impl :: Impl ( const ss :: string & s ) {
	RightParser :: result_t info = parseFromToContactHeader ( s );
	if ( ! info.full )
		throw std :: runtime_error ( std :: string ( "error parsing contactheader: " ).append ( s.begin ( ), s.end ( ) ) );
	RightParser :: iter_t nu = info.trees.begin ( );
	RightParser :: iter_t i = nu -> children.begin ( );
	if ( i -> value.id ( ).to_long ( ) != fromtocontactheader_grammar_uri_id ) {
		displayName.assign ( i -> value.begin ( ), i -> value.end ( ) );
		++ i;
	}
	parseUri ( uri, i );
	while ( ++ i != nu -> children.end ( ) ) {
		RightParser :: iter_t j = i -> children.begin ( );
		ss :: string pname ( j -> value.begin ( ), j -> value.end ( ) );
		boost :: algorithm :: to_lower ( pname );
		ss :: string pvalue;
		if ( ++ j != i -> children.end ( ) )
			pvalue.assign ( j -> value.begin ( ), j -> value.end ( ) );
		if ( pname == "q" )
			q.swap ( pvalue );
		else if ( pname == "expires" )
			expires = std :: atoi ( pvalue.c_str ( ) );
		else
			unknownParameters [ pname ].swap ( pvalue );
	}
}

void ContactHeader :: Impl :: printOn ( std :: ostream & os ) const {
	if ( ! displayName.empty ( ) )
		os << displayName << ' ';
	os << '<' << uri << '>';
	if ( ! q.empty ( ) )
		os << ";q=" << q;
	if ( expires )
		os << ";expires=" << * expires;
	for ( StringStringMap :: const_iterator i = unknownParameters.begin ( ); i != unknownParameters.end ( ); ++ i ) {
		os << ';' << i -> first;
		if ( ! i -> second.empty ( ) )
			os << '=' << i -> second;
	}
}

ContactHeader :: ContactHeader ( ) : impl ( new Impl ) { }

ContactHeader :: ContactHeader ( const ss :: string & s ) : impl ( new Impl ( s ) ) { }

ContactHeader :: ~ContactHeader ( ) {
	delete impl;
}

ContactHeader :: ContactHeader ( const ContactHeader & n ) : impl ( new Impl ( * n.impl ) ) { }

void ContactHeader :: printOn ( std :: ostream & os ) const {
	impl -> printOn ( os );
}

const URI & ContactHeader :: getUri ( ) const {
	return impl -> getUri ( );
}

void ContactHeader :: setUri ( const URI & u ) {
	impl -> setUri ( u );
}

void ContactHeader :: setDisplayName ( const ss :: string & n ) {
	impl -> setDisplayName ( n );
}

void ContactHeader :: setQ ( const ss :: string & t ) {
	impl -> setQ ( t );
}

void ContactHeader :: setExpires ( unsigned e ) {
	impl -> setExpires ( e );
}

bool ContactHeader :: hasExpires ( ) const {
	return impl -> hasExpires ( );
}

unsigned ContactHeader :: getExpires ( ) const {
	return impl -> getExpires ( );
}

void ContactHeader :: setUnknownParameter ( const ss :: string & pname, const ss :: string & pvalue ) {
	impl -> setUnknownParameter ( pname, pvalue );
}

class Via :: Impl : public Allocatable < __SS_ALLOCATOR > {
	ss :: string transport;
	ss :: string host;
	ss :: string branch;
	ss :: string received;
	StringStringMap unknownParameters;
	boost :: optional < int > rport; // rfc3581
	int port;
public:
	Impl ( ) : port ( 0 ) { }
	void printOn ( std :: ostream & os ) const;
	void setTransport ( const ss :: string & t ) {
		transport = t;
	}
	void setHost ( const ss :: string & h ) {
		host = h;
	}
	const ss :: string & getHost ( ) const {
		return host;
	}
	void setPort ( int p ) {
		port = p;
	}
	int getPort ( ) const {
		return port;
	}
	void setRport ( int p ) {
		rport = p;
	}
	const boost :: optional < int > & getRport ( ) const {
		return rport;
	}
	void setBranch ( const ss :: string & b ) {
		branch = b;
	}
	const ss :: string & getBranch ( ) const {
		return branch;
	}
	void setReceived ( const ss :: string & r ) {
		received = r;
	}
	const ss :: string & getReceived ( ) const {
		return received;
	}
	void setUnknownParameter ( const ss :: string & pname, const ss :: string & pvalue ) {
		unknownParameters [ pname ] = pvalue;
	}
};

void Via :: Impl :: printOn ( std :: ostream & os ) const {
	os << "SIP/2.0/" << transport << ' ' << host;
	if ( port )
		os << ':' << port;
	os << ";branch=" << branch;
	if ( ! received.empty ( ) )
		os << ";received=" << received;
	if ( rport ) {
		os << ";rport";
		if ( int p = * rport )
			os << '=' << p;
	}
	for ( StringStringMap :: const_iterator i = unknownParameters.begin ( ); i != unknownParameters.end ( ); ++ i ) {
		os << ';' << i -> first;
		if ( ! i -> second.empty ( ) )
			os << '=' << i -> second;
	}
}

Via :: Via ( const Via & v ) : impl ( new Impl ( * v.impl ) ) { }

Via :: ~Via ( ) {
	delete impl;
}

Via :: Via ( ) : impl ( new Impl ) { }

void Via :: printOn ( std :: ostream & os ) const {
	impl -> printOn ( os );
}

void Via :: setTransport ( const ss :: string & t ) {
	impl -> setTransport ( t );
}

void Via :: setHost ( const ss :: string & h ) {
	impl -> setHost ( h );
}

const ss :: string & Via :: getHost ( ) const {
	return impl -> getHost ( );
}

void Via :: setPort ( int p ) {
	impl -> setPort ( p );
}

int Via :: getPort ( ) const {
	return impl -> getPort ( );
}

void Via :: setRport ( int p ) {
	impl -> setRport ( p );
}

const boost :: optional < int > & Via :: getRport ( ) const {
	return impl -> getRport ( );
}

void Via :: setBranch ( const ss :: string & b ) {
	impl -> setBranch ( b );
}

const ss :: string & Via :: getBranch ( ) const {
	return impl -> getBranch ( );
}

void Via :: setReceived ( const ss :: string & r ) {
	impl -> setReceived ( r );
}

const ss :: string & Via :: getReceived ( ) const {
	return impl -> getReceived ( );
}

void Via :: setUnknownParameter ( const ss :: string & pname, const ss :: string & pvalue ) {
	impl -> setUnknownParameter ( pname, pvalue );
}

class DigestChallenge :: Impl : public Allocatable < __SS_ALLOCATOR > {
	ss :: string realm;
	ss :: string nonce;
	ss :: string opaque;
	ss :: string algorithm;
	StringSet qop;
	bool stale;
public:
	explicit Impl ( const ss :: string & s );
	Impl ( const ss :: string & realm, const ss :: string & nonce, const StringSet & qop );
	void printOn ( std :: ostream & os ) const;
	const ss :: string & getAlgorithm ( ) const {
		return algorithm;
	}
	void setRealm ( const ss :: string & r ) {
		realm = r;
	}
	const ss :: string & getRealm ( ) const {
		return realm;
	}
	void setNonce ( const ss :: string & n ) {
		nonce = n;
	}
	const ss :: string & getNonce ( ) const {
		return nonce;
	}
	const ss :: string & getOpaque ( ) const {
		return opaque;
	}
	bool hasQop ( const ss :: string & q ) const {
		return qop.count ( q );
	}
	bool emptyQop ( ) const {
		return qop.empty ( );
	}
	void setStale ( bool s ) {
		stale = s;
	}
};

DigestChallenge :: Impl :: Impl ( const ss :: string & realm, const ss :: string & nonce, const StringSet & qop ) :
	realm ( realm ), nonce ( nonce ), algorithm ( "MD5" ), qop ( qop ), stale ( false ) { }

static ss :: string unQuote ( const ss :: string & s ) {
	if ( s.size ( ) < 2 )
		throw std :: range_error ( "too short quoted string");
	if ( s [ 0 ] != '"' )
		throw std :: runtime_error ( "no quote in quotedString" );
	ss :: string t;
	t.reserve ( s.size ( ) - 2 );
	for ( ss :: string :: size_type i = 1, ss = s.size ( ) - 1; i < ss; i ++ ) {
		char c = s [ i ];
		if ( c == '\\' )
			c = s [ ++ i ];
		t.push_back ( c );
	}
	return t;
}

DigestChallenge :: Impl :: Impl ( const ss :: string & s ) : stale ( false ) {
	RightParser :: result_t info = parseDigestChallenge ( s );
	if ( ! info.full )
		throw std :: runtime_error ( std :: string ( "error parsing digest challenge: " ).append ( s.begin ( ), s.end ( ) ) );
	RightParser :: iter_t nu = info.trees.begin ( );
	for ( RightParser :: iter_t i = nu -> children.begin ( ); i != nu -> children.end ( ); ++ i ) {
		ss :: string pname ( i -> value.begin ( ), i -> value.end ( ) );
		boost :: algorithm :: to_lower ( pname );
		++ i;
		ss :: string pvalue ( i -> value.begin ( ), i -> value.end ( ) );
		if ( pname == "realm" )
			unQuote ( pvalue ).swap ( realm );
		else if ( pname == "nonce" )
			unQuote ( pvalue ).swap ( nonce );
		else if ( pname == "opaque" )
			unQuote ( pvalue ).swap ( opaque );
		else if ( pname == "algorithm" )
			algorithm.swap ( pvalue );
		else if ( pname == "qop" ) {
			ss :: string t = unQuote ( pvalue );
			typedef boost :: tokenizer < boost :: char_separator < char >,
				ss :: string :: const_iterator, ss :: string > Tok;
			boost :: char_separator < char > sep ( "," );
			Tok tok ( t.begin ( ), t.end ( ), sep );
			std :: copy ( tok.begin ( ), tok.end ( ), std :: inserter ( qop, qop.begin ( ) ) );
		} else if ( pname == "stale" )
			stale = boost :: algorithm :: iequals ( pvalue, "true" );
	}
}

static void quote ( std :: ostream & os, const ss :: string & s ) {
	os << '"';
	for ( ss :: string :: size_type i = 0, ss = s.size ( ); i < ss; i ++ ) {
		switch ( s [ i ] ) {
			case '\\':
			case '"':
				os << '\\';
		}
		os << s [ i ];
	}
	os << '"';
}

void DigestChallenge :: Impl :: printOn ( std :: ostream & os ) const {
	quote ( os << "Digest realm=", realm );
	if ( ! nonce.empty ( ) )
		quote ( os << ",nonce=", nonce );
	if ( ! opaque.empty ( ) )
		quote ( os << ",opaque=", opaque );
	if ( ! algorithm.empty ( ) )
		os << ",algorithm=" << algorithm;
	if ( ! qop.empty ( ) ) {
		ss :: ostringstream os2;
		bool first = true;
		for ( StringSet :: const_iterator i = qop.begin ( ); i != qop.end ( ); ++ i ) {
			if ( first )
				first = false;
			else
				os2 << ',';
			os2 << * i;
		}
		quote ( os << ",qop=", os2.str ( ) );
	}
	if ( stale )
		os << ",stale=true";
}

DigestChallenge :: DigestChallenge ( const ss :: string & realm, const ss :: string & nonce, const StringSet & qop ) :
	impl ( new Impl ( realm, nonce, qop ) ) { }

DigestChallenge :: DigestChallenge ( const DigestChallenge & v ) : impl ( new Impl ( * v.impl ) ) { }

DigestChallenge :: ~DigestChallenge ( ) {
	delete impl;
}

DigestChallenge :: DigestChallenge ( const ss :: string & s ) : impl ( new Impl ( s ) ) { }

void DigestChallenge :: printOn ( std :: ostream & os ) const {
	impl -> printOn ( os );
}

const ss :: string & DigestChallenge :: getAlgorithm ( ) const {
	return impl -> getAlgorithm ( );
}

void DigestChallenge :: setRealm ( const ss :: string & r ) {
	impl -> setRealm ( r );
}

const ss :: string & DigestChallenge :: getRealm ( ) const {
	return impl -> getRealm ( );
}

void DigestChallenge :: setNonce ( const ss :: string & n ) {
	impl -> setNonce ( n );
}

const ss :: string & DigestChallenge :: getNonce ( ) const {
	return impl -> getNonce ( );
}

const ss :: string & DigestChallenge :: getOpaque ( ) const {
	return impl -> getOpaque ( );
}

bool DigestChallenge :: hasQop ( const ss :: string & q ) const {
	return impl -> hasQop ( q );
}

bool DigestChallenge :: emptyQop ( ) const {
	return impl -> emptyQop ( );
}

void DigestChallenge :: setStale ( bool s ) {
	impl -> setStale ( s );
}

class DigestResponse :: Impl : public Allocatable < __SS_ALLOCATOR > {
	ss :: string realm;
	ss :: string username;
	ss :: string nonce;
	ss :: string cnonce;
	URI uri;
	ss :: string response;
	ss :: string algorithm;
	ss :: string opaque;
	ss :: string qop;
	unsigned nc;
	ss :: string a2 ( const Request & r ) const;
public:
	explicit Impl ( const ss :: string & s );
	Impl ( const DigestChallenge & challenge, const Request & r, unsigned nc, const ss :: string & username, const ss :: string & pass );
	void printOn ( std :: ostream & os ) const;
	ss :: string calcDigest ( const ss :: string & pass, const Request & r ) const;
	ss :: string hash ( const ss :: string & s ) const;
	ss :: string a1 ( const ss :: string & pass ) const;
	const ss :: string & getUsername ( ) const {
		return username;
	}
	const ss :: string & getRealm ( ) const {
		return realm;
	}
	const ss :: string & getNonce ( ) const {
		return nonce;
	}
	const ss :: string & getCnonce ( ) const {
		return cnonce;
	}
	const URI & getUri ( ) const {
		return uri;
	}
	const ss :: string & getQop ( ) const {
		return qop;
	}
	unsigned getNc ( ) const {
		return nc;
	}
	const ss :: string & getResponse ( ) const {
		return response;
	}
};

ss :: string DigestResponse :: Impl :: hash ( const ss :: string & s ) const {
	if ( ! algorithm.empty ( ) && ! boost :: algorithm :: iequals ( algorithm, "MD5" ) &&
		! boost :: algorithm :: iequals ( algorithm, "MD5-sess" ) )
		throw std :: runtime_error ( "unsupported algorithm" );
	return toHex ( md5Sum ( s ) );
}

ss :: string methodName ( Request :: Methods m ) {
	switch ( m ) {
		case Request :: mInvite:
			return "INVITE";
		case Request :: mAck:
			return "ACK";
		case Request :: mOptions:
			return "OPTIONS";
		case Request :: mBye:
			return "BYE";
		case Request :: mCancel:
			return "CANCEL";
		case Request :: mRegister:
			return "REGISTER";
		case Request :: mRefer:
			return "REFER";
		case Request :: mMessage:
			return "MESSAGE";
		case Request :: mNotify:
			return "NOTIFY";
		case Request :: mSubscribe:
			return "SUBSCRIBE";
		case Request :: mInfo:
			return "INFO";
		case Request :: mUpdate:
			return "UPDATE";
		case Request :: mPrack:
			return "PRACK";
		case Request :: mPublish:
			return "PUBLISH";
		default:
			throw std :: runtime_error ( std :: string ( "unknown sip method" ) );
	}
}

ss :: string DigestResponse :: Impl :: a2 ( const Request & r ) const {
	ss :: ostringstream os;
	os << methodName ( r.getMethod ( ) ) << ':' << uri;
	if ( boost :: algorithm :: iequals ( qop, "auth-int" ) ) {
		os << ':';
		const ss :: string & orig = r.getOrigBodyMd5 ( );
		if ( ! orig.empty ( ) )
			os << toHex ( orig );
		else {
			ss :: ostringstream os2;
			r.printBody ( os2 );
			os << toHex ( md5Sum ( os2.str ( ) ) );
		}
	}
	return os.str ( );
}

ss :: string DigestResponse :: Impl :: a1 ( const ss :: string & pass ) const {
	ss :: ostringstream os;
	if ( algorithm.empty ( ) || boost :: algorithm :: iequals ( algorithm, "MD5" ) )
		os << username << ':' << realm << ':' << pass;
	else if ( boost :: algorithm :: iequals ( algorithm, "MD5-sess" ) ) {
		os << username << ':' << realm << ':' << pass;
		os.str ( hash ( os.str ( ) ) );
		os << ':' << nonce << ':' << cnonce;
	} else
		throw std :: runtime_error ( "unsupported algorithm" );
	return os.str ( );
}

ss :: string DigestResponse :: Impl :: calcDigest ( const ss :: string & pass, const Request & r ) const {
	ss :: ostringstream os;
	os << hash ( a1 ( pass ) ) << ':' << nonce << ':';
	if ( ! qop.empty ( ) )
		os << std :: hex << std :: setfill ( '0' ) << std :: setw ( 8 ) << nc << ':' << cnonce << ':' << qop << ':';
	os << hash ( a2 ( r ) );
	return toHex ( md5Sum ( os.str ( ) ) );
}

DigestResponse :: Impl :: Impl ( const ss :: string & s ) : nc ( 0 ) {
	RightParser :: result_t info = parseDigestChallenge ( s );
	if ( ! info.full )
		throw std :: runtime_error ( std :: string ( "error parsing digest challenge: " ).append ( s.begin ( ), s.end ( ) ) );
	RightParser :: iter_t nu = info.trees.begin ( );
	for ( RightParser :: iter_t i = nu -> children.begin ( ); i != nu -> children.end ( ); ++ i ) {
		ss :: string pname ( i -> value.begin ( ), i -> value.end ( ) );
		boost :: algorithm :: to_lower ( pname );
		++ i;
		ss :: string pvalue ( i -> value.begin ( ), i -> value.end ( ) );
		if ( pname == "realm" )
			unQuote ( pvalue ).swap ( realm );
		else if ( pname == "username" )
			unQuote ( pvalue ).swap ( username );
		else if ( pname == "nonce" )
			unQuote ( pvalue ).swap ( nonce );
		else if ( pname == "cnonce" )
			unQuote ( pvalue ).swap ( cnonce );
		else if ( pname == "uri" )
			URI ( unQuote ( pvalue ) ).swap ( uri );
		else if ( pname == "response" )
			unQuote ( pvalue ).swap ( response );
		else if ( pname == "algorithm" )
			algorithm.swap ( pvalue );
		else if ( pname == "opaque" )
			unQuote ( pvalue ).swap ( opaque );
		else if ( pname == "qop" ) {
			if ( ! pvalue.empty ( ) && pvalue [ 0 ] == '"' )
				unQuote ( pvalue ).swap ( qop ); // buggy devices send quoted qop in responses
			else
				qop.swap ( pvalue );
		} else if ( pname == "nc" )
			nc = unsigned ( std :: strtol ( pvalue.c_str ( ), 0, 16 ) );
	}
}

static ss :: string makeCnonce ( ) {
	ss :: ostringstream os;
	os << std :: setw ( 8 ) << std :: setfill ( '0' ) << std :: hex << Random :: number ( );
	return os.str ( );
}

DigestResponse :: Impl :: Impl ( const DigestChallenge & challenge, const Request & r, unsigned nc, const ss :: string & username,
	const ss :: string & pass ) : realm ( challenge.getRealm ( ) ), username ( username ), nonce ( challenge.getNonce ( ) ),
	cnonce ( challenge.emptyQop ( ) ? ss :: string ( ) : makeCnonce ( ) ),
	uri ( r.getRequestUri ( ) ), algorithm ( challenge.getAlgorithm ( ) ), opaque ( challenge.getOpaque ( ) ), nc ( nc ) {
	if ( challenge.hasQop ( "auth-int" ) )
		qop = "auth-int";
	else if ( challenge.hasQop ( "auth" ) )
		qop = "auth";
	else if ( ! challenge.emptyQop ( ) )
		throw std :: runtime_error ( std :: string ( "unsupported qop" ) );
	response = calcDigest ( pass, r );
}

void DigestResponse :: Impl :: printOn ( std :: ostream & os ) const {
	quote ( os << "Digest realm=", realm );
	if ( ! username.empty ( ) )
		quote ( os << ",username=", username );
	if ( ! nonce.empty ( ) )
		quote ( os << ",nonce=", nonce );
	if ( ! cnonce.empty ( ) )
		quote ( os << ",cnonce=", cnonce );
	if ( ! uri.getHost ( ).empty ( ) )
		os << ",uri=\"" << uri << '"';
	if ( ! response.empty ( ) )
		quote ( os << ",response=", response );
	if ( ! algorithm.empty ( ) )
		os << ",algorithm=" << algorithm;
	if ( ! opaque.empty ( ) )
		quote ( os << ",opaque=", opaque );
	if ( ! qop.empty ( ) )
		os << ",qop=" << qop;
	if ( nc ) {
		boost :: io :: ios_flags_saver ifs ( os );
		os << ",nc=" << std :: hex << std :: setfill ( '0' ) << std :: setw ( 8 ) << nc;
	}
}

DigestResponse :: DigestResponse ( const DigestResponse & v ) : impl ( new Impl ( * v.impl ) ) { }

DigestResponse :: ~DigestResponse ( ) {
	delete impl;
}

DigestResponse :: DigestResponse ( const ss :: string & s ) : impl ( new Impl ( s ) ) { }

DigestResponse :: DigestResponse ( const DigestChallenge & challenge, const Request & r, unsigned nc, const ss :: string & username,
	const ss :: string & pass ) : impl ( new Impl ( challenge, r, nc, username, pass ) ) { }

void DigestResponse :: printOn ( std :: ostream & os ) const {
	impl -> printOn ( os );
}

ss :: string DigestResponse :: calcDigest ( const ss :: string & pass, const Request & r ) const {
	return impl -> calcDigest ( pass, r );
}

ss :: string DigestResponse :: hash ( const ss :: string & s ) const {
	return impl -> hash ( s );
}

ss :: string DigestResponse :: a1 ( const ss :: string & pass ) const {
	return impl -> a1 ( pass );
}

const ss :: string & DigestResponse :: getUsername ( ) const {
	return impl -> getUsername ( );
}

const ss :: string & DigestResponse :: getRealm ( ) const {
	return impl -> getRealm ( );
}

const ss :: string & DigestResponse :: getNonce ( ) const {
	return impl -> getNonce ( );
}

const ss :: string & DigestResponse :: getCnonce ( ) const {
	return impl -> getCnonce ( );
}

const URI & DigestResponse :: getUri ( ) const {
	return impl -> getUri ( );
}

const ss :: string & DigestResponse :: getQop ( ) const {
	return impl -> getQop ( );
}

unsigned DigestResponse :: getNc ( ) const {
	return impl -> getNc ( );
}

const ss :: string & DigestResponse :: getResponse ( ) const {
	return impl -> getResponse ( );
}

class AuthenticationInfo :: Impl : public Allocatable < __SS_ALLOCATOR > {
	ss :: string nextnonce;
	ss :: string qop;
	ss :: string cnonce;
	unsigned nc;
	ss :: string rspauth;
	ss :: string a2 ( const Response & r, const DigestResponse & d ) const;
public:
	Impl ( const DigestResponse & d, const Response & r, const ss :: string & pass ) : qop ( d.getQop ( ) ),
		cnonce ( d.getCnonce ( ) ), nc ( d.getNc ( ) ), rspauth ( calcDigest ( pass, r, d ) ) { }
	explicit Impl ( const ss :: string & s );
	void printOn ( std :: ostream & os ) const;
	ss :: string calcDigest ( const ss :: string & pass, const Response & r, const DigestResponse & d ) const;
	void setNextnonce ( const ss :: string & s ) {
		nextnonce = s;
	}
	const ss :: string & getNextnonce ( ) const {
		return nextnonce;
	}
	void setQop ( const ss :: string & s ) {
		qop = s;
	}
	void setRspauth ( const ss :: string & s ) {
		rspauth = s;
	}
	const ss :: string & getRspauth ( ) const {
		return rspauth;
	}
	void setCnonce ( const ss :: string & s ) {
		cnonce = s;
	}
	const ss :: string & getCnonce ( ) const {
		return cnonce;
	}
	void setNc ( unsigned s ) {
		nc = s;
	}
	unsigned getNc ( ) const {
		return nc;
	}
};

AuthenticationInfo :: Impl :: Impl ( const ss :: string & s ) : nc ( 0 ) {
	RightParser :: result_t info = parseAuthenticationInfo ( s );
	if ( ! info.full )
		throw std :: runtime_error ( std :: string ( "error parsing auth info: " ).append ( s.begin ( ), s.end ( ) ) );
	RightParser :: iter_t nu = info.trees.begin ( );
	for ( RightParser :: iter_t i = nu -> children.begin ( ); i != nu -> children.end ( ); ++ i ) {
		ss :: string pname ( i -> value.begin ( ), i -> value.end ( ) );
		boost :: algorithm :: to_lower ( pname );
		++ i;
		ss :: string pvalue ( i -> value.begin ( ), i -> value.end ( ) );
		if ( pname == "cnonce" )
			unQuote ( pvalue ).swap ( cnonce );
		else if ( pname == "rspauth" )
			unQuote ( pvalue ).swap ( rspauth );
		else if ( pname == "nextnonce" )
			unQuote ( pvalue ).swap ( nextnonce );
		else if ( pname == "qop" )
			qop.swap ( pvalue );
		else if ( pname == "nc" )
			nc = unsigned ( std :: strtol ( pvalue.c_str ( ), 0, 16 ) );
	}
}

void AuthenticationInfo :: Impl :: printOn ( std :: ostream & os ) const {
	bool first = true;
	if ( ! nextnonce.empty ( ) ) {
		quote ( os << "nextnonce=", nextnonce );
		first = false;
	}
	if ( ! qop.empty ( ) ) {
		if ( ! first )
			os << ',';
		first = false;
		os << "qop=" << qop;
	}
	if ( ! rspauth.empty ( ) ) {
		if ( ! first )
			os << ',';
		first = false;
		quote ( os << "rspauth=", rspauth );
	}
	if ( ! cnonce.empty ( ) ) {
		if ( ! first )
			os << ',';
		first = false;
		quote ( os << "cnonce=", cnonce );
	}
	if ( nc ) {
		if ( ! first )
			os << ',';
		boost :: io :: ios_flags_saver ifs ( os );
		os << "nc=" << std :: hex << std :: setfill ( '0' ) << std :: setw ( 8 ) << nc;
	}
}

ss :: string AuthenticationInfo :: Impl :: a2 ( const Response & r, const DigestResponse & d ) const {
	ss :: ostringstream os;
	os << ':' << d.getUri ( );
	if ( boost :: algorithm :: iequals ( qop, "auth-int" ) ) {
		os << ':';
		const ss :: string & orig = r.getOrigBodyMd5 ( );
		if ( ! orig.empty ( ) )
			os << toHex ( orig );
		else {
			ss :: ostringstream os2;
			r.printBody ( os2 );
			os << toHex ( md5Sum ( os2.str ( ) ) );
		}
	}
	return os.str ( );
}

ss :: string AuthenticationInfo :: Impl :: calcDigest ( const ss :: string & pass, const Response & r,
	const DigestResponse & d ) const {
	ss :: ostringstream os;
	os << d.hash ( d.a1 ( pass ) ) << ':' << d.getNonce ( ) << ':';
	if ( ! qop.empty ( ) )
		os << std :: hex << std :: setfill ( '0' ) << std :: setw ( 8 ) << nc << ':' << cnonce << ':' << qop << ':';
	os << d.hash ( a2 ( r, d ) );
	return toHex ( md5Sum ( os.str ( ) ) );
}

AuthenticationInfo :: AuthenticationInfo ( const AuthenticationInfo & v ) : impl ( new Impl ( * v.impl ) ) { }

AuthenticationInfo :: ~AuthenticationInfo ( ) {
	delete impl;
}

AuthenticationInfo :: AuthenticationInfo ( const DigestResponse & d, const Response & r, const ss :: string & pass ) :
	impl ( new Impl ( d, r, pass ) ) { }

AuthenticationInfo :: AuthenticationInfo ( const ss :: string & s ) : impl ( new Impl ( s ) ) { }

void AuthenticationInfo :: printOn ( std :: ostream & os ) const {
	impl -> printOn ( os );
}

ss :: string AuthenticationInfo :: calcDigest ( const ss :: string & pass, const Response & r, const DigestResponse & d ) const {
	return impl -> calcDigest ( pass, r, d );
}

void AuthenticationInfo :: setNextnonce ( const ss :: string & s ) {
	impl -> setNextnonce ( s );
}

const ss :: string & AuthenticationInfo :: getNextnonce ( ) const {
	return impl -> getNextnonce ( );
}

void AuthenticationInfo :: setRspauth ( const ss :: string & s ) {
	impl -> setRspauth ( s );
}

const ss :: string & AuthenticationInfo :: getRspauth ( ) const {
	return impl -> getRspauth ( );
}

const ss :: string & AuthenticationInfo :: getCnonce ( ) const {
	return impl -> getCnonce ( );
}

unsigned AuthenticationInfo :: getNc ( ) const {
	return impl -> getNc ( );
}

class RetryAfter :: Impl : public Allocatable < __SS_ALLOCATOR > {
	unsigned seconds;
	unsigned duration;
	ss :: string comment;
public:
	explicit Impl ( const ss :: string & s );
	explicit Impl ( unsigned secs ) : seconds ( secs ), duration ( 0 ) { }
	void printOn ( std :: ostream & os ) const;
	unsigned getSeconds ( ) const {
		return seconds;
	}
	void setDuration ( unsigned dur ) {
		duration = dur;
	}
	unsigned getDuration ( ) const {
		return duration;
	}
};

RetryAfter :: Impl :: Impl ( const ss :: string & s ) : duration ( 0 ) {
	RightParser :: result_t info = parseRetryAfter ( s );
	if ( ! info.full )
		throw std :: runtime_error ( std :: string ( "error parsing retry-after: " ).append ( s.begin ( ), s.end ( ) ) );
	RightParser :: iter_t nu = info.trees.begin ( );
	RightParser :: iter_t i = nu -> children.begin ( );
	seconds = std :: atoi ( ss :: string ( i -> value.begin ( ), i -> value.end ( ) ).c_str ( ) );
	if ( ++ i == nu -> children.end ( ) )
		return;
	if ( i -> value.id ( ).to_long ( ) == retryAfter_grammar_comment_id ) {
		RightParser :: iter_t j = i -> children.begin ( );
		if ( j != i -> children.end ( ) )
			comment.assign ( j -> value.begin ( ), j -> value.end ( ) );
		++ i;
	}
	for ( ; i != nu -> children.end ( ); ++ i ) {
		RightParser :: iter_t j = i -> children.begin ( );
		ss :: string pname ( j -> value.begin ( ), j -> value.end ( ) );
		boost :: algorithm :: to_lower ( pname );
		ss :: string pvalue;
		if ( ++ j != i -> children.end ( ) )
			pvalue.assign ( j -> value.begin ( ), j -> value.end ( ) );
		if ( pname == "duration" ) {
			duration = std :: atoi ( pvalue.c_str ( ) );
			break;
		}
	}
}

void RetryAfter :: Impl :: printOn ( std :: ostream & os ) const {
	os << seconds;
	if ( ! comment.empty ( ) )
		os << '(' << comment << ')';
	if ( duration )
		os << ";duration=" << duration;
}

RetryAfter :: RetryAfter ( const RetryAfter & v ) : impl ( new Impl ( * v.impl ) ) { }

RetryAfter :: ~RetryAfter ( ) {
	delete impl;
}

RetryAfter :: RetryAfter ( const ss :: string & s ) : impl ( new Impl ( s ) ) { }

RetryAfter :: RetryAfter ( unsigned secs ) : impl ( new Impl ( secs ) ) { }

void RetryAfter :: printOn ( std :: ostream & os ) const {
	impl -> printOn ( os );
}

unsigned RetryAfter :: getSeconds ( ) const {
	return impl -> getSeconds ( );
}

void RetryAfter :: setDuration ( unsigned dur ) {
	impl -> setDuration ( dur );
}

unsigned RetryAfter :: getDuration ( ) const {
	return impl -> getDuration ( );
}

class Warning :: Impl : public Allocatable < __SS_ALLOCATOR > {
	unsigned code;
	ss :: string agent;
	ss :: string text;
public:
	Impl ( unsigned c, const ss :: string & a, const ss :: string & t ) : code ( c ), agent ( a ), text ( t ) {
		if ( code < 100 || code > 999 )
			throw std :: range_error ( "wrong warning code" );
		if ( agent.empty ( ) )
			throw std :: range_error ( "empty warning agent" );
	}
	void printOn ( std :: ostream & os ) const;
};

void Warning :: Impl :: printOn ( std :: ostream & os ) const {
	quote ( os << code << ' ' << agent << ' ', text );
}

Warning :: Warning ( const Warning & v ) : impl ( new Impl ( * v.impl ) ) { }

Warning :: ~Warning ( ) {
	delete impl;
}

Warning :: Warning ( unsigned c, const ss :: string & a, const ss :: string & t ) : impl ( new Impl ( c, a, t ) ) { }

void Warning :: printOn ( std :: ostream & os ) const {
	impl -> printOn ( os );
}

class SessionExpires :: Impl : public Allocatable < __SS_ALLOCATOR > {
	unsigned deltaSeconds;
	ss :: string refresher;
public:
	Impl ( unsigned d, const ss :: string & r ) : deltaSeconds ( d ), refresher ( r ) { }
	explicit Impl ( const ss :: string & s );
	void printOn ( std :: ostream & os ) const;
};

SessionExpires :: Impl :: Impl ( const ss :: string & s ) {
	RightParser :: result_t info = parseSessionExpires ( s );
	if ( ! info.full )
		throw std :: runtime_error ( std :: string ( "error parsing session-expires: " ).append ( s.begin ( ), s.end ( ) ) );
	RightParser :: iter_t nu = info.trees.begin ( );
	RightParser :: iter_t i = nu -> children.begin ( );
	deltaSeconds = std :: atoi ( ss :: string ( i -> value.begin ( ), i -> value.end ( ) ).c_str ( ) );
	for ( ++ i; i != nu -> children.end ( ); ++ i ) {
		RightParser :: iter_t j = i -> children.begin ( );
		ss :: string pname ( j -> value.begin ( ), j -> value.end ( ) );
		boost :: algorithm :: to_lower ( pname );
		ss :: string pvalue;
		if ( ++ j != i -> children.end ( ) ) {
			pvalue.assign ( j -> value.begin ( ), j -> value.end ( ) );
		}
		if ( pname == "refresher" ) {
			refresher.swap ( pvalue );
			boost :: algorithm :: to_lower ( refresher );
			break;
		}
	}
}

void SessionExpires :: Impl :: printOn ( std :: ostream & os ) const {
	os << deltaSeconds;
	if ( ! refresher.empty ( ) )
		os << ";refresher=" << refresher;
}

SessionExpires :: SessionExpires ( const SessionExpires & s ) : impl ( new Impl ( * s.impl ) ) { }

SessionExpires :: ~SessionExpires ( ) {
	delete impl;
}

SessionExpires :: SessionExpires ( unsigned d, const ss :: string & r ) : impl ( new Impl ( d, r ) ) { }

SessionExpires :: SessionExpires ( const ss :: string & s ) : impl ( new Impl ( s ) ) { }

void SessionExpires :: printOn ( std :: ostream & os ) const {
	impl -> printOn ( os );
}

class ContentType :: Impl : public Allocatable < __SS_ALLOCATOR > {
	ss :: string type, subType;
public:
	explicit Impl ( const ss :: string & s );
	void printOn ( std :: ostream & os ) const;
	const ss :: string & getType ( ) const {
		return type;
	}
	const ss :: string & getSubType ( ) const {
		return subType;
	}
};

ContentType :: Impl :: Impl ( const ss :: string & s ) {
	RightParser :: result_t info = parseContentType ( s );
	if ( ! info.full )
		throw std :: runtime_error ( std :: string ( "error parsing ContentType: " ).append ( s.begin ( ), s.end ( ) ) );
	RightParser :: iter_t nu = info.trees.begin ( );
	RightParser :: iter_t i = nu -> children.begin ( );
	type.assign ( i -> value.begin ( ), i -> value.end ( ) );
	boost :: algorithm :: to_lower ( type );
	++ i;
	subType.assign ( i -> value.begin ( ), i -> value.end ( ) );
	boost :: algorithm :: to_lower ( subType );
	//dalshe idut parametri
}

void ContentType :: Impl :: printOn ( std :: ostream & os ) const {
	os << type << '/' << subType;
}

ContentType :: ContentType ( const ContentType & s ) : impl ( new Impl ( * s.impl ) ) { }

ContentType :: ~ContentType ( ) {
	delete impl;
}

ContentType :: ContentType ( const ss :: string & s ) : impl ( new Impl ( s ) ) { }

void ContentType :: printOn ( std :: ostream & os ) const {
	impl -> printOn ( os );
}

const ss :: string & ContentType :: getType ( ) const {
	return impl -> getType ( );
}

const ss :: string & ContentType :: getSubType ( ) const {
	return impl -> getSubType ( );
}

static void parseAccept ( boost :: optional < AcceptRangeSet > & a, const ss :: string & s ) {
	RightParser :: result_t info = parseAccept ( s );
	if ( ! info.full )
		throw std :: runtime_error ( std :: string ( "error parsing accept: " ).append ( s.begin ( ), s.end ( ) ) );
	if ( ! a )
		a = boost :: in_place ( );
	RightParser :: iter_t v = info.trees.begin ( );
	for ( RightParser :: iter_t i = v -> children.begin ( ), ie = v -> children.end ( ); i != ie; ++ i ) {
		RightParser :: iter_t j = i -> children.begin ( ), j2 = j;
		++ j2;
		static boost :: algorithm :: detail :: to_lowerF < char > tl ( std :: locale :: classic ( ) );
		a -> insert ( AcceptRange ( boost :: make_transform_iterator ( j -> value.begin ( ), tl ),
			boost :: make_transform_iterator ( j -> value.end ( ), tl ),
			boost :: make_transform_iterator ( j2 -> value.begin ( ), tl ),
			boost :: make_transform_iterator ( j2 -> value.end ( ), tl ) ) );
	}
}

static void parseVia ( ViaVector & via, const ss :: string & s ) {
	RightParser :: result_t info = parseVia ( s );
	if ( ! info.full )
		throw std :: runtime_error ( std :: string ( "error parsing via: " ).append ( s.begin ( ), s.end ( ) ) );
	RightParser :: iter_t v = info.trees.begin ( );
	RightParser :: iter_t i = v -> children.begin ( );
	while ( i != v -> children.end ( ) ) {
		RightParser :: iter_t j = i -> children.begin ( );
		Via t;
		t.setTransport ( ss :: string ( j -> value.begin ( ), j -> value.end ( ) ) );
		++ j;
		t.setHost ( ss :: string ( j -> value.begin ( ), j -> value.end ( ) ) );
		++ j;
		if ( j != i -> children.end ( ) && j -> value.id ( ).to_long ( ) != via_grammar_genericParam_id ) {
			t.setPort ( std :: atoi ( ss :: string ( j -> value.begin ( ), j -> value.end ( ) ).c_str ( ) ) );
			++ j;
		}
		while ( j != i -> children.end ( ) ) {
			RightParser :: iter_t k = j -> children.begin ( );
			ss :: string pname ( k -> value.begin ( ), k -> value.end ( ) );
			boost :: algorithm :: to_lower ( pname );
			ss :: string pvalue;
			if ( ++ k != j -> children.end ( ) )
				pvalue.assign ( k -> value.begin ( ), k -> value.end ( ) );
			if ( pname == "branch" )
				t.setBranch ( pvalue );
			else if ( pname == "received" )
				t.setReceived ( pvalue );
			else if ( pname == "rport" )
				t.setRport ( std :: atoi ( pvalue.c_str ( ) ) );
			else
				t.setUnknownParameter ( pname, pvalue );
			++ j;
		}
		via.push_back ( t );
		++ i;
	}
}

static void parseRecordRoute ( RouteHeaderVector & recordRoute, const ss :: string & s ) {
	RightParser :: result_t info = parseRecordRoute ( s );
	if ( ! info.full )
		throw std :: runtime_error ( std :: string ( "error parsing route or record-route: " ).append ( s.begin ( ), s.end ( ) ) );
	RightParser :: iter_t r = info.trees.begin ( );
	RightParser :: iter_t i = r -> children.begin ( );
	while ( i != r -> children.end ( ) ) {
		RightParser :: iter_t j = i -> children.begin ( );
		RouteHeader rr;
		if ( j -> value.id ( ).to_long ( ) != recordRoute_grammar_uri_id ) {
			rr.setDisplayName ( ss :: string ( j -> value.begin ( ), j -> value.end ( ) ) );
			++ j;
		}
		URI u;
		parseUri ( u, j );
		rr.setUri ( u );
		while ( ++ j != i -> children.end ( ) ) {
			RightParser :: iter_t k = j -> children.begin ( );
			ss :: string pname ( k -> value.begin ( ), k -> value.end ( ) );
			boost :: algorithm :: to_lower ( pname );
			ss :: string pvalue;
			if ( ++ k != j -> children.end ( ) )
				pvalue.assign ( k -> value.begin ( ), k -> value.end ( ) );
			rr.setUnknownParameter ( pname, pvalue );
		}
		recordRoute.push_back ( rr );
		++ i;
	}
}

static void parseWarning ( WarningVector & v, const ss :: string & s ) {
	RightParser :: result_t info = parseWarning ( s );
	if ( ! info.full )
		throw std :: runtime_error ( std :: string ( "error parsing warning: " ).append ( s.begin ( ), s.end ( ) ) );
	RightParser :: iter_t nu = info.trees.begin ( );
	for ( RightParser :: iter_t i = nu -> children.begin ( ); i != nu -> children.end ( ); ++ i ) {
		RightParser :: iter_t j = i -> children.begin ( );
		unsigned code = std :: atoi ( ss :: string ( j -> value.begin ( ), j -> value.end ( ) ).c_str ( ) );
		++ j;
		ss :: string agent ( j -> value.begin ( ), j -> value.end ( ) );
		++ j;
		ss :: string text ( unQuote ( ss :: string ( j -> value.begin ( ), j -> value.end ( ) ) ) );
		v.push_back ( Warning ( code, agent, text ) );
	}
}

static void parseSupported ( StringVector & v, const ss :: string & s ) {
	RightParser :: result_t info = parseSupported ( s );
	if ( ! info.full )
		throw std :: runtime_error ( std :: string ( "error parsing supported: " ).append ( s.begin ( ), s.end ( ) ) );
	RightParser :: iter_t nu = info.trees.begin ( );
	for ( RightParser :: iter_t i = nu -> children.begin ( ); i != nu -> children.end ( ); ++ i )
		v.push_back ( ss :: string ( i -> value.begin ( ), i -> value.end ( ) ) );
}

static void parseRequire ( StringVector & v, const ss :: string & s ) {
	RightParser :: result_t info = parseRequire ( s );
	if ( ! info.full )
		throw std :: runtime_error ( std :: string ( "error parsing require: " ).append ( s.begin ( ), s.end ( ) ) );
	RightParser :: iter_t nu = info.trees.begin ( );
	for ( RightParser :: iter_t i = nu -> children.begin ( ); i != nu -> children.end ( ); ++ i )
		v.push_back ( ss :: string ( i -> value.begin ( ), i -> value.end ( ) ) );
}

static Request :: Methods parseMethod ( const ss :: string & m ) {
	switch ( m [ 0 ] ) {
		case 'I':
			if ( m == "INVITE" )
				return Request :: mInvite;
			if ( m == "INFO" )
				return Request :: mInfo;
			break;
		case 'A':
			if ( m == "ACK" )
				return Request :: mAck;
			break;
		case 'O':
			if ( m == "OPTIONS" )
				return Request :: mOptions;
			break;
		case 'B':
			if ( m == "BYE" )
				return Request :: mBye;
			break;
		case 'C':
			if ( m == "CANCEL" )
				return Request :: mCancel;
			break;
		case 'R':
			if ( m == "REGISTER" )
				return Request :: mRegister;
			if ( m == "REFER" )
				return Request :: mRefer;
			break;
		case 'M':
			if ( m == "MESSAGE" )
				return Request :: mMessage;
			break;
		case 'N':
			if ( m == "NOTIFY" )
				return Request :: mNotify;
			break;
		case 'S':
			if ( m == "SUBSCRIBE" )
				return Request :: mSubscribe;
			break;
		case 'U':
			if ( m == "UPDATE" )
				return Request :: mUpdate;
			break;
		case 'P':
			if ( m == "PRACK" )
				return Request :: mPrack;
			if ( m == "PUBLISH" )
				return Request :: mPublish;
			break;
	}
	throw std :: runtime_error ( std :: string ( "unknown sip method: " ).append ( m.begin ( ), m.end ( ) ) );
}

static void parseAllow ( std :: bitset < Request :: mNumMethods > & a, const ss :: string & s ) {
	RightParser :: result_t info = parseSupported ( s );
	if ( ! info.full )
		throw std :: runtime_error ( std :: string ( "error parsing allow: " ).append ( s.begin ( ), s.end ( ) ) );
	RightParser :: iter_t nu = info.trees.begin ( );
	for ( RightParser :: iter_t i = nu -> children.begin ( ); i != nu -> children.end ( ); ++ i ) {
		try {
			a.set ( parseMethod ( ss :: string ( i -> value.begin ( ), i -> value.end ( ) ) ) );
		} catch ( ... ) { } //just skip unknown methods
	}
}

static InfoPackages parseInfoPackage ( const ss :: string & s ) {
	if ( s == "nil" )
		return ipNil;
	if ( s == "dtmf" )
		return ipDtmf;
	return ipNumPackages;
	//opyat ge ta parametri otdelyayutsya tochkami
}

static void parseRecvInfo ( std :: bitset < ipNumPackages > & r, const ss :: string & s ) {
	RightParser :: result_t info = parseSupported ( s );
	//na samom dele tam eshe parametri otdeleni tochkami
	//no nam poka ne nado i s parametrami ono ne vlezet v bitset
	//zato ne v bitsete mogno sohranyat poryadok, chto toge poka ne nado
	if ( ! info.full )
		throw std :: runtime_error ( std :: string ( "error parsing recv-info: " ).append ( s.begin ( ), s.end ( ) ) );
	RightParser :: iter_t nu = info.trees.begin ( );
	for ( RightParser :: iter_t i = nu -> children.begin ( ); i != nu -> children.end ( ); ++ i ) {
		ss :: string p ( i -> value.begin ( ), i -> value.end ( ) );
		try {
			r.set ( parseInfoPackage ( p ) );
		} catch ( ... ) { } //just skip unknown packages
	}
}

template < typename T > ContentDisposition :: Disposition parseDisposition ( T range ) {
	switch ( * boost :: begin ( range ) ) {
		case 'a':
		case 'A':
			if ( boost :: algorithm :: iequals ( range, "alert" ) )
				return ContentDisposition :: alert;
			if ( boost :: algorithm :: iequals ( range, "attachment" ) )
				return ContentDisposition :: attachment;
			break;
		case 'i':
		case 'I':
			if ( boost :: algorithm :: iequals ( range, "icon" ) )
				return ContentDisposition :: icon;
			if ( boost :: algorithm :: iequals ( range, "Info-Package" ) )
				return ContentDisposition :: infoPackage;
			break;
		case 'r':
		case 'R':
			if ( boost :: algorithm :: iequals ( range, "render" ) )
				return ContentDisposition :: render;
			break;
		case 's':
		case 'S':
			if ( boost :: algorithm :: iequals ( range, "session" ) )
				return ContentDisposition :: session;
			if ( boost :: algorithm :: iequals ( range, "signal" ) )
				return ContentDisposition :: signal;
			break;
		
	}
	throw std :: runtime_error ( std :: string ( "unsupported content-disposition: " ).append ( boost :: begin ( range ), boost :: end ( range ) ) );
}

static void parseContentDisposition ( ContentDisposition & d, const ss :: string & s ) {
	RightParser :: result_t info = parseContentDisposition ( s );
	if ( ! info.full )
		throw std :: runtime_error ( std :: string ( "error parsing content-disposition: " ).append ( s.begin ( ), s.end ( ) ) );
	RightParser :: iter_t nu = info.trees.begin ( );
	RightParser :: iter_t i = nu -> children.begin ( );
	ContentDisposition :: Disposition disp = parseDisposition ( std :: make_pair ( i -> value.begin ( ), i -> value.end ( ) ) );
	bool r = true;
	while ( ++ i != nu -> children.end ( ) ) {
		RightParser :: iter_t j = i -> children.begin ( );
		if ( ! boost :: algorithm :: iequals ( std :: make_pair ( j -> value.begin ( ), j -> value.end ( ) ), "handling" ) )
			continue;
		if ( ++ j == i -> children.end ( ) )
			throw std :: runtime_error ( "handling without value" );
		std :: pair < ss :: string :: const_iterator, ss :: string :: const_iterator > h ( j -> value.begin ( ), j-> value.end ( ) );
		if ( boost :: algorithm :: iequals ( h, "optional" ) )
			r = false;
		else if ( boost :: algorithm :: iequals ( h, "optional" ) )
			break;
		throw std :: runtime_error ( std :: string ( "unsupported handling: " ).append ( h.first, h.second ) );
	}
	d.disposition = disp;
	d.handlingRequired = r;
}

std :: ostream & operator<< ( std :: ostream & os, ContentDisposition d ) {
	switch ( d.disposition ) {
		case ContentDisposition :: missing:
			return os;
		case ContentDisposition :: alert:
			os << "alert";
			break;
		case ContentDisposition :: attachment:
			os << "attachment";
			break;
		case ContentDisposition :: icon:
			os << "icon";
			break;
		case ContentDisposition :: infoPackage:
			os << "Info-Package";
			break;
		case ContentDisposition :: render:
			os << "render";
			break;
		case ContentDisposition :: session:
			os << "session";
			break;
		case ContentDisposition :: signal:
			os << "signal";
			break;
	}
	return os << "; handling=" << ( d.handlingRequired ? "required" : "optional" );
}

class RequestMIMEInfo :: Impl : public Allocatable < __SS_ALLOCATOR > {
	boost :: optional < AcceptRangeSet > accept;
	std :: bitset < Request :: mNumMethods > allow;
	DigestResponseVector authorization;
	ss :: string callId;
	ContactHeader contact;
	ContentDisposition contentDisposition;
	CSeq cseq;
	boost :: optional < unsigned > expires;
	FromToHeader from;
	InfoPackages infoPackage;
	unsigned maxForwards;
	unsigned minSE;
	ss :: string m_PAssertID;
	ss :: string m_Privacy;
	DigestResponseVector proxyAuthorization;
	RAck rack;
	RouteHeaderVector recordRoute;
	std :: bitset < ipNumPackages > recvInfo;
	ss :: string m_remotePartyID;
	StringVector require;
	RouteHeaderVector route;
	boost :: optional < SessionExpires > sessionExpires;
	StringVector supported;
	ss :: string timestamp;
	FromToHeader to;
	ss :: string userAgent;
	ViaVector via;
public:
	Impl ( ) : infoPackage ( ipNil ), maxForwards ( 0 ), minSE ( 0 ) { }
	void addAccept ( const ss :: string & s ) {
		parseAccept ( accept, s );
	}
	void addAccept ( const AcceptRange & a ) {
		if ( ! accept )
			accept = boost :: in_place ( );
		accept -> insert ( a );
	}
	const boost :: optional < AcceptRangeSet > & getAccept ( ) const {
		return accept;
	}
	void setAllow ( const ss :: string & s ) {
		parseAllow ( allow, s );
	}
	void setAllow ( unsigned a ) {
		allow = std :: bitset < Request :: mNumMethods > ( a );
	}
	bool getAllow ( Request :: Methods m ) const {
		return ! allow.any ( ) || allow [ m ];
	}
	void addAuthorization ( const DigestResponse & s ) {
		authorization.push_back ( s );
	}
	const DigestResponseVector & getAuthorization ( ) const {
		return authorization;
	}
	void setCallId ( const ss :: string & s ) {
		callId = s;
	}
	const ss :: string & getCallId ( ) const {
		return callId;
	}
	void setContact ( const ss :: string & s ) {
		ContactHeader ( s ).swap ( contact );
	}
	void setContact ( const ContactHeader & s ) {
		contact = s;
	}
	const ContactHeader & getContact ( ) const {
		return contact;
	}
	void setContentDisposition ( const ss :: string & s ) {
		parseContentDisposition ( contentDisposition, s );
	}
	void setContentDisposition ( const ContentDisposition & d ) {
		contentDisposition = d;
	}
	ContentDisposition getContentDisposition ( ) const {
		return contentDisposition;
	}
	void setCseq ( const ss :: string & s ) {
		cseq = CSeq ( s );
	}
	void setCseq ( const CSeq & s ) {
		cseq = s;
	}
	const CSeq & getCseq ( ) const {
		return cseq;
	}
	void setExpires ( unsigned e ) {
		expires = e;
	}
	bool hasExpires ( ) const {
		return expires;
	}
	unsigned getExpires ( ) const {
		return * expires;
	}
	void setFrom ( const FromToHeader & s ) {
		from = s;
	}
	void setFrom ( const ss :: string & s ) {
		FromToHeader ( s ).swap ( from );
	}
	const FromToHeader & getFrom ( ) const {
		return from;
	}
	void setInfoPackage ( InfoPackages p ) {
		infoPackage = p;
	}
	void setInfoPackage ( const ss :: string & s ) {
		infoPackage = parseInfoPackage ( s );
	}
	InfoPackages getInfoPackage ( ) const {
		return infoPackage;
	}
	void setMaxForwards ( unsigned s ) {
		maxForwards = s;
	}
	unsigned getMaxForwards ( ) const {
		return maxForwards;
	}
	void setMinSE ( unsigned s ) {
		minSE = s;
	}
	unsigned getMinSE ( ) const {
		return minSE;
	}
	void setPAssertID ( const ss :: string & s ) {
		m_PAssertID = s;
	}
	const ss :: string & getPAssertID ( ) const {
		return m_PAssertID;
	}
	void setPrivacy ( const ss :: string & s ) {
		m_Privacy = s;
	}
	const ss :: string & getPrivacy ( ) const {
		return m_Privacy;
	}
	void addProxyAuthorization ( const DigestResponse & s ) {
		proxyAuthorization.push_back ( s );
	}
	void setRAck ( const RAck & r ) {
		rack = r;
	}
	void addRecordRoute ( const ss :: string & s ) {
		parseRecordRoute ( recordRoute, s );
	}
	void addRecordRoute ( const RouteHeader & s ) {
		recordRoute.push_back ( s );
	}
	const RouteHeaderVector & getRecordRoute ( ) const {
		return recordRoute;
	}
	void setRecvInfo ( const ss :: string & s ) {
		parseRecvInfo ( recvInfo, s );
	}
	void setRecvInfo ( unsigned a ) {
		recvInfo = std :: bitset < ipNumPackages > ( a );
	}
	bool getRecvInfo ( InfoPackages p ) const {
		return recvInfo [ p ];
	}
	bool hasRecvInfo ( ) const {
		return recvInfo.any ( );
	}
	void setRemotePartyID ( const ss :: string & s ) {
		m_remotePartyID = s;
	}
	const ss :: string & getRemotePartyID ( ) const {
		return m_remotePartyID;
	}
	void setRequire ( const ss :: string & s ) {
		parseRequire ( require, s );
	}
	void addRequire ( const ss :: string & s ) {
		require.push_back ( s );
	}
	const StringVector & getRequire ( ) const {
		return require;
	}
	void addRoute ( const ss :: string & s ) {
		parseRecordRoute ( route, s );
	}
	void addRoute ( const RouteHeader & s ) {
		route.push_back ( s );
	}
	void setRoute ( const RouteHeaderVector & v ) {
		route = v;
	}
	const RouteHeaderVector & getRoute ( ) const {
		return route;
	}
	void setSessionExpires ( const SessionExpires & s ) {
		sessionExpires = s;
	}
	void setSessionExpires ( unsigned d, const ss::string & r ) {
		sessionExpires = boost :: in_place ( d, r );
	}
	void setSessionExpires ( const ss :: string & s ) {
		sessionExpires = boost :: in_place ( s );
	}
	bool hasSessionExpires ( ) const {
		return sessionExpires;
	}
	const SessionExpires & getSessionExpires ( ) const {
		return * sessionExpires;
	}
	void setSupported ( const ss :: string & s ) {
		parseSupported ( supported, s );
	}
	void addSupported ( const ss :: string & s ) {
		supported.push_back ( s );
	}
	const StringVector & getSupported ( ) const {
		return supported;
	}
	void setTimestamp ( const ss :: string & s ) {
		timestamp = s;
	}
	const ss :: string & getTimestamp ( ) const {
		return timestamp;
	}
	void setTo ( const FromToHeader & s ) {
		to = s;
	}
	void setTo ( const ss :: string & s ) {
		FromToHeader ( s ).swap ( to );
	}
	const FromToHeader & getTo ( ) const {
		return to;
	}
	void setUserAgent ( const ss :: string & s ) {
		userAgent = s;
	}
	const ss :: string & getUserAgent ( ) const {
		return userAgent;
	}
	void addVia ( const ss :: string & s ) {
		parseVia ( via, s );
	}
	void addVia ( const Via & s ) {
		via.push_back ( s );
	}
	const ViaVector & getVia ( ) const {
		return via;
	}
	ViaVector & getVia ( ) {
		return via;
	}
	void printOn ( std :: ostream & os ) const;
};

static void printAccept ( std :: ostream & os, const boost :: optional < AcceptRangeSet > & accept ) {
	if ( ! accept )
		return;
	os << "Accept: ";
	bool first = true;
	for ( AcceptRangeSet :: const_iterator i = accept -> begin ( ), ie = accept -> end ( ); i != ie; ++ i ) {
		if ( first )
			first = false;
		else
			os << ", ";
		os << * i;
	}
	os << "\r\n";
}

static void printAllow ( std :: ostream & os, const std :: bitset < Request :: mNumMethods > & allow ) {
	if ( ! allow.any ( ) )
		return;
	os << "Allow: ";
	bool first = true;
	for ( std :: size_t i = allow._Find_first ( ), as = allow.size ( ); i < as; i = allow._Find_next ( i ) ) {
		if ( first )
			first = false;
		else
			os << ", ";
		os << methodName ( Request :: Methods ( i ) );
	}
	os << "\r\n";
}

ss :: string infoPackageName ( InfoPackages p ) {
	switch ( p ) {
		case ipNil:
			return "nil";
		case ipDtmf:
			return "dtmf";
		default:
			throw std :: runtime_error ( std :: string ( "unknown info package" ) );
	}
}

static void printRecvInfo ( std :: ostream & os, const std :: bitset < ipNumPackages > & recvInfo ) {
	if ( ! recvInfo.any ( ) )
		return;
	os << "Recv-Info: ";
	bool first = true;
	for ( std :: size_t i = recvInfo._Find_first ( ), rs = recvInfo.size ( ); i < rs; i = recvInfo._Find_next ( i ) ) {
		if ( first )
			first = false;
		else
			os << ", ";
		os << infoPackageName ( InfoPackages ( i ) );
	}
	os << "\r\n";
}

static void printInfoPackage ( std :: ostream & os, InfoPackages ip ) {
	if ( ip != ipNil && ip != ipNumPackages )
		os << "Info-Package: " << infoPackageName ( ip );
}

void RequestMIMEInfo :: Impl :: printOn ( std :: ostream & os ) const {
	printAccept ( os, accept );
	printAllow ( os, allow );
	for ( std :: size_t i = 0, as = authorization.size ( ); i < as; i ++ )
		os << "Authorization: " << authorization [ i ] << "\r\n";
	os << "Call-ID: " << callId << "\r\n";
	if ( ! contact.getUri ( ).getHost ( ).empty ( ) )
		os << "Contact: " << contact << "\r\n";
	if ( contentDisposition.disposition != ContentDisposition :: missing )
		os << "Content-Disposition: " << contentDisposition << "\r\n";
	os << "CSeq: " << cseq << "\r\n";
	if ( expires )
		os << "Expires: " << * expires << "\r\n";
	os << "From: " << from << "\r\n";
	printInfoPackage ( os, infoPackage );
	os << "Max-Forwards: " << maxForwards << "\r\n";
	if ( minSE )
		os << "Min-SE: " << minSE << "\r\n";
	if ( ! m_PAssertID.empty ( ) )
		os << "P-Asserted-Identity: " << m_PAssertID << "\r\n";
	if ( ! m_Privacy.empty ( ) )
		os << "Privacy: " << m_Privacy << "\r\n";
	for ( std :: size_t i = 0, ps = proxyAuthorization.size ( ); i < ps; i ++ )
		os << "Proxy-Authorization: " << proxyAuthorization [ i ] << "\r\n";
	if ( rack.responseNum )
		os << "RAck: " << rack << "\r\n";
	for ( std :: size_t i = 0, rs = recordRoute.size ( ); i < rs; i ++ )
		os << "Record-Route: " << recordRoute [ i ] << "\r\n";
	printRecvInfo ( os, recvInfo );
	if ( ! m_remotePartyID.empty ( ) )
		os << "Remote-Party-ID: " << m_remotePartyID << "\r\n";
	for ( std :: size_t i = 0, rs = route.size ( ); i < rs; i ++ )
		os << "Route: " << route [ i ] << "\r\n";
	if ( ! require.empty ( ) ) {
		os << "Require: ";
		for ( std :: size_t i = 0, rs = require.size ( ); i < rs; i ++ ) {
			if ( i )
				os << ", ";
			os << require [ i ];
		}
		os << "\r\n";
	}
	if ( sessionExpires )
		os << "Session-Expires: " << * sessionExpires << "\r\n";
	if ( ! supported.empty ( ) ) {
		os << "Supported: ";
		for ( std :: size_t i = 0, ss = supported.size ( ); i < ss; i ++ ) {
			if ( i )
				os << ", ";
			os << supported [ i ];
		}
		os << "\r\n";
	}
	if ( ! timestamp.empty ( ) )
		os << "Timestamp: " << timestamp << "\r\n";
	os << "To: " << to << "\r\n";
	if ( ! userAgent.empty ( ) )
		os << "User-Agent: " << userAgent << "\r\n";
	for ( std :: size_t i = 0, vs = via.size ( ); i < vs; i ++ )
		os << "Via: " << via [ i ] << "\r\n";
}

RequestMIMEInfo :: RequestMIMEInfo ( const RequestMIMEInfo & m ) : impl ( new Impl ( * m.impl ) ) { }

RequestMIMEInfo :: ~RequestMIMEInfo ( ) {
	delete impl;
}

RequestMIMEInfo :: RequestMIMEInfo ( ) : impl ( new Impl ) { }

void RequestMIMEInfo :: printOn ( std :: ostream & os ) const {
	impl -> printOn ( os );
}

void RequestMIMEInfo :: addAccept ( const ss :: string & s ) {
	impl -> addAccept ( s );
}

void RequestMIMEInfo :: addAccept ( const AcceptRange & s ) {
	impl -> addAccept ( s );
}

const boost :: optional < AcceptRangeSet > & RequestMIMEInfo :: getAccept ( ) const {
	return impl -> getAccept ( );
}

void RequestMIMEInfo :: setAllow ( const ss :: string & s ) {
	impl -> setAllow ( s );
}

void RequestMIMEInfo :: setAllow ( unsigned a ) {
	impl -> setAllow ( a );
}

bool RequestMIMEInfo :: getAllow ( Request :: Methods m ) const {
	return impl -> getAllow ( m );
}

void RequestMIMEInfo :: addAuthorization ( const DigestResponse & s ) {
	impl -> addAuthorization ( s );
}

const DigestResponseVector & RequestMIMEInfo :: getAuthorization ( ) const {
	return impl -> getAuthorization ( );
}

void RequestMIMEInfo :: setCallId ( const ss :: string & s ) {
	impl -> setCallId ( s );
}

const ss :: string & RequestMIMEInfo :: getCallId ( ) const {
	return impl -> getCallId ( );
}

void RequestMIMEInfo :: setContact ( const ss :: string & s ) {
	impl -> setContact ( s );
}

void RequestMIMEInfo :: setContact ( const ContactHeader & s ) {
	impl -> setContact ( s );
}

const ContactHeader & RequestMIMEInfo :: getContact ( ) const {
	return impl -> getContact ( );
}

void RequestMIMEInfo :: setContentDisposition ( const ss :: string & s ) {
	impl -> setContentDisposition ( s );
}

void RequestMIMEInfo :: setContentDisposition ( const ContentDisposition & d ) {
	impl -> setContentDisposition ( d);
}

ContentDisposition RequestMIMEInfo :: getContentDisposition ( ) const {
	return impl -> getContentDisposition ( );
}

void RequestMIMEInfo :: setCseq ( const ss :: string & s ) {
	impl -> setCseq ( s );
}

void RequestMIMEInfo :: setCseq ( const CSeq & s ) {
	impl -> setCseq ( s );
}

const CSeq & RequestMIMEInfo :: getCseq ( ) const {
	return impl -> getCseq ( );
}

void RequestMIMEInfo :: setExpires ( unsigned e ) {
	impl -> setExpires ( e );
}

bool RequestMIMEInfo :: hasExpires ( ) const {
	return impl -> hasExpires ( );
}

unsigned RequestMIMEInfo :: getExpires ( ) const {
	return impl -> getExpires ( );
}

void RequestMIMEInfo :: setFrom ( const ss :: string & s ) {
	impl -> setFrom ( s );
}

void RequestMIMEInfo :: setFrom ( const FromToHeader & s ) {
	impl -> setFrom ( s );
}

const FromToHeader & RequestMIMEInfo :: getFrom ( ) const {
	return impl -> getFrom ( );
}

void RequestMIMEInfo :: setInfoPackage ( InfoPackages p ) {
	impl -> setInfoPackage ( p );
}

void RequestMIMEInfo :: setInfoPackage ( const ss :: string & s ) {
	impl -> setInfoPackage ( s );
}

InfoPackages RequestMIMEInfo :: getInfoPackage ( ) const {
	return impl -> getInfoPackage ( );
}

void RequestMIMEInfo :: setMaxForwards ( unsigned s ) {
	impl -> setMaxForwards ( s );
}

unsigned RequestMIMEInfo :: getMaxForwards ( ) const {
	return impl -> getMaxForwards ( );
}

void RequestMIMEInfo :: setMinSE ( unsigned s ) {
	impl -> setMinSE ( s );
}

unsigned RequestMIMEInfo :: getMinSE ( ) const {
	return impl -> getMinSE ( );
}

void RequestMIMEInfo :: addProxyAuthorization ( const DigestResponse & s ) {
	impl -> addProxyAuthorization ( s );
}

void RequestMIMEInfo :: setRAck ( const RAck & r ) {
	impl -> setRAck ( r );
}

void RequestMIMEInfo :: addRecordRoute ( const ss :: string & s ) {
	impl -> addRecordRoute ( s );
}

void RequestMIMEInfo :: addRecordRoute ( const RouteHeader & s ) {
	impl -> addRecordRoute ( s );
}

const RouteHeaderVector & RequestMIMEInfo :: getRecordRoute ( ) const {
	return impl -> getRecordRoute ( );
}

void RequestMIMEInfo :: setRecvInfo ( const ss :: string & s ) {
	impl -> setRecvInfo ( s );
}

void RequestMIMEInfo :: setRecvInfo ( unsigned a ) {
	impl -> setRecvInfo ( a );
}

bool RequestMIMEInfo :: getRecvInfo ( InfoPackages p ) const {
	return impl -> getRecvInfo ( p );
}

bool RequestMIMEInfo :: hasRecvInfo ( ) const {
	return impl -> hasRecvInfo ( );
}

void RequestMIMEInfo :: setRemotePartyID ( const ss :: string & s ) {
	impl -> setRemotePartyID ( s );
}

const ss :: string & RequestMIMEInfo :: getRemotePartyID ( ) const {
	return impl -> getRemotePartyID ( );
}

void RequestMIMEInfo :: setRequire ( const ss :: string & s ) {
	impl -> setRequire ( s );
}

void RequestMIMEInfo :: addRequire ( const ss :: string & s ) {
	impl -> addRequire ( s );
}

const StringVector & RequestMIMEInfo :: getRequire ( ) const {
	return impl -> getRequire ( );
}

void RequestMIMEInfo :: addRoute ( const ss :: string & s ) {
	impl -> addRoute ( s );
}

void RequestMIMEInfo :: addRoute ( const RouteHeader & s ) {
	impl -> addRoute ( s );
}

void RequestMIMEInfo :: setRoute ( const RouteHeaderVector & v ) {
	impl -> setRoute ( v );
}

const RouteHeaderVector & RequestMIMEInfo :: getRoute ( ) const {
	return impl -> getRoute ( );
}

void RequestMIMEInfo :: setSessionExpires ( const SessionExpires & e ) {
	impl -> setSessionExpires ( e );
}

void RequestMIMEInfo :: setSessionExpires ( unsigned d, const ss :: string & r ) {
	impl -> setSessionExpires ( d, r );
}

void RequestMIMEInfo :: setSessionExpires ( const ss :: string & s ) {
	impl -> setSessionExpires ( s );
}

bool RequestMIMEInfo :: hasSessionExpires ( ) const {
	return impl -> hasSessionExpires ( );
}

const SessionExpires & RequestMIMEInfo :: getSessionExpires ( ) const {
	return impl -> getSessionExpires ( );
}

void RequestMIMEInfo :: setSupported ( const ss :: string & s ) {
	impl -> setSupported ( s );
}

void RequestMIMEInfo :: addSupported ( const ss :: string & s ) {
	impl -> addSupported ( s );
}

const StringVector & RequestMIMEInfo :: getSupported ( ) const {
	return impl -> getSupported ( );
}

void RequestMIMEInfo :: setTimestamp ( const ss :: string & s ) {
	impl -> setTimestamp ( s );
}

const ss :: string & RequestMIMEInfo :: getTimestamp ( ) const {
	return impl -> getTimestamp ( );
}

void RequestMIMEInfo :: setTo ( const ss :: string & s ) {
	impl -> setTo ( s );
}

void RequestMIMEInfo :: setTo ( const FromToHeader & s ) {
	impl -> setTo ( s );
}

const FromToHeader & RequestMIMEInfo :: getTo ( ) const {
	return impl -> getTo ( );
}

void RequestMIMEInfo :: setUserAgent ( const ss :: string & s ) {
	impl -> setUserAgent ( s );
}

const ss :: string & RequestMIMEInfo :: getUserAgent ( ) const {
	return impl -> getUserAgent ( );
}

void RequestMIMEInfo :: addVia ( const ss :: string & s ) {
	impl -> addVia ( s );
}

void RequestMIMEInfo :: addVia ( const Via & s ) {
	impl -> addVia ( s );
}

const ViaVector & RequestMIMEInfo :: getVia ( ) const {
	return const_cast < const Impl * > ( impl ) -> getVia ( );
}

ViaVector & RequestMIMEInfo :: getVia ( ) {
	return impl -> getVia ( );
}

void RequestMIMEInfo :: setPAssertID ( const ss :: string & s ) {
	impl -> setPAssertID ( s );
}

const ss :: string & RequestMIMEInfo :: getPAssertID ( ) const {
	return impl -> getPAssertID ( );
}

void RequestMIMEInfo :: setPrivacy ( const ss :: string & s ) {
	impl -> setPrivacy ( s );
}

const ss :: string & RequestMIMEInfo :: getPrivacy ( ) const {
	return impl -> getPrivacy ( );
}

class ResponseMIMEInfo :: Impl : public Allocatable < __SS_ALLOCATOR > {
	boost :: optional < AcceptRangeSet > accept;
	std :: bitset < Request :: mNumMethods > allow;
	boost :: optional < AuthenticationInfo > authenticationInfo;
	ss :: string callId;
	ContactHeader contact;
	ContentDisposition contentDisposition;
	CSeq cseq;
	boost :: optional < boost :: posix_time :: ptime > date;
	boost :: optional < unsigned > expires;
	FromToHeader from;
	unsigned minExpires;
	unsigned minSE;
	DigestChallengeVector proxyAuthenticate;
	RouteHeaderVector recordRoute;
	std :: bitset < ipNumPackages > recvInfo;
	StringVector require;
	boost :: optional < RetryAfter > retryAfter;
	RouteHeaderVector route;
	unsigned rseq;
	ss :: string server;
	boost :: optional < SessionExpires > sessionExpires;
	StringVector supported;
	ss :: string timestamp;
	FromToHeader to;
	StringVector unsupported;
	ViaVector via;
	WarningVector warning;
	DigestChallengeVector wwwAuthenticate;
	ss :: string m_remotePartyID;
public:
	Impl ( ) : minExpires ( 0 ), minSE ( 0 ), rseq ( 0 ) { }
	void addAccept ( const ss  :: string & s ) {
		parseAccept ( accept, s );
	}
	void addAccept ( const AcceptRange & a ) {
		if ( ! accept )
			accept = boost :: in_place ( );
		accept -> insert ( a );
	}
	const boost :: optional < AcceptRangeSet > & getAccept ( ) const {
		return accept;
	}
	void setAllow ( const ss :: string & s ) {
		parseAllow ( allow, s );
	}
	void setAllow ( unsigned a ) {
		allow = std :: bitset < Request :: mNumMethods > ( a );
	}
	bool getAllow ( Request :: Methods m ) const {
		return ! allow.any ( ) || allow [ m ];
	}
	void setAuthenticationInfo ( const ss :: string & s ) {
		if ( authenticationInfo )
			AuthenticationInfo ( s ).swap ( * authenticationInfo );
		else
			authenticationInfo = boost :: in_place ( s );
	}
	void setAuthenticationInfo ( const AuthenticationInfo & s ) {
		authenticationInfo = s;
	}
	bool hasAuthenticationInfo ( ) const {
		return authenticationInfo;
	}
	const AuthenticationInfo & getAuthenticationInfo ( ) const {
		return * authenticationInfo;
	}
	void setCallId ( const ss :: string & s ) {
		callId = s;
	}
	const ss :: string & getCallId ( ) const {
		return callId;
	}
	void setContact ( const ss :: string & s ) {
		ContactHeader ( s ).swap ( contact );
	}
	void setContact ( const ContactHeader & s ) {
		contact = s;
	}
	const ContactHeader & getContact ( ) const {
		return contact;
	}
	void setContentDisposition ( const ss :: string & s ) {
		parseContentDisposition ( contentDisposition, s );
	}
	void setContentDisposition ( const ContentDisposition & d ) {
		contentDisposition = d;
	}
	ContentDisposition getContentDisposition ( ) const {
		return contentDisposition;
	}
	void setCseq ( const ss :: string & s ) {
		cseq = CSeq ( s );
	}
	void setCseq ( const CSeq & s ) {
		cseq = s;
	}
	const CSeq & getCseq ( ) const {
		return cseq;
	}
	void setDate ( const boost :: posix_time :: ptime & s ) {
		date = s;
	}
	void setExpires ( unsigned e ) {
		expires = e;
	}
	bool hasExpires ( ) const {
		return expires;
	}
	unsigned getExpires ( ) const {
		return * expires;
	}
	void setFrom ( const ss :: string & s ) {
		FromToHeader ( s ).swap ( from );
	}
	void setFrom ( const FromToHeader & s ) {
		from = s;
	}
	const FromToHeader & getFrom ( ) const {
		return from;
	}
	void setMinExpires ( unsigned s ) {
		minExpires = s;
	}
	unsigned getMinExpires ( ) const {
		return minExpires;
	}
	void setMinSE ( unsigned s ) {
		minSE = s;
	}
	unsigned getMinSE ( ) const {
		return minSE;
	}
	void addProxyAuthenticate ( const DigestChallenge & s ) {
		proxyAuthenticate.push_back ( s );
	}
	void addRecordRoute ( const ss :: string & s ) {
		parseRecordRoute ( recordRoute, s );
	}
	void addRecordRoute ( const RouteHeader & s ) {
		recordRoute.push_back ( s );
	}
	const RouteHeaderVector & getRecordRoute ( ) const {
		return recordRoute;
	}
	void setRecvInfo ( const ss :: string & s ) {
		parseRecvInfo ( recvInfo, s );
	}
	void setRecvInfo ( unsigned a ) {
		recvInfo = std :: bitset < ipNumPackages > ( a );
	}
	bool getRecvInfo ( InfoPackages p ) const {
		return recvInfo [ p ];
	}
	bool hasRecvInfo ( ) const {
		return recvInfo.any ( );
	}
	void setRequire ( const ss :: string & s ) {
		parseRequire ( require, s );
	}
	void addRequire ( const ss :: string & s ) {
		require.push_back ( s );
	}
	const StringVector & getRequire ( ) const {
		return require;
	}
	void setRetryAfter ( const RetryAfter & s ) {
		retryAfter = s;
	}
	void addRoute ( const ss :: string & s ) {
		parseRecordRoute ( route, s );
	}
	void addRoute ( const RouteHeader & s ) {
		route.push_back ( s );
	}
	void setRSeq ( unsigned r ) {
		rseq = r;
	}
	void setServer ( const ss :: string & s ) {
		server = s;
	}
	void setSessionExpires ( const SessionExpires & s ) {
		sessionExpires = s;
	}
	void setSessionExpires ( unsigned d, const ss::string & r ) {
		sessionExpires = boost :: in_place ( d, r );
	}
	void setSessionExpires ( const ss::string & s ) {
		sessionExpires = boost :: in_place ( s );
	}
	bool hasSessionExpires ( ) const {
		return sessionExpires;
	}
	const SessionExpires & getSessionExpires ( ) const {
		return * sessionExpires;
	}
	void setSupported ( const ss :: string & s ) {
		parseSupported ( supported, s );
	}
	void addSupported ( const ss :: string & s ) {
		supported.push_back ( s );
	}
	const StringVector & getSupported ( ) const {
		return supported;
	}
	void setTimestamp ( const ss :: string & s ) {
		timestamp = s;
	}
	void setTo ( const ss :: string & s ) {
		FromToHeader ( s ).swap ( to );
	}
	void setTo ( const FromToHeader & s ) {
		to = s;
	}
	const FromToHeader & getTo ( ) const {
		return to;
	}
	void setUnsupported ( const ss :: string & s ) {
		parseRequire ( unsupported, s );
	}
	void addUnsupported ( const ss :: string & s ) {
		unsupported.push_back ( s );
	}
	const StringVector & getUnsupported ( ) const {
		return unsupported;
	}
	void addVia ( const ss :: string & s ) {
		parseVia ( via, s );
	}
	void addVia ( const Via & s ) {
		via.push_back ( s );
	}
	const ViaVector & getVia ( ) const {
		return via;
	}
	void addWarning ( const Warning & s ) {
		warning.push_back ( s );
	}
	void addWarning ( const ss :: string & s ) {
		parseWarning ( warning, s );
	}
	void addWwwAuthenticate ( const DigestChallenge & s ) {
		wwwAuthenticate.push_back ( s );
	}
	const DigestChallengeVector & getWwwAuthenticate ( ) const {
		return wwwAuthenticate;
	}
	void setRemotePartyID ( const ss :: string & s ) {
		m_remotePartyID = s;
	}
	const ss :: string & getRemotePartyID ( ) const {
		return m_remotePartyID;
	}
	void printOn ( std :: ostream & os ) const;
};

static ss :: string toRfc1123 ( const boost :: posix_time :: ptime & t ) {
	static std :: locale loc ( std :: locale :: classic ( ),
		new boost :: posix_time :: time_facet ( "%a, %d %b %Y %H:%M:%S GMT" ) );
	ss :: ostringstream os;
	os.imbue ( loc );
	os << t;
	return os.str ( );
}

void ResponseMIMEInfo :: Impl :: printOn ( std :: ostream & os ) const {
	printAccept ( os, accept );
	printAllow ( os, allow );
	if ( authenticationInfo )
		os << "Authentication-Info: " << * authenticationInfo << "\r\n";
	os << "Call-ID: " << callId << "\r\n";
	if ( ! contact.getUri ( ).getHost ( ).empty ( ) )
		os << "Contact: " << contact << "\r\n";
	if ( contentDisposition.disposition != ContentDisposition :: missing )
		os << "Content-Disposition: " << contentDisposition << "\r\n";
	os << "CSeq: " << cseq << "\r\n";
	if ( expires )
		os << "Expires: " << * expires << "\r\n";
	os << "From: " << from << "\r\n" << "To: " << to << "\r\n";
	if ( date )
		os << "Date: " << toRfc1123 ( * date ) << "\r\n";
	if ( minExpires )
		os << "Min-Expires: " << minExpires << "\r\n";
	if ( minSE )
		os << "Min-SE: " << minSE << "\r\n";
	for ( std :: size_t i = 0, ps = proxyAuthenticate.size ( ); i < ps; i ++ )
		os << "Proxy-Authenticate: " << proxyAuthenticate [ i ] << "\r\n";
	for ( std :: size_t i = 0, rs = recordRoute.size ( ); i < rs; i ++ )
		os << "Record-Route: " << recordRoute [ i ] << "\r\n";
	printRecvInfo ( os, recvInfo );
	if ( ! m_remotePartyID.empty ( ) )
		os << "Remote-Party-ID: " << m_remotePartyID << "\r\n";
	if ( ! require.empty ( ) ) {
		os << "Require: ";
		for ( std :: size_t i = 0, rs = require.size ( ); i < rs; i ++ ) {
			if ( i )
				os << ", ";
			os << require [ i ];
		}
		os << "\r\n";
	}
	if ( retryAfter )
		os << "Retry-After: " << * retryAfter << "\r\n";
	for ( std :: size_t i = 0, rs = route.size ( ); i < rs; i ++ )
		os << "Route: " << route [ i ] << "\r\n";
	if ( rseq )
		os << "RSeq: " << rseq << "\r\n";
	if ( ! server.empty ( ) )
		os << "Server: " << server << "\r\n";
	if ( sessionExpires )
		os << "Session-Expires: " << * sessionExpires << "\r\n";
	if ( ! supported.empty ( ) ) {
		os << "Supported: ";
		for ( std :: size_t i = 0, ss = supported.size ( ); i < ss; i ++ ) {
			if ( i )
				os << ", ";
			os << supported [ i ];
		}
		os << "\r\n";
	}
	if ( ! timestamp.empty ( ) )
		os << "Timestamp: " << timestamp << "\r\n";
	if ( ! unsupported.empty ( ) ) {
		os << "Unsupported: ";
		for ( std :: size_t i = 0, us = unsupported.size ( ); i < us; i ++ ) {
			if ( i )
				os << ", ";
			os << unsupported [ i ];
		}
		os << "\r\n";
	}
	for ( std :: size_t i = 0, vs = via.size ( ); i < vs; i ++ )
		os << "Via: " << via [ i ] << "\r\n";
	for ( std :: size_t i = 0, ws = warning.size ( ); i < ws; ++ i )
		os << "Warning: " << warning [ i ] << "\r\n";
	for ( std :: size_t i = 0, ws = wwwAuthenticate.size ( ); i < ws; i ++ )
		os << "WWW-Authenticate: " << wwwAuthenticate [ i ] << "\r\n";
}

ResponseMIMEInfo :: ResponseMIMEInfo ( const ResponseMIMEInfo & m ) : impl ( new Impl ( * m.impl ) ) { }

ResponseMIMEInfo :: ~ResponseMIMEInfo ( ) {
	delete impl;
}

ResponseMIMEInfo :: ResponseMIMEInfo ( ) : impl ( new Impl ) { }

void ResponseMIMEInfo :: printOn ( std :: ostream & os ) const {
	impl -> printOn ( os );
}

void ResponseMIMEInfo :: addAccept ( const ss :: string & s ) {
	impl -> addAccept ( s );
}

void ResponseMIMEInfo :: addAccept ( const AcceptRange & s ) {
	impl -> addAccept ( s );
}

const boost :: optional < AcceptRangeSet > & ResponseMIMEInfo :: getAccept ( ) const {
	return impl -> getAccept ( );
}

void ResponseMIMEInfo :: setAllow ( const ss :: string & s ) {
	impl -> setAllow ( s );
}

void ResponseMIMEInfo :: setAllow ( unsigned a ) {
	impl -> setAllow ( a );
}

bool ResponseMIMEInfo :: getAllow ( Request :: Methods m ) const {
	return impl -> getAllow ( m );
}

void ResponseMIMEInfo :: setAuthenticationInfo ( const ss :: string & s ) {
	impl -> setAuthenticationInfo ( s );
}

void ResponseMIMEInfo :: setAuthenticationInfo ( const AuthenticationInfo & s ) {
	impl -> setAuthenticationInfo ( s );
}

bool ResponseMIMEInfo :: hasAuthenticationInfo ( ) const {
	return impl -> hasAuthenticationInfo ( );
}

const AuthenticationInfo & ResponseMIMEInfo :: getAuthenticationInfo ( ) const {
	return impl -> getAuthenticationInfo ( );
}

void ResponseMIMEInfo :: setCallId ( const ss :: string & s ) {
	impl -> setCallId ( s );
}

const ss :: string & ResponseMIMEInfo :: getCallId ( ) const {
	return impl -> getCallId ( );
}

void ResponseMIMEInfo :: setContact ( const ss :: string & s ) {
	impl -> setContact ( s );
}

void ResponseMIMEInfo :: setContact ( const ContactHeader & s ) {
	impl -> setContact ( s );
}

const ContactHeader & ResponseMIMEInfo :: getContact ( ) const {
	return impl -> getContact ( );
}

void ResponseMIMEInfo :: setContentDisposition ( const ss :: string & s ) {
	impl -> setContentDisposition ( s );
}

void ResponseMIMEInfo :: setContentDisposition ( const ContentDisposition & d ) {
	impl -> setContentDisposition ( d);
}

ContentDisposition ResponseMIMEInfo :: getContentDisposition ( ) const {
	return impl -> getContentDisposition ( );
}

void ResponseMIMEInfo :: setCseq ( const ss :: string & s ) {
	impl -> setCseq ( s );
}

void ResponseMIMEInfo :: setCseq ( const CSeq & s ) {
	impl -> setCseq ( s );
}

const CSeq & ResponseMIMEInfo :: getCseq ( ) const {
	return impl -> getCseq ( );
}

void ResponseMIMEInfo :: setDate ( const boost :: posix_time :: ptime & s ) {
	impl -> setDate ( s );
}

void ResponseMIMEInfo :: setExpires ( unsigned e ) {
	impl -> setExpires ( e );
}

bool ResponseMIMEInfo :: hasExpires ( ) const {
	return impl -> hasExpires ( );
}

unsigned ResponseMIMEInfo :: getExpires ( ) const {
	return impl -> getExpires ( );
}

void ResponseMIMEInfo :: setFrom ( const ss :: string & s ) {
	impl -> setFrom ( s );
}

void ResponseMIMEInfo :: setFrom ( const FromToHeader & s ) {
	impl -> setFrom ( s );
}

const FromToHeader & ResponseMIMEInfo :: getFrom ( ) const {
	return impl -> getFrom ( );
}

void ResponseMIMEInfo :: setMinExpires ( unsigned s ) {
	impl -> setMinExpires ( s );
}

unsigned ResponseMIMEInfo :: getMinExpires ( ) const {
	return impl -> getMinExpires ( );
}

void ResponseMIMEInfo :: setMinSE ( unsigned s ) {
	impl -> setMinSE ( s );
}

unsigned ResponseMIMEInfo :: getMinSE ( ) const {
	return impl -> getMinSE ( );
}

void ResponseMIMEInfo :: addProxyAuthenticate ( const DigestChallenge & s ) {
	impl -> addProxyAuthenticate ( s );
}

void ResponseMIMEInfo :: addRecordRoute ( const ss :: string & s ) {
	impl -> addRecordRoute ( s );
}

void ResponseMIMEInfo :: addRecordRoute ( const RouteHeader & s ) {
	impl -> addRecordRoute ( s );
}

void ResponseMIMEInfo :: setRecvInfo ( const ss :: string & s ) {
	impl -> setRecvInfo ( s );
}

void ResponseMIMEInfo :: setRecvInfo ( unsigned a ) {
	impl -> setRecvInfo ( a );
}

bool ResponseMIMEInfo :: getRecvInfo ( InfoPackages p ) const {
	return impl -> getRecvInfo ( p );
}

bool ResponseMIMEInfo :: hasRecvInfo ( ) const {
	return impl -> hasRecvInfo ( );
}

void ResponseMIMEInfo :: setRemotePartyID ( const ss :: string & s ) {
	impl -> setRemotePartyID ( s );
}

const ss :: string & ResponseMIMEInfo :: getRemotePartyID ( ) const {
	return impl -> getRemotePartyID ( );
}

const RouteHeaderVector & ResponseMIMEInfo :: getRecordRoute ( ) const {
	return impl -> getRecordRoute ( );
}

void ResponseMIMEInfo :: setRequire ( const ss :: string & s ) {
	impl -> setRequire ( s );
}

void ResponseMIMEInfo :: addRequire ( const ss :: string & s ) {
	impl -> addRequire ( s );
}

const StringVector & ResponseMIMEInfo :: getRequire ( ) const {
	return impl -> getRequire ( );
}

void ResponseMIMEInfo :: setRetryAfter ( const RetryAfter & s ) {
	impl -> setRetryAfter ( s );
}

void ResponseMIMEInfo :: addRoute ( const ss :: string & s ) {
	impl -> addRoute ( s );
}

void ResponseMIMEInfo :: addRoute ( const RouteHeader & s ) {
	impl -> addRoute ( s );
}

void ResponseMIMEInfo :: setRSeq ( unsigned r ) {
	impl -> setRSeq ( r );
}

void ResponseMIMEInfo :: setServer ( const ss :: string & s ) {
	impl -> setServer ( s );
}

void ResponseMIMEInfo :: setSessionExpires ( const SessionExpires & e ) {
	impl -> setSessionExpires ( e );
}

void ResponseMIMEInfo :: setSessionExpires ( unsigned d, const ss :: string & r ) {
	impl -> setSessionExpires ( d, r );
}

void ResponseMIMEInfo :: setSessionExpires ( const ss :: string & s ) {
	impl -> setSessionExpires ( s );
}

bool ResponseMIMEInfo :: hasSessionExpires ( ) const {
	return impl -> hasSessionExpires ( );
}

const SessionExpires & ResponseMIMEInfo :: getSessionExpires ( ) const {
	return impl -> getSessionExpires ( );
}

void ResponseMIMEInfo :: setSupported ( const ss :: string & s ) {
	impl -> setSupported ( s );
}

void ResponseMIMEInfo :: addSupported ( const ss :: string & s ) {
	impl -> addSupported ( s );
}

const StringVector & ResponseMIMEInfo :: getSupported ( ) const {
	return impl -> getSupported ( );
}

void ResponseMIMEInfo :: setTimestamp ( const ss :: string & s ) {
	impl -> setTimestamp ( s );
}

void ResponseMIMEInfo :: setTo ( const ss :: string & s ) {
	impl -> setTo ( s );
}

void ResponseMIMEInfo :: setTo ( const FromToHeader & s ) {
	impl -> setTo ( s );
}

const FromToHeader & ResponseMIMEInfo :: getTo ( ) const {
	return impl -> getTo ( );
}

void ResponseMIMEInfo :: setUnsupported ( const ss :: string & s ) {
	impl -> setUnsupported ( s );
}

void ResponseMIMEInfo :: addUnsupported ( const ss :: string & s ) {
	impl -> addUnsupported ( s );
}

const StringVector & ResponseMIMEInfo :: getUnsupported ( ) const {
	return impl -> getUnsupported ( );
}

void ResponseMIMEInfo :: addVia ( const ss :: string & s ) {
	impl -> addVia ( s );
}

void ResponseMIMEInfo :: addVia ( const Via & s ) {
	impl -> addVia ( s );
}

const ViaVector & ResponseMIMEInfo :: getVia ( ) const {
	return impl -> getVia ( );
}

void ResponseMIMEInfo :: addWarning ( const Warning & s ) {
	impl -> addWarning ( s );
}

void ResponseMIMEInfo :: addWarning ( const ss :: string & s ) {
	impl -> addWarning ( s );
}

void ResponseMIMEInfo :: addWwwAuthenticate ( const DigestChallenge & s ) {
	impl -> addWwwAuthenticate ( s );
}

const DigestChallengeVector & ResponseMIMEInfo :: getWwwAuthenticate ( ) const {
	return impl -> getWwwAuthenticate ( );
}

class PrintPtrVisitor : public boost :: static_visitor < std :: ostream & > {
        std :: ostream & os;
public:
        explicit PrintPtrVisitor ( std :: ostream & os ) : os ( os ) { }
        template < typename T > std :: ostream & operator() ( T * const & t ) const {
                return os << * t;
        }
        template < typename T > std :: ostream & operator() ( T const & t ) const {
                return os << t;
        }
};

class DeepCopyVisitor : public boost :: static_visitor < > {
public:
        template < typename T > void operator() ( T & ) const { }
        template < typename T > void operator() ( T * & t ) const {
                t = new T ( * t );
        }
};

class  DeleteVisitor : public boost :: static_visitor < > {
public:
        template < typename T > void operator() ( T & ) const { }
        template < typename T > void operator() ( T * & t ) const {
                safeDel ( t );
        }
};

class Request :: Impl : public Allocatable < __SS_ALLOCATOR > {
	Methods method;
	URI requestUri;
	RequestMIMEInfo mime;
	ss :: string origBodyMd5;
	boost :: variant < boost :: blank, SDP :: SessionDescription *, DTMF :: Relay > body;
	Impl & operator= ( const Impl & );
public:
	Impl ( const Impl & i ) : Allocatable < __SS_ALLOCATOR > ( i ), method ( i.method ), requestUri ( i.requestUri ),
		mime ( i.mime ), origBodyMd5 ( i.origBodyMd5 ), body ( i.body ) {
		boost :: apply_visitor ( DeepCopyVisitor ( ), body );
	}
	~Impl ( ) {
		boost :: apply_visitor ( DeleteVisitor ( ), body );
	}
	Impl ( const ss :: string :: const_iterator & ib, const ss :: string :: const_iterator & ie );
	Impl ( Dialog & d, Methods m, int snum = 0, const ss :: string & branch = ss :: string ( ) ); //request inside dialog
	Impl ( const ss :: string & host, const ss :: string & user, int port, const ss :: string & localHost,
		const ss :: string & localUser, const ss :: string & callId, unsigned maxForwards, unsigned cseq,
		std :: auto_ptr < SDP :: SessionDescription > s ); //invite
	Impl ( const ss :: string & host, const ss :: string & user, int port, const ss :: string & callId, unsigned seq );
	//register
	Impl ( const Request & invite, const Response & r ); // ack for non-2xx
	void printOn ( std :: ostream & os ) const;
	RequestMIMEInfo & getMime ( ) {
		return mime;
	}
	const RequestMIMEInfo & getMime ( ) const {
		return mime;
	}
	Methods getMethod ( ) const {
		return method;
	}
	const URI & getRequestUri ( ) const {
		return requestUri;
	}
	void setSdp ( SDP :: SessionDescription * s ) {
		std :: auto_ptr < SDP :: SessionDescription > t ( s );
		boost :: apply_visitor ( DeleteVisitor ( ), body );
		body = s;
		mime.setContentDisposition ( ContentDisposition ( ContentDisposition :: session, true ) );
		t.release ( );
	}
	const SDP :: SessionDescription * getSdp ( ) const {
		return body.which ( ) == 1 ? boost :: get < SDP :: SessionDescription * > ( body ) : 0;
	}
	void setDtmfRelay ( const DTMF :: Relay & r, bool supportsPackage ) {
		boost :: apply_visitor ( DeleteVisitor ( ), body );
		body = r;
		if ( supportsPackage ) {
			mime.setContentDisposition ( ContentDisposition ( ContentDisposition :: infoPackage, true ) );
			mime.setInfoPackage ( ipDtmf );
		} else {
			mime.setContentDisposition ( ContentDisposition ( ContentDisposition :: signal, true ) );
			mime.setInfoPackage ( ipNil );
		}
	}
	const DTMF :: Relay * getDtmfRelay ( ) const {
		return boost :: get < DTMF :: Relay > ( & body );
		
	}
	bool emptyBody ( ) const {
		return body.which ( ) == 0;
	}
	const ss :: string & getOrigBodyMd5 ( ) const {
		return origBodyMd5;
	}
	void printBody ( std :: ostream & os ) const {
		boost :: apply_visitor ( PrintPtrVisitor ( os ), body );
	}
};

Request :: Impl :: Impl ( const ss :: string :: const_iterator & ib, const ss :: string :: const_iterator & ie ) {
	RightParser :: result_t info = parseSip ( ib, ie );
	if ( ! info.full )
		throw std :: runtime_error ( std :: string ( "error parsing sip request: " ).append ( ib, ie ) );
	RightParser :: iter_t message = info.trees.begin ( );
	RightParser :: iter_t line = message -> children.begin ( );
	if ( line -> value.id ( ).to_long ( ) != sip_grammar_requestLine_id )
		throw std :: runtime_error ( std :: string (
			"sip response passed to request constructor: " ).append ( ib, ie ) );
	RightParser :: iter_t i = line -> children.begin ( );
	ss :: string m ( i -> value.begin ( ), i -> value.end ( ) );
	method = parseMethod ( m );
	++ i;
	parseUri ( requestUri, i );
	++ line;
	ss :: string contentType;
	while ( line != message -> children.end ( ) && line -> value.id ( ).to_long ( ) == sip_grammar_messageHeader_id ) {
		i = line -> children.begin ( );
		ss :: string hname ( i -> value.begin ( ), i -> value.end ( ) );
		boost :: algorithm :: to_lower ( hname );
		ss :: string hvalue;
		if ( ++ i != line -> children.end ( ) )
			hvalue.assign ( i -> value.begin ( ), i -> value.end ( ) );
		if ( hname == "accept" )
			mime.addAccept ( hvalue );
		else if ( hname == "allow" )
			mime.setAllow ( hvalue );
		else if ( hname == "authorization" )
			mime.addAuthorization ( DigestResponse ( hvalue ) );
		else if ( hname == "call-id" || hname == "i" )
			mime.setCallId ( hvalue );
		else if ( hname == "contact" || hname == "m" )
			mime.setContact ( hvalue );
		else if ( hname == "content-type" || hname == "c" )
			contentType.swap ( hvalue );
		else if ( hname == "cseq" )
			mime.setCseq ( hvalue );
		else if ( hname == "expires" )
			mime.setExpires ( std :: atoi ( hvalue.c_str ( ) ) );
		else if ( hname == "from" || hname == "f" )
			mime.setFrom ( hvalue );
		else if ( hname == "info-package" )
			mime.setInfoPackage ( hvalue );
		else if ( hname == "max-forwards" )
			mime.setMaxForwards ( std :: atoi ( hvalue.c_str ( ) ) );
		else if ( hname == "min-se" )
			mime.setMinSE ( std :: atoi ( hvalue.c_str ( ) ) );
		else if ( hname == "p-asserted-identity" )
			mime.setPAssertID ( hvalue );
		else if ( hname == "privacy" )
			mime.setPrivacy ( hvalue );
		else if ( hname == "proxy-authorization" )
			mime.addProxyAuthorization ( DigestResponse ( hvalue ) );
		else if ( hname == "rack" )
			mime.setRAck ( RAck ( hvalue ) );
		else if ( hname == "record-route" )
			mime.addRecordRoute ( hvalue );
		else if ( hname == "recv-info" )
			mime.setRecvInfo ( hvalue );
		else if ( hname == "require" )
			mime.setRequire ( hvalue );
		else if ( hname == "remote-party-id" )
			mime.setRemotePartyID ( hvalue );
		else if ( hname == "route" )
			mime.addRoute ( hvalue );
		else if ( hname == "session-expires" || hname == "x" )
			mime.setSessionExpires ( hvalue );
		else if ( hname == "supported" || hname == "k" )
			mime.setSupported ( hvalue );
		else if ( hname == "timestamp" )
			mime.setTimestamp ( hvalue );
		else if ( hname == "to" || hname == "t" )
			mime.setTo ( hvalue );
		else if ( hname == "user-agent" )
			mime.setUserAgent ( hvalue );
		else if ( hname == "via" || hname == "v" )
			mime.addVia ( hvalue );
		++ line;
	}
	if ( line != message -> children.end ( ) ) {
		if ( ! mime.getAuthorization ( ).empty ( ) )
			origBodyMd5 = md5Sum ( & * line -> value.begin ( ), line -> value.end ( ) - line -> value.begin ( ) );
		ContentType ct ( contentType );
		if ( ct.getType ( ) == "application" ) {
			if ( ct.getSubType ( ) == "sdp" ) {
				body = new SDP :: SessionDescription ( line -> value.begin ( ), line -> value.end ( ) );
				return;
			}
			if ( ct.getSubType ( ) == "dtmf-relay" ) {
				body = DTMF :: Relay ( line -> value.begin ( ), line -> value.end ( ) );
				return;
			}
		}
		std :: ostringstream os;
		os << "unsupported content-type: " << ct;
		throw std :: runtime_error ( os.str ( ) );
	}
}

ss :: string toHex ( const ss :: string & s ) { // from sipcommon
	ss :: ostringstream os;
	typedef unsigned char uchar;
	for ( ss :: string :: size_type i = 0, ss = s.size ( ); i < ss; i ++ )
		os << std :: setw ( 2 ) << std :: setfill ( '0' ) << std :: hex << unsigned ( uchar ( s [ i ] ) );
	return os.str ( );
}

static ss :: string generateBranchLikeUuidWith_z9hG4bK ( ) {
	return "z9hG4bK" + toHex ( H323 :: globallyUniqueId ( ) );
}

static ss :: string generateTagLikeUuid ( ) {
	return toHex ( H323 :: globallyUniqueId ( ) );
}

template < class T > void setAllow ( T & m ) {
	m.setAllow ( ( 1 << Request :: mInvite ) | ( 1 << Request :: mAck ) | ( 1 << Request :: mCancel ) |
		( 1 << Request :: mBye ) | ( 1 << Request :: mRegister ) | ( 1 << Request :: mOptions ) );
}

template < class T > void setRecvInfo ( T & m ) {
	m.setRecvInfo ( 1 << ipDtmf );
}

template < class T > void setAccept ( T & m ) {
	static ss :: string application = "application", sdp = "sdp", dtmfRelay = "dtmf-relay";
	m.addAccept ( AcceptRange ( application, sdp ) );
	m.addAccept ( AcceptRange ( application, dtmfRelay ) );
}

Request :: Impl :: Impl ( Dialog & d, Methods m, int snum, const ss :: string & branch ) : method ( m ) {
	//request inside dialog
	FromToHeader t;
	t.setUri ( d.getRemoteUri ( ) );
	t.setTag ( d.getId ( ).remoteTag );
	mime.setTo ( t );
	t.setUri ( d.getLocalUri ( ) );
	t.setTag ( d.getId ( ).localTag );
	mime.setFrom ( t );
	mime.setCallId ( d.getId ( ).callId );
	CSeq s;
	s.method = m;
	if ( m == mAck || m == mCancel )
		s.seq = snum;
	else {
		d.incLocalSeq ( );
		s.seq = d.getLocalSeq ( );
		setAllow ( mime );
		setAccept ( mime );
	}
	mime.setCseq ( s );
	if ( m != mCancel ) { // cancel dolgen imet route originala, t.e. invite, a on u nas pustoy
		const RouteHeaderVector & v = d.getRouteSet ( );
		if ( v.empty ( ) )
			requestUri = d.getRemoteTarget ( );
		else {
			if ( v [ 0 ].getUri ( ).hasParameter ( "lr" ) ) {
				requestUri = d.getRemoteTarget ( );
				requestUri.setParameter ( "lr", "" );
				mime.setRoute ( v );
			} else {
				requestUri = v [ 0 ].getUri ( );
				for ( std :: size_t i = 1, vs = v.size ( ); i < vs; i ++ )
					mime.addRoute ( v [ i ] );
				RouteHeader t;
				t.setUri ( d.getRemoteTarget ( ) );
				mime.addRoute ( t );
			}
		}
	} else
		requestUri = d.getRemoteUri ( );
	ContactHeader c;
	c.setUri ( d.getLocalUri ( ) );
	mime.setContact ( c );
	mime.setMaxForwards ( 70 );
	Via via;
	via.setTransport ( "UDP" );
	if ( ( m != mAck || branch.empty ( ) ) && m != mCancel ) // only ack for non-2xx have same branch
		via.setBranch ( generateBranchLikeUuidWith_z9hG4bK ( ) );
	else
		via.setBranch ( branch );
	via.setRport ( );
	mime.addVia ( via );
	mime.setUserAgent ( "PSBC SIP v2.0" );
	if ( m == mAck || m == mInvite || m == mOptions || m == mPrack || m == mUpdate )
		setRecvInfo ( mime );
}

Request :: Impl :: Impl ( const ss :: string & host, const ss :: string & user, int port, const ss :: string & localHost,
	const ss :: string & localUser, const ss :: string & callId, unsigned maxForwards, unsigned cseq,
	std :: auto_ptr < SDP :: SessionDescription > s ) :  method ( mInvite ) {
	//invite
	setAllow ( mime );
	setAccept ( mime );
	requestUri.setHost ( host );
	requestUri.setPort ( port );
	requestUri.setUser ( user );
	FromToHeader n;
	n.setUri ( requestUri );
	mime.setTo ( n );
	URI t;
	t.setHost ( localHost );
	t.setUser ( localUser );
	n.setUri ( t );
	n.setTag ( generateTagLikeUuid ( ) );
	mime.setFrom ( n );
	mime.setCallId ( callId );
	CSeq cs ( mInvite, cseq );
	mime.setCseq ( cs );
	mime.setMaxForwards ( maxForwards );
	ContactHeader c;
	c.setUri ( t );
	mime.setContact ( c );
	Via via;
	via.setTransport ( "UDP" );
	via.setBranch ( generateBranchLikeUuidWith_z9hG4bK ( ) );
	via.setRport ( );
	mime.addVia ( via );
	mime.setUserAgent ( "PSBC SIP v2.0" );
	setRecvInfo ( mime );
	body = s.get ( );
	s.release ( );
}

Request :: Impl :: Impl ( const ss :: string & host, const ss :: string & user, int port, const ss :: string & callId,
	unsigned seq ) : method ( mRegister ) {
	//register
	setAllow ( mime );
	setAccept ( mime );
	requestUri.setHost ( host );
	requestUri.setPort ( port );
	FromToHeader n;
	URI u;
	u.setHost ( host );
	u.setUser ( user );
	n.setUri ( u );
	mime.setTo ( n );
	mime.setFrom ( n );
	mime.setCallId ( callId );
	CSeq cs ( mRegister, seq );
	mime.setCseq ( cs );
	mime.setMaxForwards ( 70 );
	Via via;
	via.setTransport ( "UDP" );
	via.setBranch ( generateBranchLikeUuidWith_z9hG4bK ( ) );
	via.setRport ( );
	mime.addVia ( via );
	mime.setUserAgent ( "PSBC SIP v2.0" );
	setRecvInfo ( mime );
}

Request :: Impl :: Impl ( const Request & invite, const Response & r ) : method ( mAck ) {
	const RequestMIMEInfo & im = invite.getMime ( );
	requestUri = invite.getRequestUri ( );
	mime.setCallId ( im.getCallId ( ) );
	mime.setFrom ( im.getFrom ( ) );
	const ResponseMIMEInfo & rm = r.getMime ( );
	mime.setTo ( rm.getTo ( ) );
	mime.addVia ( im.getVia ( ).front ( ) );
	mime.setCseq ( CSeq ( mAck, im.getCseq ( ).seq ) );
	mime.setRoute ( im.getRoute ( ) );
	mime.setUserAgent ( "PSBC SIP v2.0" );
	mime.setMaxForwards ( 70 );
	setRecvInfo ( mime );
}

class PrintContentTypeVisitor : public boost :: static_visitor < > {
	std :: ostream & os;
public:
	explicit PrintContentTypeVisitor ( std :: ostream & os ) : os ( os ) { }
	void operator() ( const boost :: blank & ) const { }
	void operator() ( SDP :: SessionDescription * const & ) const {
		os << "Content-Type: application/sdp\r\n";
	}
	void operator() ( DTMF :: Relay const & ) const {
		os << "Content-Type: application/dtmf-relay\r\n";
	}
};

void Request :: Impl :: printOn ( std :: ostream & os ) const {
	os << methodName ( method ) << ' ' << requestUri << " SIP/2.0\r\n" << mime;
	ss :: string ssdp;
	if ( body.which ( ) ) {
		ss :: ostringstream osdp;
		printBody ( osdp );
		osdp.str ( ).swap ( ssdp );
		boost :: apply_visitor ( PrintContentTypeVisitor ( os ), body );
	}
	os << "Content-Length: " << ssdp.size ( ) << "\r\n\r\n" << ssdp;
}

Request :: Request ( const Request & m ) : impl ( new Impl ( * m.impl ) ) { }

Request :: ~Request ( ) {
	delete impl;
}

Request :: Request ( const ss :: string :: const_iterator & ib, const ss :: string :: const_iterator & ie ) :
	impl ( new Impl ( ib, ie ) ) { }

Request :: Request ( Dialog & d, Methods m, int snum, ss :: string branch ) : impl ( new Impl ( d, m, snum, branch ) ) { }
//request inside dialog

Request :: Request ( const ss :: string & host, const ss :: string & user, int port, const ss :: string & localHost,
	const ss :: string & localUser, const ss :: string & callId, unsigned maxForwards, unsigned cseq,
	SDP :: SessionDescription * sdp ) { //invite
	std :: auto_ptr < SDP :: SessionDescription > tsdp ( sdp );
	impl = new Impl ( host, user, port, localHost, localUser, callId, maxForwards, cseq, tsdp );
}

Request :: Request ( const ss :: string & host, const ss :: string & user, int port, const ss :: string & callId,
	unsigned seq ) : impl ( new Impl ( host, user, port, callId, seq ) ) { } //register

Request :: Request ( const Request & invite, const Response & r ) : impl ( new Impl ( invite, r ) ) { } // ack for non-2xx

RequestMIMEInfo & Request :: getMime ( ) {
	return impl -> getMime ( );
}

const RequestMIMEInfo & Request :: getMime ( ) const {
	return impl -> getMime ( );
}

Request :: Methods Request :: getMethod ( ) const {
	return impl -> getMethod ( );
}

const URI & Request :: getRequestUri ( ) const {
	return impl -> getRequestUri ( );
}

void Request :: setSdp ( SDP :: SessionDescription * s ) {
	impl -> setSdp ( s );
}

const SDP :: SessionDescription * Request :: getSdp ( ) const {
	return impl -> getSdp ( );
}

void Request :: setDtmfRelay ( const DTMF :: Relay & r, bool supportsPackage ) {
	impl -> setDtmfRelay ( r, supportsPackage );
}

const DTMF :: Relay * Request :: getDtmfRelay ( ) const {
	return impl -> getDtmfRelay ( );
}

bool Request :: emptyBody ( ) const {
	return impl -> emptyBody ( );
}

const ss :: string & Request :: getOrigBodyMd5 ( ) const {
	return impl -> getOrigBodyMd5 ( );
}

void Request :: printBody ( std :: ostream & os ) const {
	impl -> printBody ( os );
}

void Request :: printOn ( std :: ostream & os ) const {
	impl -> printOn ( os );
}

class Response :: Impl : public Allocatable < __SS_ALLOCATOR > {
	int statusCode;
	ss :: string reasonPhrase;
	ResponseMIMEInfo mime;
	ss :: string origBodyMd5;
	boost :: variant < boost :: blank, SDP :: SessionDescription * > body;
	Impl & operator= ( const Impl & );
public:
	Impl ( const Impl & i ) : Allocatable < __SS_ALLOCATOR > ( i ), statusCode ( i.statusCode ),
		reasonPhrase ( i.reasonPhrase ), mime ( i.mime ), body ( i.body ) {
		boost :: apply_visitor ( DeepCopyVisitor ( ), body );
	}
	~Impl ( ) {
		boost :: apply_visitor ( DeleteVisitor ( ), body );
	}
	Impl ( const ss :: string :: const_iterator & ib, const ss :: string :: const_iterator & ie );
	Impl ( const Request & r, StatusCodes c, const ss :: string & tag, const ss :: string & rf );
	Impl ( const Request & r, StatusCodes c, const Dialog & d, const ss :: string & rf );
	void printOn ( std :: ostream & os ) const;
	ResponseMIMEInfo & getMime ( ) {
		return mime;
	}
	const ResponseMIMEInfo & getMime ( ) const {
		return mime;
	}
	int getStatusCode ( ) const {
		return statusCode;
	}
	const ss :: string & getReasonPhrase ( ) const {
		return reasonPhrase;
	}
	void setSdp ( SDP :: SessionDescription * s ) {
		std :: auto_ptr < SDP :: SessionDescription > t ( s );
		boost :: apply_visitor ( DeleteVisitor ( ), body );
		body = s;
		mime.setContentDisposition ( ContentDisposition ( ContentDisposition :: session, true ) );
		t.release ( );
	}
	const SDP :: SessionDescription * getSdp ( ) const {
		return body.which ( ) == 1 ? boost :: get < SDP :: SessionDescription * > ( body ) : 0;
	}
	const ss :: string & getOrigBodyMd5 ( ) const {
		return origBodyMd5;
	}
	void printBody ( std :: ostream & os ) const {
		boost :: apply_visitor ( PrintPtrVisitor ( os ), body );
	}
};

static boost :: posix_time :: ptime fromRfc1123 ( const ss :: string & s ) {
	static std :: locale loc ( std :: locale :: classic ( ),
		new boost :: posix_time :: time_input_facet ( "%a, %d %b %Y %H:%M:%S GMT" ) );
	ss :: istringstream is ( s );
	is.exceptions ( std :: ios_base :: failbit | std :: ios_base :: badbit );
	is.imbue ( loc );
	boost :: posix_time :: ptime t;
	is >> t;
	return t;
}

Response :: Impl :: Impl ( const ss :: string :: const_iterator & ib, const ss :: string :: const_iterator & ie ) {
	RightParser :: result_t info = parseSip ( ib, ie );
	if ( ! info.full )
		throw std :: runtime_error ( std :: string ( "error parsing sip response: " ).append ( ib, ie ) );
	RightParser :: iter_t message = info.trees.begin ( );
	RightParser :: iter_t line = message -> children.begin ( );
	if ( line -> value.id ( ).to_long ( ) != sip_grammar_statusLine_id )
		throw std :: runtime_error ( std :: string (
			"sip request passed to response constructor: " ).append ( ib, ie ) );
	RightParser :: iter_t i = line -> children.begin ( );
	statusCode = std :: atoi ( ss :: string ( i -> value.begin ( ), i -> value.end ( ) ).c_str ( ) );
	++ i;
	reasonPhrase.assign ( unEscape ( i -> value.begin ( ), i -> value.end ( ) ) );
	++ line;
	ss :: string contentType;
	while ( line != message -> children.end ( ) && line -> value.id ( ).to_long ( ) == sip_grammar_messageHeader_id ) {
		i = line -> children.begin ( );
		ss :: string hname ( i -> value.begin ( ), i -> value.end ( ) );
		boost :: algorithm :: to_lower ( hname );
		ss :: string hvalue;
		if ( ++ i != line -> children.end ( ) )
			hvalue.assign ( i -> value.begin ( ), i -> value.end ( ) );
		if ( hname == "accept" )
			mime.addAccept ( hvalue );
		else if ( hname == "allow" )
			mime.setAllow ( hvalue );
		else if ( hname == "call-id" || hname == "i" )
			mime.setCallId ( hvalue );
		else if ( hname == "contact" || hname == "m" )
			mime.setContact ( hvalue );
		else if ( hname == "content-type" || hname == "c" )
                	contentType.swap ( hvalue );
		else if ( hname == "cseq" )
			mime.setCseq ( hvalue );
		else if ( hname == "date" )
			mime.setDate ( fromRfc1123 ( hvalue ) );
		else if ( hname == "expires" )
			mime.setExpires ( std :: atoi ( hvalue.c_str ( ) ) );
		else if ( hname == "from" || hname == "f" )
			mime.setFrom ( hvalue );
		else if ( hname == "min-expires" )
			mime.setMinExpires ( std :: atoi ( hvalue.c_str ( ) ) );
		else if ( hname == "min-se" )
			mime.setMinSE ( std :: atoi ( hvalue.c_str ( ) ) );
		else if ( hname == "proxy-authenticate" )
			mime.addProxyAuthenticate ( DigestChallenge ( hvalue ) );
		else if ( hname == "record-route" )
			mime.addRecordRoute ( hvalue );
		else if ( hname == "recv-info" )
			mime.setRecvInfo ( hvalue );
		else if ( hname == "remote-party-id" )
			mime.setRemotePartyID ( hvalue );
		else if ( hname == "require" )
			mime.setRequire ( hvalue );
		else if ( hname == "retry-after" )
			mime.setRetryAfter ( RetryAfter ( hvalue ) );
		else if ( hname == "route" )
			mime.addRoute ( hvalue );
		else if ( hname == "rseq" )
			mime.setRSeq ( atoi ( hvalue.c_str ( ) ) );
		else if ( hname == "server" )
			mime.setServer ( hvalue );
		else if ( hname == "session-expires" || hname == "x" )
			mime.setSessionExpires ( hvalue );
		else if ( hname == "supported" || hname == "k" )
			mime.setSupported ( hvalue );
		else if ( hname == "timestamp" )
			mime.setTimestamp ( hvalue );
		else if ( hname == "to" || hname == "t" )
			mime.setTo ( hvalue );
		else if ( hname == "unsupported" )
			mime.setUnsupported ( hvalue );
		else if ( hname == "via" || hname == "v" )
			mime.addVia ( hvalue );
		else if ( hname == "warning" )
			mime.addWarning ( hvalue );
		else if ( hname == "www-authenticate" )
			mime.addWwwAuthenticate ( DigestChallenge ( hvalue ) );
		++ line;
	}
	if ( line != message -> children.end ( ) ) {
		if ( mime.hasAuthenticationInfo ( ) )
			origBodyMd5 = md5Sum ( & * line -> value.begin ( ), line -> value.end ( ) - line -> value.begin ( ) );
		ContentType ct ( contentType );
		if ( ct.getType ( ) == "application" && ct.getSubType ( ) == "sdp" ) {
			body = new SDP :: SessionDescription ( line -> value.begin ( ), line -> value.end ( ) );
			return;
		}
		std :: ostringstream os;
		os << "unsupported content-type: " << ct;
		throw std :: runtime_error ( os.str ( ) );
	}
}

static const ss :: string & getReasonPhrase ( Response :: StatusCodes c ) {
	static ss :: string trying ( "Trying" ), ringing ( "Ringing" ), ok ( "OK" ), badrequest ( "Bad Request" ),
		unauthorized ( "Unauthorized" ), forbidden ( "Forbidden" ), notFound ( "Not Found" ),
		methodnotallowed ( "Method Not Allowed" ), proxyAuthRequired ( "Proxy Authentication Required" ),
		requesttimeout ( "Request Timeout" ), gone ( "Gone" ), unsupportedMediaType ( "Unsupported Media Type" ),
		badextension ( "Bad Extension" ), temporarilyUnavailable ( "Temporarily Unavailable" ),
		transactiondoesnotexist ( "Call/Transaction Does Not Exist" ), loopdetected ( "Loop Detected" ),
		toomanyhops ( "Too Many Hops" ), busyHere ( "Busy here" ), requestterminated ( "Request Terminated" ),
		notacceptablehere ( "Not Acceptable Here" ), badevent ( "Bad Event" ),
		serverinternalerror ( "Server Internal Error" ), notImplemented ( "Not Implemented" ),
		serviceUnavailable ( "Service Unavailable" ), faiulreDecline( "Decline" );
	switch ( c ) {
		case Response :: scInformationTrying:
			return trying;
		case Response :: scInformationRinging:
			return ringing;
		case Response :: scSuccessfulOK:
			return ok;
		case Response :: scFailureBadRequest:
			return badrequest;
		case Response :: scFailureUnAuthorized:
			return unauthorized;
		case Response :: scFailureForbidden:
			return forbidden;
		case Response :: scFailureNotFound:
			return notFound;
		case Response :: scFailureMethodNotAllowed:
			return methodnotallowed;
		case Response :: scFailureProxyAuthRequired:
			return proxyAuthRequired;
		case Response :: scFailureRequestTimeout:
			return requesttimeout; // sql
		case Response :: scFailureGone:
			return gone; // sql
		case Response :: scFailureUnsupportedMediaType:
			return unsupportedMediaType;
		case Response :: scFailureBadExtension:
			return badextension;
		case Response :: scFailureTemporarilyUnavailable:
			return temporarilyUnavailable;
		case Response :: scFailureTransactionDoesNotExist:
			return transactiondoesnotexist;
		case Response :: scFailureLoopDetected:
			return loopdetected;
		case Response :: scFailureTooManyHops:
			return toomanyhops;
		case Response :: scFailureBusyHere:
			return busyHere;	
		case Response :: scFailureRequestTerminated:
			return requestterminated;
		case Response :: scFailureNotAcceptableHere:
			return notacceptablehere;
		case Response :: scFailureBadEvent:
			return badevent;
		case Response :: scFailureServerInternalError:
			return serverinternalerror;
		case Response::scFailureNotImplemented:
			return notImplemented;
		case Response :: scFailureServiceUnavailable:
			return serviceUnavailable;
		case Response :: scFaiulreDecline:
			return faiulreDecline;
		default: {
			return ok;
/*  Roman temporary shit	
			std :: ostringstream os;
			os << "no reason phrase for code " << c;
			throw std :: runtime_error ( os.str ( ) );
*/
		}
	}
}

Response :: Impl :: Impl ( const Request & r, StatusCodes c, const ss :: string & tag, const ss :: string & rf ) :
	statusCode ( c ), reasonPhrase ( rf.empty ( ) ? SIP2 :: getReasonPhrase ( c ) : rf ) {
	const RequestMIMEInfo & m = r.getMime ( );
	if ( c == scInformationTrying )
		mime.setTimestamp ( m.getTimestamp ( ) );
	mime.setFrom ( m.getFrom ( ) );
	mime.setCallId ( m.getCallId ( ) );
	mime.setCseq ( m.getCseq ( ) );
	const ViaVector & v = m.getVia ( );
	for ( std :: size_t i = 0, vs = v.size ( ); i < vs; i ++ )
		mime.addVia ( v [ i ] );
	FromToHeader u = m.getTo ( );
	if ( u.getTag ( ).empty ( ) && c != scInformationTrying ) {
		if ( tag.empty ( ) )
			u.setTag ( generateTagLikeUuid ( ) );
		else
			u.setTag ( tag );
	}
	mime.setTo ( u );
	Request :: Methods method= r.getMethod ( );
	if ( c == scSuccessfulOK && method == Request :: mRegister )
		mime.setDate ( boost :: posix_time :: second_clock :: universal_time ( ) );
	mime.setServer ( "PSBC SIP v2.0" );
	if ( method != Request :: mAck && method != Request :: mCancel ) {
		setAllow ( mime );
		if ( ( c >= 200 && c < 300 ) || c == scFailureUnsupportedMediaType )
			setAccept ( mime );
	}
	if ( method == Request :: mAck || method == Request :: mOptions || method == Request :: mPrack ||
		method == Request :: mUpdate )
		setRecvInfo ( mime );
	if ( method != Request :: mInvite )
		return;
	if ( c < 300 ) {
		const RouteHeaderVector & rr = m.getRecordRoute ( );
		for ( std :: size_t i = 0, rs = rr.size ( ); i < rs; i ++ )
			mime.addRecordRoute ( rr [ i ] );
		setRecvInfo ( mime );
	}
	if ( c >= 200 && c < 300 ) {
		ContactHeader contact;
		contact.setUri ( r.getRequestUri ( ) );
		mime.setContact ( contact );
	}
}

Response :: Impl :: Impl ( const Request & r, StatusCodes c, const Dialog & d, const ss :: string & rf ) :
	statusCode ( c ), reasonPhrase ( rf.empty ( ) ? SIP2 :: getReasonPhrase ( c ) : rf ) {
	// otvet na invite no ne trying
	const RequestMIMEInfo & m = r.getMime ( );
	if ( c == scInformationTrying )
		throw std :: runtime_error ( "trying after dialog" );
	mime.setFrom ( m.getFrom ( ) );
	mime.setCallId ( m.getCallId ( ) );
	mime.setCseq ( m.getCseq ( ) );
	const ViaVector & v = m.getVia ( );
	for ( std :: size_t i = 0, vs = v.size ( ); i < vs; i ++ )
		mime.addVia ( v [ i ] );
	FromToHeader u = m.getTo ( );
	if ( u.getTag ( ).empty ( ) )
		u.setTag ( d.getId ( ).localTag );
	mime.setTo ( u );
	mime.setServer ( "PSBC SIP v2.0" );
	if ( c < 300 ) {
		const RouteHeaderVector & rr = m.getRecordRoute ( );
		for ( std :: size_t i = 0, rs = rr.size ( ); i < rs; i ++ )
			mime.addRecordRoute ( rr [ i ] );
		setRecvInfo ( mime );
	}
	if ( c >= 200 && c < 300 ) {
		ContactHeader contact;
		contact.setUri ( r.getRequestUri ( ) );
		mime.setContact ( contact );
	}
	setAllow ( mime );
	if ( ( c >= 200 && c < 300 ) || c == scFailureUnsupportedMediaType )
		setAccept ( mime );
}

static std :: bitset < 256 > initReasonChars ( ) {
	std :: bitset < 256 > s ( initAlnumChars ( "-_.!~*'()""/?:@&=+$, \t" ) );
	for ( int c = 128; c < 256; c ++ )
		s.set ( c );
	return s;
}

static std :: bitset < 256 > reasonChars = initReasonChars ( );

void Response :: Impl :: printOn ( std :: ostream & os ) const {
	os << "SIP/2.0 " << std :: setw ( 3 ) << statusCode << ' ';
	escape ( os, reasonPhrase, reasonChars );
	os << "\r\n" << mime;
	ss :: string ssdp;
	if ( body.which ( ) ) {
		ss :: ostringstream osdp;
		printBody ( osdp );
		osdp.str ( ).swap ( ssdp );
		boost :: apply_visitor ( PrintContentTypeVisitor ( os ), body );
	}
	os << "Content-Length: " << ssdp.size ( ) << "\r\n\r\n" << ssdp;
}

Response :: Response ( const ss :: string :: const_iterator & ib, const ss :: string :: const_iterator & ie ) :
	impl ( new Impl ( ib, ie ) ) { }

Response :: Response ( const Response & r ) : impl ( new Impl ( * r.impl ) ) { }

Response :: Response ( const Request & r, StatusCodes c, const ss :: string & tag, const ss :: string & rf ) :
	impl ( new Impl ( r, c, tag, rf ) ) { }

Response :: Response ( const Request & r, StatusCodes c, const Dialog & d, const ss :: string & rf ) :
	impl ( new Impl ( r, c, d, rf ) ) { }

Response :: ~Response ( ) {
	delete impl;
}

void Response :: printOn ( std :: ostream & os ) const {
	impl -> printOn ( os );
}

ResponseMIMEInfo & Response :: getMime ( ) {
	return impl -> getMime ( );
}

const ResponseMIMEInfo & Response :: getMime ( ) const {
	return impl -> getMime ( );
}

int Response :: getStatusCode ( ) const {
	return impl -> getStatusCode ( );
}

const ss :: string & Response :: getReasonPhrase ( ) const {
	return impl -> getReasonPhrase ( );
}

const SDP :: SessionDescription * Response :: getSdp ( ) const {
	return impl -> getSdp ( );
}

void Response :: setSdp ( SDP :: SessionDescription * sdp ) {
	impl -> setSdp ( sdp );
}

const ss :: string & Response :: getOrigBodyMd5 ( ) const {
	return impl -> getOrigBodyMd5 ( );
}

void Response :: printBody ( std :: ostream & os ) const {
	impl -> printBody ( os );
}

std :: ostream & operator<< ( std :: ostream & os, const CSeq & s ) {
	return os << s.seq << ' ' << methodName ( s.method );
}

std :: istream & operator>> ( std :: istream & is, CSeq & s ) {
	ss :: string t;
	is >> s.seq >> t;
	s.method = parseMethod ( t );
	return is;
}

DialogId makeDialogIdReceived ( const Request & r ) {
	const RequestMIMEInfo & m = r.getMime ( );
	DialogId id;
	id.localTag = m.getTo ( ).getTag ( );
	id.remoteTag = m.getFrom ( ).getTag ( );
	id.callId = m.getCallId ( );
	return id;
}

DialogId makeDialogIdReceived ( const Response & r ) {
	const ResponseMIMEInfo & m = r.getMime ( );
	DialogId id;
	id.localTag = m.getFrom ( ).getTag ( );
	id.remoteTag = m.getTo ( ).getTag ( );
	id.callId = m.getCallId ( );
	return id;
}

class Dialog :: Impl : public Allocatable < __SS_ALLOCATOR > {
	DialogId id;
	int localSeq;
	int remoteSeq;
	URI localUri, remoteUri;
	URI remoteTarget; // contact
	RouteHeaderVector routeSet;
	bool confirmed;
	bool supportsPackage, supportsDtmf;
	bool acceptsDtmfOverride;
	bool acceptsDtmf;
	bool allowsInfo;
	void handleAccept ( const boost :: optional < AcceptRangeSet > & accept, const SDP :: SessionDescription * sdp );
public:
	const DialogId & getId ( ) const {
		return id;
	}
	Impl ( ) : localSeq ( 0 ), remoteSeq ( 0 ), confirmed ( false ), supportsPackage ( false ), supportsDtmf ( false ),
		acceptsDtmfOverride ( false ), acceptsDtmf ( false ), allowsInfo ( false ) { }
	explicit Impl ( const Request & r ); // prishel invite
	Impl ( const Response & r, bool acceptsDtmfOverride ); // prishel otvet na invite
	void targetRefresh ( const Response & r );
	bool requestReceived ( const Request & r );
	const URI & getRemoteTarget ( ) const {
		return remoteTarget;
	}
	const URI & getRemoteUri ( ) const {
		return remoteUri;
	}
	const URI & getLocalUri ( ) const {
		return localUri;
	}
	int getLocalSeq ( ) const {
		return localSeq;
	}
	void incLocalSeq ( ) {
		localSeq ++;
	}
	const RouteHeaderVector & getRouteSet ( ) const {
		return routeSet;
	}
	bool isConfirmed ( ) const {
		return confirmed;
	}
	template < typename T > void handleRecvInfo ( const T & m ) {
		supportsPackage = m.hasRecvInfo ( );
		supportsDtmf = m.getRecvInfo ( ipDtmf );
	}
	bool getSupportsPackage ( ) const {
		return supportsPackage;
	}
	bool getSupportsDtmf ( ) const {
		return supportsDtmf;
	}
	bool getAcceptsDtmf ( ) const {
		return acceptsDtmfOverride || acceptsDtmf;
	}
	void handleAllow ( const ResponseMIMEInfo & m ) {
		allowsInfo = m.getAllow ( Request :: mInfo );
	}
	bool getAllowsInfo ( ) const {
		return allowsInfo;
	}
};

Dialog :: Impl :: Impl ( const Request & r ) : acceptsDtmfOverride ( false ) { // prishel invite
	const RequestMIMEInfo & m = r.getMime ( );
	id.callId = m.getCallId ( );
	id.localTag = generateTagLikeUuid ( );
	id.remoteTag = m.getFrom ( ).getTag ( );
	routeSet = m.getRecordRoute ( );
	remoteTarget = m.getContact ( ).getUri ( ); //teryaetsya name ??
	remoteSeq = m.getCseq ( ).seq;
	localSeq = 0;
	remoteUri = m.getFrom ( ).getUri ( );
	localUri = m.getTo ( ).getUri ( );
	confirmed = false;
	handleRecvInfo ( m );
	handleAccept ( m.getAccept ( ), 0 );
	allowsInfo = m.getAllow ( Request :: mInfo );
}

Dialog :: Impl :: Impl ( const Response & r, bool acceptsDtmfOverride ) : id ( makeDialogIdReceived ( r ) ),
	acceptsDtmfOverride ( acceptsDtmfOverride ) { // prishel otvet na invite
	if ( id.remoteTag.empty ( ) )
		throw std :: runtime_error ( "constructing dialog without remote tag" );
	const ResponseMIMEInfo & m = r.getMime ( );
	const RouteHeaderVector & v = m.getRecordRoute ( );
	routeSet.reserve ( v.size ( ) );
	std :: copy ( v.rbegin ( ), v.rend ( ), std :: back_inserter ( routeSet ) );
	remoteTarget = m.getContact ( ).getUri ( );
	localSeq = m.getCseq ( ).seq;
	remoteSeq = 0;
	remoteUri = m.getTo ( ).getUri ( );
	localUri = m.getFrom ( ).getUri ( );
	int t = r.getStatusCode ( );
	confirmed = t >= 200 && t < 300;
	handleRecvInfo ( m );
	handleAccept ( m.getAccept ( ), r.getSdp ( ) );
	allowsInfo = m.getAllow ( Request :: mInfo );
}

void Dialog :: Impl :: handleAccept ( const boost :: optional < AcceptRangeSet > & accept,
	const SDP :: SessionDescription * sdp ) {
	if ( acceptsDtmfOverride )
		return;
	if ( sdp ) {
		const ss :: string & name = sdp -> getSessionName ( );
		if ( name == "AddPac Gateway SDP" || name == "eyeBeam" ) {
			acceptsDtmfOverride = true;
			return;
		}
	}
	if ( ! accept ) {
		acceptsDtmf = false;
		return;
	}
	const AcceptRangeSet & a = * accept;
	static AcceptRange any ( "*", "*"), anyApp ( "application", "*" ), dtmf ( "application", "dtmf-relay" );
	acceptsDtmf = a.count ( dtmf ) || a.count ( anyApp ) || a.count ( any );
}

void Dialog :: Impl :: targetRefresh ( const Response & r ) {
	const ResponseMIMEInfo & m =  r.getMime ( );
	if ( m.getCseq ( ).method == Request :: mInvite )
		confirmed = true;
	const URI & u = m.getContact ( ).getUri ( );
	if ( ! u.getHost ( ).empty ( ) )
		remoteTarget = u;
	handleRecvInfo ( m );
	handleAccept ( m.getAccept ( ), r.getSdp ( ) );
	allowsInfo = m.getAllow ( Request :: mInfo );
}

bool Dialog :: Impl :: requestReceived ( const Request & r ) {
	const RequestMIMEInfo & m = r.getMime ( );
	const CSeq & cs = m.getCseq ( );
	if ( cs.method != Request :: mAck && cs.method != Request :: mCancel ) {
		if ( cs.seq <= remoteSeq )
			return false;
		remoteSeq = cs.seq;
	}
	if ( cs.method == Request :: mInvite || cs.method == Request :: mUpdate  || cs.method == Request :: mSubscribe ||
		cs.method == Request :: mNotify || cs.method == Request :: mRefer ) {
		const URI & u = m.getContact ( ).getUri ( );
		if ( ! u.getHost ( ).empty ( ) )
			remoteTarget = u;
	}
	if ( cs.method == Request :: mAck || cs.method == Request :: mInvite || cs.method == Request :: mPrack ||
		cs.method == Request :: mUpdate )
		handleRecvInfo ( m );
	if ( cs.method == Request :: mInvite || cs.method == Request :: mUpdate )
		allowsInfo = m.getAllow ( Request :: mInfo );
	if ( cs.method == Request :: mAck )
		confirmed = true;
	return true;
}

Dialog :: Dialog ( const Request & r ) : impl ( new Impl ( r ) ) { } // prishel invite

Dialog :: Dialog ( const Response & r, bool acceptsDtmfOverride ) : impl ( new Impl ( r, acceptsDtmfOverride ) ) { }
// otvet na invite

Dialog :: Dialog ( ) : impl ( new Impl ) { }

Dialog :: ~Dialog ( ) {
	delete impl;
}

Dialog :: Dialog ( const Dialog & d ) : Allocatable < __SS_ALLOCATOR > ( d ), impl ( new Impl ( * d.impl ) ) { }

const DialogId & Dialog :: getId ( ) const {
	return impl -> getId ( );
}

const URI & Dialog :: getRemoteTarget ( ) const {
	return impl -> getRemoteTarget ( );
}

const URI & Dialog :: getRemoteUri ( ) const {
	return impl -> getRemoteUri ( );
}

const URI & Dialog :: getLocalUri ( ) const {
	return impl -> getLocalUri ( );
}

int Dialog :: getLocalSeq ( ) const {
	return impl -> getLocalSeq ( );
}

void Dialog :: incLocalSeq ( ) {
	impl -> incLocalSeq ( );
}

const RouteHeaderVector & Dialog :: getRouteSet ( ) const {
	return impl -> getRouteSet ( );
}

void Dialog :: targetRefresh ( const Response & r ) {
	impl -> targetRefresh ( r );
}

bool Dialog :: requestReceived ( const Request & r ) {
	return impl -> requestReceived ( r );
}

bool Dialog :: isConfirmed ( ) const {
	return impl -> isConfirmed ( );
}

void Dialog :: handleRecvInfo ( const ResponseMIMEInfo & m ) {
	return impl -> handleRecvInfo ( m );
}

bool Dialog :: getSupportsPackage ( ) const {
	return impl -> getSupportsPackage ( );
}

bool Dialog :: getSupportsDtmf ( ) const {
	return impl -> getSupportsDtmf ( );
}

bool Dialog :: getAcceptsDtmf ( ) const {
	return impl -> getAcceptsDtmf ( );
}

void Dialog :: handleAllow ( const ResponseMIMEInfo & m ) {
	impl -> handleAllow ( m );
}

bool Dialog :: getAllowsInfo ( ) const {
	return impl -> getAllowsInfo ( );
}

class InviteClientTransaction :: Impl : public Allocatable < __SS_ALLOCATOR > {
	const CoreClientTransactionHandler & c;
	OriginateHandler * h;
	ClientTransactionId id;
	PTime lastSent, timerB;
	int timerAsecs;
	enum State {
		sCalling,
		sProceeding,
		sCompleted,
		sTerminated
	} state;
	Request request;
	StringVector savedPasswords;
	bool sendRequest ( );
	void sendAck ( const Response & r );
public:
	Impl ( const Request & r, const CoreClientTransactionHandler & c, OriginateHandler * h ) : c ( c ), h ( h ), id ( r ),
		timerB ( PTime ( ) + 32000 ), timerAsecs ( 1 ), state ( sCalling ), request ( r ) {
		sendRequest ( );
	}
	~Impl ( ) { }
	const ClientTransactionId & getId ( ) const {
		return id;
	}
	bool checkTimeout ( InviteClientTransaction * t );
	bool responseReceived ( const Response & r, InviteClientTransaction * t );
	void detachHandler ( ) {
		h = 0;
	}
	OriginateHandler * getHandler ( ) const {
		return h;
	}
	const StringVector & getSavedPasswords ( ) const {
		return savedPasswords;
	}
	void saveAuth ( const StringVector & pw ) {
		savedPasswords = pw;
	}
	const Request & getRequest ( ) const {
		return request;
	}
};

bool InviteClientTransaction :: Impl :: sendRequest ( ) {
	return transportThread -> sendMessage ( request );
}

void InviteClientTransaction :: Impl :: sendAck ( const Response & r ) {
	if ( r.getStatusCode ( ) < 300 )
		throw std :: runtime_error ( "ack for 2xx in transaction layer" );
	Request ack ( request, r );
	transportThread -> sendMessage ( ack );
}

bool InviteClientTransaction :: Impl :: checkTimeout ( InviteClientTransaction * t ) {
	PTime now;
	switch ( state ) {
		case sCalling:
			if ( timerB <= now ) {
				state = sTerminated;
				c.transactionTimedOut ( t );
				return false;
			}
			if ( lastSent + timerAsecs * 1000 <= now ) {
				lastSent = now;
				timerAsecs *= 2;
				if ( sendRequest ( ) )
					return true;
				state = sTerminated;
				c.transactionTransportError ( t );
				return false;
			}
			return true;
		case sProceeding:
			if ( ! h && timerB <= now ) { // etogo net v standarte no inache ono zavisnet
				state = sTerminated;
				c.transactionTimedOut ( t );
				return false;
			}
			if ( request.getMime ( ).getVia ( ).front ( ).getRport ( ) && lastSent + 20000 <= now ) {
				lastSent = now; // voobshe eto nado tolko esli mi za natom
				if ( ! sendRequest ( ) ) {
					state = sTerminated;
					c.transactionTransportError ( t );
					return false;
				}
			}
			return true;
		case sCompleted:
			if ( timerB <= now ) { // tam timerD, no mi ispolzuem odnu peremennuyu
				state = sTerminated;
				c.transactionFinished ( t );
				return false;
			}
			return true;
		case sTerminated:
			break;
	}
	throw std :: runtime_error ( "checkTimeout in terminated state" );
}

bool InviteClientTransaction :: Impl :: responseReceived ( const Response & r, InviteClientTransaction * t ) {
	switch ( state ) {
		case sCalling:
			if ( r.getStatusCode ( ) < 200 ) {
				state = sProceeding;
				c.responseReceived ( r, t );
				return true;
			}
			if ( r.getStatusCode ( ) >= 300 ) {
				state = sCompleted;
				timerB = PTime ( ) + 32000; // ispolzuem vmesto timerD
				sendAck ( r );
				c.responseReceived ( r, t );
				return true;
			}
			state = sTerminated;
			c.responseReceived ( r, t );
			return false;
		case sProceeding:
			if ( r.getStatusCode ( ) < 200 ) {
				c.responseReceived ( r, t );
				return true;
			}
			if ( r.getStatusCode ( ) >= 300 ) {
				state = sCompleted;
				timerB = PTime ( ) + 32000; // ispolzuem vmesto timerD
				sendAck ( r );
				c.responseReceived ( r, t );
				return true;
			}
			state = sTerminated;
			c.responseReceived ( r, t );
			return false;
		case sCompleted:
			if ( r.getStatusCode ( ) >= 300 ) {
				sendAck ( r );
				return true;
			}
			{
				std :: ostringstream os;
				os << "unsupported status code in completed state: " << r.getStatusCode ( );
				throw std :: runtime_error ( os.str ( ) );
			}
		case sTerminated:
			break;
	}
	throw std :: runtime_error ( "responseReceived in terminated state" );
}

OriginateHandler * InviteClientTransaction :: getHandler ( ) const {
	return impl -> getHandler ( );
}

const ClientTransactionId & InviteClientTransaction :: getId ( ) const {
	return impl -> getId ( );
}

bool InviteClientTransaction :: checkTimeout ( ) {
	return impl -> checkTimeout ( this );
}

bool InviteClientTransaction :: responseReceived ( const Response & r ) {
	return impl -> responseReceived ( r, this );
}

void InviteClientTransaction :: detachHandler ( ) {
	impl -> detachHandler ( );
}

const StringVector & InviteClientTransaction :: getSavedPasswords ( ) const {
	return impl -> getSavedPasswords ( );
}

void InviteClientTransaction :: saveAuth ( const StringVector & pw ) {
	impl -> saveAuth ( pw );
}

const Request & InviteClientTransaction :: getRequest ( ) const {
	return impl -> getRequest ( );
}

InviteClientTransaction :: InviteClientTransaction ( const Request & r, const CoreClientTransactionHandler & c,
	OriginateHandler * h ) : impl ( new Impl ( r, c, h ) ) { }

InviteClientTransaction :: ~InviteClientTransaction ( ) {
	delete impl;
}

InviteClientTransaction :: InviteClientTransaction ( const InviteClientTransaction & d ) :
	ClientTransaction ( d ), impl ( new Impl ( * d.impl ) ) { }

class NonInviteClientTransaction :: Impl : public Allocatable < __SS_ALLOCATOR > {
	CoreClientTransactionHandler * c;
	OriginateHandler * h;
	ClientTransactionId id;
	PTime lastSent, timerF;
	int timerEsecs;
	enum State {
		sTrying,
		sProceeding,
		sCompleted,
		sTerminated
	} state;
	Request request;
	StringVector passwords;
	bool sendRequest ( );
public:
	Impl ( const Request & r, CoreClientTransactionHandler * c, OriginateHandler * h ) : c ( c ), h ( h ), id ( r ),
		timerF ( PTime ( ) + 32000 ), timerEsecs ( 1 ), state ( sTrying ), request ( r ) {
		sendRequest ( );
	}
	const Request & getRequest ( ) const {
		return request;
	}
	const ClientTransactionId & getId ( ) const {
		return id;
	}
	bool checkTimeout ( NonInviteClientTransaction * t );
	bool responseReceived ( const Response & r, NonInviteClientTransaction * t );
	OriginateHandler * getHandler ( ) const {
		return h;
	}
	void detachHandler ( ) {
		h = 0;
	}
	const StringVector & getSavedPasswords ( ) const {
		return passwords;
	}
	void saveAuth ( const StringVector & pw ) {
		passwords = pw;
	}
};

bool NonInviteClientTransaction :: Impl :: sendRequest ( ) {
	return transportThread -> sendMessage ( request );
}

bool NonInviteClientTransaction :: Impl :: checkTimeout ( NonInviteClientTransaction * t ) {
	PTime now;
	switch ( state ) {
		case sTrying:
			if ( timerF <= now ) {
				state = sTerminated;
				if ( c )
					c -> transactionTimedOut ( t );
				return false;
			}
			if ( lastSent + timerEsecs * 1000 <= now ) {
				lastSent = now;
				timerEsecs = std :: min ( timerEsecs * 2, 4 );
				if ( sendRequest ( ) )
					return true;
				state = sTerminated;
				if ( c )
					c -> transactionTransportError ( t );
				return false;
			}
			return true;
		case sProceeding:
			if ( timerF <= now ) {
				state = sTerminated;
				if ( c )
					c -> transactionTimedOut ( t );
				return false;
			}
			if ( lastSent + timerEsecs * 1000 <= now ) {
				lastSent = now;
				timerEsecs = 4;
				if ( sendRequest ( ) )
					return true;
				state = sTerminated;
				if ( c )
					c -> transactionTransportError ( t );
				return false;
			}
			return true;
		case sCompleted:
			if ( timerF <= now ) { // tam timerK, no mi ispolzuem odnu peremennuyu
				state = sTerminated;
				if ( c )
					c -> transactionFinished ( t );
				return false;
			}
			return true;
		case sTerminated:
			break;
	}
	throw std :: runtime_error ( "checkTimeout in terminated state" );
}

bool NonInviteClientTransaction :: Impl :: responseReceived ( const Response & r, NonInviteClientTransaction * t ) {
	switch ( state ) {
		case sTrying:
			if ( r.getStatusCode ( ) < 200 ) {
				state = sProceeding;
				if ( c )
					c -> responseReceived ( r, t );
				return true;
			}
			state = sCompleted;
			timerF = PTime ( ) + 5000; // ispolzuem vmesto timerK
			if ( c )
				c -> responseReceived ( r, t );
			return true;
		case sProceeding:
			if ( r.getStatusCode ( ) < 200 ) {
				if ( c )
					c -> responseReceived ( r, t );
				return true;
			}
			state = sCompleted;
			timerF = PTime ( ) + 32000; // ispolzuem vmesto timerD
			if ( c )
				c -> responseReceived ( r, t );
			return true;
		case sCompleted:
			return true;
		case sTerminated:
			break;
	}
	throw std :: runtime_error ( "responseReceived in terminated state" );
}

const ClientTransactionId & NonInviteClientTransaction :: getId ( ) const {
	return impl -> getId ( );
}

bool NonInviteClientTransaction :: checkTimeout ( ) {
	return impl -> checkTimeout ( this );
}

bool NonInviteClientTransaction :: responseReceived ( const Response & r ) {
	return impl -> responseReceived ( r, this );
}

NonInviteClientTransaction :: NonInviteClientTransaction ( const Request & r, CoreClientTransactionHandler * c, OriginateHandler * h ) :
	impl ( new Impl ( r, c, h ) ) { }

NonInviteClientTransaction :: ~NonInviteClientTransaction ( ) {
	delete impl;
}

const Request & NonInviteClientTransaction :: getRequest ( ) const {
	return impl -> getRequest ( );
}

void NonInviteClientTransaction :: detachHandler ( ) {
	impl -> detachHandler ( );
}

OriginateHandler * NonInviteClientTransaction :: getHandler ( ) const {
	return impl -> getHandler ( );
}

const StringVector & NonInviteClientTransaction :: getSavedPasswords ( ) const {
	return impl -> getSavedPasswords ( );
}

void NonInviteClientTransaction :: saveAuth ( const StringVector & passwords ) {
	impl -> saveAuth ( passwords );
}

NonInviteClientTransaction :: NonInviteClientTransaction ( const NonInviteClientTransaction & d ) :
	ClientTransaction ( d ), impl ( new Impl ( * d.impl ) ) { }

class InviteServerTransaction :: Impl : public Allocatable < __SS_ALLOCATOR > {
	Request r;
	Response response;
	CoreServerTransactionHandler * c;
	ServerTransactionId id;
	MergedTransactionId mergedId;
	PTime lastSent, timerH;
	int timerGsecs;
	enum State {
		sProceeding,
		sCompleted,
		sConfirmed,
		sTerminated
	} state;
	int dIndex;
	ss :: string savedPassword;
	bool sendResponse ( ) const;
public:
	Impl ( const Request & r, CoreServerTransactionHandler * c );
	const ServerTransactionId & getId ( ) const {
		return id;
	}
	const MergedTransactionId & getMergedId ( ) const {
		return mergedId;
	}
	bool checkTimeout ( ServerTransaction * t );
	bool responseSent ( const Response & r, ServerTransaction * t );
	bool requestReceived ( const Request & r, ServerTransaction * t );
	const Request & getRequest ( ) const {
		return r;
	}
	const Response & getResponse ( ) const {
		return response;
	}
	const DigestResponse * getSavedDigestResponse ( ) const {
		if ( dIndex < 0 )
			return 0;
		return & r.getMime ( ).getAuthorization ( ) [ dIndex ];
	}
	const ss :: string & getSavedPassword ( ) const {
		return savedPassword;
	}
	void saveAuth ( unsigned di, const ss :: string & pw ) {
		dIndex = di;
		savedPassword = pw;
	}
};

InviteServerTransaction :: Impl :: Impl ( const Request & r, CoreServerTransactionHandler * c ) : r ( r ),
	response ( r, Response :: scInformationTrying ), c ( c ), id ( r ), mergedId ( r ), timerH ( PTime ( ) + 32000 ),
	timerGsecs ( 1 ), state ( sProceeding ), dIndex ( -1 ) {
	sendResponse ( );
}

bool InviteServerTransaction :: Impl :: sendResponse ( ) const {
	return transportThread -> sendMessage ( response );
}

bool InviteServerTransaction :: Impl :: checkTimeout ( ServerTransaction * t ) {
	PTime now;
	switch ( state ) {
		case sProceeding:
			if ( ! c && timerH <= now ) { // etogo net v standarte no inache moget zavisnut
				state = sTerminated;
				return false;
			}
			if ( response.getStatusCode ( ) != Response :: scInformationTrying && lastSent + 60000 <= now ) {
				lastSent = now;
				if ( ! sendResponse ( ) ) {
					state = sTerminated;
					if ( c )
						c -> transactionTransportError ( t );
					return false;
				}
			}
			return true;
		case sCompleted:
			if ( timerH <= now ) {
				state = sTerminated;
				return false;
			}
			if ( lastSent + timerGsecs * 1000 <= now ) {
				lastSent = now;
				timerGsecs = std :: min ( timerGsecs * 2, 4 );
				if ( ! sendResponse ( ) ) {
					state = sTerminated;
					return false;
				}
			}
			return true;
		case sConfirmed:
			if ( timerH <= now ) { // tam timerI, no mi ispolzuem odnu peremennuyu
				state = sTerminated;
				return false;
			}
			return true;
		case sTerminated:
			break;
	}
	throw std :: runtime_error ( "checkTimeout in terminated state" );
}

bool InviteServerTransaction :: Impl :: responseSent ( const Response & r, ServerTransaction * t ) {
	switch ( state ) {
		case sProceeding:
			response = r;
			if ( r.getStatusCode ( ) >= 300 ) {
				timerH = PTime ( ) + 32000;
				state = sCompleted;
			} else if ( r.getStatusCode ( ) >= 200 )
				state = sTerminated;
			lastSent = PTime ( );
			if ( sendResponse ( ) )
				return state != sTerminated;
			state = sTerminated;
			if ( c && r.getStatusCode ( ) >= 200 && r.getStatusCode ( ) < 300 )
				c -> transactionTransportError ( t );
			return false;
		case sCompleted:
			throw std :: runtime_error ( "new response in completed state" );
		case sConfirmed:
			throw std :: runtime_error ( "new response in confirmed state" );
		case sTerminated:
			break;
	}
	throw std :: runtime_error ( "responseSent in terminated state" );
}

bool InviteServerTransaction :: Impl :: requestReceived ( const Request & r, ServerTransaction * t ) {
	switch ( state ) {
		case sProceeding:
		case sCompleted:
			if ( r.getMethod ( ) == Request :: mAck ) {
				state = sConfirmed;
				timerH = PTime ( ) + 5000; // ispolzuem vmesto timerI
				return true;
			}
			if ( sendResponse ( ) )
				return true;
			state = sTerminated;
			if ( c )
				c -> transactionTransportError ( t );
			return false;
		case sConfirmed:
			return true;
		case sTerminated:
			break;
	}
	throw std :: runtime_error ( "responseReceived in terminated state" );
}

const ServerTransactionId & InviteServerTransaction :: getId ( ) const {
	return impl -> getId ( );
}

const MergedTransactionId & InviteServerTransaction :: getMergedId ( ) const {
	return impl -> getMergedId ( );
}

bool InviteServerTransaction :: checkTimeout ( ) {
	return impl -> checkTimeout ( this );
}

bool InviteServerTransaction :: requestReceived ( const Request & r ) {
	return impl -> requestReceived ( r, this );
}

bool InviteServerTransaction :: responseSent ( const Response & r ) {
	return impl -> responseSent ( r, this );
}

const Request & InviteServerTransaction :: getRequest ( ) const {
	return impl -> getRequest ( );
}

const Response & InviteServerTransaction :: getResponse ( ) const {
	return impl -> getResponse ( );
}

const DigestResponse * InviteServerTransaction :: getSavedDigestResponse ( ) const {
	return impl -> getSavedDigestResponse ( );
}

const ss :: string & InviteServerTransaction :: getSavedPassword ( ) const {
	return impl -> getSavedPassword ( );
}

void InviteServerTransaction :: saveAuth ( unsigned di, const ss :: string & pw ) {
	impl -> saveAuth ( di, pw );
}

InviteServerTransaction :: InviteServerTransaction ( const Request & r, CoreServerTransactionHandler * c ) :
	impl ( new Impl ( r, c ) ) { }

InviteServerTransaction :: ~InviteServerTransaction ( ) {
	delete impl;
}

InviteServerTransaction :: InviteServerTransaction ( const InviteServerTransaction & d ) :
	ServerTransaction ( d ), impl ( new Impl ( * d.impl ) ) { }

class NonInviteServerTransaction :: Impl : public Allocatable < __SS_ALLOCATOR > {
	CoreServerTransactionHandler * c;
	ServerTransactionId id;
	MergedTransactionId mergedId;
	PTime timerJ;
	enum State {
		sTrying,
		sProceeding,
		sCompleted,
		sTerminated
	} state;
	Pointer < Response > response;
	bool sendResponse ( ) const;
public:
	Impl ( const Request & r, CoreServerTransactionHandler * c ) : c ( c ), id ( r ), mergedId ( r ),
		timerJ ( PTime ( ) + 32000 ), state ( sTrying ) { }
	const ServerTransactionId & getId ( ) const {
		return id;
	}
	const MergedTransactionId & getMergedId ( ) const {
		return mergedId;
	}
	bool checkTimeout ( );
	bool responseSent ( const Response & r, ServerTransaction * t );
	bool requestReceived ( const Request & r, ServerTransaction * t );
};

bool NonInviteServerTransaction :: Impl :: sendResponse ( ) const {
	return transportThread -> sendMessage ( * response );
}

bool NonInviteServerTransaction :: Impl :: checkTimeout ( ) {
	PTime now;
	switch ( state ) {
		case sTrying:
		case sProceeding:
			if ( timerJ <= now ) { // etogo net v standarte no inache moget zavisnut
				state = sTerminated;
				return false;
			}
			return true;
		case sCompleted:
			if ( timerJ <= now ) { // tam timerK, no mi ispolzuem odnu peremennuyu
				state = sTerminated;
				return false;
			}
			return true;
		case sTerminated:
			break;
	}
	throw std :: runtime_error ( "checkTimeout in terminated state" );
}

bool NonInviteServerTransaction :: Impl :: responseSent ( const Response & r, ServerTransaction * t ) {
	switch ( state ) {
		case sTrying:
			response = new Response ( r );
			if ( r.getStatusCode ( ) < 200 )
				state = sProceeding;
			else {
				timerJ = PTime ( ) + 32000;
				state = sCompleted;
			}
			if ( sendResponse ( ) )
				return true;
			state = sTerminated;
			if ( c )
				c -> transactionTransportError ( t );
			return false;
		case sProceeding:
			response = new Response ( r );
			if ( r.getStatusCode ( ) >= 200 ) {
				timerJ = PTime ( ) + 32000;
				state = sCompleted;
			}
			if ( sendResponse ( ) )
				return true;
			state = sTerminated;
			if ( c )
				c -> transactionTransportError ( t );
			return false;
		case sCompleted:
			throw std :: runtime_error ( "new response in completed state" );
		case sTerminated:
			break;
	}
	throw std :: runtime_error ( "responseSent in terminated state" );
}


bool NonInviteServerTransaction :: Impl :: requestReceived ( const Request &, ServerTransaction * t ) {
	switch ( state ) {
		case sTrying:
			return true;
		case sProceeding:
		case sCompleted:
			if ( sendResponse ( ) )
				return true;
			state = sTerminated;
			if ( c )
				c -> transactionTransportError ( t );
			return false;
		case sTerminated:
			break;
	}
	throw std :: runtime_error ( "responseReceived in terminated state" );
}

const ServerTransactionId & NonInviteServerTransaction :: getId ( ) const {
	return impl -> getId ( );
}

const MergedTransactionId & NonInviteServerTransaction :: getMergedId ( ) const {
	return impl -> getMergedId ( );
}

bool NonInviteServerTransaction :: checkTimeout ( ) {
	return impl -> checkTimeout ( );
}

bool NonInviteServerTransaction :: requestReceived ( const Request & r ) {
	return impl -> requestReceived ( r, this );
}

bool NonInviteServerTransaction :: responseSent ( const Response & r ) {
	return impl -> responseSent ( r, this );
}


NonInviteServerTransaction :: NonInviteServerTransaction ( const Request & r, CoreServerTransactionHandler * c ) :
	impl ( new Impl ( r, c ) ) { }

NonInviteServerTransaction :: ~NonInviteServerTransaction ( ) {
	delete impl;
}

NonInviteServerTransaction :: NonInviteServerTransaction ( const NonInviteServerTransaction & d ) :
	ServerTransaction ( d ), impl ( new Impl ( * d.impl ) ) { }

}
