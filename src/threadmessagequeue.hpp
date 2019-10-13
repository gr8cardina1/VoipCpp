#ifndef THREADMESSAGEQUEUE_HPP_
#define THREADMESSAGEQUEUE_HPP_
#pragma interface

template < typename T > class ThreadMessageQueue : public Allocatable < __SS_ALLOCATOR >, boost :: noncopyable {
	PMutex mut;
	PCondVar v;
	std :: queue < T, std :: deque < T, __SS_ALLOCATOR < T > > > q;
public:
	ThreadMessageQueue ( ) : v ( mut ) { }
	void put ( const T & p ) {
		AutoMutex am ( mut );
		q.push ( p );
		v.Signal ( );
	}
	void wake ( ) {
		v.Signal ( );
	}
	bool get ( T & p, int s = 3000 ) {
		AutoMutex am ( mut );
		if ( q.empty ( ) && ( ! s || ! v.Wait ( s ) || q.empty ( ) ) )
			return false;
		p = q.front ( );
		q.pop ( );
		return true;
	}
	bool getNoWait ( T & p ) {
		return get ( p, 0 );
	}
};

#endif /*THREADMESSAGEQUEUE_HPP_*/
