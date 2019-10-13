#pragma implementation
#include "ss.hpp"
#include "dtmfrelay.hpp"
#include <stdexcept>
#include <boost/spirit/include/classic_parse_tree.hpp>
#include "rightparser.hpp"
#include "dtmfgrammars.hpp"

namespace DTMF {

void Relay :: printOn ( std :: ostream & os ) const {
	os << "Signal= " << signal << "\r\nDuration= " << msecs;
}

Relay :: Relay ( ss :: string :: const_iterator b, ss :: string :: const_iterator e ) {
	RightParser :: result_t info = parseRelay ( b, e );
	if ( ! info.full )
		throw std :: runtime_error ( std :: string ( "error parsing dtmf relay: " ).append ( b, e ) );
	RightParser :: iter_t i = info.trees.begin ( ) -> children.begin ( );
	signal = * i -> value.begin ( );
	b = ( ++ i ) -> value.begin ( );
	e  = i -> value.end ( );
	msecs = short ( * b - '0' );
	while ( ++ b < e ) {
		msecs = short ( msecs * 10 );
		msecs = short ( msecs + * b - '0' );
	}
}

}
