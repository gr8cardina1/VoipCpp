#ifndef HOSTNAMEGRAMMARS_HPP_
#define HOSTNAMEGRAMMARS_HPP_
#pragma interface

struct ipv4address_grammar : public boost :: spirit :: classic :: grammar < ipv4address_grammar > {
	template < typename ScannerT > struct definition {
		definition ( const ipv4address_grammar & self ) {
			namespace bs = boost :: spirit :: classic;
			ipv4address = bs :: leaf_node_d [ bs :: repeat_p ( 1, 3 ) [ bs :: digit_p ] ] >>
				bs :: no_node_d [ bs :: ch_p ( '.' ) ] >>
				bs :: leaf_node_d [ bs :: repeat_p ( 1, 3 ) [ bs :: digit_p ] ] >>
				bs :: no_node_d [ bs :: ch_p ( '.' ) ] >>
				bs :: leaf_node_d [ bs :: repeat_p ( 1, 3 ) [ bs :: digit_p ] ] >>
				bs :: no_node_d [ bs :: ch_p ( '.' ) ] >>
				bs :: leaf_node_d [ bs :: repeat_p ( 1, 3 ) [ bs :: digit_p ] ];
			if ( self.tag_id )
				ipv4address.set_id ( self.tag_id );
		}
		boost :: spirit :: classic :: rule < ScannerT, boost :: spirit :: classic :: dynamic_parser_tag > ipv4address;
		boost :: spirit :: classic :: rule < ScannerT, boost :: spirit :: classic :: dynamic_parser_tag > const & start ( ) const {
			return ipv4address;
		}
	};
	ipv4address_grammar ( int t = 0 ) : tag_id ( t ) { }
	private:
	int tag_id;
};

struct ipv6address_grammar : public boost :: spirit :: classic :: grammar < ipv6address_grammar > {
	template < typename ScannerT > struct definition {
		definition ( const ipv6address_grammar & self ) {
			namespace bs = boost :: spirit :: classic;
			ipv6address = hexpart >> ! ( bs :: no_node_d [ bs :: ch_p ( ':' ) ] >> ipv4address );
			hexpart = hexseq >> ! ( bs :: leaf_node_d [ bs :: str_p ( "::" ) ] >> ! hexseq ) |
				bs :: leaf_node_d [ bs :: str_p ( "::" ) ] >> ! hexseq;
			typedef bs :: leaf_node_parser < bs :: finite_loop < bs :: xdigit_parser, int, int > > hex4_type;
			hex4_type hex4 = bs :: leaf_node_d [
				bs :: finite_loop < bs :: xdigit_parser, int, int > ( bs :: xdigit_p, 1, 4 ) ];
			hexseq = hex4 >> * ( bs :: no_node_d [ bs :: ch_p ( ':' ) ] >> hex4 );
			if ( self.tag_id )
				ipv6address.set_id ( self.tag_id );
		}
		ipv4address_grammar ipv4address;
		boost :: spirit :: classic :: rule < ScannerT > hexseq;
		boost :: spirit :: classic :: rule < ScannerT > hexpart;
		boost :: spirit :: classic :: rule < ScannerT, boost :: spirit :: classic :: dynamic_parser_tag > ipv6address;
		boost :: spirit :: classic :: rule < ScannerT, boost :: spirit :: classic :: dynamic_parser_tag > const & start ( ) const {
			return ipv6address;
		}
	};
	ipv6address_grammar ( int t = 0 ) : tag_id ( t ) { }
	private:
	int tag_id;
};

struct hostname_grammar : public boost :: spirit :: classic :: grammar < hostname_grammar > {
	template < typename ScannerT > struct definition {
		definition ( const hostname_grammar & self ) {
			namespace bs = boost :: spirit :: classic;
			toplabel = bs :: alpha_p >> ! ( * ( ( bs :: alnum_p | '-' ) >>
				bs :: eps_p ( bs :: alnum_p | '-' ) ) >> bs :: alnum_p );
			domainlabel = bs :: alnum_p >> ! ( * ( ( bs :: alnum_p | '-' ) >>
				bs :: eps_p ( bs :: alnum_p | '-' ) ) >> bs :: alnum_p );
			hostname = * ( bs :: leaf_node_d [ domainlabel ] >> bs :: no_node_d [ bs :: ch_p ( '.' ) ] ) >>
				bs :: leaf_node_d [ toplabel ] >> bs :: no_node_d [ ! bs :: ch_p ( '.' ) ];
			if ( self.tag_id )
				hostname.set_id ( self.tag_id );
		}
		boost :: spirit :: classic :: rule < ScannerT, boost :: spirit :: classic :: dynamic_parser_tag > hostname;
		boost :: spirit :: classic :: rule < ScannerT > toplabel, domainlabel;
		boost :: spirit :: classic :: rule < ScannerT, boost :: spirit :: classic :: dynamic_parser_tag > const & start ( ) const {
			return hostname;
		}
	};
	hostname_grammar ( int t = 0 ) : tag_id ( t ) { }
	private:
	int tag_id;
};

#endif /*HOSTNAMEGRAMMARS_HPP_*/
