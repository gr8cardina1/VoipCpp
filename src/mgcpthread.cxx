#pragma implementation
#pragma implementation "mgcporiginatecallarg.hpp"
#include "ss.hpp"
#include "allocatable.hpp"
#include <ptlib.h>
#include <ptlib/sockets.h>
#include "mgcpthread.hpp"
#include "mgcp.hpp"
#include "pointer.hpp"
#include <ptlib/svcproc.h>
#include "automutex.hpp"
#include "ipport.hpp"
#include <boost/spirit/include/classic_parse_tree.hpp>
#include <boost/spirit/include/classic_utility.hpp>
#include "findmapelement.hpp"
#include "istringless.hpp"
#include "mgcpgw.hpp"
#include "rightparser.hpp"
#include "scopeguard.hpp"
#include "mgcporiginatecallarg.hpp"
#include "Log.h"
#include "ixcudpsocket.hpp"
#include "nop.hpp"
#include "mgcpconf.hpp"
#include "radgwinfo.hpp"
#include "icqcontact.hpp"
#include "aftertask.hpp"
#include "Conf.hpp"
#include "firstiterator.hpp"
#include <tr1/functional>
#include <boost/iterator/transform_iterator.hpp>
#include "transformiterator.hpp"
#include "setthreeway.hpp"
#include <boost/mem_fn.hpp>

class MgcpThread :: Impl : public Allocatable < __SS_ALLOCATOR >, boost :: noncopyable {
	PMutex mut;
	IxcUDPSocket sock;
	typedef std :: map < ss :: string, Pointer < Gw >, istringless < ss :: string >,
		__SS_ALLOCATOR < std :: pair < const ss :: string, Pointer < Gw > > > > GwMap;
	GwMap gws;
	struct Transaction {
		PTime createTime, sendTime;
		MGCP :: PDU pdu;
		bool provisioned;
	};
	typedef std :: map < unsigned int, Transaction, std :: less < unsigned int >,
		__SS_ALLOCATOR < std :: pair < const unsigned int, Transaction > > > TransactionMap;
	TransactionMap sentTransactions;
	struct Response {
		PTime createTime;
		MGCP :: PDU pdu;
		bool acknowledged;
	};
	typedef std :: map < unsigned int, Response, std :: less < unsigned int >,
		__SS_ALLOCATOR < std :: pair < const unsigned int, Response > > > IdResponseMap;
	typedef std :: map < ss :: string, IdResponseMap, istringless < ss :: string >,
		__SS_ALLOCATOR < std :: pair < const ss :: string, IdResponseMap > > > ResponseMap;
	ResponseMap sentResponses;
	typedef std :: map < unsigned int, PTime, std :: less < unsigned int >,
		__SS_ALLOCATOR < std :: pair < const unsigned int, PTime > > > IdAcknowledgementMap;
	typedef std :: map < ss :: string, IdAcknowledgementMap, istringless < ss :: string >,
		__SS_ALLOCATOR < std :: pair < const ss :: string, IdAcknowledgementMap > > > AcknowledgementMap;
	AcknowledgementMap cachedAcknowledgements;
	volatile bool needReload;
	bool readAndHandleMesg ( );
	void handleRequest ( MGCP :: PduVector & ret, const MGCP :: PDU & pdu, const PIPSocket :: Address & ia );
	void handleResponse ( MGCP :: PduVector & ret, const MGCP :: PDU & pdu );
	void handleAcknowledgements ( const MGCP :: PDU & pdu );
	void addAcknowledgements ( MGCP :: PDU & pdu );
	void cacheAcknowledgement ( unsigned int id, const ss :: string & gw );
	void checkStalledTransactions ( );
	bool send ( const ss :: string & s, const IpPort & a );
	void send ( MGCP :: PduVector & pdus );
	void addSentTransaction ( MGCP :: PDU & pdu );
	void sendChecked ( MGCP :: PduVector & pdus );
	void checkReload ( );
public:
	explicit Impl ( unsigned short port );
	void Main ( );
	void Close ( );
	void detachThreadHandler ( const ss :: string & gw, const ss :: string & ep, bool ok );
	bool originateCall ( const ss :: string & gw, const ss :: string & ep, EpOriginateThreadHandler * th,
		const ss :: string & callId, const CodecInfo & codec, unsigned telephoneEventsPayloadType, int rtpPort,
		PIPSocket :: Address localAddr );
	void sendRinging ( const ss :: string & gw, const ss :: string & ep, int localRtpPort,
		const PIPSocket :: Address & localAddr, const CodecInfo & inCodec );
	void sendOk ( const ss :: string & gw, const ss :: string & ep, int localRtpPort,
		const PIPSocket :: Address & localAddr, const CodecInfo & inCodec, unsigned telephoneEventsPayloadType );
	void reloadState ( );
};

