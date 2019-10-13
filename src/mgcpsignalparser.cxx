#pragma implementation
#pragma implementation "rightparser.hpp"
#include "ss.hpp"
#include "allocatable.hpp"
#include "pointer.hpp"
#include "mgcpsignalparser.hpp"
#include <boost/spirit/include/classic_parse_tree.hpp>
#include <boost/spirit/include/classic_utility.hpp>
#include "rightparser.hpp"
#include <boost/algorithm/string/case_conv.hpp>
#include <ptlib.h>
#include <ptlib/svcproc.h>


static inline void maybequote ( std :: ostream & os, const ss :: string & s ) {
	// nado proverit nedopustimie simvoli i vidat utf-8 quoted string esli chto
	os << s;
}

void MGCP :: EventParameter :: printOn ( std :: ostream & os ) const {
	List :: const_iterator i = parameters.begin ( );
	if ( i == parameters.end ( ) ) {
		maybequote ( os, nameOrValue );
		return;
	}
	os << nameOrValue;
	if ( ++ i == parameters.end ( ) ) {
		os << '=';
		( * parameters.begin ( ) ) -> printOn ( os );
		return;
	}
	os << '(' << parameters << ')';
}

namespace bs = boost :: spirit :: classic;

struct signal_requests_grammar : public bs :: grammar < signal_requests_grammar > {
	static const unsigned event_name_package_id = 1;
	static const unsigned event_parameters_id = 2;
	template < typename ScannerT > struct definition {
		definition ( const signal_requests_grammar & self ) {
			signal_requests = signal_request >> * ( bs :: no_node_d [ ',' >>
				* bs :: blank_p ] >> signal_request );
			signal_request = event_name >> ! ( bs :: no_node_d [
				bs :: ch_p ( '(' ) ] >> event_parameters >>
				bs :: no_node_d [ bs :: ch_p ( ')' ) ] );
			event_name_package = ( package_name | '*' );
			package_name = bs :: alnum_p >> ! ( * ( ( bs :: alnum_p | '-' )
				>> bs :: eps_p ( bs :: alnum_p | '-' ) ) >>
				bs :: alnum_p );
			event_name = ! ( bs :: leaf_node_d [ event_name_package ] >>
				bs :: no_node_d [ bs :: ch_p ( '/' ) ] ) >>
				bs :: leaf_node_d [ + ( bs :: alnum_p | '-' | '*' | '#' )
				| event_range ] >> ! ( bs :: no_node_d [
				bs :: ch_p ( '@' ) ] >> bs :: leaf_node_d [
				bs :: repeat_p ( 1, 32 ) [ bs :: xdigit_p ] | '$' | '*' ] );
			event_range = '[' >> + ( ( bs :: digit_p >> '-' >> bs :: digit_p )
				| ( dtmf_letter >> '-' >> dtmf_letter ) | digit_map_letter ) >> ']';
			dtmf_letter = bs :: chset_p ( "ABCDabcd" );
			digit_map_letter = bs :: chset_p ( "0-1a-zA-Z#*" );
			event_parameters = event_parameter >> * ( bs :: no_node_d [
				',' >> * bs :: blank_p ] >> event_parameter );
			event_parameter = ( bs :: leaf_node_d [ event_parameter_name ] >>
				( ( bs :: no_node_d [ bs :: ch_p ( '=' ) ] >>
				event_parameter ) | ( bs :: no_node_d [ bs :: ch_p ( '(' ) ]
				>> event_parameters >> bs :: no_node_d [ bs :: ch_p ( ')' ) ] ) ) ) |
				bs :: leaf_node_d [ event_parameter_value ];
			event_parameter_name = + bs :: chset_p ( "\x2d-\x3c\x21\x23-\x27\x2a\x2b\x3e-\x7e" ); //2d=='-'
			event_parameter_value = event_parameter_name | quoted_string;
			quoted_string = '"' >> * ( "\"\"" | bs :: chset_p ( "\x01-\x21\x23-\x7f\x80-\xff" ) | '\0' ) >> '"';
			if ( self.tag_id )
				signal_requests.set_id ( self.tag_id );
		}
		bs :: rule < ScannerT, bs :: parser_context < >,
			bs :: dynamic_parser_tag > signal_requests;
		bs :: rule < ScannerT > signal_request, package_name, event_name, event_range,
			dtmf_letter, digit_map_letter, event_parameter, event_parameter_value, event_parameter_name,
			quoted_string;
		#define RULE_ID( name ) bs :: rule < ScannerT, bs :: parser_context < >,\
			bs :: parser_tag < name##_id > > name;
		RULE_ID ( event_name_package );
		RULE_ID ( event_parameters );
		#undef RULE_ID
		bs :: rule < ScannerT, bs :: parser_context < >,
			bs :: dynamic_parser_tag > const & start ( ) const {
			return signal_requests;
		}
	};
	signal_requests_grammar ( int t = 0 ) : tag_id ( t ) { }
private:
	int tag_id;
};

static MGCP :: EventName parseEventName ( const RightParser :: iter_t & i ) {
	RightParser :: iter_t j = i -> children.begin ( );
	ss :: string ep, en, ec;
	if ( j -> value.id ( ).to_long ( ) == signal_requests_grammar :: event_name_package_id ) {
		ep.assign ( j -> value.begin ( ), j -> value.end ( ) );
		boost :: algorithm :: to_upper ( ep );
		++ j;
	}
	en.assign ( j -> value.begin ( ), j -> value.end ( ) );
	boost :: algorithm :: to_lower ( en );
	if ( ++ j != i -> children.end ( ) )
		ec.assign ( j -> value.begin ( ), j -> value.end ( ) );
	return MGCP :: EventName ( ep, en, ec );
}

static Pointer < MGCP :: EventParameter > parseEventParameter ( const RightParser :: iter_t & i ) {
	RightParser :: iter_t j = i -> children.begin ( );
	Pointer < MGCP :: EventParameter > p ( new MGCP :: EventParameter ( ss :: string ( j -> value.begin ( ), j -> value.end ( ) ) ) );
	if ( ++ j == i -> children.end ( ) )
		return p;
	if ( j -> value.id ( ).to_long ( ) != signal_requests_grammar :: event_parameters_id ) {
		p -> add ( parseEventParameter ( j ) );
		return p;
	}
	for ( RightParser :: iter_t k = j -> children.begin ( ); k != j -> children.end ( ); ++ k )
		p -> add ( parseEventParameter ( k ) );
	return p;
}

static MGCP :: SignalRequest parseSignalRequest ( const RightParser :: iter_t & i ) {
	RightParser :: iter_t j = i -> children.begin ( );
	MGCP :: SignalRequest rq ( parseEventName ( j ) );
	if ( ++ j == i -> children.end ( ) )
		return rq;
	for ( RightParser :: iter_t k = j -> children.begin ( ); k != j -> children.end ( ); ++ k )
		rq.add ( parseEventParameter ( k ) );
	return rq;
}

MGCP :: SignalRequestVector MGCP :: parseSignalRequests ( const ss :: string & s ) {
	SignalRequestVector v;
	if ( s.empty ( ) )
		return v;
	signal_requests_grammar g;
	RightParser :: result_t info = RightParser :: parse ( s.begin ( ), s.end ( ), g );
	if ( ! info.full ) {
		PSYSTEMLOG ( Error, "error parsing signal requests " << s );
		return v;
	}
	RightParser :: iter_t i = info.trees.begin ( );
	for ( RightParser :: iter_t j = i -> children.begin ( ); j != i -> children.end ( ); ++ j )
		v.push_back ( parseSignalRequest ( j ) );
	return v;
}
