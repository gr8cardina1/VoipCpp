#ifndef HTTPLISTENTHREAD_HPP_
#define HTTPLISTENTHREAD_HPP_
#pragma interface

class PTCPSocket;

class HTTPListenThread : public PThread, public Allocatable < __SS_ALLOCATOR > {
	PCLASSINFO ( HTTPListenThread, PThread )
	PTCPSocket * sock;
public:
	HTTPListenThread ( );
	~HTTPListenThread ( );
	void Close ( );
private:
	virtual void Main ( );
};

#endif /*HTTPLISTENTHREAD_HPP_*/
