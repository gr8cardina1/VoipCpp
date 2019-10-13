#ifndef __FINDMAPELEMENT_HPP
#define __FINDMAPELEMENT_HPP
#pragma interface

template < class _Map > typename _Map :: mapped_type * findMapElement ( _Map & m, const typename _Map :: key_type & e ) {
	typename _Map :: iterator i = m.find ( e );
	if ( i == m.end ( ) )
		return 0;
	return & i -> second;
}

template < class _Map > const typename _Map :: mapped_type * findMapElement ( const _Map & m,
	const typename _Map :: key_type & e ) {
	typename _Map :: const_iterator i = m.find ( e );
	if ( i == m.end ( ) )
		return 0;
	return & i -> second;
}

template < class _Map > typename _Map :: mapped_type :: mapped_type * findMapElement ( _Map & m,
	const typename _Map :: key_type & e1, const typename _Map :: mapped_type :: key_type & e2 ) {
	if ( typename _Map :: mapped_type * m1 = findMapElement ( m, e1 ) )
		return findMapElement ( * m1, e2 );
	return 0;
}

template < class _Map > const typename _Map :: mapped_type :: mapped_type * findMapElement ( const _Map & m,
	const typename _Map :: key_type & e1, const typename _Map :: mapped_type :: key_type & e2 ) {
	if ( const typename _Map :: mapped_type * m1 = findMapElement ( m, e1 ) )
		return findMapElement ( * m1, e2 );
	return 0;
}

template < class _Map > bool eraseMapElement ( _Map & m, const typename _Map :: key_type & e1,
	const typename _Map :: mapped_type :: key_type & e2 ) {
	typename _Map :: iterator i = m.find ( e1 );
	if ( i == m.end ( ) )
		return false;
	bool r = i -> second.erase ( e2 );
	if ( i -> second.empty ( ) )
		m.erase ( i );
	return r;
}

#endif
