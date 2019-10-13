#pragma implementation
#include <ptlib.h>
#include <ptlib/sockets.h>
#include <ptlib/svcproc.h>
#include "ss.hpp"
#include "allocatable.hpp"
#include "aftertask.hpp"
#include "ipport.hpp"
#include "icqcontact.hpp"
#include "radgwinfo.hpp"
#include "Conf.hpp"
#include "pointer.hpp"
#include <iomanip>
#include <stdexcept>
#include <limits>
#include <cstring>
#include "asn.hpp"
#include "h225.hpp"
#include "h323.hpp"
#include "rasthread.hpp"
#include "AddrUtils.h"
#include "Log.h"
#include "automutex.hpp"
#include <tr1/cstdint>
#include "md5.hpp"
#include <fstream>
#include "ssconf.hpp"

static const int bufferSize = 4096;	// Size of network buffers
const char h225protocolID [ ] = "0.0.8.2250.0.4";
RasThread * rasThread = 0;

RasThread :: RasThread ( const WORD port ) : PThread ( 1000, NoAutoDeleteThread, NormalPriority,
	"RasThread" ), rasPort ( port ) {
	rasThread = this;
	Resume ( );
}

RasThread :: ~RasThread ( ) { }

static void storeInvalidMessage ( const char * buf, std :: size_t size ) {
	struct timeval tv;
	gettimeofday ( & tv, 0 );
	ss :: ostringstream fname;
	fname << "/tmp/invalid." << tv.tv_sec << '.' << tv.tv_usec;
	std :: ofstream os ( fname.str ( ).c_str ( ) );
	os.write ( buf, size );
}

bool RasThread :: readRasReq ( PUDPSocket & sock ) {
// Task: to read a RAS message from the given socket
	char buffer [ bufferSize ];
	PIPSocket :: Address senderAddr;
	WORD senderPort;
	if ( ! sock.ReadFrom ( buffer, bufferSize, senderAddr, senderPort ) ) {
		PSYSTEMLOG ( Error, "can't read RAS packet: " << sock.GetErrorText ( PChannel :: LastReadError ) );
		return false;
	}
	H225 :: TransportAddress replyTo = AddrUtils :: convertToH225TransportAddr ( senderAddr, senderPort );
	try {
		Asn :: istream is ( ss :: string ( buffer, sock.GetLastReadCount ( ) ) );
		H225 :: RasMessage mesg ( is );
		if ( is.hasException ( ) ) {
			storeInvalidMessage ( buffer, sock.GetLastReadCount ( ) );
			PSYSTEMLOG ( Error, "readRasReq: can't decode message: " << is.getException ( ) );
		}
		Log -> logRasMsg ( mesg, OpengateLog :: Receiving, replyTo );
		H225 :: RasMessage reply;
		if ( handleRasRequest ( mesg, reply, replyTo, sock ) )
			return writeRasReply ( sock, replyTo, reply );
	} catch ( std :: exception & e ) {
		storeInvalidMessage ( buffer, sock.GetLastReadCount ( ) );
		PSYSTEMLOG ( Error, "readRasReq: can't decode message: " << e.what ( ) );
		return false;
	}
	return true;
}

bool RasThread :: writeRasReply ( PUDPSocket & sock, const H225 :: TransportAddress & replyTo,
	const H225 :: RasMessage & mesg ) {
	// Task: to write a RAS message to the given destination
	Log -> logRasMsg ( mesg, OpengateLog :: Sending, replyTo );
	if ( replyTo.getTag ( ) != H225 :: TransportAddress :: e_ipAddress ) {
		PSYSTEMLOG ( Error, "No IP address to reply to!" );
		return false;
	}
	const H225 :: TransportAddress_ipAddress & replyToIP = replyTo;
	PIPSocket :: Address replyToAddr;
	WORD replyToPort;
	AddrUtils :: convertToIPAddress ( replyToIP, replyToAddr, replyToPort );
	Asn :: ostream os;
	mesg.encode ( os );
	AutoMutex am ( mut );
	sock.SetSendAddress ( replyToAddr, replyToPort );
	return sock.Write ( os.str ( ).data ( ), PINDEX ( os.str ( ).size ( ) ) );
}
static bool allOpened ( const vector < Pointer < PUDPSocket > > & socks ) {
	if ( socks.empty ( ) )
		return false;
	for ( unsigned i = 0; i < socks.size ( ); i ++ )
		if ( ! socks [ i ] -> IsOpen ( ) )
			return false;
	return true;
}
void RasThread :: Main ( ) {
	PSYSTEMLOG ( Info, "RasThread::Main()" );
	IntVector ips = conf -> getLocalIntIps ( );
	for ( unsigned i = 0; i < ips.size ( ); i ++ ) {
		Pointer < PUDPSocket > s = new PUDPSocket ( rasPort );
		if ( ! s -> Listen ( PIPSocket :: Address ( ips [ i ] ) ) ) {
			PSYSTEMLOG ( Error, "Listen failed on ras signalling socket on " <<
				PIPSocket :: Address ( ips [ i ] ).AsString ( ) << " : " << s -> GetErrorText ( ) );
			return;
		}
		PSYSTEMLOG ( Info, "Listen succeded on ras signalling socket on " << PIPSocket :: Address ( ips [ i ] ).AsString ( ) );
		rasSockets.push_back ( s );
	}
	while ( allOpened ( rasSockets ) ) {
		PSocket :: SelectList l;
		for ( unsigned i = 0; i < rasSockets.size ( ); i ++ )
			l += * rasSockets [ i ];
		PSocket :: Select ( l, 1000 );
		for ( int i = 0; i < l.GetSize ( ); i ++ ) {
			if ( ! readRasReq ( static_cast < PUDPSocket & > ( l [ i ] ) ) )
				PSYSTEMLOG ( Info, "Failed to read RAS mesg" );
		}
	}
	PSYSTEMLOG ( Info, "RasThread::Main() ended" );
}
void RasThread :: Close ( ) {
	for ( unsigned i = 0; i < rasSockets.size ( ); i ++ )
		rasSockets [ i ] -> Close ( );
}
static bool doGCF ( const H225 :: GatekeeperRequest & gkReq, H225 :: RasMessage & reply,
	const H225 :: TransportAddress & gkAddr ) {
// Task: to confirm the given gatekeeper request
	reply.setTag ( H225 :: RasMessage :: e_gatekeeperConfirm );
	H225 :: GatekeeperConfirm & gkConf = reply;
	gkConf.m_requestSeqNum = gkReq.m_requestSeqNum;
	gkConf.m_protocolIdentifier = h225protocolID;
	gkConf.m_rasAddress = gkAddr;
	return true;
}
static bool onGRQ ( const H225 :: GatekeeperRequest & gkReq, H225 :: RasMessage & reply,
	const H225 :: TransportAddress & gkAddr ) {
	// Answers should be sent on source addresses only
	//replyTo = gkReq.m_rasAddress;
	// Check if they really want me
	return doGCF ( gkReq, reply, gkAddr );
}