enum {
	mgcpServerPort = 2427
};

MgcpThread :: Impl :: Impl ( unsigned short port ) : sock ( port ), needReload ( true ) {
	if ( ! sock.Listen ( PIPSocket :: Address ( "*" ) ) ) {
		PSYSTEMLOG ( Error, "Listen failed on mgcp socket: " <<
			sock.GetErrorText ( ) );
		return;
	}
	sock.SetReadTimeout ( 1000 );
	sock.SetWriteTimeout ( 1000 );
}

void MgcpThread :: Impl :: Main ( ) {
/*	{
		MGCP :: PduVector ret;
		AutoMutex am ( mut );
		for ( GwMap :: const_iterator i = gws.begin ( ); i != gws.end ( ); ++ i )
			i -> second -> init ( ret );
		send ( ret );
	}*/
	while ( sock.IsOpen ( ) ) {
		checkReload ( );
		checkStalledTransactions ( ); //separate thread with sleep in a loop ?
		readAndHandleMesg ( );
	}
}

void MgcpThread :: Impl :: Close ( ) {
	sock.Close ( );
}

bool MgcpThread :: Impl :: readAndHandleMesg ( ) {
	const int bufferSize = 65536;	// minimum 4000, no bolshe ne pomeshaet
	static char buffer [ bufferSize ]; // pohoge v stacke mesta ne hvataet
	PIPSocket :: Address senderAddr;
	WORD senderPort;
	if ( ! sock.Read ( buffer, bufferSize ) )
		return false;
	sock.GetLastReceiveAddress ( senderAddr, senderPort );
	ss :: string t ( buffer, sock.GetLastReadCount ( ) );
	PSYSTEMLOG ( Info, "mgcp message arrived from " << senderAddr << ':' << senderPort << " : " << t );
	ss :: istringstream is ( t );
	StringVector msgs;
	ss :: string curstr;
	ss :: ostringstream curmsg;
	while ( getline ( is, curstr ) ) {
		ss :: string :: size_type s = curstr.size ( );
		if ( s && curstr [ s - 1 ] == '\r' )
			curstr.erase ( s );
		if ( curstr == "." ) {
			msgs.push_back ( curmsg.str ( ) );
			curmsg.str ( "" );
		} else
			curmsg << curstr << '\n';
	}
	msgs.push_back ( curmsg.str ( ) );
	MGCP :: PduVector ret;
	MGCP :: PDU pdu ( static_cast < const char * > ( senderAddr.AsString ( ) ), senderPort );
	for ( unsigned j = 0; j < msgs.size ( ); j ++ ) {
		ss :: istringstream is2 ( msgs [ j ] );
		is2 >> pdu;
		PSYSTEMLOG ( Info, "mgcp message decoded: " << pdu );
		Log -> logMGCPMsg ( pdu, OpengateLog :: Receiving, senderAddr, senderPort );
		AutoMutex am ( mut );
		if ( pdu.getCode ( ) == MGCP :: PDU :: rcInvalid ) {
			PIPSocket :: Address ifAddr;
			sock.GetInterfaceAddress ( ifAddr );
			handleRequest ( ret, pdu, ifAddr );
		} else
			handleResponse ( ret, pdu );
	}
	if ( ret.empty ( ) )
		return true;
	AutoMutex am ( mut );
	send ( ret );
	return true;
}

void MgcpThread :: Impl :: cacheAcknowledgement ( unsigned int id, const ss :: string & gw ) {
	cachedAcknowledgements [ gw ] [ id ]; // = PTime ( )
}

