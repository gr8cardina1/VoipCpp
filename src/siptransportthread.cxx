#pragma implementation
#include "ss.hpp"
#include "allocatable.hpp"
#include <ptlib.h>
#include <boost/optional.hpp>
#include "sip2.hpp"
#include "pointer.hpp"
#include <ptlib/sockets.h>
#include "siptransportthread.hpp"
#include "automutex.hpp"
#include <ptlib/svcproc.h>
#include "aftertask.hpp"
#include "ipport.hpp"
#include "icqcontact.hpp"
#include "radgwinfo.hpp"
#include "Conf.hpp"
#include "ixcudpsocket.hpp"
#include <stdexcept>
#include "Log.h"
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/mem_fun.hpp>
#include "siporiginatehandler.hpp"
#include "sipanswerhandler.hpp"
#include <boost/multi_index/sequenced_index.hpp>
#include "sdp.hpp"
#include "codecinfo.hpp"
#include "tarifround.hpp"
#include "tarifinfo.hpp"
#include "pricedata.hpp"
#include "outcarddetails.hpp"
#include "signallingoptions.hpp"
#include "outchoicedetails.hpp"
#include "sourcedata.hpp"
#include "moneyspool.hpp"
#include <tr1/functional>
#include "smartguard.hpp"
#include <tr1/memory>
#include "commoncalldetails.hpp"
#include "session.hpp"
#include "rtpsession.hpp"
#include "rtpstat.hpp"
#include "h323call.hpp"
#include "answercall.hpp"
#include "sdpcommon.hpp"
#include <tr1/cstdint>
#include "md5.hpp"
#include "rasthread.hpp"
#include "random.hpp"

