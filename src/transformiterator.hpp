#ifndef TRANSFORMITERATOR_HPP_
#define TRANSFORMITERATOR_HPP_
#pragma interface

template < class _F, class _I > boost :: transform_iterator < _F, _I > make_transform_iterator ( const _F & f,
	const _I & i ) {
	return boost :: transform_iterator < _F, _I > ( i, f );
}

#endif /*TRANSFORMITERATOR_HPP_*/
