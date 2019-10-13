#ifndef __MD5_HPP
#define __MD5_HPP

class MD5 {
	std :: tr1 :: uint32_t m_A, m_B, m_C, m_D;
	std :: tr1 :: uint64_t total;
	std :: tr1 :: uint32_t buflen;
	char buffer [ 128 ] __attribute__ (( __aligned__ ( __alignof__ ( std :: tr1 :: uint64_t ) ) ));
	void processBlock ( const void * s, std :: size_t len );
public:
	MD5 ( ) : m_A ( 0x67452301 ), m_B ( 0xefcdab89 ), m_C ( 0x98badcfe ), m_D ( 0x10325476 ), total ( 0 ), buflen ( 0 ) { }
	void processBytes ( const void * s, std :: size_t len );
	ss :: string str ( ) const {
		return ss :: string ( reinterpret_cast < const char * > ( & m_A ), 16 );
	}
	void clear ( ) {
		m_A = 0x67452301;
		m_B = 0xefcdab89;
		m_C = 0x98badcfe;
		m_D = 0x10325476;
		total = 0;
		buflen = 0;
	}
	ss :: string finish ( );
};

ss :: string md5Sum ( const void * s, std :: size_t len );
ss :: string md5Sum ( const ss :: string & s );

#endif
