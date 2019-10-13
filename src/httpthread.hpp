#ifndef HTTPTHREAD_HPP_
#define HTTPTHREAD_HPP_
#pragma interface

class PTCPSocket;
class PHTTPSpace;
class HTTPThread : public PThread, public Allocatable < __SS_ALLOCATOR > {
	PCLASSINFO ( HTTPThread, PThread )
	PTCPSocket * sock;
	const PHTTPSpace & space;
public:
	HTTPThread ( PTCPSocket * s, const PHTTPSpace & sp );
	virtual ~HTTPThread ( );
protected:
	virtual void Main ( );
};

#endif /*HTTPTHREAD_HPP_*/
