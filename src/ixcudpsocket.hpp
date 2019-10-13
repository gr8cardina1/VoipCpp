#ifndef _IXCUDPSOCKET_HPP_
#define _IXCUDPSOCKET_HPP_
#pragma interface

//the special kind of PUDPSocket class with ability
//to detect the interface address where datagram was recieved
class IxcUDPSocket : public PUDPSocket, public Allocatable < __SS_ALLOCATOR > {
	PCLASSINFO(IxcUDPSocket, PUDPSocket)
public:
		// class constructors (same as PUDPSocket has)
		IxcUDPSocket(
		  WORD port = 0             /// Port number to use for the connection.
		);
		IxcUDPSocket(
		  const PString & service   /// Service name to use for the connection.
		);
		IxcUDPSocket(
		  const PString & address,  /// Address of remote machine to connect to.
		  WORD port                 /// Port number to use for the connection.
		);
		IxcUDPSocket(
		  const PString & address,  /// Address of remote machine to connect to.
		  const PString & service   /// Service name to use for the connection.
		);

		//function redefines the PUDPSocket one to recieve the interface IP address also
		BOOL Read(
		  void * buf,   /// Pointer to a block of memory to read.
		  PINDEX len    /// Number of bytes to read.
		);
#if 0
        //just test funcion
        BOOL Write(const void* buf, PINDEX len);
#endif  //0

		//function returns the interface address
		void GetInterfaceAddress(
			Address& addrInterface		//interface IP
			);

private:
        //private utility function. Called by constructors to set IP_RECVDSTADDR option
        void    SetSocketOption(void);

        //interface address where datagram was received
		Address	addrInterface;
};

#endif //_IXCUDPSOCKET_HPP_
