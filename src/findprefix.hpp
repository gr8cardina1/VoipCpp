#ifndef __FINDPREFIX_HPP
#define __FINDPREFIX_HPP
#pragma interface

template < typename _Map, typename _String > typename _Map :: const_iterator findPrefix ( const _Map & m, const _String & s ) {
	if ( m.empty ( ) )
		return m.end ( );
	typename _Map :: const_iterator i = m.upper_bound ( s );
	if ( i == m.begin ( ) )
		return m.end ( );
	-- i;
	if ( i == m.end ( ) )
		return i;
	if ( ! s.compare ( 0, i -> first.size ( ), i -> first ) )
		return i;
	for ( typename _String :: size_type j = 0; j < i -> first.size ( ) && j < s.size ( ); j ++ ) {
		if ( s [ j ] != i -> first [ j ] )
			return findPrefix ( m, s.substr ( 0, j ) );
	}
	return m.end ( );
}
#endif