static bool onGRJ ( const H225 :: GatekeeperReject & gatRej ) {
	int id = gatRej.m_requestSeqNum;
	ss :: string gkid;
	if ( gatRej.hasOptionalField ( H225 :: GatekeeperReject :: e_gatekeeperIdentifier ) )
		gkid = gatRej.get_gatekeeperIdentifier ( ).str ( );
	conf -> rejectGrq ( id, gkid, gatRej.m_rejectReason.getTag ( ) );
	return true;
}
static bool onGCF ( const H225 :: GatekeeperConfirm & gatCon ) {
	int id = gatCon.m_requestSeqNum;
	ss :: string gkid;
	if ( gatCon.hasOptionalField ( H225 :: GatekeeperConfirm :: e_gatekeeperIdentifier ) )
		gkid = gatCon.get_gatekeeperIdentifier ( ).str ( );
	conf -> confirmGrq ( id, gkid );
	return true;
}

static void getLRQReplyTo ( const H225 :: LocationRequest & locReq, H225 :: TransportAddress & replyTo ) {
// Task: to return the reply to address for the given LRQ request
	replyTo = locReq.m_replyAddress;
}
static bool doRCF ( const H225::RegistrationRequest & regReq, H225::RasMessage & reply,
	const StringSet & online, unsigned int ttl, const IcqContactsVector & icqContacts ) {
	reply.setTag ( H225 :: RasMessage :: e_registrationConfirm );
	H225 :: RegistrationConfirm & regConf = reply;
	regConf.m_requestSeqNum = regReq.m_requestSeqNum;
	regConf.m_protocolIdentifier = h225protocolID;
	regConf.m_callSignalAddress = regReq.m_callSignalAddress;
	regConf.m_endpointIdentifier = "aaa";
	if ( ! online.empty ( ) || ! icqContacts.empty ( ) ) {
		regConf.includeOptionalField ( H225 :: RegistrationConfirm :: e_tokens );
		H225 :: ArrayOf_ClearToken & tokens = regConf.get_tokens ( );
		std :: size_t nsize = online.size ( );
		tokens.setSize ( nsize + icqContacts.size ( ) );
		StringSet :: const_iterator si = online.begin ( );
		for ( std :: size_t i = 0; i < nsize; i ++, ++ si ) {
			tokens [ i ].m_tokenOID = "0.0.0.0";
			tokens [ i ].includeOptionalField ( H235 :: ClearToken :: e_challenge );
			tokens [ i ].get_challenge ( ) = '+' + * si;
		}
		for ( IcqContactsVector :: size_type i = 0, isz = icqContacts.size ( ); i < isz; i ++ ) {
			IcqContactsVector :: size_type j = i + nsize;
			tokens [ j ].m_tokenOID = "0.0.0.0";
			tokens [ j ].includeOptionalField ( H235 :: ClearToken :: e_challenge );
			Asn :: ostringstream os;
			const IcqContact & c = icqContacts [ i ];
			os << '*' << c.id << ',' << c.number << ',' << c.name;
			tokens [ j ].get_challenge ( ) = os.str ( );
		}
	}
	if ( ttl ) {
		regConf.includeOptionalField ( H225 :: RegistrationConfirm :: e_timeToLive );
		regConf.get_timeToLive ( ) = ttl;
	}
	return true;
}

static bool doRRJ ( const H225 :: RegistrationRequest & regReq, H225 :: RasMessage & reply,
	H225 :: RegistrationRejectReason :: Choices reason = H225 :: RegistrationRejectReason :: e_invalidAlias ) {
	reply.setTag ( H225 :: RasMessage :: e_registrationReject );
	H225 :: RegistrationReject & rrj = reply;
	rrj.m_requestSeqNum = regReq.m_requestSeqNum;
	rrj.m_protocolIdentifier = h225protocolID;
	rrj.m_rejectReason.setTag ( reason );
	return true;
}

