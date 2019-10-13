#ifndef _BASEGKCLIENTTHREAD_HPP_
#define _BASEGKCLIENTTHREAD_HPP_
#pragma interface

class BaseGkClientThread : public PThread, public Allocatable < __SS_ALLOCATOR > {
	PCLASSINFO ( BaseGkClientThread, PThread )
public:
	BaseGkClientThread ( );
	virtual void Close ( ) = 0;
	virtual bool getAddr ( const ss :: string & digits, PIPSocket :: Address & destAddr, int & destPort ) = 0;
	virtual void updateState ( const ss :: string & n, const ss :: string & ln, int rs ) = 0;
};

#endif //_GKCLIENTTHREAD_HPP_
