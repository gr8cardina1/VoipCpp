#ifndef __H245THREAD_HPP
#define __H245THREAD_HPP
#pragma interface

namespace H225 {
	class TransportAddress;
}
class H245Handler;
namespace H245 {
	class MultimediaSystemControlMessage;
}
class CallDetails;
class H245Thread : public PThread, public Allocatable < __SS_ALLOCATOR > {
	PCLASSINFO ( H245Thread, PThread );
public:
	H245Thread ( const H225 :: TransportAddress & calledAddress, H245Handler * _h, int ref, bool fd, bool bc,
		bool l2 = false );
	~H245Thread ( );

	WORD getListenerPort ( );
	WORD getListenerPort2 ( );
	void abortListen ( const PIPSocket :: Address & addr, WORD port );
	// Task: to return the port of the listener socket to give to the caller.
	void Close ( );
	// Task: to stop processing of the H245 connection and close it
	bool getBC ( );
protected:
	void Main ( );
	void receiveMesg ( PTCPSocket * from, PTCPSocket * to );
	// Task: to receive an H.245 message and handle it as appropriate
	void processMesg ( H245 :: MultimediaSystemControlMessage & mesg, bool fromCaller,
		PTCPSocket * target, PTCPSocket * target2 );
	// Task: to handle the given H.245 message
	PTCPSocket listener;
	PTCPSocket listener2;
	PTCPSocket * callerSocket;
	PTCPSocket * calledSocket;
	PIPSocket :: Address calledAddr;
	bool sessionEnded;
	H245Handler * h;
	bool forceDebug;
private:
	bool connectCaller ( );
	bool connectionEstablished ( );
	PIPSocket :: Address origAddr;
	WORD origPort;
	PMutex mut;
	bool byCaller;
	bool listening;
	bool needConnect;
	PIPSocket :: Address connectAddr;
	WORD connectPort;
};
#endif