static void setPlusNull ( Asn :: BMPString & b, const ss :: string & s ) {
	ss :: string t ( s );
	if ( t.size ( ) == 0 || t [ t.size ( ) - 1 ] != 0 )
		t.push_back ( 0 );
	b = t;
}

class ClearTokenValidator : public Allocatable < __SS_ALLOCATOR > {
public:
	virtual bool validate ( const H235 :: ClearToken & token, const ss :: string & login,
		const ss :: string & password ) const = 0;
	virtual ~ClearTokenValidator ( ) { }
	virtual const ss :: string & getName ( ) const = 0;
	virtual ss :: string getLogin ( const H235 :: ClearToken & token ) const = 0;
};

class CryptoTokenValidator : public Allocatable < __SS_ALLOCATOR > {
public:
	virtual bool validate ( const H225 :: CryptoH323Token & token, const ss :: string & login, const ss :: string & password ) const = 0;
	virtual ~CryptoTokenValidator ( ) { }
	virtual const ss :: string & getName ( ) const = 0;
	virtual ss :: string getLogin ( const H225 :: CryptoH323Token & token ) const = 0;
};

class CATValidator : public ClearTokenValidator {
//	int timestampGracePeriod;
public:
	CATValidator ( ) /*: timestampGracePeriod ( 2 * 60 * 60 + 10 )*/ { }
	bool validate ( const H235 :: ClearToken & token, const ss :: string & login, const ss :: string & password ) const;
	const ss :: string & getName ( ) const;
	ss :: string getLogin ( const H235 :: ClearToken & token ) const;
};

const ss :: string & CATValidator :: getName ( ) const {
	static ss :: string name = "CAT";
	return name;
}

ss :: string CATValidator :: getLogin ( const H235 :: ClearToken & token ) const {
	static const ss :: string oidCAT = "1.2.840.113548.10.1.2.1";
	if ( token.m_tokenOID.str ( ) != oidCAT )
		return "";
	if ( ! token.hasOptionalField ( H235 :: ClearToken :: e_generalID ) ||
		! token.hasOptionalField ( H235 :: ClearToken :: e_timeStamp ) ||
		! token.hasOptionalField ( H235 :: ClearToken :: e_random) ||
		! token.hasOptionalField ( H235 :: ClearToken :: e_challenge ) ) {
		PSYSTEMLOG ( Error, "CAT requires generalID, timeStamp, random and challenge" );
		return "";
	}
	if ( token.get_challenge ( ).size ( ) != 16 ) {
		PSYSTEMLOG ( Error, "challenge should be 16 bytes long" );
		return "";
	}
	PTime now;
//	int deltaTime = now.GetTimeInSeconds ( ) - token.get_timeStamp ( );
//	if ( abs ( deltaTime ) > timestampGracePeriod ) {
//		PSYSTEMLOG ( Error, "Invalid timestamp ABS(" << now.GetTimeInSeconds ( ) << '-'
//			<< int ( token.get_timeStamp ( ) ) << ") > " << timestampGracePeriod );
//		return "";
//	}
	int rnd = token.get_random ( );
	if ( rnd > 255 || rnd < - 127 ) {
		PSYSTEMLOG ( Error, "CAT requires single byte random field, got " << rnd );
		return "";
	}
	return token.get_generalID ( ).str ( ).c_str ( ); //supress terminating zero
}

bool CATValidator :: validate ( const H235 :: ClearToken & token, const ss :: string & login, const ss :: string & password ) const {
	unsigned char rb = char ( token.get_random ( ) );
	std :: tr1 :: uint32_t timeStamp = htonl ( token.get_timeStamp ( ) );
	MD5 ctx;
	ctx.processBytes ( & rb, 1 );
	ctx.processBytes ( password.data ( ), password.size ( ) );
	ctx.processBytes ( & timeStamp, 4 );
	const ss :: string & s = token.get_challenge ( );
	if ( s != ctx.finish ( ) ) {
		PSYSTEMLOG ( Error, "password incorrect for " << login );
		return false;
	}
	return true;
}

class SimpleMD5Validator : public CryptoTokenValidator {
public:
	virtual bool validate ( const H225 :: CryptoH323Token & token, const ss :: string & login, const ss :: string & password ) const;
	const ss :: string & getName ( ) const;
	ss :: string getLogin ( const H225 :: CryptoH323Token & token ) const;
};

const ss :: string & SimpleMD5Validator :: getName ( ) const {
	static ss :: string name = "SimpleMD5";
	return name;
}

ss :: string SimpleMD5Validator :: getLogin ( const H225 :: CryptoH323Token & token ) const {
	if ( token.getTag ( ) != H225 :: CryptoH323Token :: e_cryptoEPPwdHash )
		return "";
	const H225 :: CryptoH323Token_cryptoEPPwdHash & hash = token;
	const Asn :: BitString & bits = hash.m_token.m_hash;
	if ( bits.size ( ) != 128 ) {
		PSYSTEMLOG ( Error, "SimpleMD5 requires 128 bit hash" );
		return "";
	}
	return H323 :: getAliasAddressString ( hash.m_alias );
}