namespace SIP2 {

class ClientDialog {
	ClientTransactionId id;
	Pointer < Dialog > d;
	OriginateHandler * h;
	Pointer < SDP :: SessionDescription > sentSdp;
	bool supportsPackage, supportsDtmf;
public:
	ClientDialog ( const Response & r, Pointer < SDP :: SessionDescription > sdp, OriginateHandler * h ) :
		id ( r ), d ( new Dialog ( r, h && h -> acceptsDtmfOverride ( ) ) ), h ( h ), sentSdp ( sdp ) {
		const ResponseMIMEInfo & m = r.getMime ( );
		supportsPackage = m.hasRecvInfo ( );
		supportsDtmf = m.getRecvInfo ( ipDtmf );
	}
	~ClientDialog ( ) {
	}
	void sendAck ( const Response & r ) const;
	void targetRefresh ( const Response & r ) const;
	const ClientTransactionId & getTransactionId ( ) const {
		return id;
	}
	const DialogId & getDialogId ( ) const {
		return d -> getId ( );
	}
	bool isConfirmed ( ) const {
		return d -> isConfirmed ( );
	}
	void detachOriginateHandler ( ) {
		h = 0;
	}
	bool requestReceived ( const Request & r ) const {
		if ( ! d -> requestReceived ( r ) )
			return false;
		return true;
	}
	OriginateHandler * getOriginateHandler ( ) const {
		return h;
	}
	Dialog & getDialog ( ) const {
		return * d;
	}
	const SDP :: SessionDescription * getSentSdp ( ) const {
		return sentSdp;
	}
	bool getSupportsPackage ( ) const {
		return d -> getSupportsPackage ( );
	}
	bool getSupportsDtmf ( ) const {
		return d -> getSupportsDtmf ( );
	}
};

void ClientDialog :: sendAck ( const Response & r ) const {
	Request ack ( * d, Request :: mAck, r.getMime ( ).getCseq ( ).seq );
	transportThread -> sendMessage ( ack );
}

void ClientDialog :: targetRefresh ( const Response & r ) const {
	d -> targetRefresh ( r );
}

class ServerDialog {
	ServerTransactionId id;
	Pointer < Dialog > d;
	AnswerHandler * h;
	Pointer < Response > ok;
	PTime lastSent;
	PTime timer;
	int timerSecs;
	int inviteSeq;
	Pointer < SDP :: SessionDescription > sentSdp;
	bool supportsPackage, supportsDtmf;
public:
	ServerDialog ( const Request & r, AnswerHandler * h ) : id ( r ), d ( new Dialog ( r ) ), h ( h ), timerSecs ( 1 ),
		inviteSeq ( r.getMime ( ).getCseq ( ).seq ) {
		const RequestMIMEInfo & m = r.getMime ( );
		supportsPackage = m.hasRecvInfo ( );
		supportsDtmf = m.getRecvInfo ( ipDtmf );
	}
	void sendAck ( const Response & r ) const;
	void targetRefresh ( const Response & r ) const;
	const ServerTransactionId & getTransactionId ( ) const {
		return id;
	}
	const DialogId & getDialogId ( ) const {
		return d -> getId ( );
	}
	void detachAnswerHandler ( ) {
		h = 0;
	}
	AnswerHandler * getAnswerHandler ( ) const {
		return h;
	}
	bool checkTimeout ( ) const;
	void setOk ( const Response & r ) const;
	bool requestReceived ( const Request & r ) const;
	Dialog & getDialog ( ) const {
		return * d;
	}
	bool isConfirmed ( ) const {
		return d -> isConfirmed ( );
	}
	const SDP :: SessionDescription * getSentSdp ( ) const {
		return sentSdp;
	}
	void setSupportsPackage ( bool d ) {
		supportsPackage = d;
	}
	bool getSupportsPackage ( ) const {
		return supportsPackage;
	}
	void setSupportsDtmf ( bool d ) {
		supportsDtmf = d;
	}
	bool getSupportsDtmf ( ) const {
		return supportsDtmf;
	}
};

void ServerDialog :: sendAck ( const Response & r ) const {
	Request ack ( * d, Request :: mAck, r.getMime ( ).getCseq ( ).seq );
	transportThread -> sendMessage ( ack );
}

void ServerDialog :: targetRefresh ( const Response & r ) const {
	d -> targetRefresh ( r );
}

bool ServerDialog :: checkTimeout ( ) const {
	if ( ! ok )
		return true;
	PTime now;
	if ( timer >= now )
		return false;
	if ( lastSent + timerSecs * 1000 <= now ) {
		if ( ! transportThread -> sendMessage ( * ok ) )
			return false;
		ServerDialog * self = const_cast < ServerDialog * > ( this );
		self -> timerSecs = std :: min ( timerSecs * 2, 4 );
		self -> lastSent = now;
	}
	return true;
}

void ServerDialog :: setOk ( const Response & r ) const {
	ServerDialog * self = const_cast < ServerDialog * > ( this );
	self -> ok = new Response ( r );
	self -> lastSent = PTime ( );
	if ( const SDP :: SessionDescription * sdp = r.getSdp ( ) )
		self -> sentSdp = new SDP :: SessionDescription ( * sdp );
}

bool ServerDialog :: requestReceived ( const Request & r ) const {
	Request :: Methods method = r.getMethod ( );
	if ( ! d -> requestReceived ( r ) )
		return false;
	if ( method == Request :: mAck ) {
		const_cast < ServerDialog * > ( this ) -> ok = 0;
		return h;
	}
	return true;
}

typedef boost :: multi_index :: multi_index_container < ClientDialog, boost :: multi_index :: indexed_by <
	boost :: multi_index :: ordered_unique < boost :: multi_index :: tag < DialogId >,
	boost :: multi_index :: const_mem_fun < ClientDialog, const DialogId &, & ClientDialog :: getDialogId > >,
	boost :: multi_index :: ordered_non_unique < boost :: multi_index :: tag < ClientTransactionId >,
	boost :: multi_index :: const_mem_fun < ClientDialog, const ClientTransactionId &, & ClientDialog :: getTransactionId > > >,
	__SS_ALLOCATOR < ClientDialog > > ClientDialogMap;

class CoreClientInviteHandler : public CoreClientTransactionHandler {
	TransportThread :: Impl * t;
	void responseReceived ( const Response & r, const ClientTransaction * c ) const;
	void transactionTimedOut ( const ClientTransaction * c ) const;
	void transactionTransportError ( const ClientTransaction * c ) const;
	void transactionFinished ( const ClientTransaction * c ) const;
public:
	void strayResponseReceived ( const Response & r ) const;
	explicit CoreClientInviteHandler ( TransportThread :: Impl * t ) : t ( t ) { }
};

class CoreClientRegisterHandler : public CoreClientTransactionHandler {
	TransportThread :: Impl * t;
	void responseReceived ( const Response & r, const ClientTransaction * c ) const;
	void transactionTimedOut ( const ClientTransaction * c ) const;
	void transactionTransportError ( const ClientTransaction * c ) const;
	void transactionFinished ( const ClientTransaction * /*c*/ ) const { };
public:
	explicit CoreClientRegisterHandler ( TransportThread :: Impl * t ) : t ( t ) { }
};

class CoreServerUnknownMethodHandler {
	TransportThread :: Impl * t;
public:
	void requestReceived ( const Request & r );
	explicit CoreServerUnknownMethodHandler ( TransportThread :: Impl * t ) : t ( t ) { }
};

class CoreServerBadExtensionHandler {
	TransportThread :: Impl * t;
public:
	void requestReceived ( const Request & r );
	explicit CoreServerBadExtensionHandler ( TransportThread :: Impl * t ) : t ( t ) { }
};

class CoreServerByeHandler {
	TransportThread :: Impl * t;
public:
	void requestReceived ( const Request & r );
	explicit CoreServerByeHandler ( TransportThread :: Impl * t ) : t ( t ) { }
};

class CoreServerInviteHandler : public CoreServerTransactionHandler {
	TransportThread :: Impl * t;
	void transactionTransportError ( ServerTransaction * c );
public:
	void requestReceived ( const Request & r );
	explicit CoreServerInviteHandler ( TransportThread :: Impl * t ) : t ( t ) { }
};

class CoreServerAckHandler {
	TransportThread :: Impl * t;
public:
	void requestReceived ( const Request & r );
	explicit CoreServerAckHandler ( TransportThread :: Impl * t ) : t ( t ) { }
};

class CoreServerCancelHandler {
	TransportThread :: Impl * t;
public:
	void requestReceived ( const Request & r );
	explicit CoreServerCancelHandler ( TransportThread :: Impl * t ) : t ( t ) { }
};

class ServerAuthHandler {
	typedef std :: map < unsigned, unsigned, std :: less < unsigned >,
		__SS_ALLOCATOR < std :: pair < const unsigned, unsigned > > > UnsignedUnsignedMap;
	typedef std :: map < unsigned, UnsignedUnsignedMap, std :: less < unsigned >,
		__SS_ALLOCATOR < std :: pair < const unsigned, UnsignedUnsignedMap > > > UnsignedUnsignedUnsignedMap;
	UnsignedUnsignedUnsignedMap seenNonces;
	ss :: string realm;
	ss :: string secret;
	unsigned nonceTtl;
	ss :: string generateNonce ( unsigned t, unsigned sn ) const;
	bool checkSeenNonces ( unsigned ts, unsigned sn, unsigned nc );
public:
	void addChallenge ( ResponseMIMEInfo & m, bool stale = false ) const;
	void addAuthenticationInfo ( const DigestResponse & d, Response & rsp, const ss :: string & password ) const;
	bool validNonce ( const DigestResponse & d );
	const ss :: string & getRealm ( ) const {
		return realm;
	}
	ServerAuthHandler ( );
};

class CoreServerRegisterHandler {
	TransportThread :: Impl * t;
public:
	void requestReceived ( const Request & r );
	explicit CoreServerRegisterHandler ( TransportThread :: Impl * t );
};

class CoreServerInfoHandler {
	TransportThread :: Impl * t;
public:
	void requestReceived ( const Request & r );
	explicit CoreServerInfoHandler ( TransportThread :: Impl * t ) : t ( t ) { }
};

typedef ClientDialogMap :: index < ClientTransactionId > :: type ClientDialogByTransaction;

class ServerTransactionNode {
	Pointer < ServerTransaction > p;
public:
	explicit ServerTransactionNode ( Pointer < ServerTransaction > p ) : p ( p ) { }
	const ServerTransactionId & getId ( ) const {
		return p -> getId ( );
	}
	const MergedTransactionId & getMergedId ( ) const {
		return p -> getMergedId ( );
	}
	bool checkTimeout ( ) const {
		return p -> checkTimeout ( );
	}
	void requestReceived ( const Request & r ) const {
		p -> requestReceived ( r );
	}
	const Request & getRequest ( ) const {
		return dynamic_cast < InviteServerTransaction * > ( p.data ( ) ) -> getRequest ( );
	}
	const DigestResponse * getSavedDigestResponse ( ) const {
		return dynamic_cast < InviteServerTransaction * > ( p.data ( ) ) -> getSavedDigestResponse ( );
	}
	const ss :: string & getSavedPassword ( ) const {
		return dynamic_cast < InviteServerTransaction * > ( p.data ( ) ) -> getSavedPassword ( );
	}
	void saveAuth ( unsigned dIndex, const ss :: string & password ) {
		dynamic_cast < InviteServerTransaction * > ( p.data ( ) ) -> saveAuth ( dIndex, password );
	}
	bool responseSent ( const Response & r ) const {
		return p -> responseSent ( r );
	}
	const Response & getResponse ( ) const {
		return dynamic_cast < InviteServerTransaction * > ( p.data ( ) ) -> getResponse ( );
	}
};

typedef boost :: multi_index :: multi_index_container < ServerTransactionNode, boost :: multi_index :: indexed_by <
	boost :: multi_index :: ordered_unique < boost :: multi_index :: tag < ServerTransactionId >,
	boost :: multi_index :: const_mem_fun < ServerTransactionNode, const ServerTransactionId &,
	& ServerTransactionNode :: getId > >,
	boost :: multi_index :: ordered_non_unique < boost :: multi_index :: tag < MergedTransactionId >,
	boost :: multi_index :: const_mem_fun < ServerTransactionNode, const MergedTransactionId &,
	& ServerTransactionNode :: getMergedId > > >,
	__SS_ALLOCATOR < ServerTransactionNode > > ServerTransactionMap;

typedef boost :: multi_index :: multi_index_container < ServerDialog, boost :: multi_index :: indexed_by <
	boost :: multi_index :: ordered_unique < boost :: multi_index :: tag < DialogId >,
	boost :: multi_index :: const_mem_fun < ServerDialog, const DialogId &, & ServerDialog :: getDialogId > >,
	boost :: multi_index :: ordered_unique < boost :: multi_index :: tag < ServerTransactionId >,
	boost :: multi_index :: const_mem_fun < ServerDialog, const ServerTransactionId &, & ServerDialog :: getTransactionId > > >,
	__SS_ALLOCATOR < ServerDialog > > ServerDialogMap;

typedef ServerDialogMap :: index < ServerTransactionId > :: type ServerDialogByTransaction;

struct RealmInfo {
	DigestChallenge challenge;
	ss :: string lastNonce;
	unsigned nc;
	explicit RealmInfo ( const DigestChallenge & challenge ) : challenge ( challenge ) { }
};

class TransportThread :: Impl : public Allocatable < __SS_ALLOCATOR > {
	friend class CoreClientInviteHandler;
	CoreClientInviteHandler coreClientInviteHandler;
	friend class CoreClientRegisterHandler;
	CoreClientRegisterHandler coreClientRegisterHandler;
	friend class CoreServerUnknownMethodHandler;
	CoreServerUnknownMethodHandler coreServerUnknownMethodHandler;
	friend class CoreServerBadExtensionHandler;
	CoreServerBadExtensionHandler coreServerBadExtensionHandler;
	friend class CoreServerAckHandler;
	CoreServerAckHandler coreServerAckHandler;
	friend class CoreServerByeHandler;
	CoreServerByeHandler coreServerByeHandler;
	friend class CoreServerCancelHandler;
	CoreServerCancelHandler coreServerCancelHandler;
	friend class CoreServerInfoHandler;
	CoreServerInfoHandler coreServerInfoHandler;
	friend class CoreServerInviteHandler;
	CoreServerInviteHandler coreServerInviteHandler;
	friend class CoreServerRegisterHandler;
	CoreServerRegisterHandler coreServerRegisterHandler;
	ServerAuthHandler serverAuthHandler;
	PMutex mut;
	IxcUDPSocket sock;
	typedef std :: map < ClientTransactionId, Pointer < ClientTransaction >, std :: less < ClientTransactionId >,
		__SS_ALLOCATOR < std :: pair < const ClientTransactionId, Pointer < ClientTransaction > > > >
		ClientTransactionMap;
	ClientTransactionMap clientTransactions;
	ClientDialogMap clientDialogs;
	ServerTransactionMap serverTransactions;
	ServerDialogMap serverDialogs;
	typedef std :: map < ss :: string, RealmInfo, std :: less < ss :: string >,
		__SS_ALLOCATOR < std :: pair < const ss :: string, RealmInfo > > > RealmMap;
	RealmMap realmMap; // nado kak-to limitirovat razmer ?
	StringVector rportAgents;
	volatile bool confReloaded;
	void checkTimers ( );
	void readAndHandleMesg ( );
	void handleResponse ( const ss :: string & t, const PIPSocket :: Address & addr, int port );
	void handleRequest ( const ss :: string & t, const PIPSocket :: Address & addr, int port );
	void sendBye ( Dialog & d );
	void sendCancel ( Dialog & d, int snum, const ss :: string & branch );
	void addDigestResponse ( Request & r, const ss :: string & rm, const ss :: string & login, const ss :: string & pass );
	StringVector saveChallenges ( const DigestChallengeVector & v );
	void checkAuthenticationInfo ( const Response & r, const ClientTransaction * i );
	void reloadcb ( );
	void detachHandlerInt ( const ServerDialogMap :: iterator & j, int c );
public:
	Impl ( );
	void Close ( ) {
		sock.Close ( );
	}
	void Main ( );
	bool sendMessage ( const ss :: string & host, int port, const ss :: string & msg );
	bool sendMessage ( Request & r );
	bool sendMessage ( const Response & r );
	void detachHandler ( const ClientTransactionId & id );
	void detachHandler ( const ServerTransactionId & id, int c );
	ClientTransactionId originateCall ( const ss :: string & host, const ss :: string & user,
		int port, const ss :: string & localHost, const ss :: string & localUser,
		const ss :: string & callId, unsigned maxForwards, unsigned cseq, const StringVector & realms,
		const ss :: string & remotePartyID, SDP :: SessionDescription * sdp, OriginateHandler * h,
		const ss :: string & passertid, const ss :: string & privac );
	ClientTransactionId sendRegister ( const ss :: string & host, const ss :: string & user, int port,
		const ss :: string & localHost, const ss :: string & callId, unsigned cseq, unsigned expires, const StringVector & realms,
		const ss :: string & pass, OriginateHandler * h );
	void sendRinging ( const ServerTransactionId & id, int localRtpPort,
		const PIPSocket :: Address & localAddr, const CodecInfo & inCodec );
	void sendOk ( const ServerTransactionId & id, int localRtpPort,
		const PIPSocket :: Address & localAddr, const CodecInfo & inCodec, unsigned telephoneEventsPayloadType );
	ClientTransactionId sendOnhold ( const ClientTransactionId & inviteId, int level, OriginateHandler * h );
	ClientTransactionId sendOnhold ( const ServerTransactionId & inviteId, int level, AnswerHandler * h );
	ClientTransactionId sendDtmf ( const ClientTransactionId & inviteId, const DTMF :: Relay & r );
	ClientTransactionId sendDtmf ( const ServerTransactionId & inviteId, const DTMF :: Relay & r );
};

void TransportThread :: Impl :: checkTimers ( ) {
	AutoMutex am ( mut );
	for ( ClientTransactionMap :: iterator i = clientTransactions.begin ( ); i != clientTransactions.end ( ); ) {
		if ( i -> second -> checkTimeout ( ) )
			++ i;
		else
			clientTransactions.erase ( i ++ );
	}
	for ( ServerTransactionMap :: iterator i = serverTransactions.begin ( ); i != serverTransactions.end ( ); ) {
		if ( i -> checkTimeout ( ) )
			++ i;
		else
			serverTransactions.erase ( i ++ );
	}
	for ( ServerDialogMap :: iterator i = serverDialogs.begin ( ); i != serverDialogs.end ( ); ) {
		if ( i -> checkTimeout ( ) )
			++ i;
		else {
			sendBye ( i -> getDialog ( ) );
			serverDialogs.erase ( i ++ );
		}
	}
}

void TransportThread :: Impl :: readAndHandleMesg ( ) {
	const int bufferSize = 65536;	// minimum 4000, no bolshe ne pomeshaet
	static char buffer [ bufferSize ]; // pohoge v stacke mesta ne hvataet
	PIPSocket :: Address senderAddr;
	WORD senderPort;
	if ( ! sock.Read ( buffer, bufferSize ) )
		return;
	sock.GetLastReceiveAddress ( senderAddr, senderPort );
	ss :: string t ( buffer, sock.GetLastReadCount ( ) );
	PSYSTEMLOG ( Info, "sip message arrived from " << senderAddr << ':' << senderPort << ' ' << t.size ( ) << " bytes : " << t );
	if ( t.size ( ) > 2 && t [ 0 ] == 'S' && t [ 1 ] == 'I' )
		handleResponse ( t, senderAddr, senderPort );
	else
		handleRequest ( t, senderAddr, senderPort );
}

static bool validLocalHost ( const ss :: string & s ) {
	return ! s.empty ( ); // FIXME: brat spisok iz interfaces
}

void TransportThread :: Impl :: handleResponse ( const ss :: string & t, const PIPSocket :: Address & addr, int port ) {
	Response r ( t.begin ( ), t.end ( ) );
	Log -> LogSIP2Msg ( r, OpengateLog :: Receiving, addr, WORD ( port ), false );
	const ResponseMIMEInfo & m = r.getMime ( );
	const ViaVector & vv = m.getVia ( );
	if ( vv.empty ( ) ) {
		PSYSTEMLOG ( Error, "empty via" );
		return;
	}
	const Via & v = vv.front ( );
	AutoMutex am ( mut );
	if ( ! validLocalHost ( v.getHost ( ) ) )
		return;
	ClientTransactionId id ( r );
	ClientTransactionMap :: iterator i = clientTransactions.find ( id );
	if ( i == clientTransactions.end ( ) ) {
		if ( id.method == Request :: mInvite )
			coreClientInviteHandler.strayResponseReceived ( r );
		return;
	}
	if ( ! i -> second -> responseReceived ( r ) )
		clientTransactions.erase ( i );
}

static bool oldRfcBranch ( const ss :: string & s ) {
	return s.compare ( 0, 7, "z9hG4bK", 7 );
}

void TransportThread :: Impl :: handleRequest ( const ss :: string & t, const PIPSocket :: Address & addr, int port ) {
	Request r ( t.begin ( ), t.end ( ) );
	Log -> LogSIP2Msg ( r, OpengateLog :: Receiving, addr, WORD ( port ), false );
	RequestMIMEInfo & m = r.getMime ( );
	ViaVector & vv = m.getVia ( );
	if ( vv.empty ( ) ) {
		PSYSTEMLOG ( Error, "empty via" );
		return;
	}
	ss :: string received = static_cast < const char * > ( addr.AsString ( ) );
	Via & v = vv.front ( );
	if ( ! v.getRport ( ) ) {
		const ss :: string & u = m.getUserAgent ( );
		bool found = false;
		PSYSTEMLOG ( Info, "user agent " << u );
		for ( StringVector :: const_iterator i = rportAgents.begin ( ); i != rportAgents.end ( ); ++ i ) {
			PSYSTEMLOG ( Info, "searching " << * i );
			if ( u.find ( * i ) != ss :: string :: npos ) {
				PSYSTEMLOG ( Info, "found" );
				found = true;
				break;
			}
		}
		if ( ! found )
			v.setRport ( );
	}
	if ( ! v.getReceived ( ).empty ( ) || v.getHost ( ) != received || v.getRport ( ) )
		v.setReceived ( received );
	if ( v.getRport ( ) )
		v.setRport ( port );
	Request :: Methods method = r.getMethod ( );
	if ( oldRfcBranch ( v.getBranch ( ) ) ) {
		Response rsp ( r, Response :: scFailureBadRequest, ss :: string ( ), "Branch without z9hG4bK" );
		sendMessage ( rsp );
		if ( method == Request :: mBye )
			coreServerByeHandler.requestReceived ( r );
		return;
	}
	AutoMutex am ( mut );
	ServerTransactionId id ( r );
	ServerTransactionMap :: iterator i = serverTransactions.find ( id );
	if ( i != serverTransactions.end ( ) ) {
		i -> requestReceived ( r );
		return;
	}
	if ( m.getTo ( ).getTag ( ).empty ( ) &&
		serverTransactions.get < MergedTransactionId > ( ).count ( MergedTransactionId ( r ) ) ) {
		Response rsp ( r, Response :: scFailureLoopDetected );
		sendMessage ( rsp );
		return;
	}
	switch ( method ) {
		case Request :: mAck:
		case Request :: mBye:
		case Request :: mCancel:
		case Request :: mInfo:
		case Request :: mInvite:
		case Request :: mRegister:
			break;
		default: {
			coreServerUnknownMethodHandler.requestReceived ( r );
			return;
		}
	}
	if ( ! m.getRequire ( ).empty ( ) ) {
		coreServerBadExtensionHandler.requestReceived ( r );
		return;
	}
	switch ( method ) {
		case Request :: mAck:
			coreServerAckHandler.requestReceived ( r );
			return;
		case Request :: mBye:
			coreServerByeHandler.requestReceived ( r );
			return;
		case Request :: mCancel:
			coreServerCancelHandler.requestReceived ( r );
			return;
		case Request :: mInfo:
			coreServerInfoHandler.requestReceived ( r );
			return;
		case Request :: mInvite:
			coreServerInviteHandler.requestReceived ( r );
			return;
		case Request :: mRegister:
			coreServerRegisterHandler.requestReceived ( r );
			return;
		default:
			throw std :: runtime_error ( "wrong switch in handleRequest" );
	}
}

void TransportThread :: Impl :: sendBye ( Dialog & d ) {
	Request bye ( d, Request :: mBye );
	ClientTransactionId id ( bye );
	if ( clientTransactions.count ( id ) )
		throw std :: runtime_error ( "duplicate client transaction" );
	Pointer < ClientTransaction > p = new NonInviteClientTransaction ( bye );
	clientTransactions [ id ] = p;
}

void TransportThread :: Impl :: sendCancel ( Dialog & d, int snum, const ss :: string & branch ) {
	Request cancel ( d, Request :: mCancel, snum, branch );
	ClientTransactionId id ( cancel );
	if ( clientTransactions.count ( id ) )
		throw std :: runtime_error ( "duplicate client transaction" );
	Pointer < ClientTransaction > p = new NonInviteClientTransaction ( cancel );
	clientTransactions [ id ] = p;
}

TransportThread :: Impl :: Impl ( ) : coreClientInviteHandler ( this ), coreClientRegisterHandler ( this ),
	coreServerUnknownMethodHandler ( this ), coreServerBadExtensionHandler ( this ), coreServerAckHandler ( this ),
	coreServerByeHandler ( this ), coreServerCancelHandler ( this ), coreServerInfoHandler ( this ),
	coreServerInviteHandler ( this ), coreServerRegisterHandler ( this ), confReloaded ( false ) {
	while ( ! sock.Listen ( PIPSocket :: Address ( "*" ), 5, 5060, PSocket :: CanReuseAddress ) ) {
		PSYSTEMLOG ( Error, "Listen failed on sip socket: " << sock.GetErrorText ( ) );
		if ( conf -> shuttingDown ( ) )
			break;
		PThread :: Sleep ( 1000 );
	}
}

void TransportThread :: Impl :: reloadcb ( ) {
	confReloaded = true;
}

void TransportThread :: Impl :: Main ( ) {
	PSYSTEMLOG ( Info, "sip transport thread begin" );
	conf -> registerReloadNotifier ( std :: tr1 :: bind ( & TransportThread :: Impl :: reloadcb, this ) );
	while ( sock.IsOpen ( ) ) {
		try {
			if ( confReloaded ) {
				confReloaded = false;
				conf -> getRportAgents ( rportAgents );
			}
			checkTimers ( );
			readAndHandleMesg ( );
			
		} catch ( std :: exception & e ) {
			PSYSTEMLOG ( Error, "exception: " << e.what ( ) );
		}
	}
	PSYSTEMLOG ( Info, "sip transport thread end" );
}

bool TransportThread :: Impl :: sendMessage ( const ss :: string & host, int port, const ss :: string & msg ) {
	if ( sock.WriteTo ( msg.data ( ), PINDEX ( msg.size ( ) ), PIPSocket :: Address ( host.c_str ( ) ), WORD ( port ) ) )
		return true;
	PSYSTEMLOG ( Error, "error sending: " << sock.GetErrorText ( ) );
	return false;
}

static ss :: string getLocalAddress ( ) {
	PIPSocket :: Address addr;
	PIPSocket :: GetHostAddress ( addr );
	return static_cast < const char * > ( addr.AsString ( ) );
}

bool TransportThread :: Impl :: sendMessage ( Request & r ) {
	const URI * u = & r.getRequestUri ( );
	RequestMIMEInfo & m = r.getMime ( );
	if ( u -> hasParameter ( "lr" ) ) {
		const RouteHeaderVector & rr = m.getRoute ( );
		if ( ! rr.empty ( ) )
			u = & rr [ 0 ].getUri ( );
	}
	m.getVia ( ) [ 0 ].setHost ( getLocalAddress ( ) );
	const ss :: string & host = u -> getHost ( );
	int port = u -> getPort ( ) ? : 5060;
	PIPSocket :: Address addr ( host.c_str ( ) );
	Log -> LogSIP2Msg ( r, OpengateLog :: Sending, addr, WORD ( port ), false );
	ss :: ostringstream os;
	os << r;
	return sendMessage ( host, port, os.str ( ) );
}

bool TransportThread :: Impl :: sendMessage ( const Response & r ) {
	const Via & v = r.getMime ( ).getVia ( ).front ( );
	const ss :: string & host = getFromAddr ( v );
	int port = getFromPort ( v );
	PIPSocket :: Address addr ( host.c_str ( ) );
	Log -> LogSIP2Msg ( r, OpengateLog :: Sending, addr, WORD ( port ), false );
	ss :: ostringstream os;
	os << r;
	return sendMessage ( host, port, os.str ( ) );
}

void TransportThread :: Impl :: addDigestResponse ( Request & r, const ss :: string & rm, const ss :: string & login, const ss :: string & pass ) {
	RealmInfo & realm = realmMap.find ( rm ) -> second;
	unsigned nc;
	if ( realm.lastNonce == realm.challenge.getNonce ( ) )
		nc = ++ realm.nc;
	else {
		realm.lastNonce = realm.challenge.getNonce ( );
		nc = realm.nc = 1;
	}
	DigestResponse dr ( realm.challenge, r, nc, login, pass );
	r.getMime ( ).addAuthorization ( dr );
}

ClientTransactionId TransportThread :: Impl :: sendRegister ( const ss :: string & host, const ss :: string & user, int port,
	const ss :: string & localHost, const ss :: string & callId, unsigned cseq, unsigned expires, const StringVector & realms,
	const ss :: string & pass, OriginateHandler * h ) {
	Request r ( host, user, port, callId, cseq );
	URI t;
	t.setHost ( localHost );
	t.setUser ( user );
	ContactHeader c;
	c.setUri ( t );
	c.setExpires ( expires );
	r.getMime ( ).setContact ( c );
	ClientTransactionId id ( r );
	StringVector passwords;
	passwords.reserve ( realms.size ( ) );
	AutoMutex am ( mut );
	if ( clientTransactions.count ( id ) )
		throw std :: runtime_error ( "duplicate client transaction" );
	for ( std :: size_t i = 0, rs = realms.size ( ); i < rs; i ++ ) {
		addDigestResponse ( r, realms [ i ], user, pass );
		passwords.push_back ( pass );
	}
	Pointer < ClientTransaction > p = new NonInviteClientTransaction ( r, & coreClientRegisterHandler, h );
	p -> saveAuth ( passwords );
	clientTransactions [ id ] = p;
	return id;
}

ClientTransactionId TransportThread :: Impl :: originateCall ( const ss :: string & host, const ss :: string & user,
	int port, const ss :: string & localHost, const ss :: string & localUser,
	const ss :: string & callId, unsigned maxForwards, unsigned cseq, const StringVector & realms,
	const ss :: string & remotePartyID, SDP :: SessionDescription * sdp, OriginateHandler * h,
	const ss :: string & passertid, const ss :: string & privac ) {
	SIP2 :: Request r ( host, user, port, localHost, localUser, callId, maxForwards, cseq, sdp );
	StringVector logins, passwords;
	logins.reserve ( realms.size ( ) );
	passwords.reserve ( realms.size ( ) );
	for ( std :: size_t i = 0, rs = realms.size ( ); i < rs; i ++ ) {
		ss :: string gkLogin = "anonymous", gkPswd;
		int gkPort;
		conf -> getGkInfo ( host, gkLogin, gkPswd, gkPort );
		logins.push_back ( gkLogin );
		passwords.push_back ( gkPswd );
	}
	r.getMime ( ).setRemotePartyID ( remotePartyID );
	r.getMime ( ).setPAssertID ( passertid );
	r.getMime ( ).setPrivacy ( privac );
	ClientTransactionId id ( r );
	AutoMutex am ( mut );
	if ( clientTransactions.count ( id ) )
		throw std :: runtime_error ( "duplicate client transaction" );
	for ( std :: size_t i = 0, rs = realms.size ( ); i < rs; i ++ )
		addDigestResponse ( r, realms [ i ], logins [ i ], passwords [ i ] );
	Pointer < ClientTransaction > p = new InviteClientTransaction ( r, coreClientInviteHandler, h );
	p -> saveAuth ( passwords );
	clientTransactions [ id ] = p;
	return id;
}

void TransportThread :: Impl :: detachHandler ( const ClientTransactionId & id ) {
	AutoMutex am ( mut );
	ClientTransactionMap :: iterator i = clientTransactions.find ( id );
	if ( i != clientTransactions.end ( ) )
		i -> second -> detachHandler ( );
	if ( id.method == Request :: mInvite ) {
		ClientDialogByTransaction & byTrans = clientDialogs.get < ClientTransactionId > ( );
		std :: pair < ClientDialogByTransaction :: iterator, ClientDialogByTransaction :: iterator > eqr =
			byTrans.equal_range ( id );
		bool transactionCancelled = i == clientTransactions.end ( );
		// pri lagah bivaet neskolko otvetov s raznim dialogami, a cancel moget bit tolko odin
		// plus nekotorie mogut bit ne confirmed pri tom chto transakcii uge net
		for ( ClientDialogByTransaction :: iterator j = eqr.first; j != eqr.second; ) {
			if ( j -> isConfirmed ( ) ) {
				sendBye ( j -> getDialog ( ) );
				clientDialogs.get < ClientTransactionId > ( ).erase ( j ++ );
			} else {
				if ( ! transactionCancelled ) {
					const RequestMIMEInfo & m = i -> second -> getRequest ( ).getMime ( );
					sendCancel ( j -> getDialog ( ), m.getCseq ( ).seq, m.getVia ( ).front ( ).getBranch ( ) );
					transactionCancelled = true;
				}
				if ( i == clientTransactions.end ( ) )
					byTrans.erase ( j ++ );
				else
					byTrans.modify ( j ++, std :: tr1 :: bind ( & ClientDialog :: detachOriginateHandler,
						std :: tr1 :: placeholders :: _1 ) );
			}
		}
	}
}

void TransportThread :: Impl :: sendRinging ( const ServerTransactionId & id, int localRtpPort,
	const PIPSocket :: Address & localAddr, const CodecInfo & inCodec ) {
	AutoMutex am ( mut );
	ServerDialogByTransaction & byTrans = serverDialogs.get < ServerTransactionId > ( );
	ServerDialogByTransaction :: iterator di = byTrans.find ( id );
	if ( di == byTrans.end ( ) ) {
		PSYSTEMLOG ( Error, "sendringing for unknown dialog" );
		return;
	}
	ServerTransactionMap :: iterator ti = serverTransactions.find ( id );
	if ( ti == serverTransactions.end ( ) ) {
		PSYSTEMLOG ( Error, "sendringing for unknown transaction" );
		return;
	}
	Response ringing ( ti -> getRequest ( ), Response :: scInformationRinging, di -> getDialog ( ) );
	if ( inCodec.getCodec ( ) != "unknown" ) {
		ss :: string addr;
		if ( localAddr == INADDR_ANY )
			getLocalAddress ( ).swap ( addr );
		else
			addr = static_cast < const char * > ( localAddr.AsString ( ) );
		std :: auto_ptr < SDP :: SessionDescription > sdp ( new SDP :: SessionDescription ( addr ) );
		sdp -> setConnectionAddress ( addr ); // TODO: nado sdelat razdelenie na media i signal kak v originatehandler
		SDP :: MediaDescription media ( SDP :: mediaAudio, localRtpPort, SDP :: protoRtpAvp );
		addFormat ( media, inCodec );
		sdp -> addMediaDescription ( media );
		ringing.setSdp ( sdp.release ( ) );
	}
	if ( ! ti -> responseSent ( ringing ) ) {
		AnswerHandler * h = di -> getAnswerHandler ( );
		h -> disconnectReceived ( Response :: scFailureServiceUnavailable );
		byTrans.erase ( di );
		serverTransactions.erase ( ti );
	}
}

void TransportThread :: Impl :: sendOk ( const ServerTransactionId & id, int localRtpPort,
	const PIPSocket :: Address & localAddr, const CodecInfo & inCodec, unsigned telephoneEventsPayloadType ) {
	AutoMutex am ( mut );
	ServerDialogByTransaction & byTrans = serverDialogs.get < ServerTransactionId > ( );
	ServerDialogByTransaction :: iterator di = byTrans.find ( id );
	if ( di == byTrans.end ( ) ) {
		PSYSTEMLOG ( Error, "sendok for unknown dialog" );
		return;
	}
	ServerTransactionMap :: iterator ti = serverTransactions.find ( id );
	if ( ti == serverTransactions.end ( ) ) {
		PSYSTEMLOG ( Error, "sendok for unknown transaction" );
		return;
	}
	Response ok ( ti -> getRequest ( ), Response :: scSuccessfulOK, di -> getDialog ( ) );
	if ( inCodec.getCodec ( ) != "unknown" ) {
		ss :: string addr;
		if ( localAddr == INADDR_ANY )
			getLocalAddress ( ).swap ( addr );
		else
			addr = static_cast < const char * > ( localAddr.AsString ( ) );
		std :: auto_ptr < SDP :: SessionDescription > sdp ( new SDP :: SessionDescription ( addr ) );
		sdp -> setConnectionAddress ( addr ); // TODO: nado sdelat razdelenie na media i signal kak v originatehandler
		SDP :: MediaDescription media ( SDP :: mediaAudio, localRtpPort, SDP :: protoRtpAvp );
		addFormat ( media, inCodec );
		if ( telephoneEventsPayloadType )
			addTelephoneEvents ( media, telephoneEventsPayloadType );
		sdp -> addMediaDescription ( media );
		ok.setSdp ( sdp.release ( ) );
	}
	if ( const DigestResponse * d = ti -> getSavedDigestResponse ( ) )
		serverAuthHandler.addAuthenticationInfo ( * d, ok, ti -> getSavedPassword ( ) );
	di -> setOk ( ok );
	ti -> responseSent ( ok );
	serverTransactions.erase ( ti );
}

void TransportThread :: Impl :: detachHandlerInt ( const ServerDialogMap :: iterator & j, int c ) {
	if ( j -> isConfirmed ( ) ) {
		sendBye ( j -> getDialog ( ) );
		serverDialogs.erase ( j );
		return;
	}
	ServerTransactionMap :: iterator i = serverTransactions.find ( j -> getTransactionId ( ) );
	if ( i == serverTransactions.end ( ) ) { // not acked yet
		serverDialogs.modify ( j, std :: tr1 :: bind ( & ServerDialog :: detachAnswerHandler,
			std :: tr1 :: placeholders :: _1 ) );
		return;
	}
	if ( i -> getResponse ( ).getStatusCode ( ) >= 200 )
		return;
	Response rsp ( i -> getRequest ( ), Response :: StatusCodes ( c ), j -> getDialog ( ) );
	if ( ! i -> responseSent ( rsp ) ) {
		serverTransactions.erase ( i );
		serverDialogs.erase ( j );
	} else
		serverDialogs.modify ( j, std :: tr1 :: bind ( & ServerDialog :: detachAnswerHandler,
			std :: tr1 :: placeholders :: _1 ) );
}

void TransportThread :: Impl :: detachHandler ( const ServerTransactionId & id, int c ) {
	if ( id.method != Request :: mInvite )
		return;
	AutoMutex am ( mut );
	ServerDialogByTransaction & byTrans = serverDialogs.get < ServerTransactionId > ( );
	ServerDialogByTransaction :: iterator j = byTrans.find ( id );
	if ( j == byTrans.end ( ) )
		return;
	detachHandlerInt ( serverDialogs.iterator_to ( * j ), c );
}

StringVector TransportThread :: Impl :: saveChallenges ( const DigestChallengeVector & v ) {
	StringVector realms;
	realms.reserve ( v.size ( ) );
	for ( std :: size_t i = 0, vs = v.size ( ); i < vs; i ++ ) {
		const DigestChallenge & challenge = v [ i ];
		const ss :: string & realm = challenge.getRealm ( );
		RealmMap :: iterator j = realmMap.find ( realm );
		if ( j == realmMap.end ( ) )
			realmMap.insert ( std :: make_pair ( realm, RealmInfo ( challenge ) ) );
		else
			j -> second.challenge = challenge;
		realms.push_back ( realm );
	}
	return realms;
}

void TransportThread :: Impl :: checkAuthenticationInfo ( const Response & r, const ClientTransaction * i ) {
	const ResponseMIMEInfo & m = r.getMime ( );
	if ( ! m.hasAuthenticationInfo ( ) )
		return;
	const AuthenticationInfo & ai = m.getAuthenticationInfo ( );
	const DigestResponseVector & v = i -> getRequest ( ).getMime ( ).getAuthorization ( );
	const StringVector & p = i -> getSavedPasswords ( );
	StringVector :: size_type s = std :: min ( v.size ( ), p.size ( ) );
	for ( StringVector :: size_type j = 0; j < s; j ++ ) {
		const DigestResponse & d = v [ j ];
		if ( d.getCnonce ( ) == ai.getCnonce ( ) && d.getNc ( ) == ai.getNc ( ) ) {
			if ( ai.calcDigest ( p [ j ], r, d ) != ai.getRspauth ( ) ) {
				PSYSTEMLOG ( Error, "rspauth doesnt match " << ai ); // FIXME: tut nado razrivat
				return;
			}
			const ss :: string & nn = ai.getNextnonce ( );
			if ( ! nn.empty ( ) ) {
				RealmMap :: iterator j = realmMap.find ( d.getRealm ( ) );
				if ( j == realmMap.end ( ) )
					PSYSTEMLOG ( Error, "lost realmmap " << d.getRealm ( ) );
				else
					j -> second.challenge.setNonce ( nn );
			}
			return;
		}
	}
	PSYSTEMLOG ( Error, "authenticationinfo doesnt match " << ai ); // FIXME: tut nado razrivat
}

ClientTransactionId TransportThread :: Impl :: sendOnhold ( const ClientTransactionId & inviteId, int level, OriginateHandler * h ) {
	AutoMutex am ( mut );
	std :: pair < ClientDialogByTransaction :: iterator, ClientDialogByTransaction :: iterator > eqr =
		clientDialogs.get < ClientTransactionId > ( ).equal_range ( inviteId );
	for ( ClientDialogByTransaction :: iterator i = eqr.first; i != eqr.second; ) {
		if ( ! i -> isConfirmed ( ) )
			continue;
		const SDP :: SessionDescription * s = i -> getSentSdp ( );
		if ( ! s )
			continue;
		std :: auto_ptr < SDP :: SessionDescription > sdp ( new SDP :: SessionDescription ( * s ) );
		const SDP :: MediaDescription * media = sdp -> getMediaDescription ( SDP :: mediaAudio );
		if ( ! media )
			continue;
		SDP :: MediaDescription * m = const_cast < SDP :: MediaDescription * > ( media );
		m -> delAttributeField ( "sendonly" );
		m -> delAttributeField ( "recvonly" );
		m -> delAttributeField ( "sendrecv" );
		m -> delAttributeField ( "inactive" );
		if ( level & 4 ) {
			m -> clearConnectionField ( );
			m -> addConnectionField ( "0.0.0.0" );
		}
		if ( ( level & 3 ) == 3 )
			m -> addAttributeField ( "inactive", ss :: string ( ) );
		else if ( level & 1 )
			m -> addAttributeField ( "sendonly", ss :: string ( ) );
		Request r ( i -> getDialog ( ), Request :: mInvite );
		ClientTransactionId id ( r );
		if ( clientTransactions.count ( id ) )
			throw std :: runtime_error ( "duplicate client transaction" );
		r.setSdp ( sdp.release ( ) );
		Pointer < ClientTransaction > p = new InviteClientTransaction ( r, coreClientInviteHandler, h );
		clientTransactions [ id ] = p;
		return id;
		// TODO: auth, handler for request pending
	}
	throw std :: runtime_error ( "cant send onhold" );
}

ClientTransactionId TransportThread :: Impl :: sendOnhold ( const ServerTransactionId & inviteId, int level, AnswerHandler * /*h*/ ) {
	AutoMutex am ( mut );
	ServerDialogByTransaction & byTrans = serverDialogs.get < ServerTransactionId > ( );
	ServerDialogByTransaction :: iterator di = byTrans.find ( inviteId );
	if ( di == byTrans.end ( ) )
		throw std :: runtime_error ( "sendonhold for unknown dialog" );
	if ( ! di -> isConfirmed ( ) )
		throw std :: runtime_error ( "sendonhold for unknown dialog" );
		//TODO: esli takoe bivaet to nado slat posle confirma
	const SDP :: SessionDescription * s = di -> getSentSdp ( );
	if ( ! s )
		throw std :: runtime_error ( "sendonhold w/o sentsdp" );
	std :: auto_ptr < SDP :: SessionDescription > sdp ( new SDP :: SessionDescription ( * s ) );
	const SDP :: MediaDescription * media = sdp -> getMediaDescription ( SDP :: mediaAudio );
	if ( ! media )
		throw std :: runtime_error ( "sendonhold w/o media" );
	SDP :: MediaDescription * m = const_cast < SDP :: MediaDescription * > ( media );
	m -> delAttributeField ( "sendonly" );
	m -> delAttributeField ( "recvonly" );
	m -> delAttributeField ( "sendrecv" );
	m -> delAttributeField ( "inactive" );
	if ( level & 4 ) {
		m -> clearConnectionField ( );
		m -> addConnectionField ( "0.0.0.0" );
	}
	if ( ( level & 3 ) == 3 )
		m -> addAttributeField ( "inactive", ss :: string ( ) );
	else if ( level & 1 )
		m -> addAttributeField ( "sendonly", ss :: string ( ) );
	Request r ( di -> getDialog ( ), Request :: mInvite );
	ClientTransactionId id ( r );
	if ( clientTransactions.count ( id ) )
		throw std :: runtime_error ( "duplicate client transaction" );
	r.setSdp ( sdp.release ( ) );
	Pointer < ClientTransaction > p = new InviteClientTransaction ( r, coreClientInviteHandler, 0/*h*/ );
	//FIXME: tut mi ne mogem peredat h, t.k. on answerhandler, a nado originate
	clientTransactions [ id ] = p;
	return id;
	// TODO: auth, handler for request pending
}

ClientTransactionId TransportThread :: Impl :: sendDtmf ( const ClientTransactionId & inviteId, const DTMF :: Relay & relay ) {
	AutoMutex am ( mut );
	PSYSTEMLOG(Info, "TransportThread :: Impl :: sendDtmf: client");
	std :: pair < ClientDialogByTransaction :: iterator, ClientDialogByTransaction :: iterator > eqr =
		clientDialogs.get < ClientTransactionId > ( ).equal_range ( inviteId );
	for ( ClientDialogByTransaction :: iterator i = eqr.first; i != eqr.second; ++ i ) {
		if ( ! i -> isConfirmed ( ) ) {
			PSYSTEMLOG ( Info, "skipping not confirmed dialog " << i -> getDialogId ( ) );
			continue;
		}
		Dialog & d = i -> getDialog ( );
		if ( ! d.getAllowsInfo ( ) ) {
			PSYSTEMLOG ( Error, "not allows info " << d.getId ( ) );
			continue;
		}
		if ( ! d.getAcceptsDtmf ( ) ) {
			PSYSTEMLOG ( Error, "not accepts dtmf " << d.getId ( ) );
			continue;
		}
		bool supportsPackage = d.getSupportsPackage ( );
		if ( supportsPackage && ! d.getSupportsDtmf ( ) ) {
			PSYSTEMLOG ( Error, "supports info packages but not dtmf " << d.getId ( ) );
			continue;
		}
		Request r ( d, Request :: mInfo );
		ClientTransactionId id ( r );
		if ( clientTransactions.count ( id ) )
			throw std :: runtime_error ( "duplicate client transaction" );
		r.setDtmfRelay ( relay, supportsPackage );
		Pointer < ClientTransaction > p = new NonInviteClientTransaction ( r );
		clientTransactions [ id ] = p;
		return id;
	}
	throw std :: runtime_error ( "cant send dtmf" );
}

ClientTransactionId TransportThread :: Impl :: sendDtmf ( const ServerTransactionId & inviteId, const DTMF :: Relay & relay ) {
	PSYSTEMLOG(Info, "TransportThread :: Impl :: sendDtmf: server");
	AutoMutex am ( mut );
	ServerDialogByTransaction & byTrans = serverDialogs.get < ServerTransactionId > ( );
	ServerDialogByTransaction :: iterator di = byTrans.find ( inviteId );
	if ( di == byTrans.end ( ) )
		throw std :: runtime_error ( "senddtmf for unknown dialog" );
	Dialog & d = di -> getDialog ( );
	if ( ! d.getAllowsInfo ( ) )
		throw std :: runtime_error ( "not allows info " );
	if ( ! d.getAcceptsDtmf ( ) )
		throw std :: runtime_error ( "not accepts dtmf " );
	bool supportsPackage = d.getSupportsPackage ( );
	if ( supportsPackage && ! d.getSupportsDtmf ( ) )
		throw std :: runtime_error ( "supports info packages but not dtmf " );
	Request r ( d, Request :: mInfo );
	ClientTransactionId id ( r );
	if ( clientTransactions.count ( id ) )
		throw std :: runtime_error ( "duplicate client transaction" );
	r.setDtmfRelay ( relay, supportsPackage );
	Pointer < ClientTransaction > p = new NonInviteClientTransaction ( r );
	clientTransactions [ id ] = p;
	return id;
}

TransportThread :: TransportThread ( ) : PThread ( 1000, NoAutoDeleteThread, NormalPriority, "SIP :: TransportThread" ),
	impl ( new Impl ) {
	transportThread = this;
	Resume ( );
}

TransportThread :: ~TransportThread ( ) {
	transportThread = 0;
	delete impl;
}

void TransportThread :: Main ( ) {
	impl -> Main ( );
}

void TransportThread :: Close ( ) {
	impl -> Close ( );
}

bool TransportThread :: sendMessage ( const ss :: string & host, int port, const ss :: string & msg ) {
	return impl -> sendMessage ( host, port, msg );
}

bool TransportThread :: sendMessage ( Request & r ) {
	return impl -> sendMessage ( r );
}

bool TransportThread :: sendMessage ( const Response & r ) {
	return impl -> sendMessage ( r );
}

ClientTransactionId TransportThread :: originateCall ( const ss :: string & host, const ss :: string & user,
	int port, const ss :: string & localHost, const ss :: string & localUser,
	const ss :: string & callId, unsigned maxForwards, unsigned cseq, const StringVector & realms,
	const ss :: string & remotePartyID, SDP :: SessionDescription * sdp, OriginateHandler * h,
	const ss :: string & passertid, const ss :: string & privac ) {
	return impl -> originateCall ( host, user, port, localHost, localUser, callId, maxForwards, cseq, realms, remotePartyID, sdp, h, passertid, privac );
}

ClientTransactionId TransportThread :: sendRegister ( const ss :: string & host, const ss :: string & user, int port,
	const ss :: string & localHost, const ss :: string & callId, unsigned cseq, unsigned expires, const StringVector & realms,
	const ss :: string & pass, OriginateHandler * h ) {
	return impl -> sendRegister ( host, user, port, localHost, callId, cseq, expires, realms, pass, h );
}

void TransportThread :: detachHandler ( const ClientTransactionId & id ) {
	impl -> detachHandler ( id );
}

void TransportThread :: sendRinging ( const ServerTransactionId & id, int localRtpPort,
	const PIPSocket :: Address & localAddr, const CodecInfo & inCodec ) {
	impl -> sendRinging ( id, localRtpPort, localAddr, inCodec );
}

void TransportThread :: sendOk ( const ServerTransactionId & id, int localRtpPort,
	const PIPSocket :: Address & localAddr, const CodecInfo & inCodec, unsigned telephoneEventsPayloadType ) {
	impl -> sendOk ( id, localRtpPort, localAddr, inCodec, telephoneEventsPayloadType );
}

void TransportThread :: detachHandler ( const ServerTransactionId & id, int c ) {
	impl -> detachHandler ( id, c );
}

ClientTransactionId TransportThread :: sendOnhold ( const ClientTransactionId & inviteId, int level, OriginateHandler * h ) {
	return impl -> sendOnhold ( inviteId, level, h );
}

ClientTransactionId TransportThread :: sendOnhold ( const ServerTransactionId & inviteId, int level, AnswerHandler * h ) {
	return impl -> sendOnhold ( inviteId, level, h );
}

ClientTransactionId TransportThread :: sendDtmf ( const ClientTransactionId & inviteId, const DTMF :: Relay & r ) {
	return impl -> sendDtmf ( inviteId, r );
}

ClientTransactionId TransportThread :: sendDtmf ( const ServerTransactionId & inviteId, const DTMF :: Relay & r ) {
	return impl -> sendDtmf ( inviteId, r );
}

TransportThread * transportThread = 0;

void CoreClientInviteHandler :: strayResponseReceived ( const Response & r ) const {
	if ( r.getStatusCode ( ) < 200 || r.getStatusCode ( ) >= 300 )
		return;
	DialogId id = makeDialogIdReceived ( r );
	ClientDialogMap :: iterator i = t -> clientDialogs.find ( id );
	if ( i == t -> clientDialogs.end ( ) ) {
		ClientDialog d ( r, 0, 0 );
		d.sendAck ( r );
		t -> sendBye ( d.getDialog ( ) );
		return;
	}
	bool confirmedBefore = i -> isConfirmed ( );
	i -> targetRefresh ( r );
	i -> sendAck ( r );
	if ( ! confirmedBefore ) {
		t -> sendBye ( i -> getDialog ( ) );
		t -> clientDialogs.erase ( i );
	}
}

void CoreClientInviteHandler :: responseReceived ( const Response & r, const ClientTransaction * c ) const {
	OriginateHandler * h = c -> getHandler ( );
	DialogId id = makeDialogIdReceived ( r );
	const ResponseMIMEInfo & m = r.getMime ( );
	if ( r.getStatusCode ( ) < 200 ) {
		Dialog * d = 0;
		bool inserted = false;
		if ( ! id.remoteTag.empty ( ) ) {
			ClientDialogMap :: const_iterator i = t -> clientDialogs.find ( id );
			if ( i == t -> clientDialogs.end ( ) ) {
				ServerDialogMap :: const_iterator j = t -> serverDialogs.find ( id );
				if ( j == t -> serverDialogs.end ( ) ) {
					Pointer < SDP :: SessionDescription > p;
					if ( const SDP :: SessionDescription * sdp = c -> getRequest ( ).getSdp ( ) )
						p = new SDP :: SessionDescription ( * sdp );
					ClientDialog cd ( r, p, h );
					d = & cd.getDialog ( );
					t -> clientDialogs.insert ( cd );
					inserted = true;
				}
			}
		}
		if ( d ) {
			if ( ! h ) // FIXME: pri reinvite slat ne nado po idee
				t -> sendCancel ( * d, m.getCseq ( ).seq, m.getVia ( ).front ( ).getBranch ( ) );
			else if ( ! inserted ) {
				d -> handleRecvInfo ( m );
				d -> handleAllow ( m );
			}
		}
	} else if ( r.getStatusCode ( ) < 300 ) {
		ClientDialogMap :: iterator i = t -> clientDialogs.find ( id );
		if ( i == t -> clientDialogs.end ( ) ) {
			ServerDialogMap :: iterator i = t -> serverDialogs.find ( id );
			if ( i != t -> serverDialogs.end ( ) ) { //reinvite to originator
				i -> targetRefresh ( r );
				i -> sendAck ( r );
				return;
			}
			Pointer < SDP :: SessionDescription > p;
			if ( const SDP :: SessionDescription * sdp = c -> getRequest ( ).getSdp ( ) )
				p = new SDP :: SessionDescription ( * sdp );
			ClientDialog d ( r, p, h );
			d.sendAck ( r );
			if ( ! h )
				t -> sendBye ( d.getDialog ( ) );
		} else {
			h = i -> getOriginateHandler ( );
			i -> targetRefresh ( r );
			i -> sendAck ( r );
			if ( ! h ) {
				t -> sendBye ( i -> getDialog ( ) );
				t -> clientDialogs.erase ( i );
			}
		}
	} else
		t -> clientDialogs.get < ClientTransactionId > ( ).erase ( ClientTransactionId ( r ) );
	if ( ! h )
		return;
	if ( r.getStatusCode ( ) == Response :: scInformationTrying ) {
		h -> tryingReceived ( );
		return;
	}
	if ( r.getStatusCode ( ) < 200 ) {
		h -> ringingReceived ( r );
		return;
	}
	if ( r.getStatusCode ( ) < 300 ) {
		t -> checkAuthenticationInfo ( r, c );
		h -> okReceived ( r );
		return;
	}
	if ( r.getStatusCode ( ) == Response :: scFailureUnAuthorized ) {
		StringVector realms = t -> saveChallenges ( r.getMime ( ).getWwwAuthenticate ( ) );
		h -> unauthorizedReceived ( realms );
		return;
	}
	h -> disconnectReceived ( r.getStatusCode ( ), r.getMime ( ) );
}

static ResponseMIMEInfo emptyMime;

void CoreClientInviteHandler :: transactionTimedOut ( const ClientTransaction * c ) const {
	OriginateHandler * h = c -> getHandler ( );
	const ClientTransactionId & id = c -> getId ( );
	t -> clientDialogs.get < ClientTransactionId > ( ).erase ( id );
	if ( h )
		h -> disconnectReceived ( Response :: scFailureRequestTimeout, emptyMime );
}

void CoreClientInviteHandler :: transactionTransportError ( const ClientTransaction * c ) const {
	OriginateHandler * h = c -> getHandler ( );
	const ClientTransactionId & id = c -> getId ( );
	t -> clientDialogs.get < ClientTransactionId > ( ).erase ( id );
	if ( h )
		h -> disconnectReceived ( Response :: scFailureServiceUnavailable, emptyMime );
}

void CoreClientInviteHandler :: transactionFinished ( const ClientTransaction * c ) const {
	OriginateHandler * h = c -> getHandler ( );
	const ClientTransactionId & id = c -> getId ( );
	bool found = false;
	std :: pair < ClientDialogByTransaction :: iterator, ClientDialogByTransaction :: iterator > eqr =
		t -> clientDialogs.get < ClientTransactionId > ( ).equal_range ( id );
	for ( ClientDialogByTransaction :: iterator j = eqr.first; j != eqr.second; ) {
		if ( ! j -> isConfirmed ( ) )
			j = t -> clientDialogs.get < ClientTransactionId > ( ).erase ( j );
		else {
			found = true;
			++ j;
		}
	}
	if ( ! found && h )
		h -> disconnectReceived ( Response :: scFailureRequestTimeout, emptyMime );
}

void CoreClientRegisterHandler :: responseReceived ( const Response & r, const ClientTransaction * c ) const {
	if ( r.getStatusCode ( ) < 200 )
		return;
	OriginateHandler * h = c -> getHandler ( );
	if ( ! h )
		return;
	if ( r.getStatusCode ( ) < 300 ) {
		t -> checkAuthenticationInfo ( r, c );
		h -> okReceived ( r );
		return;
	}
	if ( r.getStatusCode ( ) == Response :: scFailureUnAuthorized ) {
		StringVector realms = t -> saveChallenges ( r.getMime ( ).getWwwAuthenticate ( ) );
		h -> unauthorizedReceived ( realms );
		return;
	}
	h -> disconnectReceived ( r.getStatusCode ( ), r.getMime ( ) );
}

void CoreClientRegisterHandler :: transactionTimedOut ( const ClientTransaction * c ) const {
	if ( OriginateHandler * h = c -> getHandler ( ) )
		h -> disconnectReceived ( Response :: scFailureRequestTimeout, emptyMime );
}

void CoreClientRegisterHandler :: transactionTransportError ( const ClientTransaction * c ) const {
	if ( OriginateHandler * h = c -> getHandler ( ) )
		h -> disconnectReceived ( Response :: scFailureServiceUnavailable, emptyMime );
}

void CoreServerUnknownMethodHandler :: requestReceived ( const Request & r ) {
	Pointer < ServerTransaction > p = new NonInviteServerTransaction ( r );
	Response rsp ( r, Response :: scFailureMethodNotAllowed );
	if ( p -> responseSent ( rsp ) )
		t -> serverTransactions.insert ( ServerTransactionNode ( p ) );
}

void CoreServerBadExtensionHandler :: requestReceived ( const Request & r ) {
	Pointer < ServerTransaction > p;
	if ( r.getMethod ( ) == Request :: mInvite )
		p = new InviteServerTransaction ( r );
	else
		p = new NonInviteServerTransaction ( r );
	Response rsp ( r, Response :: scFailureBadExtension );
	const StringVector & v = r.getMime ( ).getRequire ( );
	ResponseMIMEInfo & m = rsp.getMime ( );
	for ( std :: size_t i = 0, vs = v.size ( ); i < vs; i ++ )
		if ( v [ i ] != "timer" )
			m.addUnsupported ( v [ i ] );
	if ( p -> responseSent ( rsp ) )
		t -> serverTransactions.insert ( ServerTransactionNode ( p ) );
}

void CoreServerByeHandler :: requestReceived ( const Request & r ) {
	Pointer < ServerTransaction > p = new NonInviteServerTransaction ( r );
	DialogId id = makeDialogIdReceived ( r );
	ClientDialogMap :: iterator i = t -> clientDialogs.find ( id );
	if ( i != t -> clientDialogs.end ( ) ) {
		Response rsp ( r,
			i -> requestReceived ( r ) ? Response :: scSuccessfulOK : Response :: scFailureServerInternalError );
		if ( p -> responseSent ( rsp ) )
			t -> serverTransactions.insert ( ServerTransactionNode ( p ) );
		if ( OriginateHandler * h = i -> getOriginateHandler ( ) )
			h -> disconnectReceived ( Response :: scSuccessfulOK, emptyMime );
		t -> clientDialogs.erase ( i );
		return;
	}
	ServerDialogMap :: iterator si = t -> serverDialogs.find ( id );
	if ( si == t -> serverDialogs.end ( ) ) {
		Response rsp ( r, Response :: scFailureTransactionDoesNotExist );
		if ( p -> responseSent ( rsp ) )
			t -> serverTransactions.insert ( ServerTransactionNode ( p ) );
		return;
	}
	Response rs ( r, si -> requestReceived ( r ) ? Response :: scSuccessfulOK : Response :: scFailureServerInternalError );
	if ( p -> responseSent ( rs ) )
		t -> serverTransactions.insert ( ServerTransactionNode ( p ) );
	if ( AnswerHandler * h = si -> getAnswerHandler ( ) )
		h -> disconnectReceived ( Response :: scSuccessfulOK );
	t -> serverDialogs.erase ( si );
}

void CoreServerInfoHandler :: requestReceived ( const Request & r ) {
	Pointer < ServerTransaction > p = new NonInviteServerTransaction ( r );
	DialogId id = makeDialogIdReceived ( r );
	ClientDialogMap :: const_iterator ci = t -> clientDialogs.find ( id );
	const RequestMIMEInfo & m = r.getMime ( );
	ServerDialogMap :: const_iterator si; 
	if ( ci == t -> clientDialogs.end ( ) ) {
		si = t -> serverDialogs.find ( id );
		if ( si == t -> serverDialogs.end ( ) ) {
			Response rsp ( r, Response :: scFailureTransactionDoesNotExist );
			if ( p -> responseSent ( rsp ) )
				t -> serverTransactions.insert ( ServerTransactionNode ( p ) );
			return;

		}
		if ( ! si -> requestReceived ( r ) ) {
			Response rsp ( r, Response :: scFailureServerInternalError );
			if ( p -> responseSent ( rsp ) )
				t -> serverTransactions.insert ( ServerTransactionNode ( p ) );
			return;
		}
	} else {
		if ( ! ci -> requestReceived ( r ) ) {
			Response rsp ( r, Response :: scFailureServerInternalError );
			if ( p -> responseSent ( rsp ) )
				t -> serverTransactions.insert ( ServerTransactionNode ( p ) );
			return;
		}
	}
	InfoPackages ip = m.getInfoPackage ( );
	if ( ip == ipNumPackages ) {
		Response rsp ( r, Response :: scFailureBadEvent );
		if ( p -> responseSent ( rsp ) )
			t -> serverTransactions.insert ( ServerTransactionNode ( p ) );
		if ( ci != t -> clientDialogs.end ( ) ) {
			if ( OriginateHandler * h = ci -> getOriginateHandler ( ) )
				h -> disconnectReceived ( Response :: scFailureBadEvent, emptyMime );
			// on vizovet detachhandler i poshletsya bye/cancel
		} else if ( AnswerHandler * h = si -> getAnswerHandler ( ) ) {
			h -> disconnectReceived ( Response :: scFailureBadEvent );
			//a tut bye ne vizovet
			t -> detachHandlerInt ( si, Response :: scFailureBadEvent );
		}
		return;
	}
	ContentDisposition cd = m.getContentDisposition ( );
	bool emptyBody = r.emptyBody ( );
	if ( ip == ipNil && cd.disposition == ContentDisposition :: missing && emptyBody ) {
		//legacy keepalive
		Response rsp ( r, Response :: scSuccessfulOK );
		if ( p -> responseSent ( rsp ) )
			t -> serverTransactions.insert ( ServerTransactionNode ( p ) );
		return;
	}
	const DTMF :: Relay * relay = r.getDtmfRelay ( );
	if ( ( ip == ipDtmf && cd.disposition == ContentDisposition :: infoPackage && relay ) ||
		( ip == ipNil && relay ) ) {
		Response rsp ( r, Response :: scSuccessfulOK );
		if ( p -> responseSent ( rsp ) )
			t -> serverTransactions.insert ( ServerTransactionNode ( p ) );
		if ( ci != t -> clientDialogs.end ( ) ) {
			if ( OriginateHandler * h = ci -> getOriginateHandler ( ) )
				h -> dtmfRelayReceived ( * relay );
		} else if ( AnswerHandler * h = si -> getAnswerHandler ( ) )
			h -> dtmfRelayReceived ( * relay );
		return;
	}
	Response rsp ( r, ( cd.disposition != ContentDisposition :: missing && ! cd.handlingRequired ?
		Response :: scSuccessfulOK : Response :: scFailureUnsupportedMediaType ) );
	if ( p -> responseSent ( rsp ) )
		t -> serverTransactions.insert ( ServerTransactionNode ( p ) );
}

void CoreServerInviteHandler :: transactionTransportError ( ServerTransaction * c ) {
	const ServerTransactionId & id = c -> getId ( );
	ServerDialogByTransaction & byTrans = t -> serverDialogs.get < ServerTransactionId > ( );
	ServerDialogByTransaction :: iterator i = byTrans.find ( id );
	if ( i == byTrans.end ( ) ) {
		PSYSTEMLOG ( Error, "impossible: transactionTransportError for unknown dialog" );
		return;
	}
	if ( AnswerHandler * h = i -> getAnswerHandler ( ) )
		h -> disconnectReceived ( Response :: scFailureServiceUnavailable );
	byTrans.erase ( i );
}

void CoreServerInviteHandler :: requestReceived ( const Request & r ) {
	DialogId id = makeDialogIdReceived ( r );
	if ( ! id.localTag.empty ( ) ) {
		ServerDialogMap :: iterator is = t -> serverDialogs.find ( id );
		ClientDialogMap :: iterator ic = t -> clientDialogs.find ( id );
		if ( is != t -> serverDialogs.end ( ) ) {
			if ( ! is -> requestReceived ( r ) ) {
				PSYSTEMLOG ( Error, "request out of order" );
				Pointer < ServerTransaction > p = new InviteServerTransaction ( r );
				Response rsp ( r, Response :: scFailureServerInternalError, id.localTag );
				if ( p -> responseSent ( rsp ) )
					t -> serverTransactions.insert ( ServerTransactionNode ( p ) );
				return;
			}
			ServerTransactionMap :: iterator it = t -> serverTransactions.find ( is -> getTransactionId ( ) );
			if ( it != t -> serverTransactions.end ( ) && it -> getResponse ( ).getStatusCode ( ) < 200 ) {
				PSYSTEMLOG ( Error, "reinvite before completion of invite" );
				Pointer < ServerTransaction > p = new InviteServerTransaction ( r );
				Response rsp ( r, Response :: scFailureServerInternalError, id.localTag );
				if ( p -> responseSent ( rsp ) )
					t -> serverTransactions.insert ( ServerTransactionNode ( p ) );
				return;
			}
		} else if ( ic != t -> clientDialogs.end ( ) ) {
			if ( ! ic -> requestReceived ( r ) ) {
				PSYSTEMLOG ( Error, "request out of order" );
				Pointer < ServerTransaction > p = new InviteServerTransaction ( r );
				Response rsp ( r, Response :: scFailureServerInternalError, id.localTag );
				if ( p -> responseSent ( rsp ) )
					t -> serverTransactions.insert ( ServerTransactionNode ( p ) );
				return;
			}
			if ( ! ic -> isConfirmed ( ) ) {
				PSYSTEMLOG ( Error, "reinvite while invite pending" );
				Pointer < ServerTransaction > p = new InviteServerTransaction ( r );
				Response rsp ( r, Response :: scFailureRequestPending, id.localTag );
				rsp.getMime ( ).setRetryAfter ( RetryAfter ( Random :: number ( ) % 11 ) );
				if ( p -> responseSent ( rsp ) )
					t -> serverTransactions.insert ( ServerTransactionNode ( p ) );
				return;
			}
		} else {
			PSYSTEMLOG ( Error, "reinvite for unknown dialog" );
			Pointer < ServerTransaction > p = new InviteServerTransaction ( r );
			Response rsp ( r, Response :: scFailureTransactionDoesNotExist, id.localTag );
			if ( p -> responseSent ( rsp ) )
				t -> serverTransactions.insert ( ServerTransactionNode ( p ) );
			return;
		}
		const SDP :: MediaDescription * media = 0;
		if ( const SDP :: SessionDescription * sdp = r.getSdp ( ) )
			media = sdp -> getMediaDescription ( SDP :: mediaAudio );
		else {
			PSYSTEMLOG ( Error, "reinvite without sdp" );
			Pointer < ServerTransaction > p = new InviteServerTransaction ( r );
			Response rsp ( r, Response :: scFailureNotAcceptableHere, id.localTag );
			if ( p -> responseSent ( rsp ) )
				t -> serverTransactions.insert ( ServerTransactionNode ( p ) );
			return;
		}
		if ( ! media ) {
			PSYSTEMLOG ( Error, "sdp without audio" );
			Pointer < ServerTransaction > p = new InviteServerTransaction ( r );
			Response rsp ( r, Response :: scFailureNotAcceptableHere, id.localTag );
			if ( p -> responseSent ( rsp ) )
				t -> serverTransactions.insert ( ServerTransactionNode ( p ) );
			return;
		}
		unsigned holdlevel = 0;
		if ( media -> getAttributeValue ( "sendonly" ) )
			holdlevel = 1;
		else if ( media -> getAttributeValue ( "inactive") )
			holdlevel = 3;
		ss :: string connectionAddress = r.getSdp ( ) -> getConnectionAddress ( );
		int port = media -> getPort ( );
		if ( const ss :: string * c = media -> getConnectionAddress ( ) ) {
			if ( * c == "0.0.0.0" )
				holdlevel |= 4;
			else
				connectionAddress = * c;
		}
		if ( ! holdlevel && ! media -> getAttributeValue ( "sendrecv" ) ) {
			PSYSTEMLOG ( Error, "not a hold" );
			Pointer < ServerTransaction > p = new InviteServerTransaction ( r );
			Response rsp ( r, Response :: scFailureNotAcceptableHere, id.localTag );
			if ( p -> responseSent ( rsp ) )
				t -> serverTransactions.insert ( ServerTransactionNode ( p ) );
			return;
		}
		std :: auto_ptr < SDP :: SessionDescription > sdp;
		if ( is != t -> serverDialogs.end ( ) ) {
			if ( const SDP :: SessionDescription * s = is -> getSentSdp ( ) )
				sdp.reset ( new SDP :: SessionDescription ( * s ) );
		} else {
			if ( const SDP :: SessionDescription * s = ic -> getSentSdp ( ) )
				sdp.reset ( new SDP :: SessionDescription ( * s ) );
		}
		media = 0;
		if ( sdp.get ( ) )
			media = sdp -> getMediaDescription ( SDP :: mediaAudio );
		if ( media ) {
			SDP :: MediaDescription * m = const_cast < SDP :: MediaDescription * > ( media );
			m -> delAttributeField ( "sendonly" );
			m -> delAttributeField ( "recvonly" );
			m -> delAttributeField ( "sendrecv" );
			m -> delAttributeField ( "inactive" );
			if ( holdlevel & 4 ) {
				m -> clearConnectionField ( );
				m -> addConnectionField ( "0.0.0.0" );
			}
			if ( ( holdlevel & 3 ) == 3 )
				m -> addAttributeField ( "inactive", ss :: string ( ) );
			else if ( holdlevel & 1 )
				m -> addAttributeField ( "recvonly", ss :: string ( ) );
		}
		Pointer < ServerTransaction > p = new InviteServerTransaction ( r );
		Response rsp ( r, Response :: scSuccessfulOK, id.localTag );
		rsp.setSdp ( sdp.release ( ) );
		if ( p -> responseSent ( rsp ) )
			t -> serverTransactions.insert ( ServerTransactionNode ( p ) );

		if ( is != t -> serverDialogs.end ( ) ) {
			is -> getAnswerHandler ( ) -> onholdReceived ( holdlevel, port, connectionAddress );
		} else {
			ic -> getOriginateHandler ( ) -> onholdReceived ( holdlevel, port, connectionAddress );
		}
		return;
	}
	if ( ! r.getSdp ( ) ) {
		Pointer < ServerTransaction > p = new InviteServerTransaction ( r );
		Response rsp ( r, Response :: scFailureServiceUnavailable/*scFailureForbidden*/ );
		if ( p -> responseSent ( rsp ) )
			t -> serverTransactions.insert ( ServerTransactionNode ( p ) );
		return;
	}
	const RequestMIMEInfo & m = r.getMime ( );
	if ( ! m.getMaxForwards ( ) ) {
		Pointer < ServerTransaction > p = new InviteServerTransaction ( r );
		Response rsp ( r, Response :: scFailureTooManyHops );
		if ( p -> responseSent ( rsp ) )
			t -> serverTransactions.insert ( ServerTransactionNode ( p ) );
		return;
	}
	if ( ! conf -> tryTake ( ) ) {
		PSYSTEMLOG ( Error, "tryTake failed" );
		Pointer < ServerTransaction > p = new InviteServerTransaction ( r );
		Response rsp ( r, Response :: scFailureServiceUnavailable/*scFailureForbidden*/ );
		if ( p -> responseSent ( rsp ) )
			t -> serverTransactions.insert ( ServerTransactionNode ( p ) );
		return;
	}
	CommonCallDetails common;
	common.setFromNat ( false );
	const Via & v = m.getVia ( ).front ( );
	common.setCallerIp ( getFromAddr ( v ) );
	common.setCallerPort ( getFromPort ( v ) );
	common.setConfId ( m.getCallId ( ) );
	const DigestResponse * d = 0;
	unsigned dIndex = 0;
	ss :: string password;
	if ( conf -> isRegisteredCardAddr ( common.getCallerIp ( ), WORD ( common.getCallerPort ( ) ), common.source ( ).acctn ) )
		common.source ( ).type = SourceData :: card;
	else if ( conf -> isValidInPeerAddress ( common.getCallerIp ( ) ) )
		common.source ( ).type = SourceData :: inbound;
	else {
		PSYSTEMLOG( Info, "requestReceived: Autorization" );
		const DigestResponseVector & auth = m.getAuthorization ( );
		bool succ = false;
		const ss :: string & realm = t -> serverAuthHandler.getRealm ( );
		for ( std :: size_t as = auth.size ( ); dIndex < as; dIndex ++ ) {
			d = & auth [ dIndex ];
			if ( d -> getRealm ( ) != realm )
				continue;
			succ = true;
			break;
		}
		if ( ! succ ) {
			common.release ( );
			Pointer < ServerTransaction > p = new InviteServerTransaction ( r );
			Response rsp ( r, Response :: scFailureUnAuthorized );
			t -> serverAuthHandler.addChallenge ( rsp.getMime ( ) );
			if ( p -> responseSent ( rsp ) )
				t -> serverTransactions.insert ( ServerTransactionNode ( p ) );
			PSYSTEMLOG( Info, "requestReceived: Authotentication 1" );
			return;
		}
		const ss :: string & username = d -> getUsername ( );
		IpPortSet addresses;
		addresses.insert ( IpPort ( common.getCallerIp ( ), common.getCallerPort ( ) ) );
		PIPSocket :: Address localAddr;
		t -> sock.GetInterfaceAddress ( localAddr );
		bool isCard;
		if ( ! conf -> getPassword ( localAddr, addresses, username, password, isCard ) ) {
			PSYSTEMLOG ( Info, "sip invite: unknown username " << username );
			common.release ( );
			Pointer < ServerTransaction > p = new InviteServerTransaction ( r );
			Response rsp ( r, Response :: scFailureUnAuthorized );
			t -> serverAuthHandler.addChallenge ( rsp.getMime ( ) );
			if ( p -> responseSent ( rsp ) )
				t -> serverTransactions.insert ( ServerTransactionNode ( p ) );
			return;
		}
		if ( d -> calcDigest ( password, r ) != d -> getResponse ( ) ) {
			PSYSTEMLOG ( Info, "sip invite: bad password for " << username );
			common.release ( );
			Pointer < ServerTransaction > p = new InviteServerTransaction ( r );
			Response rsp ( r, Response :: scFailureUnAuthorized );
			t -> serverAuthHandler.addChallenge ( rsp.getMime ( ) );
			if ( p -> responseSent ( rsp ) )
				t -> serverTransactions.insert ( ServerTransactionNode ( p ) );
			return;
		}
		if ( ! t -> serverAuthHandler.validNonce ( * d ) ) {
			common.release ( );
			Pointer < ServerTransaction > p = new InviteServerTransaction ( r );
			Response rsp ( r, Response :: scFailureUnAuthorized );
			t -> serverAuthHandler.addChallenge ( rsp.getMime ( ), true );
			if ( p -> responseSent ( rsp ) )
				t -> serverTransactions.insert ( ServerTransactionNode ( p ) );
			return;
		}
		if ( conf -> getSipInviteRegistrationPeriod ( ) > 0 ) { // temporary registration for hunting
			IcqContactsVector icqContacts;
			StringSet onlineNumbers, neededNumbers;
			bool h323 = false;
			bool fromNat = true;
			conf -> registerPeer ( username, PIPSocket :: Address ( common.getCallerIp ( ).c_str ( ) ),
				common.getCallerPort ( ), h323, fromNat,
				PTime ( ) + conf -> getSipInviteRegistrationPeriod ( ) * 1000,
				isCard, PIPSocket :: Address ( common.getCallerIp ( ).c_str ( ) ),
				fromNat ? common.getCallerPort ( ) : 5060, neededNumbers,
				onlineNumbers, 0, * ( PUDPSocket * ) 0, icqContacts );
		}
		common.source ( ).type = isCard ? SourceData :: card : common.source ( ).type = SourceData :: inbound;
	}
	common.setCallingDigitsIn ( m.getFrom ( ).getUri ( ).getUser ( ) );
	common.setDialedDigits ( m.getTo ( ).getUri ( ).getUser ( ) );
	const SDP :: SessionDescription & sdp = * r.getSdp ( );
	getIncomingCodecsFromSDP ( sdp, common.incomingCodecs ( ) );
	OutChoiceDetailsVectorVector choiceForks;
	StringVector forkOutAcctns;
	if ( ! conf -> getCallInfo ( choiceForks, forkOutAcctns, common, true, true, true ) ||
		common.getIncomingCodecs ( ).c.empty ( ) ) {
		common.release ( );
		Pointer < ServerTransaction > p = new InviteServerTransaction ( r );
		int retCode;
		ss :: string textResp; //eto mogno bilo bi i v rsp peredat
		conf -> getH323ToSIPErrorResponse ( common.getDisconnectCause ( ), & retCode, & textResp ); //zachem eto delat do proverki i zatirat ?
		if ( common.source ( ).type != SourceData :: inbound )
			retCode = conf -> isCardPresentAndRegister ( m.getTo ( ).getUri ( ).getUser ( ) ) == -1 ? Response :: scFailureNotFound : Response :: scFailureServiceUnavailable;
		Response rsp ( r, SIP2 :: Response::StatusCodes ( retCode ) );
		if ( p -> responseSent ( rsp ) )
			t -> serverTransactions.insert ( ServerTransactionNode ( p ) );
		return;
	}
	ServerTransactionId tid ( r );
	AnswerHandler * h = 0;
	new AnswerCall ( choiceForks, forkOutAcctns, common, sdp, h, tid, m.hasExpires ( ) ? m.getExpires ( ) : 0,
		m.getMaxForwards ( ) - 1, m.getRemotePartyID ( ), m.getPAssertID ( ), m.getPrivacy ( ) );
	InviteServerTransaction * ip = new InviteServerTransaction ( r, this );
	Pointer < ServerTransaction > p = ip;
	if ( d )
		ip -> saveAuth ( dIndex, password );
	t -> serverTransactions.insert ( ServerTransactionNode ( p ) );
	t -> serverDialogs.insert ( ServerDialog ( r, h ) );
}

void CoreServerAckHandler :: requestReceived ( const Request & r ) {
	DialogId id = makeDialogIdReceived ( r );
	ServerDialogMap :: iterator i = t -> serverDialogs.find ( id );
	if ( i == t -> serverDialogs.end ( ) ) {
		if ( t -> clientDialogs.find ( id ) == t -> clientDialogs.end ( ) )
			PSYSTEMLOG ( Error, "ack for unknown dialog" );
		// TODO: else nado udalit invite transaction ?
		return;
	}
	if ( ! i -> requestReceived ( r ) ) {
		t -> sendBye ( i -> getDialog ( ) );
		t -> serverDialogs.erase ( i );
	}
}

void CoreServerCancelHandler :: requestReceived ( const Request & r ) {
	ServerTransactionId id ( r );
	id.method = Request :: mInvite;
	Pointer < ServerTransaction > p = new NonInviteServerTransaction ( r );
	ServerTransactionMap :: iterator i = t -> serverTransactions.find ( id );
	if ( i == t -> serverTransactions.end ( ) ) {
		Response rsp ( r, Response :: scFailureTransactionDoesNotExist );
		if ( p -> responseSent ( rsp ) )
			t -> serverTransactions.insert ( ServerTransactionNode ( p ) );
		PSYSTEMLOG ( Info, "CoreServerCancelHandler :: requestReceived. Transactions is absent. " << rsp );
		return;
	}
	const ss :: string * tag = & i -> getResponse ( ).getMime ( ).getTo ( ).getTag ( );
	ServerDialogByTransaction & byTrans = t -> serverDialogs.get < ServerTransactionId > ( );
	ServerDialogByTransaction :: iterator j = byTrans.find ( id );
	if ( tag -> empty ( ) )
		tag = & j -> getDialogId ( ).localTag;
	Response rsp ( r, Response :: scSuccessfulOK, * tag );
	if ( p -> responseSent ( rsp ) )
		t -> serverTransactions.insert ( ServerTransactionNode ( p ) );
	if ( i -> getResponse ( ).getStatusCode ( ) >= 200 ) {
		PSYSTEMLOG ( Info, "CoreServerCancelHandler :: requestReceived. StatusCode = " << i -> getResponse ( ).getStatusCode ( ) );
		return;
	}
	if ( i -> getRequest ( ).getMime ( ).getVia ( ).front ( ).getReceived ( ) !=
		r.getMime ( ).getVia ( ).front ( ).getReceived ( ) ) {
		PSYSTEMLOG(Info, "CoreServerCancelHandler :: requestReceived. tipa security");
		return; // tipa security
	}
	if ( AnswerHandler * h = j -> getAnswerHandler ( ) )
		h -> cancelReceived ( );
}

ServerAuthHandler :: ServerAuthHandler ( ) : realm ( "SIParea-" + conf -> getName ( ) + '@' + getLocalAddress ( ) ),
	nonceTtl ( 100 ) {
	ss :: ostringstream os;
	os << "SecretKey" << std :: hex << Random :: number ( ) << std :: hex << Random :: number ( );
	secret = os.str ( );
}

ss :: string ServerAuthHandler :: generateNonce ( unsigned t, unsigned sn ) const {
	ss :: ostringstream os;
	os << std :: hex << std :: setw ( 8 ) << std :: setfill ( '0' ) << t;
	os << std :: hex << std :: setw ( 8 ) << std :: setfill ( '0' ) << sn;
	ss :: ostringstream os2;
	os2 << os.str ( ) << ':' << secret;
	os << toHex ( md5Sum ( os2.str ( ) ) );
	return os.str ( );
}

static unsigned getNonceSeqNum ( ) {
	static unsigned seqNum = 0;
	return __sync_add_and_fetch ( & seqNum, 1 );
}

void ServerAuthHandler :: addChallenge ( ResponseMIMEInfo & m, bool stale ) const {
	StringSet qop;
	qop.insert ( "auth" );
	qop.insert ( "auth-int" );
	DigestChallenge c ( realm, generateNonce ( unsigned ( PTime ( ).GetTimeInSeconds ( ) ), getNonceSeqNum ( ) ), qop );
	c.setStale ( stale );
	m.addWwwAuthenticate ( c );
}

void ServerAuthHandler :: addAuthenticationInfo ( const DigestResponse & d, Response & rsp, const ss :: string & password ) const {
	AuthenticationInfo ai ( d, rsp, password );
	unsigned ts = unsigned ( std :: strtoul ( d.getNonce ( ).substr ( 0, 8 ).c_str ( ), 0, 16 ) );
	unsigned ct = unsigned ( PTime ( ).GetTimeInSeconds ( ) );
	if ( ct >= ts + nonceTtl / 2 )
		ai.setNextnonce ( generateNonce ( ct, getNonceSeqNum ( ) ) );
	rsp.getMime ( ).setAuthenticationInfo ( ai );
}

bool ServerAuthHandler :: checkSeenNonces ( unsigned ts, unsigned sn, unsigned nc ) {
	unsigned t = unsigned ( PTime ( ).GetTimeInSeconds ( ) ) - nonceTtl;
	if ( t >= ts )
		t = ts - 1;
	seenNonces.erase ( seenNonces.begin ( ), seenNonces.upper_bound ( t ) );
	UnsignedUnsignedMap & m = seenNonces [ ts ];
	if ( m [ sn ] >= nc )
		return false;
	m [ sn ] = nc;
	return true;
}

bool ServerAuthHandler :: validNonce ( const DigestResponse & d ) {
	const ss :: string & n = d.getNonce ( );
	if ( n.size ( ) != 48 )
		return false;
	unsigned ts = unsigned ( std :: strtoul ( n.substr ( 0, 8 ).c_str ( ), 0, 16 ) );
	if ( unsigned ( PTime ( ).GetTimeInSeconds ( ) ) >= ts + nonceTtl )
		return false;
	unsigned sn = unsigned ( std :: strtoul ( n.substr ( 8, 8 ).c_str ( ), 0, 16 ) );
	if ( generateNonce ( ts, sn ) != n )
		return false;
	return checkSeenNonces ( ts, sn, d.getNc ( ) );
}

CoreServerRegisterHandler :: CoreServerRegisterHandler ( TransportThread :: Impl * t ) : t ( t ) { }

void CoreServerRegisterHandler :: requestReceived ( const Request & r ) {
	Pointer < ServerTransaction > p = new NonInviteServerTransaction ( r );
	const RequestMIMEInfo & m = r.getMime ( );
	const Via & v = m.getVia ( ).front ( );
	const ss :: string & fromIp = getFromAddr ( v );
	int fromPort = getFromPort ( v );
	bool isCard = false;
	ss :: string username;
	unsigned expires;
	const ContactHeader & c = m.getContact ( );
	if ( c.hasExpires ( ) )
		expires = c.getExpires ( );
	else if ( m.hasExpires ( ) )
		expires = m.getExpires ( );
	else
		expires = conf -> getSipDefaultRegistrationPeriod ( );
	bool authRequired = true;
	bool isRegisteredCard = conf -> isRegisteredCardAddr ( fromIp, WORD ( fromPort ), username );
	if ( ! expires && isRegisteredCard )// UNREGISTER from known card
		authRequired = false;
	if ( conf -> isValidInPeerAddress ( fromIp ) && ! isRegisteredCard ) { // authentication not required for InPeers
		if ( conf -> isValidOutPeerAddress ( fromIp, fromPort ) ) // Looks like floating OutPeer. Always need auth
			authRequired = true;
		else
			authRequired = false;
	}
	const DigestResponse * d = 0;
	ss :: string password;
	if ( authRequired ) {
		const DigestResponseVector & auth = m.getAuthorization ( );
		bool succ = false;
		const ss :: string & realm = t -> serverAuthHandler.getRealm ( );
		for ( std :: size_t i = 0, as = auth.size ( ); i < as; i ++ ) {
			d = & auth [ i ];
			if ( d -> getRealm ( ) != realm )
				continue;
			succ = true;
			break;
		}
		if ( ! succ ) {
			Response rsp ( r, Response :: scFailureUnAuthorized );
			t -> serverAuthHandler.addChallenge ( rsp.getMime ( ) );
			if ( p -> responseSent ( rsp ) )
				t -> serverTransactions.insert ( ServerTransactionNode ( p ) );
			return;
		}
		username = d -> getUsername ( );
		IpPortSet addresses;
		addresses.insert ( IpPort ( fromIp, fromPort ) );
		PIPSocket :: Address localAddr;
		t -> sock.GetInterfaceAddress ( localAddr );
		if ( ! conf -> getPassword ( localAddr, addresses, username, password, isCard ) ) {
			PSYSTEMLOG ( Info, "sip register: unknown username " << username );
			Response rsp ( r, Response :: scFailureUnAuthorized );
			t -> serverAuthHandler.addChallenge ( rsp.getMime ( ) );
			if ( p -> responseSent ( rsp ) )
				t -> serverTransactions.insert ( ServerTransactionNode ( p ) );
			return;
		}
		if ( d -> calcDigest ( password, r ) != d -> getResponse ( ) ) {
			PSYSTEMLOG ( Info, "sip register : bad password for " << username );
			Response rsp ( r, Response :: scFailureUnAuthorized );
			t -> serverAuthHandler.addChallenge ( rsp.getMime ( ) );
			if ( p -> responseSent ( rsp ) )
				t -> serverTransactions.insert ( ServerTransactionNode ( p ) );
			return;
		}
		if ( ! t -> serverAuthHandler.validNonce ( * d ) ) {
			Response rsp ( r, Response :: scFailureUnAuthorized );
			t -> serverAuthHandler.addChallenge ( rsp.getMime ( ), true );
			if ( p -> responseSent ( rsp ) )
				t -> serverTransactions.insert ( ServerTransactionNode ( p ) );
			return;
		}
	}
	Conf :: UnregisteredContactsMap unregisteredContacts;
	IcqContactsVector icqContacts;
	StringSet onlineNumbers;
	StringSet neededNumbers;
	bool h323 = false;
	bool fromNat = v.getRport ( ) && * v.getRport ( ) != ( v.getPort ( ) ? : 5060 );
	PIPSocket :: Address fromAddr ( fromIp.c_str ( ) );
	if ( expires ) // FIXME: tut vse ne pravilno - adresa nado brat iz contactov
		conf -> registerPeer ( username, fromAddr, fromPort, h323, fromNat,
			PTime ( ) + expires * 1000, isCard, fromAddr, fromPort, neededNumbers,
			onlineNumbers, & unregisteredContacts, * ( PUDPSocket * ) 0, icqContacts );
	else
		conf -> unregisterInPeer ( fromAddr, fromPort, unregisteredContacts );
	rasThread -> sendNotifications ( unregisteredContacts );

	Response rsp ( r, Response :: scSuccessfulOK );
	if ( expires ) { // FIXME: tut nado vse bindingi vislat
		ContactHeader nc = c;
		nc.setExpires ( expires );
		rsp.getMime ( ).setContact ( nc );
	}
	if ( authRequired )
		t -> serverAuthHandler.addAuthenticationInfo ( * d, rsp, password );
	if ( p -> responseSent ( rsp ) )
		t -> serverTransactions.insert ( ServerTransactionNode ( p ) );
}

}

