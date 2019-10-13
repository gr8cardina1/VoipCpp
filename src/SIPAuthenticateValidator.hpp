#ifndef _SIPAUTHENTICATEVALIDATOR_HPP_
#define _SIPAUTHENTICATEVALIDATOR_HPP_
#pragma interface

#define HASHLEN 16
#define HASHHEXLEN 32
typedef char HASH [ HASHLEN ];
typedef char HASHHEX [ HASHHEXLEN + 1 ];

class SIPAuthenticateValidator {
public:
	SIPAuthenticateValidator ( );
	virtual ~SIPAuthenticateValidator ( );

	ss :: string getLogin ( const ss :: string & line ) const;
	ss :: string getRealm ( const ss :: string & line ) const;
	ss :: string getNonce ( const ss :: string & line ) const;
	ss :: string getOpaque ( const ss :: string & line ) const;
	ss :: string getAlgorithm ( const ss :: string & line ) const;
	bool isValid ( const ss :: string & authLine,
		const ss :: string & method, const ss :: string & password, bool isUseCNonce );
	void composeRequest ( ss :: string & line );
	ss :: string buildResponse ( const ss :: string & line, const ss :: string & username, const ss :: string & pswd,
		const ss :: string & uri, const ss :: string & method, bool isUseCNonce );
protected:
	ss :: string getItem ( const ss :: string & line, const ss :: string & item, int pos = 0 ) const;
	void CvtHex ( HASH Bin, HASHHEX & Hex );
	void DigestCalcHA1 ( const ss :: string & alg, const ss :: string & login, const ss :: string & realm,
		const ss :: string & password, const ss :: string & nonce,
		const ss :: string & cnonce, HASHHEX & SessionKey, bool isUseCNonce );
	void DigestCalcResponse( HASHHEX HA1, const ss :: string & nonce, const ss :: string & nc,
		const ss :: string & cnonce, const ss :: string & qop, const ss :: string & method,
		const ss :: string & uri, HASHHEX HEntity, HASHHEX & Response, bool isUseCNonce);
	ss :: string _realm, _qop, _algorithm, _nonce, _opaque, _secret;
	ss :: string _generateNonce ( long v );
	long _nonceTTL;

};



#endif //_SIPAUTHENTICATEVALIDATOR_HPP_