void MgcpThread :: Impl :: handleResponse ( MGCP :: PduVector & ret, const MGCP :: PDU & pdu ) {
	if ( pdu.getCode ( ) <= MGCP :: PDU :: rcProvisional ) {
		// ne mogu prinyat 000 response - neizvestno ot kakogo gw. razvhe chto poisk po ip
		// no oni i ne dolgni prihodit - ya ne shlyu final responses
		PSYSTEMLOG ( Error, "000 response received - unsupported" );
		return;
	}
	bool hasRA = pdu.getMIME ( ).hasResponseAck ( );
	if ( hasRA )
		makeResponse ( ret, pdu, MGCP :: PDU :: rcResponseAck );
	TransactionMap :: iterator i = sentTransactions.find ( pdu.getId ( ) );
	if ( i == sentTransactions.end ( ) ) {
		PSYSTEMLOG ( Error, "response to unknown transaction " << pdu.getId ( ) );
		return;
	}
	if ( pdu.getCode ( ) < MGCP :: PDU :: rcOK && i -> second.provisioned ) {
		PSYSTEMLOG ( Error, "dublicate provisioning received" );
		return;
	}
	const ss :: string & gw = i -> second.pdu.getGw ( );
	if ( hasRA )
		ret.back ( ).setGw ( gw );
	GwMap :: iterator g = gws.find ( gw );
	if ( g == gws.end ( ) )
		PSYSTEMLOG ( Error, "response to unknown gateway " << gw );
	else
		g -> second -> handleResponse ( ret, pdu, i -> second.pdu.getEp ( ) );
	if ( pdu.getCode ( ) >= MGCP :: PDU :: rcOK ) {
		if ( ! hasRA )
			cacheAcknowledgement ( pdu.getId ( ), gw );
		sentTransactions.erase ( i );
	} else
		i -> second.provisioned = true;
}

namespace bs = boost :: spirit :: classic;

struct rangelist_grammar : public bs :: grammar < rangelist_grammar > {
	template < typename ScannerT > struct definition {
		definition ( const rangelist_grammar & self ) {
			rangelist = range >>
				* ( bs :: no_node_d [ ',' >> * bs :: blank_p ] >> range );
			range = bs :: leaf_node_d [ bs :: repeat_p ( 1, 9 )
				[ bs :: digit_p ] ] >> ! ( bs :: no_node_d
				[ bs :: ch_p ( '-' ) ] >> bs :: leaf_node_d
				[ bs :: repeat_p ( 1, 9 ) [ bs :: digit_p ] ] );
			if ( self.tag_id )
				rangelist.set_id ( self.tag_id );
		}
		bs :: rule < ScannerT, bs :: parser_context < >,
			bs :: dynamic_parser_tag > rangelist;
		bs :: rule < ScannerT > range;
		bs :: rule < ScannerT, bs :: parser_context < >,
			bs :: dynamic_parser_tag > const & start ( ) const {
			return rangelist;
		}
	};
	rangelist_grammar ( int t = 0 ) : tag_id ( t ) { }
private:
	int tag_id;
};

typedef std :: vector < std :: pair < unsigned int, unsigned int >,
	__SS_ALLOCATOR < std :: pair < unsigned int, unsigned int > > > RangeVector;

static RangeVector parseRange ( const ss :: string & s ) {
	rangelist_grammar g;
	RightParser :: result_t info = RightParser :: parse ( s.begin ( ), s.end ( ), g );
	RangeVector v;
	if ( ! info.full ) {
		PSYSTEMLOG ( Error, "error parsing range " << s );
		return v;
	}
	for ( RightParser :: iter_t i = info.trees.begin ( ); i != info.trees.end ( ); ++ i ) {
		RightParser :: iter_t j = i -> children.begin ( );
		unsigned int f = std :: atoi ( ss :: string ( j -> value.begin ( ), j -> value.end ( ) ).c_str ( ) );
		unsigned int s = f;
		if ( ++ j != i -> children.end ( ) )
			s = std :: atoi ( ss :: string ( j -> value.begin ( ), j -> value.end ( ) ).c_str ( ) );
		v.push_back ( std :: make_pair ( f, s ) );
	}
	return v;
}


