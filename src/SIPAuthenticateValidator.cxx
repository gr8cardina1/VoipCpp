#pragma implementation
#include "ss.hpp"
#include "SIPAuthenticateValidator.hpp"
#include <ptlib.h>
#include <ptlib/svcproc.h>
#include <tr1/cstdint>
#include "md5.hpp"
#include <cstring>

SIPAuthenticateValidator :: SIPAuthenticateValidator ( ) {
	_qop = "auth";
	_realm = "SIParea";
	_algorithm = "MD5";
	_nonce = "";
	_opaque = "";
	_secret = "SecretKey";
	_nonceTTL = 10;
}

SIPAuthenticateValidator :: ~SIPAuthenticateValidator ( ) { }

void SIPAuthenticateValidator :: CvtHex( HASH Bin, HASHHEX & Hex ) {
	for ( unsigned short i = 0; i < HASHLEN; i++ ) {
		unsigned char j = char ( ( Bin[i] >> 4 ) & 0xf );
		if (j <= 9) Hex[i*2] = char (j + '0');
		else Hex[i*2] = char (j + 'a' - 10);
		j = char ( Bin[i] & 0xf );
		if (j <= 9) Hex[i*2+1] = char (j + '0');
		else Hex[i*2+1] = char (j + 'a' - 10);
	}
	Hex[HASHHEXLEN] = '\0';
}

/* calculate H(A1) as per spec */
void SIPAuthenticateValidator :: DigestCalcHA1 ( const ss :: string & alg, const ss :: string & login,
		const ss :: string & realm, const ss :: string & password, const ss :: string & nonce,
		const ss :: string & cnonce, HASHHEX & SessionKey, bool isUseCNonce ) {
	MD5 Md5Ctx;
	HASH HA1;

	Md5Ctx.processBytes ( login.data ( ), login.size ( ) );
	Md5Ctx.processBytes ( ":", 1 );
	Md5Ctx.processBytes ( realm.data ( ), realm.size ( ) );
	Md5Ctx.processBytes ( ":", 1 );
	Md5Ctx.processBytes ( password.data ( ), password.size ( ) );
	std :: memcpy ( HA1, Md5Ctx.finish ( ).data ( ), 16 );
 	if ( strcasecmp ( alg.c_str ( ), "md5-sess" ) == 0) {
		Md5Ctx.clear ( );
		Md5Ctx.processBytes ( HA1, HASHLEN );
		Md5Ctx.processBytes ( ":", 1 );
		Md5Ctx.processBytes ( nonce.data ( ), nonce.size ( ) );
		if ( isUseCNonce ) {
			Md5Ctx.processBytes ( ":", 1 );
			Md5Ctx.processBytes ( cnonce.data ( ), cnonce.size ( ) );
		}
		std :: memcpy ( HA1, Md5Ctx.finish ( ).data ( ), 16 );
	}
	CvtHex( HA1, SessionKey );
}

/* calculate request-digest/response-digest as per HTTP Digest spec */
void SIPAuthenticateValidator :: DigestCalcResponse( HASHHEX HA1, const ss :: string & nonce,
		const ss :: string & nc, const ss :: string & cnonce,      /* client nonce */
		const ss :: string & qop, const ss :: string & method, const ss :: string & uri,
		HASHHEX HEntity, HASHHEX & Response,
		bool isUseCNonce )
{
	MD5 Md5Ctx;
	HASH HA2;
	HASH RespHash;
	HASHHEX HA2Hex;

	// calculate H(A2)
	Md5Ctx.processBytes ( method.data ( ), method.size ( ) );
	Md5Ctx.processBytes ( ":", 1 );
	Md5Ctx.processBytes ( uri.data ( ), uri.size ( ) );
	if ( strcasecmp ( qop.c_str ( ), "auth-int" ) == 0 ) {
		Md5Ctx.processBytes ( ":", 1 );
		Md5Ctx.processBytes ( HEntity, HASHHEXLEN );
	};
	std :: memcpy ( HA2, Md5Ctx.finish ( ).data ( ), 16 );
	CvtHex ( HA2, HA2Hex );

	// calculate response
	Md5Ctx.clear ( );
	Md5Ctx.processBytes ( HA1, HASHHEXLEN );
	Md5Ctx.processBytes ( ":", 1 );
	Md5Ctx.processBytes ( nonce.data ( ), nonce.size ( ) );
	Md5Ctx.processBytes ( ":", 1 );
	if ( ! qop.empty ( ) ) {
		Md5Ctx.processBytes ( nc.data ( ), nc.size ( ) );
		if ( isUseCNonce ) {
			Md5Ctx.processBytes ( ":", 1 );
			Md5Ctx.processBytes ( cnonce.data ( ), cnonce.size ( ) );
		}
		Md5Ctx.processBytes ( ":", 1 );
		Md5Ctx.processBytes ( qop.data ( ), qop.size ( ) );
		Md5Ctx.processBytes ( ":", 1 );
	};
	Md5Ctx.processBytes ( HA2Hex, HASHHEXLEN );
	std :: memcpy ( RespHash, Md5Ctx.finish ( ).data ( ), 16 );
	CvtHex ( RespHash, Response );
}

ss :: string SIPAuthenticateValidator :: getItem ( const ss :: string & auth, const ss :: string & item, int pos ) const {
	ss :: string ::size_type startpos = auth.find ( item, pos );
	if ( startpos == ss :: string :: npos )
		return "";
	startpos += item.size ( );
	ss :: string ::size_type endpos;
	if ( auth.size ( ) > startpos && auth [ startpos ] == '\"' ) {
		startpos ++;
		endpos = auth.find ( '\"', startpos );
		if ( endpos == ss :: string :: npos )
			return ss :: string ( );
	} else {
		endpos = auth.find ( ',', startpos );
	}
	return auth.substr ( startpos, endpos - startpos );
}

