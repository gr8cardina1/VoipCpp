#ifndef __CALLBUFFER_HPP
#define __CALLBUFFER_HPP
#pragma interface

class CallBuffer : public Allocatable < __SS_ALLOCATOR > {
	int secs;
	IntSet codes;
	struct Call {
		ss :: string digits;
		PTime when;
		int code;
		Call ( const ss :: string d, int c ) : digits ( d ), when ( ), code ( c ) { }
	};
	typedef boost :: multi_index :: multi_index_container < Call,
		boost :: multi_index :: indexed_by < boost :: multi_index :: sequenced < >,
		boost :: multi_index :: ordered_unique < boost :: multi_index :: tag < Call >,
		boost :: multi_index :: member < Call, ss :: string, & Call :: digits > > >,
		__SS_ALLOCATOR < Call > > CallVector;
	CallVector calls;
public:
	CallBuffer ( int s, const IntSet & cs );
	int getCode ( const ss :: string & digits );
	void addCall ( const ss :: string & digits, int code );
};

#endif