bool SimpleMD5Validator :: validate ( const H225 :: CryptoH323Token & token, const ss :: string & login,
	const ss :: string & password ) const {
	const H225 :: CryptoH323Token_cryptoEPPwdHash & hash = token;
	H235 :: PwdCertToken clearToken;
	clearToken.m_tokenOID = "0.0";
	setPlusNull ( clearToken.get_generalID ( ), login );
	clearToken.includeOptionalField ( H235 :: ClearToken :: e_password );
	setPlusNull ( clearToken.get_password ( ), password );
	clearToken.get_timeStamp ( ) = hash.m_timeStamp;
	Asn :: ostream os;
	clearToken.encode ( os );
	const Asn :: BitString & bits = hash.m_token.m_hash;
	if ( bits.str ( ) != md5Sum ( os.str ( ) ) ) {
		PSYSTEMLOG ( Error, "password incorrect for " << login );
		return false;
	}
	return true;
}

static vector < Pointer < ClearTokenValidator > > initClearValidators ( ) {
	vector < Pointer < ClearTokenValidator > > v;
	v.push_back ( new CATValidator );
	return v;
}

static vector < Pointer < CryptoTokenValidator > > initCryptoValidators ( ) {
	vector < Pointer < CryptoTokenValidator > > v;
//	v.push_back ( new SimpleMD5Validator );
	return v;
}

static vector < Pointer < ClearTokenValidator > > clearValidators = initClearValidators ( );
static vector < Pointer < CryptoTokenValidator > > cryptoValidators = initCryptoValidators ( );

static bool getLogin ( const H225 :: AliasAddress & alias, ss :: string & login, ss :: string & pass,
	bool & fromNat ) {
	ss :: string tmp = H323 :: getAliasAddressString ( alias );
	if ( tmp.size ( ) < 2 )
		return false;
	switch ( tmp [ 0 ] ) {
		case '0':
			fromNat = false;
			break;
		case '1':
			fromNat = true;
			break;
		default:
			PSYSTEMLOG ( Error, "first symbol is not '0' or '1'" );
			return false;
	}
	char l = tmp [ 1 ];
	if ( l < '0' || l > '9' || l - '0' > int ( tmp.size ( ) ) - 2 ) {
		PSYSTEMLOG ( Error, "incorrect login length '" << l << '\'' );
		return false;
	}
	l = char ( l - '0' );
	login = tmp.substr ( 2, l );
	pass = tmp.substr ( 2 + l );
	return true;
}

static bool getLogin ( const H225 :: ArrayOf_AliasAddress & terminalAlias, ss :: string & login, ss :: string & pass,
	bool & fromNat ) {
	for ( std :: size_t i = 0; i < terminalAlias.size ( ); i ++ ) {
		const H225 :: AliasAddress & a = terminalAlias [ i ];
		if ( a.getTag ( ) == H225 :: AliasAddress :: e_h323_ID )
			return getLogin ( a, login, pass, fromNat );
	}
	for ( std :: size_t i = 0; i < terminalAlias.size ( ); i ++ ) {
		const H225 :: AliasAddress & a = terminalAlias [ i ];
		if ( a.getTag ( ) == H225 :: AliasAddress :: e_dialedDigits )
			return getLogin ( a, login, pass, fromNat );
	}
	return false;
}

ss :: string RasThread :: getLoginFromRRQ ( const H225 :: RegistrationRequest & regReq ) {
	ss :: string login = "";
	if ( regReq.hasOptionalField ( H225 :: RegistrationRequest :: e_cryptoTokens ) ) {
		const H225 :: ArrayOf_CryptoH323Token & tokens = regReq.get_cryptoTokens ( );
		for ( unsigned i = 0; i < tokens.size ( ); i ++ ) {
			for ( unsigned j = 0; j < cryptoValidators.size ( ); j ++ ) {
				login = cryptoValidators [ j ] -> getLogin ( tokens [ i ] );
				if ( login.empty ( ) )
					continue;
				if ( login [ 0 ] == '!' ) {
					login.erase ( 0, 1 );
					if ( login.empty ( ) )
						continue;
				}
			}

		}
	}
	if ( regReq.hasOptionalField ( H225 :: RegistrationRequest :: e_tokens ) ) {
		const H225 :: ArrayOf_ClearToken & tokens = regReq.get_tokens ( );
		for ( unsigned i = 0; i < tokens.size ( ); i ++ ) {
			for ( unsigned j = 0; j < clearValidators.size ( ); j ++ ) {
				login = clearValidators [ j ] -> getLogin ( tokens [ i ] );
				if ( login.empty ( ) )
					continue;
				if ( login [ 0 ] == '!' ) {
					login.erase ( 0, 1 );
					if ( login.empty ( ) )
						continue;
				}
			}

		}
	}
	return login;
}

