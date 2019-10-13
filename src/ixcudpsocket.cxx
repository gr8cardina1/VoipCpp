#pragma implementation
#include "ss.hpp"
#include "allocatable.hpp"
#include <ptlib.h>
#include <ptlib/sockets.h>
#include "ixcudpsocket.hpp"
#include <sys/uio.h>

#include <ptlib/svcproc.h>


///////////////////////////////////////////////////////////////
// class IxcUDPSocket
///////////////////////////////////////////////////////////////
//
// Constructors
IxcUDPSocket::IxcUDPSocket(WORD port)
: PUDPSocket(port), addrInterface(0)
{
    SetSocketOption();
}
///////////////////////////////////////////////////////////////
IxcUDPSocket::IxcUDPSocket(const PString& address, WORD port)
: PUDPSocket(address, port), addrInterface(0)
{
    SetSocketOption();
}
///////////////////////////////////////////////////////////////
IxcUDPSocket::IxcUDPSocket(const PString &address, const PString& service)
: PUDPSocket(address, service), addrInterface(0)
{
    SetSocketOption();
}
///////////////////////////////////////////////////////////////
IxcUDPSocket::IxcUDPSocket(const PString& service)
: PUDPSocket(service), addrInterface(0)
{
    SetSocketOption();
}
///////////////////////////////////////////////////////////////
// function returns an interface IP address
void IxcUDPSocket::GetInterfaceAddress(Address& interface)
{
	interface = addrInterface;
}
///////////////////////////////////////////////////////////////
// function sets the UDP socket options
void IxcUDPSocket :: SetSocketOption ( ) {
#ifdef linux
	SetOption(IP_PKTINFO, 1, IPPROTO_IP);
#else
	SetOption(IP_RECVDSTADDR, 1, IPPROTO_IP);
#endif
}
///////////////////////////////////////////////////////////////
// function reads from the socket


//////////////////

BOOL IxcUDPSocket :: Read ( void * buf, PINDEX len ) {
	lastReadCount = 0;
	if ( ! PXSetIOBlock ( PXReadBlock, readTimeout ) )
		return FALSE;
	//try to read the data
	struct msghdr msg;
	struct iovec iov [ 1 ];
	sockaddr_in sockAddr;
	PINDEX addrLen = sizeof ( sockAddr );
	union {
		struct cmsghdr  cm;
#ifdef linux
		char control [ CMSG_SPACE ( sizeof ( in_pktinfo ) ) ];
#else
		char control [ CMSG_SPACE ( sizeof ( in_addr ) ) ];
#endif
	} control_un;

	msg.msg_control = control_un.control;
	msg.msg_controllen = sizeof ( control_un );
	msg.msg_flags = 0;

	msg.msg_name = ( sockaddr * ) & sockAddr;
	msg.msg_namelen = ( socklen_t ) addrLen;
	iov [ 0 ].iov_base = ( char * ) buf;
	iov [ 0 ].iov_len = len;
	msg.msg_iov = iov;
	msg.msg_iovlen = 1;

	int flags = MSG_WAITALL | MSG_EOR;
	ssize_t r = :: recvmsg ( os_handle, & msg, flags );

	if ( ! ConvertOSError ( int ( r ), LastReadError ) )
		return FALSE;

	lastReadCount = PINDEX ( r );
	lastReceiveAddress = sockAddr.sin_addr;
	lastReceivePort = ntohs ( sockAddr.sin_port );

	//if control info is truncated we cannot expect a valid result
	if ( msg.msg_controllen < sizeof ( cmsghdr ) || ( msg.msg_flags & MSG_CTRUNC ) ) {
		PSYSTEMLOG (Info, "UDPSocket :: Read. LastReadError = " << LastReadError << "; msg.msg_controllen = "
			<< msg.msg_controllen << "; sizeof ( cmsghdr ) = " << (int)sizeof ( cmsghdr ));
		return lastReadCount > 0;
	}
	//read the control info
	struct cmsghdr * cmptr;
	for ( cmptr = CMSG_FIRSTHDR ( & msg ); cmptr; cmptr = CMSG_NXTHDR ( & msg, cmptr ) ) {
		if ( cmptr -> cmsg_level != IPPROTO_IP )
			continue;
#ifdef linux
		if ( cmptr -> cmsg_type == IP_PKTINFO ) {
			addrInterface = reinterpret_cast < in_pktinfo * > ( CMSG_DATA ( cmptr ) ) -> ipi_addr;
#else
		if ( cmptr -> cmsg_type == IP_RECVDSTADDR ) {
			addrInterface = * reinterpret_cast < in_addr * > ( CMSG_DATA ( cmptr ) );
#endif
			break;
		}
	}

	return lastReadCount > 0;
}

//just for testing
#if 0
static void DumpSockOptions(int os_handle)
{
    int val = 0;
    int val_len = sizeof(val);
    const char* szState = NULL;

    if(getsockopt(os_handle, SOL_SOCKET, SO_REUSEADDR, &val, (socklen_t*)(&val_len)) < 0)
    {
        szState = "Error !!!";
    }
    else
    {
        szState = (val > 0) ? "On " : "Off ";
    }
    PSYSTEMLOG(Info, "SO_REUSEADDR state " << szState << val);
}
//////////////////////////////////
// just for test
BOOL IxcUDPSocket::Write(const void* buf, PINDEX len)
{
//    DumpSockOptions(os_handle);

    return PUDPSocket::Write(buf, len);
}
#endif // 0
