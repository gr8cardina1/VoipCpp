#ifndef _CALLTHREAD_H
#define _CALLTHREAD_H
#pragma interface

// This thread handles the dispatching of connections to new signalling threads
class CallThread : public PThread {
	PCLASSINFO ( CallThread, PThread )
public:
	CallThread ( );

	~CallThread ( );

	void Close ( );

protected:
	void Main ( );
	PTCPSocket callSocket;
};

int getNewCallId ( );

#endif // _CALLTHREAD_H