bool RasThread :: checkTokens ( const H225 :: RegistrationRequest & regReq, const PIPSocket :: Address & srcAddr,
	int srcPort, int & ttl, const PIPSocket :: Address & sigAddr, int sigPort, const StringSet & neededNumbers,
	StringSet & onlineNumbers, Conf :: UnregisteredContactsMap & unregisteredContacts, ss :: string & login,
	PUDPSocket & sock, IcqContactsVector & icqContacts ) const {
	PIPSocket :: Address localIp;
	sock.GetLocalAddress ( localIp );
	IpPortSet addresses;
	ttl = 300;
	if ( regReq.hasOptionalField ( H225 :: RegistrationRequest :: e_timeToLive ) )
		ttl = std :: min ( ttl, int ( regReq.get_timeToLive ( ) ) );
	const H225 :: ArrayOf_TransportAddress & array = regReq.m_callSignalAddress;
	for ( unsigned i = 0; i < array.size ( ); i ++ ) {
		if ( array [ i ].getTag ( ) != H225 :: TransportAddress :: e_ipAddress )
			continue;
		H225 :: TransportAddress_ipAddress ip = array [ i ];
		PIPSocket :: Address sa;
		WORD port;
		AddrUtils :: convertToIPAddress ( ip, sa, port );
		addresses.insert ( IpPort ( static_cast < const char * > ( sa.AsString ( ) ), port ) );
	}
	if ( addresses.empty ( ) ) {
		PSYSTEMLOG ( Error, "no call signal addresses in rrq" );
		return false;
	}
	bool hasSomeTokens = false;
	if ( regReq.hasOptionalField ( H225 :: RegistrationRequest :: e_cryptoTokens ) ) {
		hasSomeTokens = true;
		const H225 :: ArrayOf_CryptoH323Token & tokens = regReq.get_cryptoTokens ( );
		for ( unsigned i = 0; i < tokens.size ( ); i ++ ) {
			for ( unsigned j = 0; j < cryptoValidators.size ( ); j ++ ) {
				login = cryptoValidators [ j ] -> getLogin ( tokens [ i ] );
				if ( login.empty ( ) )
					continue;
				bool fromNat = false;
				if ( login [ 0 ] == '!' ) {
					fromNat = true;
					login.erase ( 0, 1 );
					if ( login.empty ( ) )
						continue;
				}
				ss :: string password;
				bool isCard;
				if ( ! conf -> getPassword ( localIp, addresses, login, password, isCard ) )
					return false;
				if ( cryptoValidators [ j ] -> validate ( tokens [ i ], login, password ) ) {
					PSYSTEMLOG ( Info, "validated via " << cryptoValidators [ j ] -> getName ( ) );
					bool h323 = true;
					conf -> registerPeer ( login, srcAddr, srcPort, h323, fromNat,
						PTime ( ) + ttl * 1000, isCard, sigAddr, sigPort, neededNumbers,
						onlineNumbers, & unregisteredContacts, sock, icqContacts );
					return true;
				}
			}

		}
	}
	if ( regReq.hasOptionalField ( H225 :: RegistrationRequest :: e_tokens ) ) {
		hasSomeTokens = true;
		const H225 :: ArrayOf_ClearToken & tokens = regReq.get_tokens ( );
		for ( unsigned i = 0; i < tokens.size ( ); i ++ ) {
			for ( unsigned j = 0; j < clearValidators.size ( ); j ++ ) {
				login = clearValidators [ j ] -> getLogin ( tokens [ i ] );
				if ( login.empty ( ) )
					continue;
				bool fromNat = false;
				if ( login [ 0 ] == '!' ) {
					fromNat = true;
					login.erase ( 0, 1 );
					if ( login.empty ( ) )
						continue;
				}
				ss :: string password;
				bool isCard;
				if ( ! conf -> getPassword ( localIp, addresses, login, password, isCard ) )
					return false;
				if ( clearValidators [ j ] -> validate ( tokens [ i ], login, password ) ) {
					PSYSTEMLOG ( Info, "validated via " << clearValidators [ j ] -> getName ( ) );
					bool h323 = true;
					conf -> registerPeer ( login, srcAddr, srcPort, h323, fromNat,
						PTime ( ) + ttl * 1000, isCard, sigAddr, sigPort, neededNumbers,
						onlineNumbers, & unregisteredContacts, sock, icqContacts );
					return true;
				}
			}

		}
	}
	while ( ! hasSomeTokens ) {
		if ( ! regReq.hasOptionalField ( H225 :: RegistrationRequest :: e_terminalAlias ) )
			break;
		const H225 :: ArrayOf_AliasAddress & terminalAlias = regReq.get_terminalAlias ( );
		ss :: string aliasPass;
		bool fromNat;
		if ( ! getLogin ( terminalAlias, login, aliasPass, fromNat ) )
			break;
		if ( login.empty ( ) )
			break;
		ss :: string password;
		bool isCard;
		if ( ! conf -> getPassword ( localIp, addresses, login, password, isCard ) )
			break;
		if ( password != aliasPass )
			break;
		PSYSTEMLOG ( Info, "validated terminalAlias" );
		bool h323 = true;
		conf -> registerPeer ( login, srcAddr, srcPort, h323, fromNat, PTime ( ) + ttl * 1000, isCard, sigAddr,
			sigPort, neededNumbers, onlineNumbers, & unregisteredContacts, sock, icqContacts );
		return true;
	}
	PSYSTEMLOG ( Error, "no tokens processed successfully" );
	return false;
}

