#ifndef __SQLOUTHREAD_HPP
#define __SQLOUTHREAD_HPP
#pragma interface


class SQLOutThread : public PThread, public Allocatable < __SS_ALLOCATOR > {
	PCLASSINFO ( SQLOutThread, PThread )
	class Impl;
	Impl * impl;
public:
	explicit SQLOutThread ( SQLOutThread * & o );
	~SQLOutThread ( );
	void add ( const ss :: string & q );
	void Close ( );
protected:
	void Main ( );
};
extern SQLOutThread * sqlOut;
#endif
