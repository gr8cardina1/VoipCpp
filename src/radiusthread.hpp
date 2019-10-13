#ifndef __RADIUSTHREAD_HPP
#define __RADIUSTHREAD_HPP
#pragma interface

struct RequestInfo;

namespace Radius {
	class Request;
}

class RadGWInfo;

class RadiusThread : public PThread, public Allocatable < __SS_ALLOCATOR > {
	PCLASSINFO ( RadiusThread, PThread )
public:
	RadiusThread ( );
	~RadiusThread ( );
	void Close ( );
protected:
	void Main ( );
private:
	PUDPSocket authSock, accSock;
	Pointer < unsigned char > recvBuf;
	void readAuth ( );
	void readAcc ( );
	void handleAuth ( const Radius :: Request * r, const RadGWInfo & gw );
	void handleAcc ( const Radius :: Request * r, const RadGWInfo & gw );
	int handleAuthInt ( const Radius :: Request * r, RequestInfo & ri, const RadGWInfo & gw );
	void handleAccInt ( const Radius :: Request * r, Radius :: Request & answer, const RadGWInfo & gw );
	int getCreditTime ( const Radius :: Request * r, RequestInfo & ri );
};
#endif
