#ifndef _ADDR_UTILS_H
#define _ADDR_UTILS_H
#pragma interface

namespace AddrUtils {
	void getIPAddress ( H225 :: TransportAddress_ipAddress & ipAddr,
		const H225 :: ArrayOf_TransportAddress & addresses );
	// Task: given an array of transport address, returns the first IP address
	//       will throw an error if no IP address

	void getIPAddress ( H225 :: TransportAddress & ipAddr,
		const H225 :: ArrayOf_TransportAddress & addresses );
	// Task: given an array of transport address, returns the first IP address
	//       will throw an error if no IP address


	class IPAddrNotFoundError : public std :: runtime_error {
		public:
		IPAddrNotFoundError ( const std :: string & what_arg ) : std :: runtime_error ( what_arg ) { }
	};

	H225 :: TransportAddress convertToH225TransportAddr ( const PIPSocket :: Address & addr,
		WORD port );
	// Task: to convert an IP address into an H225 transport address

	void convertToIPAddress ( const H225 :: TransportAddress_ipAddress & h225ip,
		PIPSocket :: Address & addr, WORD & port );
	// Task: to convert an IP address in H225 format to PWLIB format
}


#endif // _ADDR_UTILS_H