void RasThread :: sendNotifications ( const Conf :: UnregisteredContactsMap & changes ) {
	for ( Conf :: UnregisteredContactsMap :: const_iterator i = changes.begin ( ); i != changes.end ( ); ++ i ) {
		const StringBoolMap & contacts = i -> second;
		if ( contacts.empty ( ) )
			continue;
		PIPSocket :: Address addr;
		int port;
		PUDPSocket * sock;
		if ( ! conf -> getRegisteredCardRasAddr ( i -> first, addr, port, sock ) )
			continue;
		if ( ! sock ) // SIP peers do not have RAS socket
			continue;
		H225::RasMessage msg;
		msg.setTag ( H225 :: RasMessage :: e_infoRequest );
		H225 :: InfoRequest & irq = msg;
		irq.includeOptionalField ( H225 :: InfoRequest :: e_tokens );
		H225 :: ArrayOf_ClearToken & tokens = irq.get_tokens ( );
		StringBoolMap :: size_type nsize = contacts.size ( );
		tokens.setSize ( nsize );
		StringBoolMap :: const_iterator j = contacts.begin ( );
		for ( StringBoolMap :: size_type k = 0; k < nsize; k ++, ++ j ) {
			tokens [ k ].m_tokenOID = "0.0.0.0";
			tokens [ k ].includeOptionalField ( H235 :: ClearToken :: e_challenge );
			tokens [ k ].get_challenge ( ) = ( j -> second ? '+' : '-' ) + j -> first;
		}
		writeRasReply ( * sock, AddrUtils :: convertToH225TransportAddr ( addr, WORD ( port ) ), msg );
	}
}

bool RasThread :: onRRQ ( const H225 :: RegistrationRequest & regReq, H225 :: RasMessage & reply,
	H225 :: TransportAddress & replyTo, PUDPSocket & sock ) {
	int ttl = 300;
	if ( regReq.hasOptionalField ( H225 :: RegistrationRequest :: e_timeToLive ) )
		ttl = std :: min ( ttl, int ( regReq.get_timeToLive ( ) ) );
	const H225 :: TransportAddress_ipAddress & replyToIP = replyTo;
	PIPSocket :: Address srcAddr;
	WORD srcPort;
	AddrUtils :: convertToIPAddress ( replyToIP, srcAddr, srcPort );
	StringSet onlineNumbers;
	Conf :: UnregisteredContactsMap unregisteredContacts;
	IcqContactsVector icqContacts;
	if ( regReq.hasOptionalField ( H225 :: RegistrationRequest :: e_keepAlive ) && regReq.get_keepAlive ( ) ) {
		StringSet contacts;
		ss :: string login = getLoginFromRRQ ( regReq );
		if ( regReq.hasOptionalField ( H225 :: RegistrationRequest :: e_terminalAlias ) ) {
			const H225 :: ArrayOf_AliasAddress & aa = regReq.get_terminalAlias ( );
			for ( unsigned i = 1; i < aa.size ( ); i ++ ) {
				if ( aa [ i ].getTag ( ) == H225 :: AliasAddress :: e_dialedDigits ) {
					const H225 :: AliasAddress_dialedDigits & digits = aa [ i ];
					contacts.insert ( digits );
				}
			}
		}
		bool c = conf -> checkKeepAlive ( srcAddr, srcPort, PTime ( ) + ttl * 1000, unregisteredContacts );
		if ( c ) {
			conf -> checkNewContacts ( login, contacts, unregisteredContacts );
			sendNotifications ( unregisteredContacts );
			return doRCF ( regReq, reply, onlineNumbers, ttl, icqContacts );
		}
		return doRRJ ( regReq, reply, H225 :: RegistrationRejectReason :: e_fullRegistrationRequired );
	}
	StringSet nums;
	H225::TransportAddress_ipAddress sigIp;
	PIPSocket :: Address sigAddr;
	WORD sigPort;
	AddrUtils :: getIPAddress ( sigIp, regReq.m_callSignalAddress );
	AddrUtils :: convertToIPAddress ( sigIp, sigAddr, sigPort );
	if ( regReq.hasOptionalField ( H225 :: RegistrationRequest :: e_terminalAlias ) ) {
		const H225 :: ArrayOf_AliasAddress & aa = regReq.get_terminalAlias ( );
		for ( unsigned i = 1; i < aa.size ( ); i ++ ) {
			if ( aa [ i ].getTag ( ) == H225 :: AliasAddress :: e_dialedDigits ) {
				const H225 :: AliasAddress_dialedDigits & digits = aa [ i ];
				nums.insert ( digits );
			}
		}
	}
	ss :: string login;
	bool c = checkTokens ( regReq, srcAddr, srcPort, ttl, sigAddr, sigPort, nums, onlineNumbers,
		unregisteredContacts, login, sock, icqContacts );
	sendNotifications ( unregisteredContacts );
	if ( ! c )
		return doRRJ ( regReq, reply, H225 :: RegistrationRejectReason :: e_securityDenial );
	return doRCF ( regReq, reply, onlineNumbers, ttl, icqContacts );
}

static bool onRRJ ( const H225 :: RegistrationReject & regRej ) {
	int id = regRej.m_requestSeqNum;
	conf -> rejectRrq ( id, regRej.m_rejectReason.getTag ( ) );
	return true;
}

static bool onRCF ( const H225 :: RegistrationConfirm & regCon ) {
	int id = regCon.m_requestSeqNum;
	ss :: string eid = regCon.m_endpointIdentifier.str ( );
	conf -> confirmRrq ( id, eid );
	return true;
}

