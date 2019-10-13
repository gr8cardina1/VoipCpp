#pragma implementation
#include "ss.hpp"

#include <boost/spirit/include/classic_parse_tree.hpp>
#include <boost/spirit/include/classic_utility.hpp>
#include "rightparser.hpp"
#include "dtmfgrammars.hpp"

namespace bs = boost :: spirit :: classic;

namespace DTMF {

struct Relay_grammar : public bs :: grammar < Relay_grammar > {
	template < typename ScannerT > struct definition {
		definition ( const Relay_grammar & self ) {
			relay = bs :: no_node_d [ "Signal=" >> ! bs :: ch_p ( ' ' ) ] >> bs :: chset_p ( "0-9A-D*#" ) >>
				bs :: no_node_d [ "\r\nDuration=" >> ! bs :: ch_p ( ' ' ) ] >>
				bs :: leaf_node_d [ bs :: repeat_p ( 1, 4 ) [ bs :: digit_p ] ]
				>> bs :: no_node_d [ ! bs :: chseq_p ( "\r\n" ) ];
			if ( self.tag_id )
				relay.set_id ( self.tag_id );
		}
		bs :: rule < ScannerT, bs :: dynamic_parser_tag > relay;
		bs :: rule < ScannerT, bs :: dynamic_parser_tag > const & start ( ) const {
			return relay;
		}
	};
	Relay_grammar ( int t = 0 ) : tag_id ( t ) { }
private:
	int tag_id;
};

RightParser :: result_t parseRelay ( const ss :: string :: const_iterator & ib, const ss :: string :: const_iterator & ie ) {
	Relay_grammar g;
	return RightParser :: parse ( ib, ie, g );
}

}