void MgcpThread :: Impl :: handleAcknowledgements ( const MGCP :: PDU & pdu ) {
	ss :: string s;
	if ( ! pdu.getMIME ( ).getResponseAck ( s ) )
		return;
	RangeVector v = parseRange ( s );
	if ( v.empty ( ) )
		return;
	const ss :: string & gw = pdu.getGw ( );
	ResponseMap :: iterator i = sentResponses.find ( gw );
	if ( i == sentResponses.end ( ) ) {
		PSYSTEMLOG ( Error, "acknowledgement for unknown response " << gw );
		return;
	}
	IdResponseMap & im = i -> second;
	for ( RangeVector :: const_iterator i = v.begin ( ); i != v.end ( ); ++ i ) {
		for ( unsigned j = i -> first; j <= i -> second; j ++ ) {
			//mogno ispolzovat upper/lower bound na sluchay dlinnih ranges
			IdResponseMap :: iterator k = im.find ( j );
			if ( k == im.end ( ) ) {
				PSYSTEMLOG ( Error, "acknowledgement for unknown response " << gw << ':' << j );
				continue;
			}
			if ( k -> second.acknowledged )
				PSYSTEMLOG ( Error, "double acknowledgement from " << gw << ':' << j );
			else
				k -> second.acknowledged = true; //tut mogno pdu udalyat
		}
	}
}


void MgcpThread :: Impl :: handleRequest ( MGCP :: PduVector & ret, const MGCP :: PDU & pdu, const PIPSocket :: Address & ia ) {
	// if ( badversion ) sendError ( rcIncompatibleProtocolVersion )
	ResponseMap :: const_iterator i = sentResponses.find ( pdu.getGw ( ) );
	if ( i != sentResponses.end ( ) ) {
		const IdResponseMap & im = i -> second;
		IdResponseMap :: const_iterator j = im.find ( pdu.getId ( ) );
		if ( j != im.end ( ) ) {
			if ( j -> second.acknowledged )
				PSYSTEMLOG ( Error, "received duplicate acknowledged command " << pdu.getGw ( ) <<
					':' << pdu.getId ( ) );
			else {
				PSYSTEMLOG ( Error, "received duplicate command " << pdu.getGw ( ) << ':' <<
					pdu.getId ( ) );
				ret.push_back ( j -> second.pdu );
			}
			return;
		}
	}
	handleAcknowledgements ( pdu );
	GwMap :: iterator g = gws.find ( pdu.getGw ( ) );
	if ( g == gws.end ( ) ) {
		PSYSTEMLOG ( Error, "request to unknown gateway " << pdu.getGw ( ) );
		makeResponse ( ret, pdu, MGCP :: PDU :: rcUnknownEndpoint );
	} else
		g -> second -> handleRequest ( ret, pdu, ia );
}

void MgcpThread :: Impl :: addAcknowledgements ( MGCP :: PDU & pdu ) {
	AcknowledgementMap :: iterator i = cachedAcknowledgements.find ( pdu.getGw ( ) );
	if ( i == cachedAcknowledgements.end ( ) )
		return;
	const IdAcknowledgementMap & acks = i -> second;
	ss :: ostringstream os;
	bool first = true;
	PTime now;
	const int tHist = 30;
	for ( IdAcknowledgementMap :: const_iterator a = acks.begin ( ); a != acks.end ( ); ++ a ) {
		if ( ( now - a -> second ).GetSeconds ( ) > tHist )
			continue;
		if ( first )
			first = false;
		else
			os << ", ";
		os << a -> first;
	}
	cachedAcknowledgements.erase ( i );
	const ss :: string & s = os.str ( );
	if ( ! s.empty ( ) )
		pdu.getMIME ( ).setResponseAck ( s );
}

void MgcpThread :: Impl :: addSentTransaction ( MGCP :: PDU & pdu ) {
	if ( pdu.getCode ( ) == MGCP :: PDU :: rcInvalid ) {
		if ( sentTransactions.count ( pdu.getId ( ) ) )
			return;
		addAcknowledgements ( pdu );
		Transaction t;
		t.pdu = pdu;
		t.createTime = t.sendTime = PTime ( );
		t.provisioned = false;
		sentTransactions.insert ( std :: make_pair ( pdu.getId ( ), t ) );
	} else if ( pdu.getCode ( ) >= MGCP :: PDU :: rcProvisional ) {
		if ( pdu.getGw ( ).empty ( ) )
			PSYSTEMLOG ( Error, "empty gw in addSentTransaction" );
		if ( findMapElement ( sentResponses, pdu.getGw ( ), pdu.getId ( ) ) )
			return;
		Response r;
		r.acknowledged = false;
		r.pdu = pdu;
		r.createTime = PTime ( );
		sentResponses [ pdu.getGw ( ) ].insert ( std :: make_pair ( pdu.getId ( ), r ) );
	}
}

