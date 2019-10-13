#ifndef __SETTHREEWAY_HPP
#define __SETTHREEWAY_HPP
#pragma interface

template < typename _InputIter1, typename _InputIter2, typename _OutputIterd1, typename _OutputIteri,
	typename _OutputIterd2 > void set_threeway ( _InputIter1 __first1, _InputIter1 __last1, _InputIter2 __first2,
		_InputIter2 __last2, _OutputIterd1 __resultd1, _OutputIteri __resulti, _OutputIterd2 __resultd2 ) {
	// concept requirements
	__glibcxx_function_requires ( _InputIteratorConcept < _InputIter1 > )
	__glibcxx_function_requires ( _InputIteratorConcept < _InputIter2 > )
	__glibcxx_function_requires ( _OutputIteratorConcept < _OutputIterd1,
		typename iterator_traits < _InputIter1 > :: value_type > )
	__glibcxx_function_requires ( _OutputIteratorConcept < _OutputIteri,
		typename iterator_traits < _InputIter1 > :: value_type > )
	__glibcxx_function_requires ( _OutputIteratorConcept < _OutputIterd2,
		typename iterator_traits < _InputIter1 > :: value_type > )
	__glibcxx_function_requires ( _SameTypeConcept < typename iterator_traits < _InputIter1> :: value_type,
		typename iterator_traits < _InputIter2 > :: value_type > )
	__glibcxx_function_requires ( _LessThanComparableConcept < typename iterator_traits < _InputIter1 > :: value_type > )
	__glibcxx_requires_valid_range ( __first1, __last1 );
	__glibcxx_requires_valid_range ( __first2, __last2 );

	while ( __first1 != __last1 && __first2 != __last2 )
		if ( * __first1 < * __first2 ) {
			* __resultd1 = * __first1;
			++ __first1;
			++ __resultd1;
		} else if ( * __first2 < * __first1 ) {
			* __resultd2 = * __first2;
			++ __resultd2;
			++ __first2;
		} else {
			* __resulti = * __first1;
			++ __resulti;
			++ __first1;
			++ __first2;
		}
	std :: copy ( __first1, __last1, __resultd1 );
	std :: copy ( __first2, __last2, __resultd2 );
}

#endif
