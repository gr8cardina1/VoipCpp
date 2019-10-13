#pragma implementation
#include "ss.hpp"

#include <boost/spirit/include/classic_parse_tree.hpp>
#include <boost/spirit/include/classic_utility.hpp>
#include "rightparser.hpp"
#include "hostnamegrammars.hpp"

#include "sipgrammars.hpp"

namespace bs = boost :: spirit :: classic;

namespace SIP2 {

typedef bs :: chseq < const char * > crlf_t;
const crlf_t crlf_p ( "\r\n" );

typedef bs :: sequence < bs :: optional < bs :: sequence < bs :: kleene_star < bs :: blank_parser >, crlf_t > >,
	bs :: positive < bs :: blank_parser > > lws_t;
const lws_t lws_p ( ! ( * bs :: blank_p >> crlf_p ) >> + bs :: blank_p );

typedef bs :: optional < lws_t > sws_t;
const sws_t sws_p ( ! lws_p );

typedef bs :: sequence < sws_t, bs :: chlit < char > > laquot_t;
const laquot_t laquot_p ( sws_p >> '<' );

typedef bs :: sequence < bs :: chlit < char >, sws_t > raquot_t;
const raquot_t raquot_p ( '>' >> sws_p );

typedef bs :: sequence < laquot_t, sws_t > equal_t;
const equal_t equal_p ( sws_p >> '=' >> sws_p ), comma_p ( sws_p >> ',' >> sws_p ), semi_p ( sws_p >> ';' >> sws_p ),
	slash_p ( sws_p >> '/' >> sws_p ), colon_p ( sws_p >> ':' >> sws_p );

typedef bs :: positive < bs :: alternative < bs :: alnum_parser, bs :: chset < char > > > token_t;
const token_t token_p ( + ( bs :: alnum_p | bs :: chset_p ( "-.!%*_+`'~" ) ) );

typedef bs :: chset < char > utf8cont_t;
const utf8cont_t utf8cont_p ( "\x80-\xbf" );

typedef bs :: sequence < bs :: chset < char >, bs :: chset < char > > utf82char_t;
typedef bs :: sequence < bs :: chset < char >, bs :: fixed_loop < bs :: chset < char >, int > > utf836char_t;

typedef bs :: alternative < bs :: alternative < bs :: alternative < bs :: alternative < utf82char_t, utf836char_t >,
	utf836char_t >, utf836char_t >, utf836char_t > utf8nonascii_t;

const utf8nonascii_t utf8nonascii_p = ( bs :: chset_p ( "\xc0-\xdf" ) >> utf8cont_p ) |
	( bs :: chset_p ( "\xe0-\xef" ) >> bs :: repeat_p ( 2 ) [ utf8cont_p ] ) |
	( bs :: chset_p ( "\xf0-\xf7" ) >> bs :: repeat_p ( 3 ) [ utf8cont_p ] ) |
	( bs :: chset_p ( "\xf8-\xfb" ) >> bs :: repeat_p ( 4 ) [ utf8cont_p ] ) |
	( bs :: chset_p ( "\xfc\xfd" ) >> bs :: repeat_p ( 5 ) [ utf8cont_p ] );

typedef bs :: sequence < bs :: sequence < bs :: chlit < char >, bs :: xdigit_parser >, bs :: xdigit_parser > escaped_t;
const escaped_t escaped_p ( '%' >> bs :: xdigit_p >> bs :: xdigit_p );

typedef bs :: chset < char > mark_t;
const mark_t mark_p ( "-_.!~*'()" );

typedef bs :: alternative < bs :: alnum_parser, bs :: chset < char > > unreserved_t;
const unreserved_t unreserved_p ( bs :: alnum_p | mark_p );

typedef bs :: sequence < bs :: chlit < char >, bs :: chset < char > > quotedPair_t;
const quotedPair_t quotedPair_p ( '\\' >> ( '\x00' | bs :: chset_p ( "\x01-\x09\x0b-\x0c\x0e-\x7f" ) ) );

struct quotedString_grammar : public bs :: grammar < quotedString_grammar > {
	template < typename ScannerT > struct definition {
		definition ( const quotedString_grammar & self ) {
			bs :: alternative < bs :: alternative < lws_t, bs :: chset < char > >, utf8nonascii_t > qdtext =
				lws_p | bs :: chset_p ( "\x21\x23-\x5b\x5d-\x7e" ) | utf8nonascii_p;
			qs = bs :: no_node_d [ '"' >> * ( qdtext | quotedPair_p ) >> '"' ];
			if ( self.tag_id )
				qs.set_id ( self.tag_id );
		}
		bs :: rule < ScannerT, bs :: dynamic_parser_tag > qs;
		bs :: rule < ScannerT, bs :: dynamic_parser_tag > const & start ( ) const {
			return qs;
		}
	};
	quotedString_grammar ( int t = 0 ) : tag_id ( t ) { }
	private:
	int tag_id;
};

struct host_grammar : public bs :: grammar < host_grammar > {
	static const unsigned hostname_id = 1;
	static const unsigned ipv4address_id = 2;
	static const unsigned ipv6address_id = 3;
	template < typename ScannerT > struct definition {
		definition ( const host_grammar & self ) : hostname ( hostname_id ), ipv4address ( ipv4address_id ), ipv6address ( ipv6address_id ) {
			bs :: sequence < bs :: sequence < bs :: no_tree_gen_node_parser < bs :: chlit < char > >,
				ipv6address_grammar >, bs :: no_tree_gen_node_parser < bs :: chlit < char > > > ipv6reference =
				bs :: no_node_d [ bs :: ch_p ( '[' ) ] >> ipv6address >> bs :: no_node_d [ bs :: ch_p ( ']' ) ];
			host = hostname | ipv4address | ipv6reference;
			if ( self.tag_id )
				host.set_id ( self.tag_id );
		}
		bs :: rule < ScannerT, bs :: dynamic_parser_tag > host;
		hostname_grammar hostname;
		ipv4address_grammar ipv4address;
		ipv6address_grammar ipv6address;
		bs :: rule < ScannerT, bs :: dynamic_parser_tag > const & start ( ) const {
			return host;
		}
	};
	host_grammar ( int t = 0 ) : tag_id ( t ) { }
	private:
	int tag_id;
};

struct genericParam_grammar : public bs :: grammar < genericParam_grammar > {
	static const unsigned host_id = 1;
	static const unsigned quotedString_id = 2;
	template < typename ScannerT > struct definition {
		definition ( const genericParam_grammar & self ) : host ( host_id ), quotedString ( quotedString_id ) {
			bs :: leaf_node_parser < token_t > token = bs :: leaf_node_d [ token_p ];
			gp = token >> ! ( bs :: no_node_d [ equal_p ] >> ( token | host | quotedString ) );
			if ( self.tag_id )
				gp.set_id ( self.tag_id );
		}
		bs :: rule < ScannerT, bs :: dynamic_parser_tag > gp;
		host_grammar host;
		quotedString_grammar quotedString;
		bs :: rule < ScannerT, bs :: dynamic_parser_tag > const & start ( ) const {
			return gp;
		}
	};
	genericParam_grammar ( int t = 0 ) : tag_id ( t ) { }
	private:
	int tag_id;
};

struct uri_grammar : public bs :: grammar < uri_grammar > {
	static const unsigned user_id = uri_grammar_user_id;
	static const unsigned password_id = uri_grammar_password_id;
	static const unsigned host_id = uri_grammar_host_id;
	static const unsigned port_id = uri_grammar_port_id;
	static const unsigned uriParameter_id = uri_grammar_uriParameter_id;
	static const unsigned header_id = uri_grammar_header_id;
	template < typename ScannerT > struct definition {
		definition ( const uri_grammar & self ) : host ( host_id ) {
			if ( self.noParams )
				uri = bs :: no_node_d [ bs :: chseq_p ( "sip:" ) ] >> ! ( user >>
					! ( bs :: no_node_d [ bs :: ch_p ( ':' ) ] >> password ) >>
					bs :: no_node_d [ bs :: ch_p ( '@' ) ] ) >> host >>
					! ( bs :: no_node_d [ bs :: ch_p ( ':' ) ] >> port ) >>
					! ( bs :: no_node_d [ bs :: ch_p ( '?' ) ] >> header >>
					* ( bs :: no_node_d [ bs :: ch_p ( '&' ) ] >> header ) );
			else
				uri = bs :: no_node_d [ bs :: chseq_p ( "sip:" ) ] >> ! ( user >>
					! ( bs :: no_node_d [ bs :: ch_p ( ':' ) ] >> password ) >>
					bs :: no_node_d [ bs :: ch_p ( '@' ) ] ) >> host >>
					! ( bs :: no_node_d [ bs :: ch_p ( ':' ) ] >> port ) >>
					* ( bs :: no_node_d [ bs :: ch_p ( ';' ) ] >> uriParameter ) >>
					! ( bs :: no_node_d [ bs :: ch_p ( '?' ) ] >> header >>
					* ( bs :: no_node_d [ bs :: ch_p ( '&' ) ] >> header ) );
			bs :: chset < char > userUnreserved ( "&=+$,:?/" );
			user = bs :: no_node_d [ + ( unreserved_p | userUnreserved | escaped_p ) ];
			password = bs :: no_node_d [ * ( unreserved_p | bs :: chset_p ( "&=+$," ) | escaped_p ) ];
			port = + bs :: digit_p;
			if ( ! self.noParams ) {
				bs :: chset < char > paramUnreserved ( "[]/:&+$" );
				typedef bs :: leaf_node_parser < bs :: positive < bs :: alternative < bs :: alternative <
					bs :: chset < char >, bs :: alternative < bs :: alnum_parser, bs :: chset < char > > >,
					bs :: sequence < bs :: sequence < bs :: chlit < char >, bs :: xdigit_parser >,
					bs :: xdigit_parser > > > > pname_t;
				pname_t pname = bs :: leaf_node_d [ + ( paramUnreserved | unreserved_p | escaped_p ) ];
				pname_t pvalue = bs :: leaf_node_d [ + ( paramUnreserved | unreserved_p | escaped_p ) ];
				uriParameter = pname >> ! ( bs :: no_node_d [ bs :: ch_p ( '=' ) ] >> pvalue );
			}
			bs :: chset < char > hnvUnreserved ( "[]/?:+$" );
			typedef bs :: leaf_node_parser < bs :: positive < bs :: alternative < bs :: alternative < bs :: chset < char >,
				bs :: alternative < bs :: alnum_parser, bs :: chset < char > > >,
				bs :: sequence < bs :: sequence < bs :: chlit < char >, bs :: xdigit_parser >,
				bs :: xdigit_parser > > > > hname_t;
			hname_t hname = bs :: leaf_node_d [ + ( hnvUnreserved | unreserved_p | escaped_p ) ];
			typedef bs :: leaf_node_parser < bs :: kleene_star < bs :: alternative < bs :: alternative < bs :: chset < char >,
				bs :: alternative < bs :: alnum_parser, bs :: chset < char > > >,
				bs :: sequence < bs :: sequence < bs :: chlit < char >, bs :: xdigit_parser >,
				bs :: xdigit_parser > > > > hvalue_t;
			hvalue_t hvalue = bs :: leaf_node_d [ * ( hnvUnreserved | unreserved_p | escaped_p ) ];
			header = hname >> bs :: no_node_d [ bs :: ch_p ( '=' ) ] >> hvalue;
			if ( self.tag_id )
				uri.set_id ( self.tag_id );
		}
		bs :: rule < ScannerT, bs :: dynamic_parser_tag > uri;
		host_grammar host;
#define RULE_ID( name ) bs :: rule < ScannerT, bs :: parser_tag < name##_id > > name;
		RULE_ID ( user );
		RULE_ID ( password );
		RULE_ID ( port );
		RULE_ID ( uriParameter );
		RULE_ID ( header );
#undef RULE_ID
		bs :: rule < ScannerT, bs :: dynamic_parser_tag > const & start ( ) const {
			return uri;
		}
	};
	uri_grammar ( int t = 0, bool np = false ) : tag_id ( t ), noParams ( np ) { }
	private:
	int tag_id;
	bool noParams;
};

struct recordRoute_grammar : public bs :: grammar < recordRoute_grammar > {
	static const unsigned uri_id = recordRoute_grammar_uri_id;
	template < typename ScannerT > struct definition {
		definition ( const recordRoute_grammar & self ) : uri ( uri_id ) {
			recordRoute = recRoute >> * ( bs :: no_node_d [ comma_p ] >> recRoute );
			typedef bs :: leaf_node_parser < bs :: alternative < bs :: sequence < token_t,
				bs :: kleene_star < bs :: sequence < lws_t, token_t > > >, quotedString_grammar > > displayName_t;
			displayName_t displayName = bs :: leaf_node_d [ token_p >> * ( lws_p >> token_p ) | quotedString ];
			recRoute = ! displayName >> bs :: no_node_d [ laquot_p ] >> uri >> bs :: no_node_d [ raquot_p ] >>
				* ( bs :: no_node_d [ semi_p ] >> genericParam );
			if ( self.tag_id )
				recordRoute.set_id ( self.tag_id );
		}
		bs :: rule < ScannerT, bs :: dynamic_parser_tag > recordRoute;
		bs :: rule < ScannerT > recRoute;
		uri_grammar uri;
		quotedString_grammar quotedString;
		genericParam_grammar genericParam;
		bs :: rule < ScannerT, bs :: dynamic_parser_tag > const & start ( ) const {
			return recordRoute;
		}
	};
	recordRoute_grammar ( int t = 0 ) : tag_id ( t ) { }
	private:
	int tag_id;
};

struct via_grammar : public bs :: grammar < via_grammar > {
	static const unsigned genericParam_id = via_grammar_genericParam_id;
	template < typename ScannerT > struct definition {
		definition ( const via_grammar & self ) : genericParam ( genericParam_id ) {
			via = viaParm >> * ( bs :: no_node_d [ comma_p ] >> viaParm );
			bs :: leaf_node_parser < token_t > transport = bs :: leaf_node_d [ token_p ];
			bs :: leaf_node_parser < bs :: positive < bs :: digit_parser > > port = bs :: leaf_node_d [ + bs :: digit_p ];
			viaParm = bs :: no_node_d [ "SIP" >> slash_p >> "2.0" >> slash_p ] >> transport >>
				bs :: no_node_d [ lws_p ] >> host >> ! ( bs :: no_node_d [ colon_p ] >> port ) >>
				* ( bs :: no_node_d [ semi_p ] >> genericParam );
			if ( self.tag_id )
				via.set_id ( self.tag_id );
		}
		bs :: rule < ScannerT, bs :: dynamic_parser_tag > via;
		bs :: rule < ScannerT > viaParm;
		host_grammar host;
		genericParam_grammar genericParam;
		bs :: rule < ScannerT, bs :: dynamic_parser_tag > const & start ( ) const {
			return via;
		}
	};
	via_grammar ( int t = 0 ) : tag_id ( t ) { }
	private:
	int tag_id;
};

struct fromtocontactheader_grammar : public bs :: grammar < fromtocontactheader_grammar > {
	static const unsigned uri_id = fromtocontactheader_grammar_uri_id;
	template < typename ScannerT > struct definition {
		definition ( const fromtocontactheader_grammar & self ) : uri ( uri_id ), uriNoParams ( uri_id, true ) {
			typedef bs :: leaf_node_parser < bs :: alternative < bs :: sequence < token_t,
				bs :: kleene_star < bs :: sequence < lws_t, token_t > > >, quotedString_grammar > > displayName_t;
			displayName_t displayName = bs :: leaf_node_d [ token_p >> * ( lws_p >> token_p ) | quotedString ];
			fromtocontactheader = ( ! displayName >> bs :: no_node_d [ laquot_p ] >> uri >>
				bs :: no_node_d [ raquot_p ] | uriNoParams ) >> * ( bs :: no_node_d [ semi_p ] >> genericParam );
			if ( self.tag_id )
				fromtocontactheader.set_id ( self.tag_id );
		}
		bs :: rule < ScannerT, bs :: dynamic_parser_tag > fromtocontactheader;
		uri_grammar uri, uriNoParams;
		quotedString_grammar quotedString;
		genericParam_grammar genericParam;
		bs :: rule < ScannerT, bs :: dynamic_parser_tag > const & start ( ) const {
			return fromtocontactheader;
		}
	};
	fromtocontactheader_grammar ( int t = 0 ) : tag_id ( t ) { }
	private:
	int tag_id;
};

struct digestChallenge_grammar : public bs :: grammar < digestChallenge_grammar > {
	template < typename ScannerT > struct definition {
		definition ( const digestChallenge_grammar & self ) {
			//tut vse ne quoted strings case-insensitieve nado sdelat
			typedef bs :: sequence < bs :: sequence < bs :: leaf_node_parser < token_t >, bs :: no_tree_gen_node_parser < equal_t > >,
				bs :: leaf_node_parser < bs :: alternative < token_t, quotedString_grammar > > > authParam_t;
			authParam_t authParam = bs :: leaf_node_d [ token_p ] >> bs :: no_node_d [ equal_p ] >>
				bs :: leaf_node_d [ token_p | quotedString ];
			dc = bs :: no_node_d [ "Digest" >> lws_p ] >> authParam >> * ( bs :: no_node_d [ comma_p ] >> authParam );
			if ( self.tag_id )
				dc.set_id ( self.tag_id );
		}
		bs :: rule < ScannerT, bs :: dynamic_parser_tag > dc;
		quotedString_grammar quotedString;
		bs :: rule < ScannerT, bs :: dynamic_parser_tag > const & start ( ) const {
			return dc;
		}
	};
	digestChallenge_grammar ( int t = 0 ) : tag_id ( t ) { }
	private:
	int tag_id;
};

struct authInfo_grammar : public bs :: grammar < authInfo_grammar > {
	template < typename ScannerT > struct definition {
		definition ( const authInfo_grammar & self ) {
			//tut vse ne quoted strings case-insensitieve nado sdelat
			typedef bs :: sequence < bs :: sequence < bs :: leaf_node_parser < token_t >, bs :: no_tree_gen_node_parser < equal_t > >,
				bs :: leaf_node_parser < bs :: alternative < token_t, quotedString_grammar > > > authParam_t;
			authParam_t authParam = bs :: leaf_node_d [ token_p ] >> bs :: no_node_d [ equal_p ] >>
				bs :: leaf_node_d [ token_p | quotedString ];
			ai = authParam >> * ( bs :: no_node_d [ comma_p ] >> authParam );
			if ( self.tag_id )
				ai.set_id ( self.tag_id );
		}
		bs :: rule < ScannerT, bs :: dynamic_parser_tag > ai;
		quotedString_grammar quotedString;
		bs :: rule < ScannerT, bs :: dynamic_parser_tag > const & start ( ) const {
			return ai;
		}
	};
	authInfo_grammar ( int t = 0 ) : tag_id ( t ) { }
	private:
	int tag_id;
};

struct comment_grammar : public bs :: grammar < comment_grammar > {
	template < typename ScannerT > struct definition {
		definition ( const comment_grammar & self ) {
			equal_t lparen = sws_p >> '(' >> sws_p;
			equal_t rparen = sws_p >> ')' >> sws_p;
			comment = bs :: no_node_d [ lparen ] >> bs :: leaf_node_d [ * ( bs :: chset_p (
				"\x20-\x27\x2a-\x5b\x5d-\x7e" ) | utf8nonascii_p | lws_p | quotedPair_p | comment ) ] >>
				bs :: no_node_d [ rparen ];
			if ( self.tag_id )
				comment.set_id ( self.tag_id );
		}
		bs :: rule < ScannerT, bs :: dynamic_parser_tag > comment;
		bs :: rule < ScannerT, bs :: dynamic_parser_tag > const & start ( ) const {
			return comment;
		}
	};
	comment_grammar ( int t = 0 ) : tag_id ( t ) { }
	private:
	int tag_id;
};

struct retryAfter_grammar : public bs :: grammar < retryAfter_grammar > {
	static const unsigned comment_id = retryAfter_grammar_comment_id;
	template < typename ScannerT > struct definition {
		definition ( const retryAfter_grammar & self ) : comment ( comment_id ) {
			//tut vse ne quoted strings case-insensitieve nado sdelat
			ra = bs :: leaf_node_d [ + bs :: digit_p ] >> ! comment >>
				* ( bs :: no_node_d [ semi_p ] >> genericParam );
			if ( self.tag_id )
				ra.set_id ( self.tag_id );
		}
		genericParam_grammar genericParam;
		comment_grammar comment;
		bs :: rule < ScannerT, bs :: dynamic_parser_tag > ra;
		bs :: rule < ScannerT, bs :: dynamic_parser_tag > const & start ( ) const {
			return ra;
		}
	};
	retryAfter_grammar ( int t = 0 ) : tag_id ( t ) { }
	private:
	int tag_id;
};

struct warning_grammar : public bs :: grammar < warning_grammar > {
	template < typename ScannerT > struct definition {
		definition ( const warning_grammar & self ) {
			warnAgent = bs :: discard_node_d [ host >> ! ( ':' >> + bs :: digit_p ) | token_p ];
			warning = warningValue >> * ( bs :: no_node_d [ comma_p ] >> warningValue );
			warningValue = bs :: leaf_node_d [ bs :: repeat_p ( 3 ) [ bs :: digit_p ] ] >>
				bs :: no_node_d [ bs :: ch_p ( ' ' ) ] >> warnAgent >>
				bs :: no_node_d [ bs :: ch_p ( ' ' ) ] >> quotedString;
			if ( self.tag_id )
				warning.set_id ( self.tag_id );
		}
		host_grammar host;
		quotedString_grammar quotedString;
		bs :: rule < ScannerT, bs :: dynamic_parser_tag > warning;
		bs :: rule < ScannerT > warningValue, warnAgent;
		bs :: rule < ScannerT, bs :: dynamic_parser_tag > const & start ( ) const {
			return warning;
		}
	};
	warning_grammar ( int t = 0 ) : tag_id ( t ) { }
	private:
	int tag_id;
};

struct sessionExpires_grammar : public bs :: grammar < sessionExpires_grammar > {
	template < typename ScannerT > struct definition {
		definition ( const sessionExpires_grammar & self ) {
			//tut vse ne quoted strings case-insensitieve nado sdelat
			se = bs :: leaf_node_d [ + bs :: digit_p ] >> * ( bs :: no_node_d [ semi_p ] >> genericParam );
			if ( self.tag_id )
				se.set_id ( self.tag_id );
		}
		genericParam_grammar genericParam;
		bs :: rule < ScannerT, bs :: dynamic_parser_tag > se;
		bs :: rule < ScannerT, bs :: dynamic_parser_tag > const & start ( ) const {
			return se;
		}
	};
	sessionExpires_grammar ( int t = 0 ) : tag_id ( t ) { }
	private:
	int tag_id;
};

struct supported_grammar : public bs :: grammar < supported_grammar > {
	template < typename ScannerT > struct definition {
		definition ( const supported_grammar & self ) {
			bs :: leaf_node_parser < token_t > token = bs :: leaf_node_d [ token_p ];
			supported = ! ( token >> * ( bs :: no_node_d [ comma_p ] >> token ) );
			if ( self.tag_id )
				supported.set_id ( self.tag_id );
		}
		bs :: rule < ScannerT, bs :: dynamic_parser_tag > supported;
		bs :: rule < ScannerT, bs :: dynamic_parser_tag > const & start ( ) const {
			return supported;
		}
	};
	supported_grammar ( int t = 0 ) : tag_id ( t ) { }
	private:
	int tag_id;
};

struct require_grammar : public bs :: grammar < require_grammar > {
	template < typename ScannerT > struct definition {
		definition ( const require_grammar & self ) {
			bs :: leaf_node_parser < token_t > token = bs :: leaf_node_d [ token_p ];
			require = token >> * ( bs :: no_node_d [ comma_p ] >> token );
			if ( self.tag_id )
				require.set_id ( self.tag_id );
		}
		bs :: rule < ScannerT, bs :: dynamic_parser_tag > require;
		bs :: rule < ScannerT, bs :: dynamic_parser_tag > const & start ( ) const {
			return require;
		}
	};
	require_grammar ( int t = 0 ) : tag_id ( t ) { }
	private:
	int tag_id;
};

struct contentType_grammar : public bs :: grammar < contentType_grammar > {
	template < typename ScannerT > struct definition {
		definition ( const contentType_grammar & self ) {
			bs :: leaf_node_parser < token_t > token = bs :: leaf_node_d [ token_p ];
			contentType = token >> bs :: no_node_d [ slash_p ] >> token >>
				* ( bs :: no_node_d [ semi_p ] >> token >> bs :: no_node_d [ equal_p ] >>
				( token | quotedString ) );
			if ( self.tag_id )
				contentType.set_id ( self.tag_id );
		}
		quotedString_grammar quotedString;
		bs :: rule < ScannerT, bs :: dynamic_parser_tag > contentType;
		bs :: rule < ScannerT, bs :: dynamic_parser_tag > const & start ( ) const {
			return contentType;
		}
	};
	contentType_grammar ( int t = 0 ) : tag_id ( t ) { }
	private:
	int tag_id;
};

struct accept_grammar : public bs :: grammar < accept_grammar > {
	template < typename ScannerT > struct definition {
		definition ( const accept_grammar & self ) {
			bs :: leaf_node_parser < token_t > token = bs :: leaf_node_d [ token_p ];
			accept = ! ( acceptRange >> * ( bs :: no_node_d [ comma_p ] >> acceptRange ) );
			bs :: leaf_node_parser < bs :: chlit < char > > mul_p = bs :: leaf_node_d [ bs :: ch_p ( '*' ) ];
			bs :: no_tree_gen_node_parser < equal_t > slash_np = bs :: no_node_d [ slash_p ],
				semi_np = bs :: no_node_d [ semi_p ], equal_np = bs :: no_node_d [ equal_p ];
			acceptRange = ( mul_p >> slash_np >> mul_p | token >> slash_np >> mul_p | token >> slash_np >> token ) >>
				* ( semi_np >> ( token - 'q' ) >> equal_np >> ( token | quotedString ) ) >>
				! ( qparam >> * genericParam );
			qparam = semi_np >> 'q' >> equal_np >> bs :: leaf_node_d [ '0' >> ! ( '.' >>
				bs :: repeat_p ( 0, 3 ) [ bs :: digit_p ] ) | '1' >> ! ( '.' >>
				bs :: repeat_p ( 0, 3 ) [ bs :: ch_p ( '0' ) ] ) ];
			token >> bs :: no_node_d [ slash_p ] >> token >>  * ( bs :: no_node_d [ semi_p ] >> token >>
				bs :: no_node_d [ equal_p ] >> ( token | quotedString ) );
			if ( self.tag_id )
				accept.set_id ( self.tag_id );
		}
		quotedString_grammar quotedString;
		genericParam_grammar genericParam;
		bs :: rule < ScannerT > acceptRange, qparam;
		bs :: rule < ScannerT, bs :: dynamic_parser_tag > accept;
		bs :: rule < ScannerT, bs :: dynamic_parser_tag > const & start ( ) const {
			return accept;
		}
	};
	accept_grammar ( int t = 0 ) : tag_id ( t ) { }
	private:
	int tag_id;
};

struct contentDisposition_grammar : public bs :: grammar < contentDisposition_grammar > {
	template < typename ScannerT > struct definition {
		definition ( const contentDisposition_grammar & self ) {
			//tut vse ne quoted strings case-insensitieve nado sdelat
			cd = bs :: leaf_node_d [ token_p ] >> * ( bs :: no_node_d [ semi_p ] >> genericParam );
			if ( self.tag_id )
				cd.set_id ( self.tag_id );
		}
		genericParam_grammar genericParam;
		bs :: rule < ScannerT, bs :: dynamic_parser_tag > cd;
		bs :: rule < ScannerT, bs :: dynamic_parser_tag > const & start ( ) const {
			return cd;
		}
	};
	contentDisposition_grammar ( int t = 0 ) : tag_id ( t ) { }
	private:
	int tag_id;
};

struct sip_grammar : public bs :: grammar < sip_grammar > {
	static const unsigned requestLine_id = sip_grammar_requestLine_id;
	static const unsigned statusLine_id = sip_grammar_statusLine_id;
	static const unsigned messageHeader_id = sip_grammar_messageHeader_id;
	template < typename ScannerT > struct definition {
		definition ( const sip_grammar & self ) {
			bs :: kleene_star < bs :: anychar_parser > messageBody = * bs :: anychar_p;
			message = ( requestLine | statusLine ) >> + messageHeader >> bs :: no_node_d [ crlf_p ] >>
				bs :: leaf_node_d [ messageBody ];
			typedef bs :: sequence < bs :: sequence < bs :: kleene_star < bs :: blank_parser >,
				bs :: chlit < char > >, sws_t > hcolon_t;
			hcolon_t hcolon ( * ( bs :: blank_p ) >> ':' >> sws_p );
			typedef bs :: alternative < utf8nonascii_t, bs :: chset < char > > textutf8char_t;
			textutf8char_t textutf8char = utf8nonascii_p | bs :: chset_p ( "\x21-\x7e" );
			typedef bs :: leaf_node_parser < bs :: kleene_star < bs :: alternative < bs :: alternative < textutf8char_t,
				utf8cont_t >, lws_t > > > headerValue_t;
			headerValue_t headerValue = bs :: leaf_node_d [ * ( textutf8char | utf8cont_p | lws_p ) ];
			messageHeader = bs :: leaf_node_d [ token_p ] >> bs :: no_node_d [ hcolon ] >> headerValue >>
				bs :: no_node_d [ crlf_p ];
			typedef bs :: leaf_node_parser < token_t > method_t;
			method_t method = bs :: leaf_node_d [ token_p ];
			requestLine = method >> bs :: no_node_d [ bs :: ch_p ( ' ' ) ] >>  requestUri >>
				bs :: no_node_d [ " SIP/2.0" >> crlf_p ];
			bs :: fixed_loop < bs :: digit_parser, int > statusCode = bs :: repeat_p ( 3 ) [ bs :: digit_p ];
			statusLine = bs :: no_node_d [ bs :: chseq_p ( "SIP/2.0 " ) ] >> bs :: leaf_node_d [ statusCode ] >>
				bs :: no_node_d [ bs :: ch_p ( ' ' ) ] >> reasonPhrase >> bs :: no_node_d [ crlf_p ];
			bs :: chset < char > reserved ( "/?:@&=+$," );
			reasonPhrase = bs :: no_node_d [ * ( reserved | unreserved_p | escaped_p | utf8nonascii_p | utf8cont_p ) >>
				* ( + bs :: blank_p >> + ( reserved | unreserved_p | escaped_p | utf8nonascii_p | utf8cont_p ) ) ];
			if ( self.tag_id )
				message.set_id ( self.tag_id );
		}
		bs :: rule < ScannerT, bs :: dynamic_parser_tag > message;
		bs :: rule < ScannerT > reasonPhrase;
		uri_grammar requestUri;
#define RULE_ID( name ) bs :: rule < ScannerT, bs :: parser_tag < name##_id > > name;
		RULE_ID ( requestLine );
		RULE_ID ( statusLine );
		RULE_ID ( messageHeader );
#undef RULE_ID
		bs :: rule < ScannerT, bs :: dynamic_parser_tag > const & start ( ) const {
			return message;
		}
	};
	sip_grammar ( int t = 0 ) : tag_id ( t ) { }
	private:
	int tag_id;
};

RightParser :: result_t parseUri ( const ss :: string & s ) {
	uri_grammar g;
	const ss :: string :: const_iterator ib = s.begin ( ), ie = s.end ( );
	return RightParser :: parse ( ib, ie, g );
}

RightParser :: result_t parseFromToContactHeader ( const ss :: string & s ) {
	fromtocontactheader_grammar g;
	const ss :: string :: const_iterator ib = s.begin ( ), ie = s.end ( );
	return RightParser :: parse ( ib, ie, g );
}

RightParser :: result_t parseDigestChallenge ( const ss :: string & s ) {
	digestChallenge_grammar g;
	const ss :: string :: const_iterator ib = s.begin ( ), ie = s.end ( );
	return RightParser :: parse ( ib, ie, g );
}

RightParser :: result_t parseVia ( const ss :: string & s ) {
	via_grammar g;
	const ss :: string :: const_iterator ib = s.begin ( ), ie = s.end ( );
	return RightParser :: parse ( ib, ie, g );
}

RightParser :: result_t parseSip ( const ss :: string :: const_iterator & ib, const ss :: string :: const_iterator & ie ) {
	sip_grammar g;
	return RightParser :: parse ( ib, ie, g );
}

RightParser :: result_t parseAuthenticationInfo ( const ss :: string & s ) {
	authInfo_grammar g;
	const ss :: string :: const_iterator ib = s.begin ( ), ie = s.end ( );
	return RightParser :: parse ( ib, ie, g );
}

RightParser :: result_t parseRetryAfter ( const ss :: string & s ) {
	retryAfter_grammar g;
	const ss :: string :: const_iterator ib = s.begin ( ), ie = s.end ( );
	return RightParser :: parse ( ib, ie, g );
}

RightParser :: result_t parseRecordRoute ( const ss :: string & s ) {
	recordRoute_grammar g;
	const ss :: string :: const_iterator ib = s.begin ( ), ie = s.end ( );
	return RightParser :: parse ( ib, ie, g );
}

RightParser :: result_t parseWarning ( const ss :: string & s ) {
	warning_grammar g;
	const ss :: string :: const_iterator ib = s.begin ( ), ie = s.end ( );
	return RightParser :: parse ( ib, ie, g );
}

RightParser :: result_t parseSessionExpires ( const ss :: string & s ) {
	sessionExpires_grammar g;
	const ss :: string :: const_iterator ib = s.begin ( ), ie = s.end ( );
	return RightParser :: parse ( ib, ie, g );
}

RightParser :: result_t parseSupported ( const ss :: string & s ) {
	supported_grammar g;
	const ss :: string :: const_iterator ib = s.begin ( ), ie = s.end ( );
	return RightParser :: parse ( ib, ie, g );
}

RightParser :: result_t parseRequire ( const ss :: string & s ) {
	require_grammar g;
	const ss :: string :: const_iterator ib = s.begin ( ), ie = s.end ( );
	return RightParser :: parse ( ib, ie, g );
}

RightParser :: result_t parseContentType ( const ss :: string & s ) {
	contentType_grammar g;
	const ss :: string :: const_iterator ib = s.begin ( ), ie = s.end ( );
	return RightParser :: parse ( ib, ie, g );
}

RightParser :: result_t parseAccept ( const ss :: string & s ) {
	accept_grammar g;
	const ss :: string :: const_iterator ib = s.begin ( ), ie = s.end ( );
	return RightParser :: parse ( ib, ie, g );
}

RightParser :: result_t parseContentDisposition ( const ss :: string & s ) {
	contentDisposition_grammar g;
	const ss :: string :: const_iterator ib = s.begin ( ), ie = s.end ( );
	return RightParser :: parse ( ib, ie, g );
}

}