bool MgcpThread :: Impl :: send ( const ss :: string & s, const IpPort & a ) {
	PSYSTEMLOG ( Info, "sending mgcp message to " << a.ip << ':' << a.port << " : " << s );
	if ( sock.WriteTo ( s.data ( ), PINDEX ( s.size ( ) ), PIPSocket :: Address ( a.ip.c_str ( ) ), WORD ( a.port ) ) )
		return true;
	PSYSTEMLOG ( Error, "error sending: " << sock.GetErrorText ( ) );
	return false;
}

void MgcpThread :: Impl :: send ( MGCP :: PduVector & pdus ) {
	if ( pdus.empty ( ) ) {
		PSYSTEMLOG ( Error, "empty pdu vector in send" );
		return;
	}
	typedef std :: map < IpPort, IntVector, std :: less < IpPort >,
		__SS_ALLOCATOR < std :: pair < const IpPort, IntVector > > > SendMap;
	SendMap sm;
	for ( unsigned i = 0; i < pdus.size ( ); i ++ )
		sm [ IpPort ( pdus [ i ].getIp ( ), pdus [ i ].getPort ( ) ) ].push_back ( i );
	for ( SendMap :: const_iterator i = sm.begin ( ); i != sm.end ( ); ++ i ) {
		const IntVector & indexes = i -> second;
		ss :: string packet;
		unsigned maxDatagram = 1; //4000, no chto podelat
		for ( unsigned j = 0; j < indexes.size ( ); ++ j ) {
			GwMap :: const_iterator g = gws.find ( pdus [ indexes [ j ] ].getGw ( ) );
			if ( g != gws.end ( ) ) {
				maxDatagram = g -> second -> getMaxDatagram ( );
				break;
			}
		}
		for ( unsigned j = 0; j < indexes.size ( ); ++ j ) {
			MGCP :: PDU & pdu = pdus [ indexes [ j ] ];
			addSentTransaction ( pdu );
			Log -> logMGCPMsg ( pdu, OpengateLog :: Sending, i -> first.ip, WORD ( i -> first.port ) );
			if ( packet.empty ( ) )
				packet = pdu.str ( );
			else {
				ss :: string msg = pdu.str ( );
				ss :: string :: size_type newSize = packet.size ( ) + msg.size ( ) + 3;
				if ( newSize > maxDatagram ) {
					send ( packet, i -> first );
					packet.swap ( msg );
				} else {
					packet.reserve ( newSize );
					packet += ".\r\n";
					packet += msg;
				}
			}
		}
		if ( ! packet.empty ( ) )
			send ( packet, i -> first );
	}
}

void MgcpThread :: Impl :: sendChecked ( MGCP :: PduVector & pdus ) {
	if ( ! pdus.empty ( ) )
		send ( pdus );
}

void MgcpThread :: Impl :: checkStalledTransactions ( ) {
	const int tHist = 30;
	const int rto = 1; //eto ne constanta, sm 3.5.3
	const int longtranTimer = 5;
	const int tMax = 20;
	static PTime lastTime;
	PTime now;
	if ( ( now - lastTime ).GetSeconds ( ) == 0 )
		return; //v otdelnom threade prosto budet sleep
	lastTime = now;
	MGCP :: PduVector ret;
	AutoMutex am ( mut );
	for ( TransactionMap :: iterator i = sentTransactions.begin ( ); i != sentTransactions.end ( ); ) {
		// nado bi poryadok po vremeni sozdaniya a ne po id
		// a to pri perehode k 0 poshlyutsya v nepravilnom poryadke
		// takge cikl po vsem slishkom dolgo
		// obe problemi reshayutsya multiindexom
		long age = ( now - i -> second.createTime ).GetSeconds ( );
		if ( age > tHist * 2 ) {
			const MGCP :: PDU & pdu = i -> second.pdu;
			const ss :: string & gw = pdu.getGw ( );
			GwMap :: iterator g = gws.find ( gw );
			if ( g == gws.end ( ) ) {
				PSYSTEMLOG ( Error, "dropping transaction to unknown gateway " << gw );
			} else {
				PSYSTEMLOG ( Error, "dropping transaction " << pdu );
				g -> second -> transactionTimedOut ( ret, pdu );
			}
			TransactionMap :: iterator t = i;
			++ i;
			sentTransactions.erase ( t );
			continue;
		}
		if ( age <= tMax && ( now - i -> second.sendTime ).GetSeconds ( ) >
			( i -> second.provisioned ? longtranTimer : rto ) ) {
			const MGCP :: PDU & pdu = i -> second.pdu;
			PSYSTEMLOG ( Info, "resending mgcp request to " << pdu.getIp ( ) << ':' << pdu.getPort ( ) );
			i -> second.sendTime = now;
			ret.push_back ( pdu );
		}
		++ i;
	}
	for ( ResponseMap :: iterator i = sentResponses.begin ( ); i != sentResponses.end ( ); ) {
		IdResponseMap & im = i -> second;
		for ( IdResponseMap :: iterator j = im.begin ( ); j != im.end ( ); ) {
			if ( ( now - j -> second.createTime ).GetSeconds ( ) > tHist ) {
				IdResponseMap :: iterator t = j;
				++ j;
				im.erase ( t );
			} else
				++ j;
		}
		if ( im.empty ( ) ) {
			ResponseMap :: iterator t = i;
			++ i;
			sentResponses.erase ( t );
		} else
			++ i;
	}
	if ( ! ret.empty ( ) )
		send ( ret );
}

