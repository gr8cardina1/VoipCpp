#pragma implementation
#pragma implementation "hostnamegrammars.hpp"
#include "ss.hpp"

#include <boost/spirit/include/classic_parse_tree.hpp>
#include <boost/spirit/include/classic_utility.hpp>
#include "rightparser.hpp"
#include "hostnamegrammars.hpp"

#include "sdpgrammars.hpp"

namespace bs = boost :: spirit :: classic;

namespace SDP {

struct uri_grammar : public bs :: grammar < uri_grammar > {
	static const int path_id = 1;
	template < typename ScannerT > struct definition {
		definition ( const uri_grammar & self ) {
			fragmentaddress = uri >> ! ( bs :: no_node_d [ bs :: ch_p ( '#' ) ] >>
				bs :: leaf_node_d [ fragmentid ] );
			fragmentid = * xalpha;
			xalpha = bs :: alnum_p | safe | extra | escape;
			safe = bs :: chset_p ( "-$_@.&" );
			extra = bs :: chset_p ( "!*\"'()," );
			escape = '%' >> bs :: xdigit_p >> bs :: xdigit_p;
			uri = bs :: leaf_node_d [ scheme ] >> bs :: no_node_d [ bs :: ch_p ( ':' ) ] >>
				bs :: leaf_node_d [ path ] >> ! ( bs :: no_node_d [ bs :: ch_p ( '?' ) ] >>
				bs :: leaf_node_d [ search ] );
			scheme = bs :: alpha_p >> * xalpha;
			path = * ( xalpha | bs :: chset_p ( "+/;:" ) );
			search = + ( xalpha | '+' | '=' );
			if ( self.tag_id )
				fragmentaddress.set_id ( self.tag_id );
		}
		bs :: rule < ScannerT, bs :: dynamic_parser_tag > fragmentaddress;
		bs :: rule < ScannerT > fragmentid, xalpha, safe, extra, escape, uri, scheme, search;
#define RULE_ID( name ) bs :: rule < ScannerT, bs :: parser_tag < name##_id > > name;
		RULE_ID ( path );
#undef RULE_ID
		bs :: rule < ScannerT, bs :: dynamic_parser_tag > const & start ( ) const {
			return fragmentaddress;
		}
	};
	uri_grammar ( int t = 0 ) : tag_id ( t ) { }
	private:
	int tag_id;
};

struct email_address_grammar : public bs :: grammar < email_address_grammar > {
	template < typename ScannerT > struct definition {
		definition ( const email_address_grammar & self ) {
			addr_spec = bs :: leaf_node_d [ local_part ] >> bs :: no_node_d [ bs :: ch_p ( '@' ) ] >> domain;
			local_part = word >> * ( '.' >> word );
			word = atom /*| quoted_string*/;
			atom = + ( bs :: anychar_p - ( bs :: cntrl_p | specials ) );
			specials = bs :: chset_p ( "()<>@,;:\\\".[]" );
			//Must be in quoted-string, to use within a word.
			if ( self.tag_id )
				addr_spec.set_id ( self.tag_id );
		}
		hostname_grammar domain;
		bs :: rule < ScannerT, bs :: dynamic_parser_tag > addr_spec;
		bs :: rule < ScannerT > local_part, word, atom, specials;
		bs :: rule < ScannerT, bs :: dynamic_parser_tag > const & start ( ) const {
			return addr_spec;
		}
	};
	email_address_grammar ( int t = 0 ) : tag_id ( t ) { }
	private:
	int tag_id;
};

typedef bs :: positive < bs :: chset < char > > token_t;
const token_t token_p ( + bs :: chset_p ( "\x2d\x2e\x21\x23-\x27\x2a\x2b\x30-\x39\x41-\x5a\x5e-\x7e" ) );

typedef bs :: chseq < const char * > crlf_t;
const crlf_t crlf_p ( "\r\n" );

typedef bs :: chset < char > base64_char_t;
const base64_char_t base64_char_p ( "+/0-9a-zA-Z" );

typedef bs :: fixed_loop < base64_char_t, int > base64_unit_t;
const base64_unit_t base64_unit_p ( base64_char_p, 4 );

typedef bs :: sequence < bs :: sequence < bs :: fixed_loop < base64_char_t, int >,
	bs :: chset < char > >, bs :: chlit < char > > base64_pad_t;
const base64_pad_t base64_pad_p ( bs :: repeat_p ( 2 ) [ base64_char_p ] >> ( base64_char_p | '=' ) >> '=' );

typedef bs :: sequence < bs :: kleene_star < base64_unit_t >, bs :: optional < base64_pad_t > > base64_t;
const base64_t base64_p ( * base64_unit_p >> ! base64_pad_p );

const token_t non_ws_string_p ( + bs :: chset_p ( "\x21-\x7e\x80-\xff" ) );

const base64_char_t email_safe_p ( "\x01-\x09\x0e-\x27\x2a-\x3b\x3d\x3f-\x7f\x80-\xff" );

const base64_char_t pos_digit_p ( "1-9" );

typedef bs :: sequence < base64_char_t, bs :: infinite_loop < bs :: digit_parser, int > > time_t;
const time_t time_p ( pos_digit_p >> bs :: repeat_p ( 9, bs :: more ) [ bs :: digit_p ] );

struct sdp_grammar : public bs :: grammar < sdp_grammar > {
	static const unsigned hostname_id = 1;
	static const unsigned ipv4address_id = sdp_grammar_ipv4address_id;
	static const unsigned ipv6address_id = 3;
	static const unsigned information_field_id = sdp_grammar_information_field_id;
	static const unsigned uri_field_id = sdp_grammar_uri_field_id;
	static const unsigned email_fields_id = sdp_grammar_email_fields_id;
	static const unsigned email_id = 7;
	static const unsigned phone_fields_id = sdp_grammar_phone_fields_id;
	static const unsigned phone_id = 9;
	static const unsigned connection_field_id = sdp_grammar_connection_field_id;
	static const unsigned bandwidth_fields_id = sdp_grammar_bandwidth_fields_id;
	static const unsigned time_fields_id = 12;
	static const unsigned repeat_fields_id = sdp_grammar_repeat_fields_id;
	static const unsigned typed_time_id = 14;
	static const unsigned zone_adjustments_id = sdp_grammar_zone_adjustments_id;
	static const unsigned key_field_id = sdp_grammar_key_field_id;
	static const unsigned attribute_fields_id = sdp_grammar_attribute_fields_id;
	static const unsigned media_descriptions_id = 18;
	static const unsigned integer_id = sdp_grammar_integer_id;
	template < typename ScannerT > struct definition {
		definition ( const sdp_grammar & self ) : hostname ( hostname_id ), ipv4address ( ipv4address_id ),
			ipv6address ( ipv6address_id ), email ( email_id ) {
			typedef bs :: sequence < bs :: sequence < bs :: no_tree_gen_node_parser < bs :: strlit < const char * > >,
				bs :: leaf_node_parser < bs :: positive < bs :: digit_parser > > >,
				bs :: no_tree_gen_node_parser < bs :: chseq < const char * > > > proto_version_t;
			proto_version_t proto_version = bs :: no_node_d [ bs :: str_p ( "v=" ) ] >>
				bs :: leaf_node_d [ + bs :: digit_p ] >> bs :: no_node_d [ crlf_p ];
				//this memo describes version 0
			typedef bs :: alternative < bs :: alternative < bs :: alternative < ipv4address_grammar,
				hostname_grammar >, ipv6address_grammar>,
				bs :: leaf_node_parser < bs :: positive < bs :: chset < char > > > > unicast_address_t;
			unicast_address_t unicast_address = ipv4address | hostname | ipv6address |
				bs :: leaf_node_d [ non_ws_string_p ];
			typedef bs :: sequence < bs :: sequence < bs :: sequence < bs :: sequence < bs :: sequence <
				bs :: sequence < bs :: sequence < bs :: sequence < bs :: sequence < bs :: sequence <
				bs :: sequence < bs :: sequence < bs :: no_tree_gen_node_parser < bs :: strlit < const char * > >,
				bs :: leaf_node_parser < bs :: positive < bs :: chset < char > > > >,
				bs :: no_tree_gen_node_parser < bs :: chlit < char > > >,
				bs :: leaf_node_parser < bs :: positive < bs :: digit_parser > > >,
				bs :: no_tree_gen_node_parser < bs :: chlit < char > > >,
				bs :: leaf_node_parser < bs :: positive < bs :: digit_parser > > >,
				bs :: no_tree_gen_node_parser < bs :: chlit < char > > >,
				bs :: leaf_node_parser < bs :: positive < bs :: chset < char > > > >,
				bs :: no_tree_gen_node_parser < bs :: chlit < char > > >,
				bs :: leaf_node_parser < bs :: positive < bs :: chset < char > > > >,
				bs :: no_tree_gen_node_parser < bs :: chlit < char > > >, unicast_address_t >,
				bs :: no_tree_gen_node_parser < bs :: chseq < const char * > > > origin_field_t;
			origin_field_t origin_field = bs :: no_node_d [ bs :: str_p ( "o=" ) ] >>
				bs :: leaf_node_d [ non_ws_string_p ] /*username*/ >>
				bs :: no_node_d [ bs :: ch_p ( ' ' ) ] >> bs :: leaf_node_d [ + bs :: digit_p ] /*sess_id*/ >>
				bs :: no_node_d [ bs :: ch_p ( ' ' ) ] >> bs :: leaf_node_d [ + bs :: digit_p ] /*sess_version*/ >>
				bs :: no_node_d [ bs :: ch_p ( ' ' ) ] >> bs :: leaf_node_d [ token_p ] /*nettype*/ >>
				bs :: no_node_d [ bs :: ch_p ( ' ' ) ] >> bs :: leaf_node_d [ token_p ] /*addrtype*/ >>
				bs :: no_node_d [ bs :: ch_p ( ' ' ) ] >> unicast_address >> bs :: no_node_d [ crlf_p ];
			typedef bs :: leaf_node_parser < bs :: positive < bs :: chset < char > > > text_t;
			text_t text = bs :: leaf_node_d [ + bs :: chset_p ( "\x01-\x09\x0b\x0c\x0e-\x7f\x80-\xff" ) ];
			typedef bs :: sequence < bs :: sequence < bs :: no_tree_gen_node_parser < bs :: strlit < const char * > >,
				bs :: leaf_node_parser < bs :: positive < bs :: chset < char > > > >,
				bs :: no_tree_gen_node_parser < bs :: chseq < const char * > > > session_name_field_t;
			session_name_field_t session_name_field = bs :: no_node_d [ bs :: str_p ( "s=" ) ] >>
				text >> bs :: no_node_d [ crlf_p ];
			announcement = proto_version >> origin_field >> session_name_field >> information_field >>
				uri_field >> email_fields >> phone_fields >> ! connection_field >> bandwidth_fields >>
				time_fields >> key_field >> attribute_fields >> media_descriptions;
			typedef bs :: leaf_node_parser < bs :: positive < bs :: chset < char > > > byte_string_t;
			byte_string_t byte_string = bs :: leaf_node_d [ + bs :: chset_p ( "\x01-\x09\x0b\x0c\x0e-\x7f\x80-\xff" ) ];
				//any byte except NUL, CR or LF
				//7f-80 due to signedness of char
				//default is to interpret this as IS0-10646 UTF8
				//ISO 8859-1 requires a "a=charset:ISO-8859-1"
				//session-level attribute to be used
			information_field = ! ( bs :: no_node_d [ bs :: str_p ( "i=" ) ] >> text >> bs :: no_node_d [ crlf_p ] );
			uri_field = ! ( bs :: no_node_d [ bs :: str_p ( "u=" ) ] >> uri >> bs :: no_node_d [ crlf_p ] );
			email_fields = * ( bs :: no_node_d [ bs :: str_p ( "e=" ) ] >> email_address >>
				bs :: no_node_d [ crlf_p ] );
			email_address = bs :: leaf_node_d [ + email_safe_p ] >> bs :: no_node_d [ bs :: ch_p ( '<' ) ] >> email >>
				bs :: no_node_d [ bs :: ch_p ( '>' ) ] | email >> ! ( bs :: no_node_d [ + bs :: ch_p ( ' ' ) >>
				bs :: ch_p ( '(' ) ] >> bs :: leaf_node_d [ + email_safe_p ] >>
				bs :: no_node_d [ bs :: ch_p ( ')' ) ] );
			phone_fields = * ( bs :: no_node_d [ bs :: str_p ( "p=" ) ] >> phone_number >> bs :: no_node_d [ crlf_p ] );
			phone_number = bs :: leaf_node_d [ + email_safe_p ] >> bs :: no_node_d [ bs :: ch_p ( '<' ) ] >> phone >>
				bs :: no_node_d [ bs :: ch_p ( '>' ) ] | phone >> ! ( bs :: no_node_d [ * bs :: ch_p ( ' ' ) >>
				bs :: ch_p ( '(' ) ] >> bs :: leaf_node_d [ + email_safe_p ] >>
				bs :: no_node_d [ bs :: ch_p ( ')' ) ] );
			phone = bs :: no_node_d [ ! bs :: ch_p ( '+' ) >> bs :: digit_p >>
				+ bs :: chset_p ( "- 0-9" ) ];
			connection_field = bs :: no_node_d [ bs :: str_p ( "c=" ) ] >> bs :: leaf_node_d [ token_p ] /*nettype*/ >>
				bs :: no_node_d [ bs :: ch_p ( ' ' ) ] >> bs :: leaf_node_d [ token_p ] /*addrtype*/ >>
				bs :: no_node_d [ bs :: ch_p ( ' ' ) ] >> ( /*multicast-address |*/ unicast_address ) >>
				bs :: no_node_d [ crlf_p ];	//a connection field must be present
										//in every media description or at the
										//session-level
			bandwidth_fields = * ( bs :: no_node_d [ bs :: str_p ( "b=" ) ] >> bs :: leaf_node_d [ token_p ] /*bwtype*/ >>
				bs :: no_node_d [ bs :: ch_p ( ':' ) ] >> bs :: leaf_node_d [ + bs :: digit_p ] /*bandwidth*/ >>
				bs :: no_node_d [ crlf_p ] );
			time_fields = + ( bs :: no_node_d [ bs :: str_p ( "t=" ) ] >>
				bs :: leaf_node_d [ '0' | time_p ] /*start_time*/ >> bs :: no_node_d [ bs :: ch_p ( ' ' ) ] >>
				bs :: leaf_node_d [ '0' | time_p ] /*stop_time*/ >> bs :: no_node_d [ crlf_p ] >>
				* ( bs :: no_node_d [ bs :: str_p ( "r=" ) ] >> repeat_fields >> bs :: no_node_d [ crlf_p ] ) ) >>
				! ( bs :: no_node_d [ bs :: str_p ( "z=" ) ] >> zone_adjustments >> bs :: no_node_d [ crlf_p ] );
			bs :: chset < char > fixed_len_time_unit = bs :: chset_p ( "dhms" );
			typedef bs :: sequence < bs :: leaf_node_parser < bs :: sequence < bs :: chset < char >,
				bs :: kleene_star < bs :: digit_parser > > >, bs :: optional < bs :: chset < char > > > repeat_interval_t;
			repeat_interval_t repeat_interval = bs :: leaf_node_d [ pos_digit_p >> * bs :: digit_p ] >> ! fixed_len_time_unit;
			repeat_fields = repeat_interval >> bs :: repeat_p ( 2, bs :: more )
				[ bs :: no_node_d [ bs :: ch_p ( ' ' ) ] >> typed_time ];
			typed_time = bs :: leaf_node_d [ + bs :: digit_p ] >> ! fixed_len_time_unit;
			zone_adjustments = bs :: list_p ( bs :: leaf_node_d [ time_p ] >>
				bs :: no_node_d [ bs :: ch_p ( ' ' ) ] >> ! bs :: ch_p ( '-' ) >>
				typed_time, bs :: no_node_d [ bs :: ch_p ( ' ' ) ] );
			typedef bs :: alternative < bs :: alternative < bs :: alternative < bs :: leaf_node_parser <
				bs :: strlit < const char * > >, bs :: sequence < bs :: leaf_node_parser < bs :: strlit < const char * > >,
				bs :: leaf_node_parser < bs :: positive < bs :: chset < char > > > > >,
				bs :: sequence < bs :: leaf_node_parser < bs :: strlit < const char * > >,
				bs :: leaf_node_parser < bs :: sequence < bs :: kleene_star < bs :: fixed_loop <
				bs :: chset < char >, int > >, bs :: optional < bs :: sequence < bs :: sequence <
				bs :: fixed_loop < bs :: chset < char >, int >, bs :: chset < char > >, bs :: chlit < char > > > > > > >,
				bs :: sequence < bs :: leaf_node_parser < bs :: strlit < const char * > >, uri_grammar > > key_type_t;
			key_type_t key_type = bs :: leaf_node_d [ bs :: str_p ( "prompt" ) ] |
				bs :: leaf_node_d [ bs :: str_p ( "clear:" ) ] >> text |
				bs :: leaf_node_d [ bs :: str_p ( "base64:" ) ] >>
				bs :: leaf_node_d [ base64_p ] |
				bs :: leaf_node_d [ bs :: str_p ( "uri:" ) ] >> uri;
			key_field = ! ( bs :: no_node_d [ bs :: str_p ( "k=" ) ] >> key_type >>
				bs :: no_node_d [ crlf_p ] );
			attribute_fields = * ( bs :: no_node_d [ bs :: str_p ( "a=" ) ] >>
				attribute >> bs :: no_node_d [ crlf_p ] );
			attribute = bs :: leaf_node_d [ token_p ] /*att_field*/ >> ! ( bs :: no_node_d [ bs :: ch_p ( ':' ) ] >> byte_string );
			media_descriptions = * ( media_field >> information_field >> * connection_field >>
				bandwidth_fields >> key_field >> attribute_fields );
			media_field = bs :: no_node_d [ bs :: str_p ( "m=" ) ] >> bs :: leaf_node_d [ token_p ] /*media*/ >>
				bs :: no_node_d [ bs :: ch_p ( ' ' ) ] >> bs :: leaf_node_d [ + bs :: digit_p ] /*port*/ >>
				! ( bs :: no_node_d [ bs :: ch_p ( '/' ) ] >> integer ) >>
				bs :: no_node_d [ bs :: ch_p ( ' ' ) ] >> bs :: leaf_node_d [ token_p >> * ( '/' >> token_p ) ] /*proto*/ >>
				+ ( bs :: no_node_d [ bs :: ch_p ( ' ' ) ] >> bs :: leaf_node_d [ token_p ] /*fmt*/ ) >>
				bs :: no_node_d [ crlf_p ];
			integer = bs :: no_node_d [ pos_digit_p >> * bs :: digit_p ];
			if ( self.tag_id )
				announcement.set_id ( self.tag_id );
		}
		hostname_grammar hostname;
		ipv4address_grammar ipv4address;
		ipv6address_grammar ipv6address;
		uri_grammar uri;
		email_address_grammar email;
		bs :: rule < ScannerT, bs :: dynamic_parser_tag > announcement;
		bs :: rule < ScannerT > email_address, phone_number, attribute, media_field;
#define RULE_ID( name ) bs :: rule < ScannerT, bs :: parser_tag < name##_id > > name;
		RULE_ID ( information_field );
		RULE_ID ( uri_field );
		RULE_ID ( email_fields );
		RULE_ID ( phone_fields );
		RULE_ID ( phone );
		RULE_ID ( connection_field );
		RULE_ID ( bandwidth_fields );
		RULE_ID ( time_fields );
		RULE_ID ( repeat_fields );
		RULE_ID ( typed_time );
		RULE_ID ( zone_adjustments );
		RULE_ID ( key_field );
		RULE_ID ( attribute_fields );
		RULE_ID ( media_descriptions );
		RULE_ID ( integer );
#undef RULE_ID
		bs :: rule < ScannerT, bs :: dynamic_parser_tag > const & start ( ) const {
			return announcement;
		}
	};
	sdp_grammar ( int t = 0 ) : tag_id ( t ) { }
	private:
	int tag_id;
};

RightParser :: result_t parseSdp ( const ss :: string :: const_iterator & ib, const ss :: string :: const_iterator & ie ) {
	sdp_grammar g;
	return RightParser :: parse ( ib, ie, g );
}


}