ss :: string SIPAuthenticateValidator :: getLogin ( const ss :: string & line ) const {
	return getItem ( line, "username=" );
}

ss :: string SIPAuthenticateValidator :: getRealm ( const ss :: string & line ) const {
	return getItem ( line, "realm=" );
}

ss :: string SIPAuthenticateValidator :: getOpaque ( const ss :: string & line ) const {
	return getItem ( line, "opaque=" );
}

ss :: string SIPAuthenticateValidator :: getAlgorithm ( const ss :: string & line ) const {
	return getItem ( line, "algorithim=" );
}

bool SIPAuthenticateValidator :: isValid ( const ss :: string & line, const ss :: string & method,
										const ss :: string & password, bool isUseCNonce  ) {
	ss :: string pswd = password;
	ss :: string login = getLogin ( line );
	ss :: string realm = getItem ( line, "realm=" );
	ss :: string uri = getItem ( line, "uri=" );
	ss :: string nonce = getNonce ( line );
	long tt = PTime ( ).GetTimeInSeconds ( );
	// request must not be older then _nonceTTL seconds
	bool nonceOk = false;
	for ( int i = 0; i < _nonceTTL; i++ )  {
		if ( nonce == _generateNonce ( tt - i ) ) {
			nonceOk = true;
			break;
		}
	}
	if ( ! nonceOk ) {
		PSYSTEMLOG ( Info, "SIPValidator : nonce too old! (" << nonce << ")" );
		return false;
	}
	ss :: string opaque = getItem ( line, "opaque=" );
	if ( opaque != _opaque ) {
		PSYSTEMLOG ( Info, "SIPValidator : opaque missmatch! " << opaque << "/" << _opaque );
		return false;
	}
	ss :: string  qop = getItem ( line, "qop=" );
	ss :: string  nc = getItem ( line, "nc=" );
	ss :: string response = getItem ( line, "response=" );
	ss :: string cnonce = getItem ( line, "cnonce=" );
	HASHHEX HA1;
	HASHHEX HA2 = "";
	HASHHEX Response;
	ss :: string alg = "md5";

	DigestCalcHA1 ( alg, login, realm, pswd, nonce, cnonce, HA1, isUseCNonce );
	DigestCalcResponse ( HA1, nonce, nc, cnonce, qop, method, uri, HA2, Response, isUseCNonce );
	if ( memcmp ( Response, response.data ( ), 16 ) ) {
		PSYSTEMLOG ( Error, "SIPVailidator : response failed for " << login );
		return false;
	}
	return true;
}

void SIPAuthenticateValidator :: composeRequest ( ss :: string & line ) {
	_nonce = _generateNonce ( PTime ( ).GetTimeInSeconds ( ) );
	line = line + "Digest realm=\"" + _realm + "\",qop=\"" + _qop + "\", nonce=\"" +
		_nonce + "\",opaque=\"" + _opaque + "\",algorithm=" + _algorithm;
}

ss :: string SIPAuthenticateValidator :: _generateNonce ( long v ) {
	// generating NONCE
	ss :: ostringstream os;
	os << v << ':' << _secret;
	HASH binCoded;
	HASHHEX charCoded;
	std :: memcpy ( binCoded, md5Sum ( os.str ( ) ).data ( ), 16 );
	CvtHex ( binCoded, charCoded );
	return charCoded;
}

ss :: string SIPAuthenticateValidator :: buildResponse ( const ss :: string & line, const ss :: string & username, const ss :: string & pswd,
		const ss :: string & uri, const ss :: string & method, bool isUseCNonce ) {
	ss :: string realm = getItem ( line, "realm=" );
	ss :: string nonce = getNonce ( line );
	ss :: string opaque = getItem ( line, "opaque=" );
	ss :: string  qop = getItem ( line, "qop=" );
	ss :: string  nc = getItem ( line, "nc=" );
	ss :: string cnonce = getItem ( line, "cnonce=" );
	HASHHEX HA1;
	HASHHEX HA2 = "";
	HASHHEX Response;
	ss :: string alg = "MD5";
	if ( nc == "" )
		nc = "00000001";

	DigestCalcHA1 ( alg, username, realm, pswd, nonce, cnonce, HA1, isUseCNonce );
	DigestCalcResponse ( HA1, nonce, nc, cnonce, qop, method, uri, HA2, Response, isUseCNonce );

	ss :: string authLine = "Digest username=\"" + username + "\",realm=\"" + realm + "\",nonce=\"" +
		nonce + "\",nc=\"" + nc + "\",uri=\"" + uri + "\"" + ",response=\"" + Response +
		"\"";
	if ( opaque != "" )
		authLine+= ",opaque=\"" + opaque + "\"";
	if ( qop != "" )
		authLine+= ",qop=\"" + qop + "\"";
	return authLine;
}

ss :: string SIPAuthenticateValidator :: getNonce ( const ss :: string & line ) const {
	ss :: string :: size_type startpos = line.find ( "nonce=" );
	if ( startpos == ss :: string :: npos )
		return "";
	if ( ( startpos == 0 ) || ( line [ startpos - 1 ] == ' ' || line [ startpos - 1 ] == ',' ) )
		return getItem ( line, "nonce=" );
	else
		return getItem ( line, "nonce=", int ( startpos + 1 ) );
}