void MgcpThread :: Impl :: detachThreadHandler ( const ss :: string & gw, const ss :: string & ep, bool ok ) {
	AutoMutex am ( mut );
	GwMap :: iterator g = gws.find ( gw );
	if ( g == gws.end ( ) ) {
		PSYSTEMLOG ( Error, "detach handler to unknown gateway " << gw );
		return;
	}
	MGCP :: PduVector ret;
	g -> second -> detachThreadHandler ( ret, ep, ok );
	if ( ! ret.empty ( ) )
		send ( ret );
}

bool MgcpThread :: Impl :: originateCall ( const ss :: string & gw, const ss :: string & ep, EpOriginateThreadHandler * th,
	const ss :: string & callId, const CodecInfo & codec, unsigned telephoneEventsPayloadType, int rtpPort,
	PIPSocket :: Address localAddr ) {
	AutoMutex am ( mut );
	ss :: string mediaIp, signalIp;
	if ( localAddr == INADDR_ANY ) {
		PIPSocket :: GetHostAddress ( localAddr ); //tut nado brat iface u gw
		signalIp = mediaIp = static_cast < const char * > ( localAddr.AsString ( ) );
	} else {
		mediaIp = static_cast < const char * > ( localAddr.AsString ( ) );
		PIPSocket :: GetHostAddress ( localAddr );
		signalIp = static_cast < const char * > ( localAddr.AsString ( ) );
	}
	GwMap :: iterator g = gws.find ( gw );
	if ( g == gws.end ( ) ) {
		PSYSTEMLOG ( Error, "inbound call to unknown gateway " << gw );
		return false;
	}
	MGCP :: PduVector ret;
	OriginateCallArg a ( ret, th, callId, codec, telephoneEventsPayloadType, rtpPort, mediaIp, signalIp );
	bool r = g -> second -> originateCall ( ep, a );
	sendChecked ( ret );
	return r;
}

void MgcpThread :: Impl :: sendRinging ( const ss :: string & gw, const ss :: string & ep, int localRtpPort,
	const PIPSocket :: Address & localAddr, const CodecInfo & inCodec ) {
	AutoMutex am ( mut );
	GwMap :: iterator g = gws.find ( gw );
	if ( g == gws.end ( ) ) {
		PSYSTEMLOG ( Error, "sendRinging to unknown gateway " << gw );
		return;
	}
	MGCP :: PduVector ret;
	g -> second -> sendRinging ( ret, ep, localRtpPort, localAddr, inCodec );
	if ( ! ret.empty ( ) )
		send ( ret );
}

void MgcpThread :: Impl :: sendOk ( const ss :: string & gw, const ss :: string & ep, int localRtpPort,
	const PIPSocket :: Address & localAddr, const CodecInfo & inCodec, unsigned telephoneEventsPayloadType ) {
	AutoMutex am ( mut );
	GwMap :: iterator g = gws.find ( gw );
	if ( g == gws.end ( ) ) {
		PSYSTEMLOG ( Error, "sendOk to unknown gateway " << gw );
		return;
	}
	MGCP :: PduVector ret;
	g -> second -> sendOk ( ret, ep, localRtpPort, localAddr, inCodec, telephoneEventsPayloadType );
	if ( ! ret.empty ( ) )
		send ( ret );
}