static bool doACF ( const H225 :: AdmissionRequest & admReq, H225 :: RasMessage & reply,
	const H225 :: TransportAddress & gkAddr ) {
// Task: to confirm the given admission request
	reply.setTag ( H225 :: RasMessage :: e_admissionConfirm );
	H225 :: AdmissionConfirm & admConf = reply;
	admConf.m_requestSeqNum = admReq.m_requestSeqNum;
	admConf.m_bandWidth = admReq.m_bandWidth;
	admConf.m_callModel.setTag ( H225 :: CallModel :: e_direct );
	if ( admReq.hasOptionalField ( H225 :: AdmissionRequest :: e_destCallSignalAddress ) )
		admConf.m_destCallSignalAddress = admReq.get_destCallSignalAddress ( );
	else
		admConf.m_destCallSignalAddress = gkAddr;
	return true;
}

static bool doARJ ( const H225 :: AdmissionRequest & admReq, H225 :: RasMessage & reply,
		H225 :: AdmissionRejectReason :: Choices reason ) {
// Task: to reject the given admission request
	reply.setTag ( H225 :: RasMessage :: e_admissionReject );
	H225 :: AdmissionReject & admRej = reply;
	admRej.m_requestSeqNum = admReq.m_requestSeqNum;
	admRej.m_rejectReason.setTag ( reason );
	return true;
}

static bool onARQ ( const H225 :: AdmissionRequest & admissReq, H225 :: RasMessage & reply,
	const H225 :: TransportAddress & replyTo, const H225 :: TransportAddress & gkAddr ) {
// Task: to handle an admission request message....
// replyTo must be set to the source address of message. Use it for
// security checks (the given RAS message must be sent from the
// registerred RAS address).

	if ( ! admissReq.m_answerCall ) {
		const H225 :: TransportAddress_ipAddress & replyToIP = replyTo;
		PIPSocket :: Address srcAddr;
		WORD srcPort;
		AddrUtils :: convertToIPAddress ( replyToIP, srcAddr, srcPort );
		if ( conf -> admissCall ( srcAddr, srcPort, admissReq.m_conferenceID ) )
			return doACF ( admissReq, reply, gkAddr );
		else
			return doARJ ( admissReq, reply, H225 :: AdmissionRejectReason :: e_securityDenial );
	}
	if ( ! admissReq.hasOptionalField ( H225 :: AdmissionRequest :: e_srcCallSignalAddress ) ||
		admissReq.get_srcCallSignalAddress ( ).getTag ( ) != H225 :: TransportAddress :: e_ipAddress ||
		! admissReq.hasOptionalField ( H225 :: AdmissionRequest :: e_destinationInfo ) ||
		admissReq.get_destinationInfo ( ).size ( ) < 1 ) {
		PSYSTEMLOG ( Error, "invalid ARQ" );
		return doARJ ( admissReq, reply, H225 :: AdmissionRejectReason :: e_securityDenial );
	}
	// They are answering a call from another endpoint....
	const H225 :: TransportAddress_ipAddress & srcIP = admissReq.get_srcCallSignalAddress ( );
	PIPSocket :: Address srcAddr, replyAddr;
	WORD srcPort;
	AddrUtils :: convertToIPAddress ( srcIP, srcAddr, srcPort );
	const H225 :: TransportAddress_ipAddress & replyIP = replyTo;
	AddrUtils :: convertToIPAddress ( replyIP, replyAddr, srcPort );
	PSYSTEMLOG ( Info, replyAddr << " want to answer call from " << srcAddr );
	ss :: string inIp = static_cast < const char * > ( srcAddr.AsString ( ) );
	if ( conf -> isLocalIp ( inIp ) )
		return doACF ( admissReq, reply, gkAddr );
	ss :: string dialedDigits = H323 :: getAliasAddressString ( admissReq.get_destinationInfo ( ) [ 0 ] );
	PSYSTEMLOG ( Error, "ARQ from invalid peer: " << replyAddr << ", " << srcAddr << ", " << dialedDigits );
	return doARJ ( admissReq, reply, H225 :: AdmissionRejectReason :: e_securityDenial );
}

static bool onARJ ( const H225 :: AdmissionReject & admRej ) {
	int id = admRej.m_requestSeqNum;
	conf -> rejectArq ( id );
	return true;
}

static bool onACF ( const H225 :: AdmissionConfirm & admCon ) {
	int id = admCon.m_requestSeqNum;
	conf -> confirmArq ( id, admCon.m_destCallSignalAddress );
	return true;
}

static bool doDCF ( const H225 :: DisengageRequest & disReq, H225 :: RasMessage & reply ) {
// Task: to confirm the given disengage request
	reply.setTag ( H225 :: RasMessage :: e_disengageConfirm );
	H225 :: DisengageConfirm & disConf = reply;
	disConf.m_requestSeqNum = disReq.m_requestSeqNum;
	return true;
}

static bool doUCF ( const H225 :: UnregistrationRequest & unrReq, H225 :: RasMessage & reply ) {
// Task: to confirm the given disengage request
	reply.setTag ( H225 :: RasMessage :: e_unregistrationConfirm );
	H225 :: UnregistrationConfirm & unrConf = reply;
	unrConf.m_requestSeqNum = unrReq.m_requestSeqNum;
	return true;
}

static bool onDRQ ( const H225 :: DisengageRequest & disReq, H225 :: RasMessage & reply ) {
	//do not check tokens - why anyone would disengage someone else's call ?
	conf -> disengageCall ( disReq.m_conferenceID );
	return doDCF ( disReq, reply );
}