void MgcpThread :: Impl :: reloadState ( ) {
	needReload = true;
}

struct GwNameCmp {
	bool operator() ( const MgcpGatewayInfo & gw, const ss :: string & name ) {
		return gw.name < name;
	}
};

void MgcpThread :: Impl :: checkReload ( ) {
	if ( ! needReload )
		return;
	needReload = false;
	MgcpGatewayInfoVector gv;
	if ( ! conf -> getMgcpConf ( gv ) )
		return;
	StringVector o, s, n;
	s.reserve ( gv.size ( ) );
	//std::tr1::mem_fn ne rabotaet s boost::transform_iterator
	set_threeway ( firster ( gws.begin ( ) ), firster ( gws.end ( ) ),
		make_transform_iterator ( boost :: mem_fn ( & MgcpGatewayInfo :: name ), gv.begin ( ) ),
		make_transform_iterator ( boost :: mem_fn ( & MgcpGatewayInfo :: name ), gv.end ( ) ),
		std :: back_inserter ( o ), std :: back_inserter ( s ), std :: back_inserter ( n ) );
	MGCP :: PduVector ret;
	AutoMutex am ( mut );
	for ( std :: size_t i = o.size ( ); i > 0; )
		gws.erase ( o [ -- i ] );
	for ( std :: size_t i = n.size ( ); i > 0; ) {
		const ss :: string & name = n [ -- i ];
		const MgcpGatewayInfo & gi = * std :: lower_bound ( gv.begin ( ), gv.end ( ), name, GwNameCmp ( ) );
		Pointer < Gw > p = new Gw ( name, gi.ip, gi.eps, mgcpServerPort, gi.cls );
		gws.insert ( std :: make_pair ( name, p ) );
		p -> init ( ret );
	}
	for ( std :: size_t i = s.size ( ); i > 0; ) {
		const ss :: string & name = s [ -- i ];
		const MgcpGatewayInfo & gi = * std :: lower_bound ( gv.begin ( ), gv.end ( ), name, GwNameCmp ( ) );
		gws [ name ] -> reloadConf ( ret, gi );
	}
	if ( ! ret.empty ( ) )
		send ( ret );
}

MgcpThread * mgcpThread = 0;

MgcpThread :: MgcpThread ( const WORD port ) : PThread ( 1000, NoAutoDeleteThread, NormalPriority,
	"MgcpThread" ), impl ( new Impl ( port ) ) {
	mgcpThread = this;
	Resume ( );
}

MgcpThread :: ~MgcpThread ( ) {
	mgcpThread = 0;
	delete impl;
}

void MgcpThread :: Main ( ) {
	impl -> Main ( );
}

void MgcpThread :: Close ( ) {
	impl -> Close ( );
}

void MgcpThread :: detachThreadHandler ( const ss :: string & gw, const ss :: string & ep, bool ok ) {
	impl -> detachThreadHandler ( gw, ep, ok );
}

bool MgcpThread :: originateCall ( const ss :: string & gw, const ss :: string & ep, EpOriginateThreadHandler * th,
	const ss :: string & callId, const CodecInfo & codec, unsigned telephoneEventsPayloadType, int rtpPort,
	PIPSocket :: Address localAddr ) {
	return impl -> 	originateCall ( gw, ep, th, callId, codec, telephoneEventsPayloadType, rtpPort, localAddr );
}

void MgcpThread :: sendRinging ( const ss :: string & gw, const ss :: string & ep, int localRtpPort,
	const PIPSocket :: Address & localAddr, const CodecInfo & inCodec ) {
	impl -> sendRinging ( gw, ep, localRtpPort, localAddr, inCodec );
}

void MgcpThread :: sendOk ( const ss :: string & gw, const ss :: string & ep, int localRtpPort,
	const PIPSocket :: Address & localAddr, const CodecInfo & inCodec, unsigned telephoneEventsPayloadType ) {
	impl -> sendOk ( gw, ep, localRtpPort, localAddr, inCodec, telephoneEventsPayloadType );
}

void MgcpThread :: reloadState ( ) {
	impl -> reloadState ( );
	PXAbortBlock ( );
}