bool RasThread :: onURQ ( const H225 :: UnregistrationRequest & unrReq, H225 :: RasMessage & reply,
	H225 :: TransportAddress & replyTo ) {
	const H225 :: TransportAddress_ipAddress & replyToIP = replyTo;
	PIPSocket :: Address srcAddr;
	WORD srcPort;
	AddrUtils :: convertToIPAddress ( replyToIP, srcAddr, srcPort );
	Conf :: UnregisteredContactsMap unregisteredContacts;
	conf -> unregisterInPeer ( srcAddr, srcPort, unregisteredContacts );
	sendNotifications ( unregisteredContacts );
	return doUCF ( unrReq, reply );
}

static bool doLCF ( const H225 :: LocationRequest & locReq, H225 :: RasMessage & reply,
	const H225 :: TransportAddress & signalAddr, const H225 :: TransportAddress & rasAddr ) {
	reply.setTag ( H225 :: RasMessage :: e_locationConfirm );
	H225 :: LocationConfirm & locConf = reply;
	locConf.m_requestSeqNum = locReq.m_requestSeqNum;
	locConf.m_callSignalAddress = signalAddr;
	locConf.m_rasAddress = rasAddr;
	return true;
}

static bool onLRQ ( const H225 :: LocationRequest & locReq, H225 :: RasMessage & reply,
	H225 :: TransportAddress & replyTo, const H225 :: TransportAddress & signalAddr,
	const H225 :: TransportAddress & rasAddr ) {
	getLRQReplyTo ( locReq, replyTo );
	return doLCF ( locReq, reply, signalAddr, rasAddr );
}

static bool doBCF ( const H225 :: BandwidthRequest & banReq, H225 :: RasMessage & reply ) {
	reply.setTag ( H225 :: RasMessage :: e_bandwidthConfirm );
	H225 :: BandwidthConfirm & banConf = reply;
	banConf.m_requestSeqNum = banReq.m_requestSeqNum;
	banConf.m_bandWidth = banReq.m_bandWidth;
	return true;
}

static bool onBRQ ( const H225 :: BandwidthRequest & banReq, H225 :: RasMessage & reply ) {
	return doBCF ( banReq, reply );
}

static H225 :: TransportAddress getSignalAddr ( PUDPSocket & sock ) {
// Task: to return the gatekeeper address visible from specified Destination address.
	PIPSocket :: Address addr;
	sock.GetLocalAddress ( addr );
	return AddrUtils :: convertToH225TransportAddr ( addr, STD_SIGNALLING_PORT );
}

H225 :: TransportAddress RasThread :: getGatekeeperAddr ( PUDPSocket & sock ) const {
// Task: to return the gatekeeper address visible from specified Destination address.
	PIPSocket :: Address addr;
	sock.GetLocalAddress ( addr );
	return AddrUtils :: convertToH225TransportAddr ( addr, rasPort );
}

bool RasThread :: handleRasRequest ( const H225 :: RasMessage & request, H225 :: RasMessage & reply,
	H225 :: TransportAddress & replyTo, PUDPSocket & sock ) {
// Task: to handle the given Ras request.
// replyTo must be set to the source address of message. OnRRQ use it for
// security checks (the given RAS and Call ip addresses must correspond
// to the sender's ip).
	switch ( request.getTag ( ) ) {
		case H225 :: RasMessage :: e_registrationRequest:
			return onRRQ ( request, reply, replyTo, sock );
		case H225 :: RasMessage :: e_registrationReject:
			return onRRJ ( request ), false;
		case H225 :: RasMessage :: e_registrationConfirm:
			return onRCF ( request ), false;
		case H225 :: RasMessage :: e_unregistrationRequest:
			return onURQ ( request, reply, replyTo );
		case H225 :: RasMessage :: e_admissionRequest:
			return onARQ ( request, reply, replyTo, getSignalAddr ( sock ) );
		case H225 :: RasMessage :: e_admissionReject:
			return onARJ ( request ), false;
		case H225 :: RasMessage :: e_admissionConfirm:
			return onACF ( request ), false;
		case H225 :: RasMessage :: e_bandwidthRequest:
			return onBRQ ( request, reply );
		case H225 :: RasMessage :: e_disengageRequest:
			return onDRQ ( request, reply );
		case H225 :: RasMessage :: e_locationRequest:
			return onLRQ ( request, reply, replyTo, getSignalAddr ( sock ), getGatekeeperAddr ( sock ) );
		case H225 :: RasMessage :: e_gatekeeperRequest:
			return onGRQ ( request, reply, getGatekeeperAddr ( sock ) );
		case H225 :: RasMessage :: e_gatekeeperReject:
			return onGRJ ( request ), false;
		case H225 :: RasMessage :: e_gatekeeperConfirm:
			return onGCF ( request ), false;
/*		case H225 :: RasMessage :: e_infoRequestResponse:
			OnIRR ( Request );
		case H225 :: RasMessage :: e_unregistrationConfirm:
		case H225 :: RasMessage :: e_unregistrationReject:
			return false;*/
		default :
			// We should really return an unknown message response
			// for messages we don't understand, but I've got no idea
			// how to extract the message sequence number....
			Log -> logNote ( "Unknown message Request" );
			PSYSTEMLOG ( Error, "Unknown mesg " << request.getTag ( ) << ' ' << request );
			PSYSTEMLOG ( Error, request );
			return false;
	}
}

