#pragma implementation
#pragma implementation "mgcpconf.hpp"
#include "ss.hpp"
#include "allocatable.hpp"
#include "pointer.hpp"
#include "ipport.hpp"
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/sequenced_index.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include "codecinfo.hpp"
#include "signallingoptions.hpp"
#include "latencylimits.hpp"
#include "profitguard.hpp"
#include "tarifround.hpp"
#include "tarifinfo.hpp"
#include "pricedata.hpp"
#include "outcarddetails.hpp"
#include "outchoice.hpp"
#include "priceelement.hpp"
#include "icqcontact.hpp"
#include "radgwinfo.hpp"
#include "sourcedata.hpp"
#include <boost/regex.hpp>
#include "mysql.hpp"
#include <ptlib.h>
#include "aftertask.hpp"
#include "moneyspool.hpp"
#include "outpriceelement.hpp"
#include "customercalls.hpp"
#include "registeredcard.hpp"
#include "RegisteredOutPeersMap.hpp"
#include <boost/multi_index/member.hpp>
#include "callbuffer.hpp"
#include "mgcpconf.hpp"
#include <tr1/functional>
#include "confdata.hpp"
//-----------
#include <ptlib/svcproc.h>
#include <ptlib/sockets.h>
#include "unknowndigit.hpp"
#include "ssconf.hpp"
#include <ptlib/pipechan.h>
#include "rtppipe.hpp"
#include "rtpstat.hpp"
#include "rtprequest.hpp"
#include <tr1/cstdint>
#include "md5.hpp"
#include "findprefix.hpp"
#include "Conf.hpp"
#include "sqloutthread.hpp"
#include "q931.hpp"
#include "ixcudpsocket.hpp"
#include <boost/multi_index/mem_fun.hpp>
#include "sip.hpp"
#include <boost/range.hpp>
#include "slprint.hpp"
#include "random.hpp"
#include <cstdlib>
#include "radius.hpp"
#include "translateipmask.hpp"

extern RTPPipe * rtpPipe;

static IntVector loadInterfaces ( MySQL & m, const char * huntGroup , int nas ) {
	MyResult r ( m.getQuery ( "select localIp from InterfaceHuntGroupList left join "
		"Interfaces on interface = Interfaces.id where uid = %s and localIp is not null and "
		"nas = %i order by prio", huntGroup, nas ) );
	IntVector interfaces;
	if ( int c = r.rowCount ( ) ) {
		interfaces.reserve ( c );
		while ( const char * * p = r.fetchRow ( ) )
			interfaces.push_back ( PIPSocket :: Address ( * p ) );
	} else {
		interfaces.reserve ( 1 );
		interfaces.push_back ( INADDR_ANY );
	}
	return interfaces;
}

static SigOptionsPeer loadSigOptions ( MySQL & m, const char * sigOptions ) {
	MyResult r ( m.getQuery ( "select * from SignallingOptions where id = %s", sigOptions ) );
	if ( const char * * p = r.fetchRow ( ) )
		return SigOptionsPeer ( p );
	return SigOptionsPeer ( );
}

static ProfitGuard loadProfitOptions ( MySQL & m, const char * profitOptions, double valuteRate ) {
	MyResult r ( m.getQuery ( "select * from ProfitOptions where id = %s", profitOptions ) );
	if ( const char * * p = r.fetchRow ( ) )
		return ProfitGuard ( p, valuteRate );
	return ProfitGuard ( );
}

class AnumTranslatorPrefix : public AnumTranslatorOut {
	StringStringMap prefixes;
	void translate ( ss :: string & num ) const {
		StringStringMap :: const_iterator i = findPrefix ( prefixes, num );
		if ( i != prefixes.end ( ) )
			num.replace ( 0, i -> first.size ( ), i -> second );
	}
public:
	explicit AnumTranslatorPrefix ( StringStringMap & p ) {
		prefixes.swap ( p );
	}
};

class AnumTranslatorOne : public AnumTranslatorOut {
	ss :: string to;
	void translate ( ss :: string & num ) const {
		num = to;
	}
public:
	explicit AnumTranslatorOne ( const ss :: string & to ) : to ( to ) { }
};

class AnumTranslatorRange : public AnumTranslatorOut {
	unsigned long long from;
	unsigned long long range;
	void translate ( ss :: string & num ) const {
		ss :: ostringstream os;
		unsigned long long r = Random :: number ( );
		r <<= 32;
		r |= Random :: number ( );
		r %= range;
		os << from + r;
		num = os.str ( );
	}
public:
	AnumTranslatorRange ( const ss :: string & s1, const ss :: string & s2 ) {
		unsigned long long a1 = std :: strtoull ( s1.c_str ( ), 0, 10 );
		unsigned long long a2 = std :: strtoull ( s2.c_str ( ), 0, 10 );
		if ( a1 < a2 ) {
			from = a1;
			range = a2 - a1 + 1;
		} else {
			from = a2;
			range = a1 - a2 + 1;
		}
		if ( ! range )
			range = 1;
	}
};

class AnumTranslatorList : public AnumTranslatorOut {
	StringVector list;
	void translate ( ss :: string & num ) const {
		num = list [ Random :: number ( ) % list.size ( ) ];
	}
public:
	explicit AnumTranslatorList ( StringVector & l ) {
		list.swap ( l );
	}
};

class AnumTranslatorNone : public AnumTranslatorOut {
	void translate ( ss :: string & ) const { }
};

void AnumTranslatorIn :: translate ( int outgrp, ss :: string & num ) const {
	OutMap :: const_iterator i = outMap.find ( outgrp );
	if ( i == outMap.end ( ) )
		i = outMap.find ( 0 );
	if ( i != outMap.end ( ) )
		i -> second -> translate ( num );
}

void AnumTranslatorIn :: add ( int outgrp, Pointer < AnumTranslatorOut > tr ) {
	outMap [ outgrp ] = tr;
}

const AnumTranslatorOut * AnumTranslatorIn :: getOut ( ) const {
	return ( -- outMap.end ( ) ) -> second;
}

bool AnumTranslatorIn :: empty ( ) const {
	return outMap.empty ( );
}

void OutPeerInfo :: translateAnum ( ss :: string & num ) const {
	at -> translate ( num );
}

void InPeerInfo :: translateAnum ( int grp, ss :: string & num ) const {
	at -> translate ( grp, num );
}

void CardInfo :: translateAnum ( int grp, ss :: string & num ) const {
	at -> translate ( grp, num );
}

RouteInfo :: RouteInfo ( MySQL & m, int nas, const OutPeerInfo * o, const RouteInfoData * p ) :
	interfaces ( p -> n.interfaces [ 0 ] == '0' ? o -> interfaces : loadInterfaces ( m, p -> n.interfaces, nas ) ),
	sigOptions ( p -> n.sigOptions [ 0 ] == '0' ? o -> sigOptions : loadSigOptions ( m, p -> n.sigOptions ) ),
	prio ( std :: atoi ( p -> n.prio ) ), lim ( std :: atoi ( p -> n.lim ) ),
	directRTP ( p -> n.direct [ 0 ] == 'D' ? o -> directRTP : p -> n.direct [ 0 ] == 'y' ),
	forking ( p -> n.forking [ 0 ] == 'y' ), inDefaultDirectRTP ( p -> n.direct [ 0 ] == 'D' ) { }

static bool operator> ( const OutChoice & c1, const OutChoice & c2 ) {
	int t = c1.getPort ( ) - c2.getPort ( );
	if ( t > 0 )
		return true;
	if ( t < 0 )
		return false;
	t = c1.getIp ( ).compare ( c2.getIp ( ) );
	if ( t > 0 )
		return true;
	if ( t < 0 )
		return false;
	return c1.getPrefix ( ) > c2.getPrefix ( );
}

struct TagIP { };

typedef boost :: multi_index :: multi_index_container < OutChoice,
	boost :: multi_index :: indexed_by <
		boost :: multi_index :: ordered_non_unique < boost :: multi_index :: tag < OutChoice >,
		boost :: multi_index :: identity < OutChoice > >,
		boost :: multi_index :: ordered_unique < boost :: multi_index :: tag < TagIP >,
		boost :: multi_index :: identity < OutChoice >, std :: greater < OutChoice > >
	>, __SS_ALLOCATOR < OutChoice > > OutChoiceContainer;

static bool deleteIp ( OutChoiceContainer & v, const OutChoice & o ) {
	typedef OutChoiceContainer :: index < TagIP > :: type ByIP;
	ByIP & byIP = v.get < TagIP > ( );
	ByIP :: iterator i = byIP.find ( o );
	if ( i == byIP.end ( ) )
		return true;
	if ( * i < o )
		return false;
	byIP.erase ( i );
	return true;
}

ConfData :: TableLocker :: TableLocker ( MySQL & mm, bool useIPAcct ) : m ( mm ) {
	const char * tables [ ] = { "InPeers", "OutPeers", "RateSheets", "InDefaultPrice", "Price", "OutPrice",
		"InPeerGWS", "OutPeerGWS", "AllowedRoutes", "CallingReplaceList", "DebugInIps",
		"InIPAddresses", "Users", "GrossBook", "Accounts", "ClientReportInCache",
		"PrepaidIds", "CardsPrice", "PrepaidPrice", "InCallingReplaceList", "CardGWS",
		"gws", "CodecClassList", "Codecs", "Rules", "TimeBased", "TarifDesc", "OutIPAddresses",
		"InAccessList", "OutAccessList", "AllowedGroups", "OutGroupPeers", "ValuteRates",
		"DefaultPrices", "Valutes", "ProtoClassList", "ProtoParameterList", "Interfaces",
		"InterfaceHuntGroupList", "DNIS", "InPrepaidPrefixes", "CardAmortiseCost",
		"AllowedDnis", "CallBackPrefixes", "RuleSets", "SignallingOptions",
		"CardCallsCache", "PrepaidAllowedRoutes", "PrepaidAllowedGroups",
		"RadConf", "RadiusClassList", "RadiusClassValues",
		"GateKeepers", "GKLogins", "Recoders", "IcqContacts", "IcqContactList", "Admins",
		"FinReportInCache", "InEndUserPrice", "IPAcct", "ReleaseCompleteReplaceMap",
		"MesgLatency", "ProfitOptions", "CallBuffers",
		"CallBufferCodes", "PrefixFilterItems", "Forking", "RXReplaces",
		"SIPToH323ErrorResponse", "H323ToSIPErrorResponse",
		"MgcpClasses", "MgcpGateways", "MgcpEndpoints", "ANumReplaceGroups", "ANumReplaceLists",
		"ANumReplacePrefixes", "RportAgents", "ANumReplaceRanges", ( useIPAcct ? "IPPeers" : 0 ), 0 };
	MySQL :: Lock locks [ ] = { MySQL :: lockRead, MySQL :: lockRead, MySQL :: lockRead,
		MySQL :: lockRead, MySQL :: lockRead, MySQL :: lockRead, MySQL :: lockRead,
		MySQL :: lockRead, MySQL :: lockRead, MySQL :: lockRead, MySQL :: lockRead,
		MySQL :: lockRead, MySQL :: lockRead, MySQL :: lockRead, MySQL :: lockRead,
		MySQL :: lockRead, MySQL :: lockRead, MySQL :: lockRead, MySQL :: lockRead,
		MySQL :: lockRead, MySQL :: lockRead, MySQL :: lockRead, MySQL :: lockRead,
		MySQL :: lockRead, MySQL :: lockRead, MySQL :: lockRead, MySQL :: lockRead,
		MySQL :: lockRead, MySQL :: lockRead, MySQL :: lockRead, MySQL :: lockRead,
		MySQL :: lockRead, MySQL :: lockRead, MySQL :: lockRead, MySQL :: lockRead,
		MySQL :: lockRead, MySQL :: lockRead, MySQL :: lockRead, MySQL :: lockRead,
		MySQL :: lockRead, MySQL :: lockRead, MySQL :: lockRead, MySQL :: lockRead,
		MySQL :: lockRead, MySQL :: lockRead, MySQL :: lockRead, MySQL :: lockRead,
		MySQL :: lockRead, MySQL :: lockRead, MySQL :: lockRead, MySQL :: lockRead,
		MySQL :: lockRead, MySQL :: lockRead, MySQL :: lockRead, MySQL :: lockRead,
		MySQL :: lockRead, MySQL :: lockRead, MySQL :: lockRead, MySQL :: lockRead,
		MySQL :: lockRead, MySQL :: lockRead, MySQL :: lockRead, MySQL :: lockRead,
		MySQL :: lockRead, MySQL :: lockRead, MySQL :: lockRead, MySQL :: lockRead,
		MySQL :: lockRead, MySQL :: lockRead, MySQL :: lockRead, MySQL :: lockRead,
		MySQL :: lockRead, MySQL :: lockRead, MySQL :: lockRead, MySQL :: lockRead,
		MySQL :: lockRead, MySQL :: lockRead, MySQL :: lockRead, MySQL :: lockRead,
		MySQL :: lockRead, MySQL :: lockRead, MySQL :: lockRead };
	if ( m.lockTables ( tables, locks ) )
		throw MySQLError ( m );
}

ConfData :: TableLocker :: ~TableLocker ( ) {
	m.unLockTables ( );
}

struct TimePrice {
	int price, discount, tarif;
	ss :: string timeToChange;
	TimePrice ( const char * * p ) : price ( std :: atoi ( p [ 0 ] ) ), discount ( std :: atoi ( p [ 1 ] ) ),
		tarif ( std :: atoi ( p [ 2 ] ) ), timeToChange ( p [ 3 ] ) { }
};

static void getTimeToChange ( ss :: string & timeToChange, const ss :: string & now, int uid, MySQL & m )
	throw ( MySQLError ) {
	ss :: string nowday = m.getString ( "select from_days( to_days( '%s' ) )", now.c_str ( ) );
	if ( m.err_no ( ) )
		throw MySQLError ( m );
	ss :: string nowmins = m.getString ( "select date_format( '%s', '%%H:%%i' )", now.c_str ( ) );
	if ( m.err_no ( ) )
		throw MySQLError ( m );
	MyResult r ( m.getQuery ( "select date_add( '%s', interval left( beginh, 5 ) "
		"hour_minute ) from TimeBased where rid = %i and beginh > '%s:00' and date_add( '%s 00:00:00', "
		"interval beginh hour_second ) < '%s' order by 1 limit 1", nowday.c_str ( ), uid, nowmins.c_str ( ),
		nowday.c_str ( ), timeToChange.c_str ( ) ) );
	if ( const char * * p = r.fetchRow ( ) )
		timeToChange = * p;
	r.reset ( m.getQuery ( "select date_add( date_add( '%s', interval left( endh, 5 ) hour_minute ), "
		"interval 1 minute ) from TimeBased where rid = %i and endh >= '%s:00' and date_add( '%s 00:00:00', "
		"interval endh hour_second ) < '%s' order by 1 limit 1", nowday.c_str ( ), uid, nowmins.c_str ( ),
		nowday.c_str ( ), timeToChange.c_str ( ) ) );
	if ( const char * * p = r.fetchRow ( ) )
		timeToChange = * p;
}

static TimePrice getTimePrice ( const ss :: string & now, int uid, ss :: string & lastTimeToChange, MySQL & m ) {
	getTimeToChange ( lastTimeToChange, now, uid, m );
	PSYSTEMLOG ( Info, "lastTimeToChange set to " << lastTimeToChange << " for rid " << uid );
	MyResult r ( m.getQuery ( "select price, discount, tariff, '%s' from TimeBased where rid=%i and "
		"type='y' and date_format( now(), '%%m%%d' ) between date_format( begind, '%%m%%d' ) "
		"and date_format( endd, '%%m%%d' ) and date_format( now(), '%%H:%%i' ) between left( "
		"beginh, 5 ) and left( endh, 5 )", lastTimeToChange.c_str ( ), uid ) );
	if ( const char * * p = r.fetchRow ( ) )
		return TimePrice ( p );
	r.reset ( m.getQuery ( "select price, discount, tariff, '%s' from TimeBased where rid=%i and type='w' and weekday( "
		"now() ) between weekday( begind ) and weekday( endd ) and date_format( now(), '%%H:%%i' ) between "
		"left( beginh, 5 ) and left( endh, 5 ) order by weekday( begind ) = 0 and weekday( endd ) = 6",
		lastTimeToChange.c_str ( ), uid ) );
	if ( const char * * p = r.fetchRow ( ) )
		return TimePrice ( p );
	r.reset ( m.getQuery ( "select price, discount, tariff, '%s' from Rules where id=%i", lastTimeToChange.c_str ( ),
		uid ) );
	if ( const char * * p = r.fetchRow ( ) )
		return TimePrice ( p );
	throw MySQLError ( m );
}

static void addChildren ( const PriceElement * inRoot, OutPriceElement * outRoot, int pid, int discount ) {
	static const char s [ ] = "0123456789*#";
	for ( int i = 0; i < 12; i ++ ) {
		char c = s [ i ];
		const PriceElement * ti = inRoot -> getAt ( c );
		if ( ! ti )
			continue;
		if ( ! outRoot -> getAt ( c ) )
			outRoot -> setAt ( c, new OutPriceElement );
		OutPriceElement * to = outRoot -> getAt ( c );
		if ( to -> hasPeer ( pid ) )
			continue;
		if ( /*ti -> getPrice ( ) &&*/ ti -> getExists ( ) ) {
			bool enabled = ti -> getEnabled ( );
			double price = ti -> getPrice ( ) / 100.0;
			double connectPrice = ti -> getConnectPrice ( ) / 100.0;
			const int maxAllowedPrice = std :: numeric_limits < int > :: max ( ) / discount;
			if ( price > maxAllowedPrice || connectPrice > maxAllowedPrice )
				enabled = false;
			to -> append ( OutChoiceInt ( pid, int ( price * discount + 0.5 ),
				int ( connectPrice * discount + 0.5 ), enabled, ti -> getPrio ( ), ti -> getRound ( ) ) );
		}
		addChildren ( ti, to, pid, discount );
	}
}

union InPriceData {
	const char * p [ 10 ];
	struct {
		const char * code;
		const char * price;
		const char * connectPrice;
		const char * uid;
		const char * rmin;
		const char * reach;
		const char * rfree;
		const char * enabled;
		const char * prio;
		const char * minDigits;
		const char * maxDigits;
	} n;
};

static ss :: string translateCode ( const ss :: string & code ) {
	if ( __builtin_expect ( code.empty ( ), false ) || __builtin_expect ( code [ 0 ] != 'i', true ) )
		return code;
	if ( code == "internet" || code == "interne" || code == "inetmin" )
		return "****11####";
	if ( code == "inetmbi" )
		return "****22####";
	if ( code == "inetmbo" )
		return "****33####";
	return code;
}

static void addInPriceRecord ( InPriceData * p, PriceElement * root, double rate = 1 ) {
	ss :: string prefix = translateCode ( p -> n.code );
	while ( ! prefix.empty ( ) ) {
		if ( ! root -> getAt ( prefix [ 0 ] ) )
			root -> setAt ( prefix [ 0 ], new PriceElement ( root ) );
		root = root -> getAt ( prefix [ 0 ] );
		prefix.erase ( 0, 1 );
	}
	bool enabled = std :: atoi ( p -> n.enabled );
	double price = std :: atof ( p -> n.price ) / rate;
	double connectPrice = std :: atof ( p -> n.connectPrice ) / rate;
	const int maxAllowedPrice = std :: numeric_limits < int > :: max ( ) / 100000;
	if ( price > maxAllowedPrice || connectPrice > maxAllowedPrice )
		enabled = false;
	root -> setPrice ( int ( price * 100000 + 0.5 ), int ( connectPrice * 100000 + 0.5 ) );
	root -> setRound ( TarifRound ( std :: atoi ( p -> n.rmin ), std :: atoi ( p -> n.reach ), std :: atoi ( p -> n.rfree ) ) );
	root -> setEnabled ( enabled );
	root -> setPrio ( char ( std :: atoi ( p -> n.prio ) ) );
	root -> setDigits ( char ( std :: atoi ( p -> n.minDigits ) ), char ( std :: atoi ( p -> n.maxDigits ) ) );
	root -> setExists ( true );
}

static void printListSmart ( ostream & os, MyResult & r ) {
	if ( r.rowCount ( ) == 1 ) {
		os << " = " << * r.fetchRow ( );
		return;
	}
	os << " in ( ";
	bool first = true;
	while ( const char * * p = r.fetchRow ( ) ) {
		if ( first )
			first = false;
		else
			os << ", ";
		os << * p;
	}
	os << " )";
}

//static void doBreakPoint ( ) {
//	PSYSTEMLOG ( Info, "doBreakPoint" );
//}

static Pointer < AnumTranslatorOut > loadAnumGroupList ( MySQL & m, int id ) {
	MyResult r ( m.getQuery ( "select num from ANumReplaceLists where uid = %i", id ) );
	StringVector v;
	while ( const char * * p = r.fetchRow ( ) )
		v.push_back ( * p );
	Pointer < AnumTranslatorOut > rep;
	if ( v.empty ( ) )
		rep = new AnumTranslatorNone;
	else if ( v.size ( ) == 1 )
		rep = new AnumTranslatorOne ( v [ 0 ] );
	else
		rep = new AnumTranslatorList ( v );
	return rep;
}

static Pointer < AnumTranslatorOut > loadAnumGroupPrefix ( MySQL & m, int id ) {
	MyResult r ( m.getQuery ( "select prefix, realPrefix from ANumReplacePrefixes where uid = %i", id ) );
	StringStringMap v;
	while ( const char * * p = r.fetchRow ( ) )
		v.insert ( std :: make_pair ( p [ 0 ], p [ 1 ] ) );
	Pointer < AnumTranslatorOut > rep;
	if ( v.empty ( ) )
		rep = new AnumTranslatorNone;
	else
		rep = new AnumTranslatorPrefix ( v );
	return rep;
}

static Pointer < AnumTranslatorOut > loadAnumGroupRange ( MySQL & m, int id ) {
	MyResult r ( m.getQuery ( "select fromNum, toNum from ANumReplaceRanges where uid = %i", id ) );
	Pointer < AnumTranslatorOut > rep;
	if ( const char * * p = r.fetchRow ( ) )
		rep = new AnumTranslatorRange ( p [ 0 ], p [ 1 ] );
	else
		rep = new AnumTranslatorNone;
	return rep;
}

static void loadAnumClass ( MySQL & m, int repClass, AnumTranslatorIn & replaces ) {
	MyResult r ( m.getQuery ( "select id, outPeerGroup, type from ANumReplaceGroups where class = %i", repClass ) );
	union GroupData {
		const char * p [ 3 ];
		struct {
			const char * id;
			const char * group;
			const char * type;
		} n;
	};
	while ( GroupData * p = reinterpret_cast < GroupData * > ( r.fetchRow ( ) ) ) {
		int grp = std :: atoi ( p -> n.group );
		int id = std :: atoi ( p -> n.id );
		switch ( p -> n.type [ 0 ] ) {
			case 'l':
				replaces.add ( grp, loadAnumGroupList ( m, id ) );
				break;
			case 'p':
				replaces.add ( grp, loadAnumGroupPrefix ( m, id ) );
				break;
			case 'r':
				replaces.add ( grp, loadAnumGroupRange ( m, id ) );
				break;
			default:
				replaces.add ( grp, new AnumTranslatorNone );
		}
	}
	if ( replaces.empty ( ) )
		replaces.add ( 0, new AnumTranslatorNone );
}

ConfData :: ConfData ( MySQL & m, int nas, int bytesPerKb ) : cm ( m ), mgcpReloaded ( false ),
	rportAgentsReloaded ( false ) {
	PTime tt = PTime ( );
	nasId = nas;
	bool useIPAcct = m.getBool ( "select ival from RadConf where name = 'useIPAcct'" );
	TableLocker tl ( m, useIPAcct );
	allowLosses = m.getBool ( "select ival from RadConf where name = 'allowLosses'" );
	ss :: string now = m.getString ( "select now( )" );
	timeToChange = m.getString ( "select date_add( date_format( '%s', "
		"'%%Y-%%m-%%d 00:00:00' ), interval 1 day )", now.c_str ( ) );
	MyResult r ( m.getQuery ( "select * from DebugInIps" ) );
	while ( const char * * p = r.fetchRow ( ) )
		debugInIps.insert ( * p );
	r.reset ( m.getQuery ( "select id, name from Valutes" ) );
	while ( const char * * p = r.fetchRow ( ) ) {
		int valute = std :: atoi ( * p );
		double rate = m.getDouble ( "select rate from ValuteRates where uid = %i and startDate "
			"<= '%s' order by startDate desc limit 1", valute, now.c_str ( ) );
		if ( m.err_no ( ) )
			throw MySQLError ( m );
		if ( rate <= 0 )
			rate = 1;
		PSYSTEMLOG ( Info, "valute rate for " << p [ 1 ] << ": " << rate );
		valuteRates.insert ( std :: make_pair ( valute, rate ) );
	}

PSYSTEMLOG ( Info, "B1");

	_sigOptionsMap.load ( m );

PSYSTEMLOG ( Info, "B2");

	loadErrorResponses ( m );

PSYSTEMLOG ( Info, "B3");

	r.reset ( m.getQuery ( "select id, timeout from CallBuffers" ) );
	while ( const char * * p = r.fetchRow ( ) ) {
		int id = std :: atoi ( * p );
		int secs = std :: atoi ( p [ 1 ] );
		MyResult r1 ( m.getQuery ( "select code from CallBufferCodes where buffer = %i", id ) );
		IntSet codes;
		while ( const char * * p = r1.fetchRow ( ) )
			codes.insert ( std :: atoi ( * p ) );
		callBuffers [ id ] = new CallBuffer ( secs, codes );
	}
	r.reset ( m.getQuery ( "select OutPeers.id, lim, remotePrice = 'y', length( realPrefix ), prio, prefix, "
		"realPrefix, isGk = 'y', direct = 'y', codec, ruleSet, valute, protoClass, "
		"interfaces, minDigits, maxDigits, sigOptions, profitOptionsClass, aNumReplaceClass, allRoutes = 'y', "
		"concat( firstName, '-', secondName, '-', tag ), ifnull( rname, '' ), callBuffer from OutPeers left join Users "
		"using ( uname ) left join OutPeerGWS on OutPeers.id = peer where nas = %i and yn = 'y'", nas ) );
	union OutPeerData {
		const char * p [ 23 ];
		struct {
			const char * id;
			const char * lim;
			const char * remotePrice;
			const char * depth;
			const char * prio;
			const char * prefix;
			const char * realPrefix;
			const char * isGk;
			const char * directRTP;
			const char * cclass;
			const char * ruleSet;
			const char * valute;
			const char * protoClass;
			const char * interfaces;
			const char * minDigits;
			const char * maxDigits;
			const char * sigOptions;
			const char * profitOptions;
			const char * aNumReplaceClass;
			const char * allRoutes;
			const char * name;
			const char * rname;
			const char * callBuffer;
		} n;
	};
	while ( OutPeerData * p = reinterpret_cast < OutPeerData * > ( r.fetchRow ( ) ) ) {
		int id = std :: atoi ( p -> n.id );
		Pointer < OutPeerInfo > o = new OutPeerInfo;
		o -> limit = std :: atoi ( p -> n.lim );
		o -> remotePrice = std :: atoi ( p -> n.remotePrice );
		o -> depth = std :: atoi ( p -> n.depth );
		o -> prio = std :: atoi ( p -> n.prio );
		o -> prefix = p -> n.prefix;
		o -> realPrefix = p -> n.realPrefix;
		o -> isGk = std :: atoi ( p -> n.isGk );
		o -> directRTP = std :: atoi ( p -> n.directRTP );
		o -> minDigits = std :: atoi ( p -> n.minDigits );
		o -> maxDigits = std :: atoi ( p -> n.maxDigits );
		o -> allRoutes = std :: atoi ( p -> n.allRoutes );
		o -> name = p -> n.name;
		o -> rname = p -> n.rname;
		o -> callBuffer = callBuffers [ std :: atoi ( p -> n.callBuffer ) ];
		try {
			TimePrice tp = getTimePrice ( now, m.getInt ( "select rule from RuleSets where id = "
				"%s", p -> n.ruleSet ), timeToChange, m );
			o -> defPrice = tp.price;
			o -> discount = tp.discount ? : 100;
			o -> tarif = tp.tarif;
		} catch ( MySQLError & e ) {
			PSYSTEMLOG ( Error, "exception: " << e.what ( ) );
			PSYSTEMLOG ( Error, "outpeer " << id << " not loaded" );
			continue;
		}
		MyResult r1 ( m.getQuery ( "select ip, port, timeout, suffix, proto from OutIPAddresses where "
			"uid = %i", id ) );
		while ( const char * * p1 = r1.fetchRow ( ) ) {
			StringVector ips = translateIpMask ( p1 [ 0 ] );
			int port = std :: atoi ( p1 [ 1 ] );
			IpPortInfo ipi ( std :: atoi ( p1 [ 2 ] ), p1 [ 3 ],
				( p1 [ 4 ] [ 0 ] == 'h' ? ptH323 : ( p1 [ 4 ] [ 0 ] == 's' ? ptSip : ptMgcp ) ) );
			for ( StringVector :: const_iterator i = ips.begin ( ); i != ips.end ( ); ++ i )
				o -> ips.insert ( std :: make_pair ( IpPort ( * i, port ), ipi ) );
		}
		o -> allCodecs = false;
		r1.reset ( m.getQuery ( "select name, frames, canRecode = 'y', supported = 'y' from CodecClassList left "
			"join Codecs on CodecClassList.codec = Codecs.id where CodecClassList.uid = %s", p -> n.cclass ) );
		while ( const char * * p1 = r1.fetchRow ( ) ) {
			if ( ! * p1 ) {
				o -> allCodecs = true;
				break;
			}
			o -> allowedCodecs.insert ( CodecInfo ( p1 [ 0 ], std :: atoi ( p1 [ 1 ] ), std :: atoi ( p1 [ 2 ] ),
				std :: atoi ( p1 [ 3 ] ) ) );
		}
		if ( ! o -> allCodecs )
			o -> allowedCodecs.insert ( "t38fax" );
		r1.reset ( m.getQuery ( "select ip from OutAccessList where uid=%i", id ) );
		while ( const char * * p1 = r1.fetchRow ( ) ) {
			StringVector ips = translateIpMask ( * p1 );
			o -> rtpIps.insert ( ips.begin ( ), ips.end ( ) );
		}
		o -> valuteRate = valuteRates [ std :: atoi ( p -> n.valute ) ];
		if ( o -> valuteRate <= 0 )
			o -> valuteRate = 1;
		r1.reset ( m.getQuery ( "select name, fromValue, toValue from ProtoClassList left join "
			"ProtoParameterList on parameter = ProtoParameterList.id where uid = %s", p -> n.protoClass ) );
		while ( const char * * p = r1.fetchRow ( ) )
			o -> protoClass [ p [ 0 ] ].insert ( std :: make_pair ( std :: atoi ( p [ 1 ] ), std :: atoi ( p [ 2 ] ) ) );

		r1.reset ( m.getQuery ( "select OutPeers.id, ReleaseCompleteReplaceMap.causeRecv, "
			"ReleaseCompleteReplaceMap.causeSend from OutPeers left join ReleaseCompleteReplaceMap on "
			"OutPeers.releaseCompleteClass = ReleaseCompleteReplaceMap.class where OutPeers.id = %i and "
			"ReleaseCompleteReplaceMap.causeRecv is not null and ReleaseCompleteReplaceMap.causeSend is not null",
			id ) );
		union ReplaceMap {
			const char * p [ 3 ];
			struct {
				const char * id;
				const char * causeRecv;
				const char * causeSend;
			} n;
		};
		while ( ReplaceMap * p = reinterpret_cast < ReplaceMap * > ( r1.fetchRow ( ) ) ) {
			PSYSTEMLOG ( Info, "OutPeerRCM: " << p -> n.id << " causeRecv " << p -> n.causeRecv <<
				" causeSend " << p -> n.causeSend );
			o -> releaseCompleteReplaceMap.insert ( std :: make_pair ( std :: atoi ( p -> n.causeRecv ),
				std :: atoi ( p -> n.causeSend ) ) );
		}
		r1.reset ( m.getQuery ( "select OutPeers.id, MesgLatency.minCallProceedingLatency, "
			"MesgLatency.minAlertingLatency, MesgLatency.minConnectLatency, "
			"MesgLatency.minCallDuration from OutPeers "
			"left join MesgLatency on OutPeers.mesgLatencyClass = MesgLatency.class "
			"where OutPeers.id = %i and minCallProceedingLatency is not null "
			"and minAlertingLatency is not null and minConnectLatency is not null "
			"and minCallDuration is not null", id ) );
		union tmpLatencyLimits {
			const char * p [ 5 ];
			struct {
				const char * id;
				const char * minCallProceedingTime;
				const char * minAlertingTime;
				const char * minConnectTime;
				const char * minCallDuration;
			} n;
		};
		while ( tmpLatencyLimits * p = reinterpret_cast < tmpLatencyLimits * > ( r1.fetchRow ( ) ) ) {
			PSYSTEMLOG ( Info, "Latency for " << p -> n.id << ": " << p -> n.minCallProceedingTime <<
				' ' << p -> n.minAlertingTime << ' ' << p -> n.minConnectTime << ' ' <<
				p -> n.minCallDuration );
			o -> latencyLimits.minCallProceedingTime = std :: atol ( p -> n.minCallProceedingTime );
			o -> latencyLimits.minAlertingTime = std :: atol ( p -> n.minAlertingTime );
			o -> latencyLimits.minConnectTime = std :: atol ( p -> n.minConnectTime );
			o -> latencyLimits.minCallDuration = std :: atol ( p -> n.minCallDuration );
			o -> latencyLimits.enabled = true;
		}

		// updating OutGroupId for OutPeer
		o -> outGroupId = -1;
		r1.reset ( m.getQuery ( "select gid from OutGroupPeers where outId = %i", id ) );
		union OutPeerGroup {
			const char * p [ 1 ];
			struct {
				const char * gid;
			} n;
		};
		while ( OutPeerGroup * p = reinterpret_cast < OutPeerGroup * > ( r1.fetchRow ( ) ) ) {
			o -> outGroupId = std :: atoi ( p -> n.gid );
		}

		// updating A-number ReplaceMap
		int repClass = std :: atoi ( p -> n.aNumReplaceClass );
		AnumTranslatorIn & replaces = anumClasses [ repClass ];
		if ( replaces.empty ( ) )
			loadAnumClass ( m, repClass, replaces );
		o -> at = replaces.getOut ( );

		outPeers.insert ( std :: make_pair ( id, o ) );
		o -> interfaces = loadInterfaces ( m, p -> n.interfaces, nas );
		o -> sigOptions = _sigOptionsMap.get ( std :: atoi ( p -> n.sigOptions ) );
		o -> profitGuard = loadProfitOptions ( m, p -> n.profitOptions, o -> valuteRate );
		if ( defaultPrices.count ( o -> defPrice ) == 0 &&
			m.getInt ( "select count(*) from RateSheets where uid = %i", o -> defPrice ) > 0 )
			defaultPrices.insert ( std :: make_pair ( o -> defPrice, Pointer < PriceElement > ( new PriceElement ) ) );
		r1.reset ( m.getQuery ( "select rxmatch, rxreplace from RXReplaces where uid = %i order by orderby", id ) );
		while ( const char * * p = r1.fetchRow ( ) ) {
			PSYSTEMLOG ( Info, "adding s/" << p [ 0 ] << '/' << p [ 1 ] << '/' );
			o -> rxReplaces.push_back ( OutPeerInfo :: RXReplace ( p [ 0 ], p [ 1 ] ) );
		}
	}
	ss :: ostringstream os;
	os << "select code, price, connectPrice, pid, enabled in ( 'y', 'yForce' ), pprio, rmin, reach, rfree from OutPrice";
	if ( ! outPeers.empty ( ) ) {
		os << " where pid in ( ";
		bool first = true;
		for ( OutPeersMap :: const_iterator i = outPeers.begin ( ); i != outPeers.end ( ); i ++ ) {
			if ( ! first )
				os << ", ";
			else
				first = false;
			os << i -> first;
		}
		os << " )";
	}
	os << " order by length( code )";
	r.reset ( m.getQuery ( os.str ( ) ) );
	union OutPriceData {
	const char * p [ 9 ];
		struct {
			const char * code;
			const char * price;
			const char * connectPrice;
			const char * pid;
			const char * enabled;
			const char * prio;
			const char * rmin;
			const char * reach;
			const char * rfree;
		} n;
	};
	outPrices = new OutPriceElement;
	while ( OutPriceData * p = reinterpret_cast < OutPriceData * > ( r.fetchRow ( ) ) ) {
		int pid = std :: atoi ( p -> n.pid );
		const OutPeerInfo * o = getOutPeerInfo ( pid );
		if ( ! o )
			continue;
		ss :: string prefix = o -> realPrefix + p -> n.code;
		if ( prefix.empty ( ) )
			continue;
		OutPriceElement * root = outPrices;
		try {
			while ( ! prefix.empty ( ) ) {
				char c = prefix [ 0 ];
				if ( ! root -> getAt ( c ) )
					root -> setAt ( c, new OutPriceElement );
				root = root -> getAt ( c );
				prefix.erase ( 0, 1 );
			}
			bool enabled = std :: atoi ( p -> n.enabled );
			double price = std :: atof ( p -> n.price ) / o -> valuteRate;
			double connectPrice = std :: atof ( p -> n.connectPrice ) / o -> valuteRate;
			const int maxAllowedPrice = std :: numeric_limits < int > :: max ( ) / 100000;
			if ( price > maxAllowedPrice || connectPrice > maxAllowedPrice )
				enabled = false;
			root -> append ( OutChoiceInt ( pid, int ( price * 100000 + 0.5 ),
				int ( connectPrice * 100000 + 0.5 ), enabled, char ( std :: atoi ( p -> n.prio ) ),
				TarifRound ( char ( std :: atoi ( p -> n.rmin ) ), char ( std :: atoi ( p -> n.reach ) ),
				char ( std :: atoi ( p -> n.rfree ) ) ) ) );
		} catch ( UnknownDigit & e ) {
			continue;
		}
	}
	r.reset ( m.getQuery ( "select uname, inDep = 'y', rule, outGroup, maxCredit, password from Admins" ) );
	union ResponsibleData {
		const char * p [ 6 ];
		struct {
			const char * uname;
			const char * inDep;
			const char * rule;
			const char * outGroup;
			const char * maxCredit;
			const char * pass;
		} n;
	};
	while ( ResponsibleData * p = reinterpret_cast < ResponsibleData * > ( r.fetchRow ( ) ) ) {
		PSYSTEMLOG ( Info, "Adding responsible " << p -> n.uname );
		Pointer < ResponsibleInfo > i = new ResponsibleInfo;
		i -> useBalance = useBalanceDisabler && std :: atoi ( p -> n.inDep );
		int correspondent = m.getInt ( "select id from Users where uname = '%s'", p -> n.uname );
		i -> balance = 0;
		if ( i -> useBalance ) {
			i -> balance = m.getDouble ( "select ifnull( sum( defSum ), 0 ) from GrossBook where "
				"correspondent = %i and what='Pay' and toAccount and account=0", correspondent );
			if ( m.err_no ( ) )
				throw MySQLError ( m );
			i -> balance -= m.getDouble ( "select ifnull( sum( defSum ), 0 ) from GrossBook left join "
				"Accounts on Accounts.id = fromAccount where correspondent = '%i' and "
				"Accounts.isDeposit ='y' and what = 'Pay' and account=0", correspondent );
			if ( m.err_no ( ) )
				throw MySQLError ( m );
			MyResult r1 ( m.getQuery ( "select InPeers.id from InPeers left join Users "
				"using ( uname ) where rname = '%s'", p -> n.uname ) );
			if ( r1.rowCount ( ) ) {
				ss :: ostringstream os;
				os << "select ifnull( sum( mlen * price + connectSum ), 0 ) from FinReportInCache where "
					"FinReportInCache.valute = 0 and peerId ";
				printListSmart ( os, r1 );
				i -> balance -= m.getDouble ( os.str ( ) );
				if ( m.err_no ( ) )
					throw MySQLError ( m );
			}
			i -> balance -= m.getDouble ( "select ifnull( sum( money ), 0 ) from CardCallsCache "
				"left join Users using ( uname ) where rname = '%s' and valute = 0", p -> n.uname );
			if ( m.err_no ( ) )
				throw MySQLError ( m );
			if ( useIPAcct ) {
				r1.reset ( m.getQuery ( "select IPPeers.id from IPPeers left join Users "
					"on IPPeers.uid = Users.id where rname = '%s'", p -> n.uname ) );
				if ( r1.rowCount ( ) ) {
					ss :: ostringstream os;
					os << "select ifnull( ( sum( inBytes * inPrice ) + sum( outBytes * "
						"outPrice ) ) / " << bytesPerKb * bytesPerKb << ", 0 ) from IPAcct "
						"where peerId ";
					printListSmart ( os, r1 );
					i -> balance -= m.getDouble ( os.str ( ) );
					if ( m.err_no ( ) )
						throw MySQLError ( m );
				}
			}
			i -> balance += std :: atof ( p -> n.maxCredit );
		}
		i -> rule = std :: atoi ( p -> n.rule );
		i -> outGroup = std :: atoi ( p -> n.outGroup );
		i -> uname = p -> n.uname;
		i -> pass = p -> n.pass;
		if ( i -> useBalance )
			PSYSTEMLOG ( Info, "balance for " << p -> n.uname << ": " << i -> balance );
		responsibles [ p -> n.uname ] = i;
	}
	r.reset ( m.getQuery ( "select Users.id, Users.uname, maxBalance, inDep = 'y', rname, groupBalance = 'y' from "
		"Users left join InPeers on Users.uname = InPeers.uname left join InPeerGWS on peer = InPeers.id "
		"left join PrepaidIds on Users.uname = PrepaidIds.uname where ( InPeerGWS.nas = %i and yn = 'y' ) or "
		"( %i and PrepaidIds.prefix != '' ) group by Users.id", nas, isMaster ) );
	union CustomersData {
		const char * p [ 6 ];
		struct {
			const char * id;
			const char * uname;
			const char * minDep;
			const char * inDep;
			const char * rname;
			const char * groupBalance;
		} n;
	};
	while ( CustomersData * p = reinterpret_cast < CustomersData * > ( r.fetchRow ( ) ) ) {
		if ( ! responsibles.count ( p -> n.rname ) ) {
			PSYSTEMLOG ( Error, "no responsible for " << p -> n.rname );
			continue;
		}
		Pointer < CustomerInfo > c = new CustomerInfo;
		PSYSTEMLOG ( Info, "Adding customer " << p -> n.uname );
		c -> parent = responsibles [ p -> n.rname ];
		bool dealer = c -> parent -> useBalance;
		c -> customerBalance = useBalanceDisabler && ( std :: atoi ( p -> n.inDep ) || std :: atoi ( p -> n.groupBalance ) );
		c -> cardBalance = ! useBalanceDisabler || ( std :: atoi ( p -> n.groupBalance ) || ! std :: atoi ( p -> n.inDep ) );
		c -> balance = m.getDouble ( "select ifnull( sum( defSum ), 0 ) from GrossBook where "
			"correspondent = %s and what='Pay' and toAccount and account=0", p -> n.id );
		if ( m.err_no ( ) )
			throw MySQLError ( m );
		c -> balance -= m.getDouble ( "select ifnull( sum( defSum ), 0 ) from GrossBook left join "
			"Accounts on Accounts.id = fromAccount where correspondent = %s and "
			"Accounts.isDeposit ='y' and what = 'Pay' and account=0", p -> n.id );
		if ( m.err_no ( ) )
			throw MySQLError ( m );
		if ( dealer )
			c -> balance -= m.getDouble ( "select ifnull( sum( dmlen * dprice + dConnectSum ), 0 ) from "
				"FinReportInCache left join InPeers on peerId = InPeers.id where uname = '%s' and "
				"FinReportInCache.valute = 0", p -> n.uname );
		else
			c -> balance -= m.getDouble ( "select ifnull( sum( money ), 0 ) from ClientReportInCache"
				" left join InPeers on peerId=id where ClientReportInCache.valute = 0 and uname='%s'",
				p -> n.uname );
		if ( m.err_no ( ) )
			throw MySQLError ( m );
		if ( dealer )
			c -> balance -= m.getDouble ( "select ifnull( sum( euMoney ), 0 ) from CardCallsCache "
				"where uname = '%s' and valute = 0", p -> n.uname );
		else
			c -> balance -= m.getDouble ( "select ifnull( sum( money ), 0 ) from CardCallsCache "
				"where uname = '%s' and valute = 0", p -> n.uname );
		if ( m.err_no ( ) )
			throw MySQLError ( m );
		if ( useIPAcct ) {
			MyResult r1 ( m.getQuery ( "select IPPeers.id from IPPeers left join Users "
				"on IPPeers.uid = Users.id where uname = '%s'", p -> n.uname ) );
			if ( r1.rowCount ( ) ) {
				ss :: ostringstream os;
				os << "select ifnull( ( sum( inBytes * inPrice ) + sum( outBytes * "
					"outPrice ) ) / " << bytesPerKb * bytesPerKb << ", 0 ) from IPAcct "
					"where peerId ";
				printListSmart ( os, r1 );
				c -> balance -= m.getDouble ( os.str ( ) );
				if ( m.err_no ( ) )
					throw MySQLError ( m );
			}
		}
		c -> balance += std :: atof ( p -> n.minDep );
		if ( c -> customerBalance )
			PSYSTEMLOG ( Info, "balance for " << p -> n.uname << ": " << c -> balance );
		customers.insert ( std :: make_pair ( ss :: string ( p -> n.uname ), c ) );
	}
	r.reset ( m.getQuery ( "select InPeers.id, InPeers.name, lim, allRoutes = 'y', remotePrice = 'y', direct = 'y', "
		"InPeers.uname, codec, ruleSet, valute, minDigits, maxDigits, sigOptions, profitOptionsClass, "
		"aNumReplaceClass, externalRouting = 'y' from InPeers left join InPeerGWS on peer = InPeers.id where "
		"nas = %i and yn = 'y'", nas ) );
	union InPeersData {
		const char * p [ 16 ];
		struct {
			const char * id;
			const char * name;
			const char * lim;
			const char * allRoutes;
			const char * remotePrice;
			const char * directRTP;
			const char * uname;
			const char * cclass;
			const char * ruleSet;
			const char * valute;
			const char * minDigits;
			const char * maxDigits;
			const char * sigOptions;
			const char * profitOptions;
			const char * aNumReplaceClass;
			const char * externalRouting;
		} n;
	};
	while ( InPeersData * p = reinterpret_cast < InPeersData * > ( r.fetchRow ( ) ) ) {
		int id = std :: atoi ( p -> n.id );
		Pointer < InPeerInfo > i = new InPeerInfo;
		i -> id = id;
		i -> name = p -> n.name;
		i -> limit = std :: atoi ( p -> n.lim );
		i -> allRoutes = std :: atoi ( p -> n.allRoutes );
		i -> remotePrice = std :: atoi ( p -> n.remotePrice );
		i -> price = new PriceElement;
		i -> euPrice = new PriceElement;
		i -> directRTP = std :: atoi ( p -> n.directRTP );
		i -> uname = p -> n.uname;
		i -> minDigits = std :: atoi ( p -> n.minDigits );
		i -> maxDigits = std :: atoi ( p -> n.maxDigits );
		i -> externalRouting = std :: atoi ( p -> n.externalRouting );
		if ( ! customers.count ( i -> uname ) ) {
			PSYSTEMLOG ( Error, "no customer for " << i -> uname );
			continue;
		}
		i -> parent = customers [ i -> uname ];
		bool dealer = i -> parent -> parent -> useBalance;
		try {
			int rule;
			bool directClient = m.getBool ( "select directClient = 'y' from RuleSets where id = %s",
				p -> n.ruleSet );
			if ( dealer )
				rule = i -> parent -> parent -> rule;
			else
				rule = m.getInt ( "select rule from RuleSets where id = %s", p -> n.ruleSet );
			TimePrice tp = getTimePrice ( now, rule, timeToChange, m );
			i -> defPrice = tp.price;
			i -> discount = tp.discount;
			i -> allPrice = tp.discount == 0;
			i -> tarif = tp.tarif;
			if ( ! directClient )
				rule = m.getInt ( "select euRule from RuleSets where id = %s", p -> n.ruleSet );
			tp = getTimePrice ( now, rule, timeToChange, m );
			i -> euDefPrice = tp.price;
			i -> euDiscount = tp.discount;
			i -> euTarif = tp.tarif;
		} catch ( MySQLError & e ) {
			PSYSTEMLOG ( Error, "exception: " << e.what ( ) );
			PSYSTEMLOG ( Error, "inpeer " << id << " not loaded" );
			continue;
		}
		if ( i -> discount && defaultPrices.count ( i -> defPrice ) == 0 &&
			m.getInt ( "select count(*) from RateSheets where uid = %i", i -> defPrice ) > 0 )
			defaultPrices.insert ( std :: make_pair ( i -> defPrice, Pointer < PriceElement > ( new PriceElement ) ) );
		if ( i -> euDiscount && defaultPrices.count ( i -> euDefPrice ) == 0 &&
			m.getInt ( "select count(*) from RateSheets where uid = %i", i -> euDefPrice ) > 0 )
			defaultPrices.insert ( std :: make_pair ( i -> euDefPrice, Pointer < PriceElement > ( new PriceElement ) ) );
		PSYSTEMLOG ( Info, "Adding inpeer " << id << ", name: " << i -> name << ", limit: " <<
			i -> limit << ", allRoutes: " << i -> allRoutes << ", allPrice: " <<
			i -> allPrice << ", remotePrice: " << i -> remotePrice );
		MyResult r1 ( m.getQuery ( "select outId, routePrio, lim, direct, "
			"interfaces, sigOptions, forking from AllowedRoutes where inId = %i", id ) );
		while ( RouteInfoData * p1 = reinterpret_cast < RouteInfoData * > ( r1.fetchRow ( ) ) ) {
			int id = std :: atoi ( p1 -> n.outId );
			if ( const OutPeerInfo * o = getOutPeerInfo ( id ) ) {
				PSYSTEMLOG ( Info, "Adding route: " << id << " with prio " << p1 -> n.prio );
				i -> allowedRoutes.insert ( std :: make_pair ( id, RouteInfo ( m, nas, o, p1 ) ) );
			}
		}
		if ( dealer )
			r1.reset ( m.getQuery ( "select outId, routePrio, lim, direct, interfaces, sigOptions, forking from "
				"OutGroupPeers where gid = %i", i -> parent -> parent -> outGroup ) );
		else
			r1.reset ( m.getQuery ( "select outId, routePrio, lim, direct, interfaces, sigOptions, forking from "
				"AllowedGroups left join OutGroupPeers using ( gid ) where inId = %i and "
				"outId is not null", id ) );
		while ( RouteInfoData * p1 = reinterpret_cast < RouteInfoData * > ( r1.fetchRow ( ) ) ) {
			int id = std :: atoi ( p1 -> n.outId );
			if ( const OutPeerInfo * o = getOutPeerInfo ( id ) ) {
				if ( i -> allowedRoutes.count ( id ) == 0 ) {
					PSYSTEMLOG ( Info, "Adding route: " << id << " with prio " << p1 -> n.prio );
					i -> allowedRoutes.insert ( std :: make_pair ( id, RouteInfo ( m, nas, o, p1 ) ) );
				}
			}
		}
//		if ( id == 589 )
//			doBreakPoint ( );
		i -> allCodecs = false;
		r1.reset ( m.getQuery ( "select name, frames, canRecode = 'y', supported = 'y' from CodecClassList left "
			"join Codecs on CodecClassList.codec = Codecs.id where CodecClassList.uid = %s", p -> n.cclass ) );
		while ( const char * * p1 = r1.fetchRow ( ) ) {
			if ( ! * p1 ) {
				i -> allCodecs = true;
				break;
			}
			i -> allowedCodecs.insert ( CodecInfo ( p1 [ 0 ], std :: atoi ( p1 [ 1 ] ), std :: atoi ( p1 [ 2 ] ),
				std :: atoi ( p1 [ 3 ] ) ) );
		}
		if ( ! i -> allCodecs )
			i -> allowedCodecs.insert ( "t38fax" );
		r1.reset ( m.getQuery ( "select ip from InAccessList where uid=%i", id ) );
		while ( const char * * p1 = r1.fetchRow ( ) ) {
			StringVector ips = translateIpMask ( * p1 );
			i -> rtpIps.insert ( ips.begin ( ), ips.end ( ) );
		}
		i -> valuteRate = valuteRates [ std :: atoi ( p -> n.valute ) ];
		if ( i -> valuteRate <= 0 )
			i -> valuteRate = 1;
		i -> sigOptions = _sigOptionsMap.get ( std :: atoi ( p -> n.sigOptions ) );

		// now lets update ReleaseCompleteReplaceMaps
		r1.reset ( m.getQuery ( "select InPeers.id, ReleaseCompleteReplaceMap.causeRecv, "
			"ReleaseCompleteReplaceMap.causeSend from InPeers left join ReleaseCompleteReplaceMap "
			"on InPeers.releaseCompleteClass = ReleaseCompleteReplaceMap.class "
			"where InPeers.id = %i and ReleaseCompleteReplaceMap.causeRecv is not null "
			"and ReleaseCompleteReplaceMap.causeSend is not null", id ) );
		union ReplaceMap {
			const char * p [ 3 ];
			struct {
				const char * id;
				const char * causeRecv;
				const char * causeSend;
			} n;
		};
		while ( ReplaceMap * p = reinterpret_cast < ReplaceMap * > ( r1.fetchRow ( ) ) ) {
			PSYSTEMLOG ( Info, "InPeerRCM: " << p -> n.id << " causeRecv " << p -> n.causeRecv <<
				" causeSend " << p -> n.causeSend );
			i -> releaseCompleteReplaceMap.insert ( std :: make_pair ( std :: atoi ( p -> n.causeRecv ),
				std :: atoi ( p -> n.causeSend ) ) );
		}

		int repClass = std :: atoi ( p -> n.aNumReplaceClass );
		AnumTranslatorIn & replaces = anumClasses [ repClass ];
		if ( replaces.empty ( ) )
			loadAnumClass ( m, repClass, replaces );
		i -> at = & replaces;

		i -> profitGuard = loadProfitOptions ( m, p -> n.profitOptions, i -> valuteRate );

		inPeers.insert ( std :: make_pair ( id, i ) );
	}
	ipToPeer.load ( m, std :: tr1 :: bind ( & ConfData :: getInPeerInfo, this, std :: tr1 :: placeholders :: _1 ) );
	r.reset ( m.getQuery ( "select login, ip, password, prefix, realPrefix, uid, peerType + 0, '', 0 "
		"from GKLogins where login != ''" ) );
	union LoginToPeerData {
		const char * p [ 9 ];
		struct {
			const char * login;
			const char * ip;
			const char * password;
			const char * prefix;
			const char * realPrefix;
			const char * peerId;
			const char * peerType;
			const char * suffix;
			const char * timeout;
		} n;
	};
	while ( LoginToPeerData * p = reinterpret_cast < LoginToPeerData * > ( r.fetchRow ( ) ) ) {
		int peerType = std :: atoi ( p -> n.peerType );
		int inPeerId = ( peerType == InPeer ) ? std :: atoi ( p -> n.peerId ) : 0;
		int outPeerId = ( peerType == OutPeer ) ? std :: atoi ( p -> n.peerId ) : 0;
		if ( ( inPeers.count ( inPeerId ) != 1 ) && ( outPeers.count ( outPeerId ) != 1 ) ) {
			PSYSTEMLOG ( Warning, "GKLogin " << p -> n.login << ": inPeers(" << inPeerId << ").count="
				<< inPeers.count ( inPeerId ) << ", outPeers(" << outPeerId << ").count=" <<
				outPeers.count ( outPeerId ) );
			continue;
		}
		LoginInfoMap :: iterator loginInfo = loginToPeer.find ( p -> n.login );
		if ( loginInfo != loginToPeer.end ( ) ) { //already exist, updating
			if ( inPeerId ) loginInfo -> second.inPeerId = inPeerId;
			if ( outPeerId ) loginInfo -> second.outPeerId = outPeerId;
		} else { // doesn't exist, inserting
			loginToPeer.insert ( std :: make_pair ( p -> n.login, LoginInfo ( p -> n.ip, p -> n.password,
				p -> n.prefix, p -> n.realPrefix, p -> n.suffix, std :: atoi ( p -> n.timeout ),
				inPeerId, outPeerId ) ) );
		}
		PSYSTEMLOG ( Info, "Adding loginToPeer " << p -> n.login << ':' << p -> n.ip << ':' <<
			p -> n.password << ':' << p -> n.prefix << ':' << p -> n.realPrefix << ':' <<
			inPeerId << ':' << outPeerId << ')' );
	}
	r.reset ( m.getQuery ( "select code, price, connectPrice, uid, rmin, reach, rfree, enabled in ( 'y', 'yForce' ), "
		"- 1, minDigits, maxDigits from Price where code != '' order by length( code )" ) );
	while ( InPriceData * p = reinterpret_cast < InPriceData * > ( r.fetchRow ( ) ) ) {
		int uid = std :: atoi ( p -> n.uid );
		const InPeerInfo * i = getInPeerInfo ( uid );
		if ( ! i )
			continue;
		try {
			addInPriceRecord ( p, i -> price, i -> valuteRate );
		} catch ( UnknownDigit & e ) {
			continue;
		}
	}
	r.reset ( m.getQuery ( "select code, price, connectPrice, uid, rmin, reach, rfree, 1, "
		"- 1, 0, 0 from InEndUserPrice where code != '' order by length( code )" ) );
	while ( InPriceData * p = reinterpret_cast < InPriceData * > ( r.fetchRow ( ) ) ) {
		int uid = std :: atoi ( p -> n.uid );
		const InPeerInfo * i = getInPeerInfo ( uid );
		if ( ! i )
			continue;
		try {
			addInPriceRecord ( p, i -> euPrice, i -> valuteRate );
		} catch ( UnknownDigit & e ) {
			continue;
		}
	}
	r.reset ( m.getQuery ( "select PrepaidIds.id, PrepaidIds.name, prefix, PrepaidIds.uname, accRights, linesCount, "
		"ruleSet, valute, options, PrepaidIds.language, callback, requireCallbackAuth, allRoutes = 'y', "
		"outRuleSet, radiusClass, icqContacts, ifnull( rname, '' ), allowedInterfaces, singleAni = 'y' from "
		"PrepaidIds left join Users using ( uname ) where 1 = %i", ( usePrefixAuth && isMaster ) || startRadius ) );
	union PrepaidIdsData {
		const char * p [ 19 ];
		struct {
			const char * id;
			const char * name;
			const char * prefix;
			const char * uname;
			const char * accRights;
			const char * lines;
			const char * ruleSet;
			const char * valute;
			const char * options;
			const char * lang;
			const char * callback;
			const char * requireCallbackAuth;
			const char * allRoutes;
			const char * outRuleSet;
			const char * radiusClass;
			const char * icqContacts;
			const char * rname;
			const char * interfaces;
			const char * singleAni;
		} n;
	};
	while ( PrepaidIdsData * p = reinterpret_cast < PrepaidIdsData * > ( r.fetchRow ( ) ) ) {
		CustomerInfo * ci = getCustomerInfo ( p -> n.uname );
		if ( ! ci )
			continue;
		int id = std :: atoi ( p -> n.id );
		PSYSTEMLOG ( Info, "Adding prepaid prefix " << p -> n.prefix );
		Pointer < PrepaidInfo > i = new PrepaidInfo;
		prepaidPrefixes.insert ( std :: make_pair ( ss :: string ( p -> n.prefix ), id ) );
		i -> id = id;
		i -> name = p -> n.name;
		i -> uname = p -> n.uname;
		i -> lines = std :: atoi ( p -> n.lines );
		i -> allRoutes = std :: atoi ( p -> n.allRoutes );
		i -> price = new PriceElement;
		i -> euPrice = new PriceElement;
		i -> radiusClass = std :: atoi ( p -> n.radiusClass );
		i -> rname = p -> n.rname;
		i -> parent = ci;
		i -> callBack = p -> n.callback [ 0 ];
		i -> accRights = p -> n.accRights [ 0 ];
		i -> requireCallbackAuth = p -> n.requireCallbackAuth [ 0 ];
		i -> options = p -> n.options;
		i -> lang = p -> n.lang;
		i -> singleAni = std :: atoi ( p -> n.singleAni );
		IntVector tmpIfaces = loadInterfaces ( m, p -> n.interfaces, nas );
		if ( tmpIfaces.size ( ) != 1 || tmpIfaces [ 0 ] != INADDR_ANY )
			i -> interfaces.insert ( tmpIfaces.begin ( ), tmpIfaces.end ( ) );
		bool cardBalance = i -> cardBalance = ci -> cardBalance;
		bool customerBalance = ci -> customerBalance;
		bool responsibleBalance = ci -> parent -> useBalance;
		try {
			int rule;
			if ( responsibleBalance )
				rule = i -> parent -> parent -> rule;
			else
				rule = m.getInt ( "select rule from RuleSets where id = %s", p -> n.ruleSet );
			TimePrice tp = getTimePrice ( now, rule, timeToChange, m );
			i -> defPrice = tp.price;
			i -> discount = tp.discount;
			i -> tarif = tp.tarif;
			tp.discount = 0;
			try {
				tp = getTimePrice ( now, m.getInt ( "select rule from RuleSets where id = %s",
					p -> n.outRuleSet ), timeToChange, m );
			} catch ( MySQLError & e ) {
				PSYSTEMLOG ( Error, "exception on loading ruleSet " << p -> n.outRuleSet <<
					" for prepaidid " << id << ": " << e.what ( ) );
			}
			i -> cbDefPrice = tp.price;
			i -> cbDiscount = tp.discount;
			i -> cbTarif = tp.tarif;
			if ( customerBalance && ! cardBalance ) {
				i -> euDefPrice = i -> defPrice;
				i -> euDiscount = i -> discount;
				i -> euTarif = i -> tarif;
				i -> cbEuDefPrice = i -> cbDefPrice;
				i -> cbEuDiscount = i -> cbDiscount;
				i -> cbEuTarif = i -> cbTarif;
			} else {
				tp = getTimePrice ( now, m.getInt ( "select euRule from RuleSets where id = %s",
					p -> n.ruleSet ), timeToChange, m );
				i -> euDefPrice = tp.price;
				i -> euDiscount = tp.discount;
				i -> euTarif = tp.tarif;
				tp.discount = 0;
				try {
					tp = getTimePrice ( now, m.getInt ( "select euRule from RuleSets where id = %s",
						p -> n.outRuleSet ), timeToChange, m );
				} catch ( MySQLError & ) { }
				i -> cbEuDefPrice = tp.price;
				i -> cbEuDiscount = tp.discount;
				i -> cbEuTarif = tp.tarif;
			}
		} catch ( MySQLError & e ) {
			PSYSTEMLOG ( Error, "exception: " << e.what ( ) );
			PSYSTEMLOG ( Error, "prepaidid " << id << " not loaded" );
			continue;
		}
		if ( i -> discount && defaultPrices.count ( i -> defPrice ) == 0 &&
			m.getInt ( "select count(*) from RateSheets where uid = %i", i -> defPrice ) > 0 )
			defaultPrices.insert ( std :: make_pair ( i -> defPrice, Pointer < PriceElement > ( new PriceElement ) ) );
		if ( i -> euDiscount && defaultPrices.count ( i -> euDefPrice ) == 0 &&
			m.getInt ( "select count(*) from RateSheets where uid = %i", i -> euDefPrice ) > 0 )
			defaultPrices.insert ( std :: make_pair ( i -> euDefPrice, Pointer < PriceElement > ( new PriceElement ) ) );
		if ( i -> cbDiscount && defaultPrices.count ( i -> cbDefPrice ) == 0 &&
			m.getInt ( "select count(*) from RateSheets where uid = %i", i -> cbDefPrice ) > 0 )
			defaultPrices.insert ( std :: make_pair ( i -> cbDefPrice, Pointer < PriceElement > ( new PriceElement ) ) );
		if ( i -> cbEuDiscount && defaultPrices.count ( i -> cbEuDefPrice ) == 0 &&
			m.getInt ( "select count(*) from RateSheets where uid = %i", i -> cbEuDefPrice ) > 0 )
			defaultPrices.insert ( std :: make_pair ( i -> cbEuDefPrice, Pointer < PriceElement > ( new PriceElement ) ) );
		i -> valuteRate = valuteRates [ std :: atoi ( p -> n.valute ) ];
		if ( i -> valuteRate <= 0 )
			i -> valuteRate = 1;
		MyResult r1 ( m.getQuery ( "select nas from CardGWS where cards = %i", id ) );
		while ( const char * * p1 = r1.fetchRow ( ) )
			i -> slaves.insert ( std :: atoi ( * p1 ) );
		r1.reset ( m.getQuery ( "select dnisId from AllowedDnis where prepId = %i", id ) );
		while ( const char * * p1 = r1.fetchRow ( ) )
			i -> allowedDnis.insert ( std :: atoi ( * p1 ) );
		r1.reset ( m.getQuery ( "select prefix, realPrefix from InPrepaidPrefixes where uid = %i", id ) );
		while ( const char * * p1 = r1.fetchRow ( ) )
			i -> prefixes.insert ( std :: make_pair ( ss :: string ( p1 [ 0 ] ), ss :: string ( p1 [ 1 ] ) ) );
		r1.reset ( m.getQuery ( "select min, cost * 100000 from CardAmortiseCost where uid = %i", id ) );
		while ( const char * * p1 = r1.fetchRow ( ) )
			i -> amortise.insert ( std :: make_pair ( std :: atoi ( p1 [ 0 ] ), std :: atoi ( p1 [ 1 ] ) ) );
		r1.reset ( m.getQuery ( "select prefix, realPrefix, dialPrefix from CallBackPrefixes where uid = %i", id ) );
		while ( const char * * p1 = r1.fetchRow ( ) ) {
			PSYSTEMLOG ( Info, "Adding callback prefix " << p1 [ 0 ] << ':' << p1 [ 1 ] <<
				':' << p1 [ 2 ] );
			CallBackPrefix cp;
			cp.realPrefix = p1 [ 1 ];
			cp.dialPrefix = p1 [ 2 ];
			i -> callbackPrefixes.insert ( std :: make_pair ( ss :: string ( p1 [ 0 ] ), cp ) );
		}
		r1.reset ( m.getQuery ( "select outId, routePrio, lim, direct, interfaces, sigOptions, forking from "
			"PrepaidAllowedRoutes where prepId = %i", id ) );
		while ( RouteInfoData * p1 = reinterpret_cast < RouteInfoData * > ( r1.fetchRow ( ) ) ) {
			int id = std :: atoi ( p1 -> n.outId );
			if ( const OutPeerInfo * o = getOutPeerInfo ( id ) ) {
				PSYSTEMLOG ( Info, "Adding route: " << id << " with prio " << p1 -> n.prio );
				i -> allowedRoutes.insert ( std :: make_pair ( id, RouteInfo ( m, nas, o, p1 ) ) );
			}
		}
		r1.reset ( m.getQuery ( "select outId, routePrio, lim, direct, interfaces, sigOptions, forking from "
			"PrepaidAllowedGroups left join OutGroupPeers using ( gid ) where prepId = %i "
			"and outId is not null", id ) );
		while ( RouteInfoData * p1 = reinterpret_cast < RouteInfoData * > ( r1.fetchRow ( ) ) ) {
			int id = std :: atoi ( p1 -> n.outId );
			if ( const OutPeerInfo * o = getOutPeerInfo ( id ) ) {
				if ( i -> allowedRoutes.count ( id ) == 0 ) {
					PSYSTEMLOG ( Info, "Adding route: " << id << " with prio " << p1 -> n.prio );
					i -> allowedRoutes.insert ( std :: make_pair ( id, RouteInfo ( m, nas, o, p1 ) ) );
				}
			}
		}
		r1.reset ( m.getQuery ( "select icqNum, number, name from IcqContactList where uid = '%s'",
			p -> n.icqContacts ) );
		i -> icqContacts.reserve ( r1.rowCount ( ) );
		while ( const char * * p1 = r1.fetchRow ( ) )
			i -> icqContacts.push_back ( IcqContact ( std :: atoi ( p1 [ 0 ] ), p1 [ 1 ], p1 [ 2 ] ) );
		prepaidIds.insert ( std :: make_pair ( id, i ) );
	}
	r.reset ( m.getQuery ( "select code, price, connectPrice, uid, rmin, reach, rfree, 1, - 1, 0, 0 from CardsPrice "
		"where code != '' order by length( code )" ) );
	while ( InPriceData * p = reinterpret_cast < InPriceData * > ( r.fetchRow ( ) ) ) {
		int uid = std :: atoi ( p -> n.uid );
		const PrepaidInfo * i = getPrepaidInfo ( uid );
		if ( ! i )
			continue;
		try {
			addInPriceRecord ( p, i -> euPrice, i -> valuteRate );
		} catch ( UnknownDigit & e ) {
			continue;
		}
	}
	r.reset ( m.getQuery ( "select code, price, connectPrice, uid, rmin, reach, rfree, 1, - 1, 0, 0 from PrepaidPrice "
		"where code != '' order by length( code )" ) );
	while ( InPriceData * p = reinterpret_cast < InPriceData * > ( r.fetchRow ( ) ) ) {
		int uid = std :: atoi ( p -> n.uid );
		const PrepaidInfo * i = getPrepaidInfo ( uid );
		if ( ! i )
			continue;
		try {
			addInPriceRecord ( p, i -> price, i -> valuteRate );
		} catch ( UnknownDigit & e ) {
			continue;
		}
	}
	IntDoubleMap defPriceRates;
	for ( DefaultPricesMap :: const_iterator i = defaultPrices.begin ( );
		i != defaultPrices.end ( ); i ++ ) {
		int defPrice = i -> first;
		int valute = m.getInt ( "select valute from DefaultPrices where id = %i", defPrice );
		if ( m.err_no ( ) )
			throw MySQLError ( m );
		double rate = valuteRates [ valute ];
		if ( rate <= 0 )
			rate = 1;
		PSYSTEMLOG ( Info, "valute rate for defPrice " << defPrice << ": " << rate );
		defPriceRates.insert ( std :: make_pair ( defPrice, rate ) );
	}
	for ( DefaultPricesMap :: const_iterator i = defaultPrices.begin ( ); i != defaultPrices.end ( ); ++ i ) {
		int uid = i -> first;
		r.reset ( m.getQuery ( "select id from RateSheets where uid = %i and date_activate <= '%s' order by "
			"date_activate desc limit 1", uid, now.c_str ( ) ) );
		while ( const char * * p = r.fetchRow ( ) ) {
			MyResult r1 ( m.getQuery ( "select code, price, connectPrice, %i, rmin, reach, "
				"rfree, enabled in ( 'y', 'yForce' ), pprio, minDigits, maxDigits from InDefaultPrice "
				"where code != '' and uid = %s order by length( code )", uid, * p ) );
			while ( InPriceData * p = reinterpret_cast < InPriceData * > ( r1.fetchRow ( ) ) ) {
				try {
					addInPriceRecord ( p, i -> second, defPriceRates [ uid ] );
				} catch ( UnknownDigit & e ) {
					continue;
				}
			}
		}
		r.reset ( m.getQuery ( "select date_activate from RateSheets where uid = %i and date_activate > '%s' and "
			"date_activate < '%s' order by date_activate limit 1", uid, now.c_str ( ),
			timeToChange.c_str ( ) ) );
		if ( const char * * p = r.fetchRow ( ) )
			timeToChange = * p;
	}
	for ( OutPeersMap :: const_iterator i = outPeers.begin ( ); i != outPeers.end ( ); i ++ ) {
		const OutPeerInfo * o = i -> second;
		DefaultPricesMap :: const_iterator it = defaultPrices.find ( o -> defPrice );
		if ( it == defaultPrices.end ( ) )
			continue;
		const PriceElement * inRoot = it -> second;
		ss :: string prefix = o -> realPrefix;
		OutPriceElement * outRoot = outPrices;
		try {
			while ( ! prefix.empty ( ) ) {
				char c = prefix [ 0 ];
				if ( ! inRoot -> getAt ( c ) )
					break;
				prefix.erase ( 0, 1 );
				inRoot = inRoot -> getAt ( c );
				if ( ! outRoot -> getAt ( c ) )
					outRoot -> setAt ( c, new OutPriceElement );
				outRoot = outRoot -> getAt ( c );
			}
			if ( ! prefix.empty ( ) )
				continue;
			if ( outRoot -> hasPeer ( i -> first ) )
				continue;
			if ( inRoot -> getPrice ( ) ) {
				bool enabled = inRoot -> getEnabled ( );
				double price = inRoot -> getPrice ( ) / 100.0;
				double connectPrice = inRoot -> getConnectPrice ( ) / 100.0;
				const int maxAllowedPrice = std :: numeric_limits < int > :: max ( ) / o -> discount;
				if ( price > maxAllowedPrice || connectPrice > maxAllowedPrice )
					enabled = false;
				outRoot -> append ( OutChoiceInt ( i -> first, int ( price * o -> discount + 0.5 ),
					int ( connectPrice * o -> discount + 0.5 ), enabled, inRoot -> getPrio ( ),
					inRoot -> getRound ( ) ) );
			}
			addChildren ( inRoot, outRoot, i -> first, o -> discount );
		} catch ( UnknownDigit & e ) {
			continue;
		}
	}
	r.reset ( m.getQuery ( "select ip, id, radiusClass from gws where ss = 'y' or isRadClient = 'y'" ) );
	while ( const char * * p = r.fetchRow ( ) ) {
		slaves.insert ( std :: make_pair ( ss :: string ( * p ), std :: atoi ( p [ 1 ] ) ) );
		Pointer < NASInfo > n = new NASInfo;
		n -> id = std :: atoi ( p [ 1 ] );
		n -> radiusClass = std :: atoi ( p [ 2 ] );
		gws [ n -> id ] = n;
	}
	r.reset ( m.getQuery ( "select distinct tid from TarifDesc" ) );
	while ( const char * * p = r.fetchRow ( ) ) {
		int id = std :: atoi ( * p );
		Pointer < TarifInfo > t = new TarifInfo;
		MyResult r1 ( m.getQuery ( "select min, sec from TarifDesc where tid=%i", id ) );
		while ( const char * * p1 = r1.fetchRow ( ) )
			t -> minsToSecs.insert ( std :: make_pair ( std :: atoi ( p1 [ 0 ] ), std :: atoi ( p1 [ 1 ] ) ) );
		tarifs.insert ( std :: make_pair ( id, t ) );
	}
	r.reset ( m.getQuery ( "select ip, passwd, useAni = 'y', useDNIS = 'y', useACode = 'y', vSecret = 'y', SType, "
		"hasStart = 'y', sendDuration = 'y', radiusUnameOnly = 'y' from gws where isRadClient = 'y'" ) );
	while ( const char * * p = r.fetchRow ( ) ) {
		RadGWInfo i;
		i.secret = p [ 1 ];
		i.useAni = std :: atoi ( p [ 2 ] );
		i.useDnis = std :: atoi ( p [ 3 ] );
		i.useAcode = std :: atoi ( p [ 4 ] );
		i.verifySecret = std :: atoi ( p [ 5 ] );
		i.defaultServiceType = ( p [ 6 ] [ 0 ] == 'I' ? Radius :: sFramed : Radius :: sLogin );
		i.hasStart = std :: atoi ( p [ 7 ] );
		i.sendDuration = std :: atoi ( p [ 8 ] );
		i.unameOnly = std :: atoi ( p [ 9 ] );
		secrets.insert ( std :: make_pair ( p [ 0 ], i ) );
	}
	r.reset ( m.getQuery ( "select id, name, price, inet = 'y', access, radiusClass, prefixFilter from DNIS" ) );
	union DnisData {
		const char * p [ 7 ];
		struct {
			const char * id;
			const char * name;
			const char * price;
			const char * inet;
			const char * access;
			const char * radiusClass;
			const char * prefixFilter;
		} n;
	};
	while ( DnisData * p = reinterpret_cast < DnisData * > ( r.fetchRow ( ) ) ) {
		Pointer < DNISInfo > d = new DNISInfo;
		d -> id = std :: atoi ( p -> n.id );
		d -> price = int ( std :: atof ( p -> n.price ) * 100000 + 0.5 );
		d -> name = p -> n.name;
		d -> inet = std :: atoi ( p -> n.inet );
		d -> access = p -> n.access [ 0 ];
		d -> radiusClass = std :: atoi ( p -> n.radiusClass );
		MyResult r1 ( m.getQuery ( "select prefix from PrefixFilterItems where uid = %s", p -> n.prefixFilter ) );
		while ( const char * * p1 = r1.fetchRow ( ) )
			d -> disabledAniPrefixes.insert ( * p1 );
		dnis.insert ( std :: make_pair ( ss :: string ( p -> n.name ), d ) );
	}
	r.reset ( m.getQuery ( "select uid, name, value from RadiusClassList left join RadiusClassValues on parameter = "
		"RadiusClassValues.id where name is not null" ) );
	union RadiusClassesData {
		const char * p [ 3 ];
		struct {
			const char * id;
			const char * parameter;
			const char * value;
		} n;
	};
	while ( RadiusClassesData * p = reinterpret_cast < RadiusClassesData * > ( r.fetchRow ( ) ) )
		radiusClasses [ std :: atoi ( p -> n.id ) ].insert ( std :: make_pair ( ss :: string ( p -> n.parameter ),
			ss :: string ( p -> n.value ) ) );
	r.reset ( m.getQuery ( "select distinct localIp from Interfaces where nas = %i", nas ) );
	while ( const char * * p = r.fetchRow ( ) ) {
		localIps.insert ( * p );
		localIntIps.push_back ( PIPSocket :: Address ( * p ) );
	}
	r.reset ( m.getQuery ( "select ip, port, name, identifier, registrationPeriod, interfaces, login, "
		"gkType+0, pswd from GateKeepers" ) );
	union GateKeepersData {
		const char * p [ 9 ];
		struct {
			const char * ip;
			const char * port;
			const char * name;
			const char * identifier;
			const char * registrationPeriod;
			const char * interfaces;
			const char * login;
			const char * gkType;
			const char * pswd;
//			const char * isCNonce_;
		} n;
	};
	while ( GateKeepersData * p = reinterpret_cast < GateKeepersData * > ( r.fetchRow ( ) ) ) {
		Pointer < GateKeeperInfo > g = new GateKeeperInfo;
		g -> ip = p -> n.ip;
		g -> port = std :: atoi ( p -> n.port );
		g -> name = p -> n.name;
		g -> identifier = p -> n.identifier;
		g -> registerPeriod = std :: atoi ( p -> n.registrationPeriod );
		g -> login = p -> n.login;
		g -> gkType = std :: atoi ( p -> n.gkType );
		g -> pswd = p -> n.pswd;
		g -> bindIps = loadInterfaces ( m, p -> n.interfaces, nas );
		if ( g -> bindIps.size ( ) == 1 && g -> bindIps [ 0 ] == INADDR_ANY )
			g -> localIps = localIntIps;
		else
			g -> localIps = g -> bindIps;
//		g -> isCNonce_ = p -> n.isCNonce_ == "y"; eto bred
		gateKeepers.insert ( std :: make_pair ( ss :: string ( p -> n.ip ), g ) );
	}
	r.reset ( m.getQuery ( "select ip, port, fromCodec, toCodec from Recoders where nas = %i", nas ) );
	while ( const char * * p = r.fetchRow ( ) ) {
		ss :: string fname = m.getString ( "select name from Codecs where id = %s", p [ 2 ] );
		ss :: string tname = m.getString ( "select name from Codecs where id = %s", p [ 3 ] );
		recoders [ fname ].insert ( std :: make_pair ( tname, IpPort ( p [ 0 ], std :: atoi ( p [ 1 ] ) ) ) );
	}
	if ( ! noMgcp ) {
		typedef std :: map < unsigned, MgcpClassInfo, std :: less < unsigned >,
			__SS_ALLOCATOR < std :: pair < const unsigned, MgcpClassInfo > > > MgcpClassInfoMap;
		MgcpClassInfoMap mgcpClasses;
		union MgcpClassData {
			const char * p [ 10 ];
			struct {
				const char * id;
				const char * maxDatagram;
				const char * sBearerInformation;
				const char * sRestartMethod;
				const char * sRestartDelay;
				const char * sReasonCode;
				const char * sPackageList;
				const char * sMaxMGCPDatagram;
				const char * sPersistentEvents;
				const char * sNotificationState;
			} n;
		};
		r.reset ( m.getQuery ( "select id, maxDatagram, sBearerInformation, sRestartMethod, sRestartDelay, "
			"sReasonCode, sPackageList, sMaxMGCPDatagram, sPersistentEvents, sNotificationState from "
			"MgcpClasses" ) );
		while ( MgcpClassData * p = reinterpret_cast < MgcpClassData * > ( r.fetchRow ( ) ) ) {
			MgcpClassInfo & i = mgcpClasses [ std :: atoi ( p -> n.id ) ];
			i.maxDatagram = short ( std :: atoi ( p -> n.maxDatagram ) );
			i.sBearerInformation = p -> n.sBearerInformation [ 0 ] == 'y';
			i.sRestartMethod = p -> n.sRestartMethod [ 0 ] == 'y';
			i.sRestartDelay = p -> n.sRestartDelay [ 0 ] == 'y';
			i.sReasonCode = p -> n.sReasonCode [ 0 ] == 'y';
			i.sPackageList = p -> n.sPackageList [ 0 ] == 'y';
			i.sMaxMGCPDatagram = p -> n.sMaxMGCPDatagram [ 0 ] == 'y';
			i.sPersistentEvents = p -> n.sPersistentEvents [ 0 ] == 'y';
			i.sNotificationState = p -> n.sNotificationState [ 0 ] == 'y';
		}
		union MgcpEndpointsData {
			const char * p [ 6 ];
			struct {
				const char * gw;
				const char * ip;
				const char * cls;
				const char * ep;
				const char * inPeer;
				const char * outPeer;
			} n;
		};
		r.reset ( m.getQuery ( "select MgcpGateways.name, ip, class, MgcpEndpoints.name, inPeer, outPeer from "
			"MgcpEndpoints left join MgcpGateways on ( gw = MgcpGateways.id ) where ip is not null and ( inPeer "
			"!= 0 or outPeer != 0 ) and MgcpGateways.name != '' and MgcpEndpoints.name != '' order by 1" ) );
		MgcpGatewayInfo g;
		while ( MgcpEndpointsData * p = reinterpret_cast < MgcpEndpointsData * > ( r.fetchRow ( ) ) ) {
			if ( g.name != p -> n.gw ) {
				if ( ! g.eps.empty ( ) )
					mgcpGws.push_back ( g );
				g.name = p -> n.gw;
				g.ip = p -> n.ip;
				g.cls = mgcpClasses [ std :: atoi ( p -> n.cls ) ];
				g.eps.clear ( );
			}
			MgcpEndpointInfo e;
			e.inPeer = std :: atoi ( p -> n.inPeer );
			if ( e.inPeer && ! getInPeerInfo ( e.inPeer ) )
				e.inPeer = 0;
			OutPeersMap :: iterator outPeer = outPeers.find ( std :: atoi ( p -> n.outPeer ) );
			if ( ! e.inPeer && outPeer == outPeers.end ( ) )
				continue;
			e.name = p -> n.ep;
			if ( outPeer != outPeers.end ( ) ) {
				OutPeerInfo & op = * outPeer -> second;
				if ( op.mgcpGw.empty ( ) ) {
					op.mgcpGw = g.name;
					op.mgcpEp = e.name;
				}
			}
			g.eps.push_back ( e );
		}
		if ( ! g.eps.empty ( ) )
			mgcpGws.push_back ( g );
	}
	r.reset ( m.getQuery ( "select userAgent from RportAgents" ) );
	rportAgents.reserve ( r.rowCount ( ) );
	while ( const char * * p = r.fetchRow ( ) )
		rportAgents.push_back ( * p );

	PSYSTEMLOG ( Info, "Updating IPSS logLevel" );
	r.reset ( m.getQuery ( "select ival from RadConf where name = 'logLevel_ipss_start'" ) );
	int ipssLogStart = 0; // FIXME: eto nado delat v reloadConf
	if ( const char * * p = r.fetchRow ( ) )
		ipssLogStart = std :: atoi ( * p );
	r.reset ( m.getQuery ( "select ival from RadConf where name = 'logLevel_ipss_end'" ) );
	int ipssLogEnd = 0;
	if ( const char * * p = r.fetchRow ( ) )
		ipssLogEnd = std :: atoi ( * p );
	RTPRequest req ( rqEnableLog );
	req.data [ 0 ] = ipssLogStart;
	req.data [ 1 ] = ipssLogEnd;
	rtpPipe -> send ( req );

	//Via header support for SIP
	supportVia = false;
	r.reset ( m.getQuery ( "select ival from RadConf where name = 'supportVia'" ) );
	if ( const char * * p = r.fetchRow ( ) )
		supportVia = std :: atoi ( * p );
	PSYSTEMLOG ( Info, "Updating isNoChoiceCode" );
	r.reset ( m.getQuery ( "select bval from RadConf where name = 'isNoChoiceCode'" ) );
	if ( const char * * p = r.fetchRow ( ) )
		isNoChoiceCode = * * p == 'y';

	PTimeInterval allTime = PTime ( ) - tt;
	m_reloadConfigTime = double ( allTime.GetMilliSeconds ( ) ) /1000;
	PSYSTEMLOG ( Error, "Config reloaded in " << allTime << " sec" );
}

bool ConfData :: isValidInPeerAddress ( const ss :: string & addr ) const {
	return ipToPeer.validInPeerAddress ( addr );
}

int ConfData :: getPrice ( const ss :: string & realDigits, const PriceElement * root, int defPrice,
	int discount, int & connectPrice, TarifRound & round, ss :: string & code, unsigned & minDigits,
	unsigned & maxDigits, bool canZero ) const {
	connectPrice = 0;
	try {
		ss :: string tmp = translateCode ( realDigits );
		code.clear ( );
		const PriceElement * last = root;
		ss :: string tmpCode;
		while ( ! tmp.empty ( ) && root -> getAt ( tmp [ 0 ] ) ) {
			root = root -> getAt ( tmp [ 0 ] );
			tmpCode += tmp [ 0 ];
			tmp.erase ( 0, 1 );
			if ( root -> getExists ( ) ) {
				last = root;
				code = tmpCode;
			}
		}
		connectPrice = last -> getConnectPrice ( );
		round = last -> getRound ( );
		minDigits = last -> getMinDigits ( );
		maxDigits = last -> getMaxDigits ( );
		if ( ! last -> getEnabled ( ) ) {
			PSYSTEMLOG ( Info, "inPrice is disabled" );
			return 0;
		}
		int price = last -> getPrice ( );
		if ( price )
			return price;
		else if ( canZero && ! code.empty ( ) )
			return - 1;
		code.clear ( );
		DefaultPricesMap :: const_iterator dpit = defaultPrices.find ( defPrice );
		if ( dpit == defaultPrices.end ( ) ) {
			PSYSTEMLOG ( Info, "no default price " << defPrice );
			return 0;
		}
		last = root = dpit -> second;
		tmpCode.clear ( );
		tmp = translateCode ( realDigits );
		while ( ! tmp.empty ( ) && root -> getAt ( tmp [ 0 ] ) ) {
			root = root -> getAt ( tmp [ 0 ] );
			tmpCode += tmp [ 0 ];
			tmp.erase ( 0, 1 );
			if ( root -> getExists ( ) ) {
				last = root;
				code = tmpCode;
			}
		}
		connectPrice = last -> getConnectPrice ( );
		round = last -> getRound ( );
		minDigits = last -> getMinDigits ( );
		maxDigits = last -> getMaxDigits ( );
		if ( ! last -> getEnabled ( ) ) {
			PSYSTEMLOG ( Info, "inPrice is disabled" );
			return 0;
		}
		price = int ( last -> getPrice ( ) / 100.0 * discount + 0.5 );
		if ( ! price ) {
			if ( canZero && ! code.empty ( ) )
				return - 1;
			PSYSTEMLOG ( Info, "inPrice is 0" );
		}
		return price;
	} catch ( std :: exception & e ) {
		PSYSTEMLOG ( Error, "getPrice: exception: " << e.what ( ) );
		return 0;
	}
}

static double getBalance ( const ss :: string & uname, const StringCustomerMoneySpoolMap & ccalls ) {
	StringCustomerMoneySpoolMap :: const_iterator it = ccalls.find ( uname );
	if ( it != ccalls.end ( ) )
		return it -> second -> getMoneyUsed ( );
	return 0;
}

bool ConfData :: getCallInfoIn ( SourceData & source, const StringCustomerMoneySpoolMap & ccalls,
	const ss :: string & realDigits, bool & allPrice, bool & allRoutes,
	const RoutesMap * & allowedRoutes, unsigned & minDigits, unsigned & maxDigits ) const {
	const InPeerInfo * ip = getInPeerInfo ( source.peer );
	if ( ! ip ) {
		PSYSTEMLOG ( Info, "no peer " << source.peer );
		return false;
	}
	const CustomerInfo * ci = ip -> parent;
	const ResponsibleInfo * ri = ci -> parent;
	double dealerBalance = getBalance ( ri -> uname, ccalls );
	if ( ri -> useBalance && ri -> balance <= dealerBalance ) {
		PSYSTEMLOG ( Info, "Dealer balance for " << source.peer << " is " << ri -> balance - dealerBalance );
		return false;
	}
	double balance = getBalance ( ip -> uname, ccalls );
	if ( ci && ci -> customerBalance && ci -> balance <= balance ) {
		PSYSTEMLOG ( Info, "Balance for " << source.peer << " is " << ci -> balance - balance );
		return false;
	}
	if ( realDigits.empty ( ) ) {
		PSYSTEMLOG ( Info, "bad prefix" );
		return false;
	}
	allRoutes = ip -> allRoutes;
	allowedRoutes = & ip -> allowedRoutes;
	allPrice = ip -> allPrice;
	source.name = ip -> name;
	source.rname = ip -> parent -> parent -> uname;
	if ( ip -> remotePrice ) {
		if ( source.price ) {
			if ( prioritizeRemotePriceOverAllPrice )
				allPrice = false;
			else if ( allPrice )
				source.price = 0;
			return true;
		}
		PSYSTEMLOG ( Info, "zero remote price" );
		return false;
	}
	if ( allPrice )
		return true;
	source.price = getPrice ( realDigits, ip -> price, ip -> defPrice, ip -> discount, source.connectPrice,
		source.round, source.code, minDigits, maxDigits, true );
	unsigned dummy;
	source.euPrice = getPrice ( realDigits, ip -> euPrice, ip -> euDefPrice, ip -> euDiscount,
		source.euConnectPrice, source.euRound, source.euCode, dummy, dummy, true );
	source.tarif = ip -> tarif;
	source.euTarif = ip -> euTarif;
	if ( ! source.price )
		PSYSTEMLOG ( Info, "zero price" );
	return source.price;
}

bool ConfData :: getCallInfoPrepaid ( SourceData & source, const StringCustomerMoneySpoolMap & ccalls,
	const ss :: string & dialedDigits, ss :: string & realDigits, int & errn,
	bool & allRoutes, const RoutesMap * & allowedRoutes ) {
	ss :: string prefDigits = dialedDigits;
	ss :: string pref;
	int peer = 0;
	for ( StringIntMultiMap :: const_iterator i = prepaidPrefixes.begin ( );
		i != prepaidPrefixes.end ( ); i ++ ) {
		if ( prefDigits.substr ( 0, i -> first.size ( ) ) == i -> first &&
			i -> first.size ( ) > pref.size ( ) ) {
			pref = i -> first;
			peer = i -> second;
		}
	}
	if ( pref.empty ( ) ) {
		PSYSTEMLOG ( Info, "no prefix" );
		return false;
	}
	ss :: string :: size_type ps = pref.size ( );
	ss :: string :: size_type pos = prefDigits.find ( pref, ps + 1 );
	if ( pos == ss :: string :: npos ) {
		PSYSTEMLOG ( Info, "no second prefix" );
		return false;
	}
	source.acctn = prefDigits.substr ( ps, pos - ps );
	const CardInfo * card = 0;
	const PrepaidInfo * pp = 0;
	for ( StringIntMultiMap :: const_iterator i = prepaidPrefixes.begin ( );
		i != prepaidPrefixes.end ( ); i ++ ) {
		if ( pref == i -> first ) {
			pp = getPrepaidInfo ( i -> second );
			if ( ! pp )
				continue;
			if ( ! pp -> slaves.count ( getSlaveId ( source.ip ) ) )
				continue;
			card = cardByAcctn ( source.acctn );
			if ( card && card -> parent != pp )
				card = 0;
			if ( card ) {
				if ( card -> status != 'c' && card -> accRightsVoice ) {
					peer = i -> second;
					break;
				} else
					card = 0;
			}
		}
	}
	if ( ! card ) {
		PSYSTEMLOG ( Info, "no card " << source.acctn << " with prefix '" << pref << '\'' );
		return false;
	}
	ss :: string :: size_type pos2 = prefDigits.find ( pref, pos + ps + 1 );
	if ( pos2 == ss :: string :: npos ) {
		PSYSTEMLOG ( Info, "no third prefix" );
		return false;
	}
	ss :: string pin = prefDigits.substr ( pos + ps, pos2 - pos - ps );
	const char * pinChar = pin.c_str ( );
	while ( * pinChar == '0' )
		pinChar ++;
	ss :: string md5pin = md5Sum ( pinChar, std :: strlen ( pinChar ) );
	const char * table = "0123456789abcdef";
	for ( int i = 0; i < 16; i ++ ) {
		if ( table [ md5pin [ i ] / 16 ] != card -> md5 [ i * 2 ] ) {
			PSYSTEMLOG ( Error, "md5 " << i * 2 << " is " << md5pin [ i ] / 16 << " should be " <<
				card -> md5 [ i * 2 ] );
			return false;
		}
		if ( table [ md5pin [ i ] % 16 ] != card -> md5 [ i * 2 + 1 ] ) {
			PSYSTEMLOG ( Error, "md5 " << i * 2 + 1 << " is " << md5pin [ i ] % 16 << " should be " <<
				card -> md5 [ i * 2 + 1 ] );
			return false;
		}
	}
	realDigits = dialedDigits.substr ( pos2 + ps );
	return getCallInfoPrepaidInt ( card, source, ccalls, realDigits, errn, allRoutes, allowedRoutes );
}

bool ConfData :: getCallInfoPrepaidInt ( const CardInfo * card, SourceData & source,
	const StringCustomerMoneySpoolMap & ccalls, ss :: string & realDigits, int & errn, bool & allRoutes,
	const RoutesMap * & allowedRoutes ) const {
	StringStringMap :: const_iterator pi = findPrefix ( card -> parent -> prefixes, realDigits );
	if ( pi == card -> parent -> prefixes.end ( ) ) {
		PSYSTEMLOG ( Error, "no realprefix for " << realDigits );
		return false;
	}
	realDigits = pi -> second + realDigits.substr ( pi -> first.size ( ) );
	const PrepaidInfo * pp = card -> parent;
	allRoutes = pp -> allRoutes;
	allowedRoutes = & pp -> allowedRoutes;
	source.type = SourceData :: card;
	unsigned minDigits, maxDigits; //dummy
	source.price = getPrice ( realDigits, pp -> price, pp -> defPrice, pp -> discount, source.connectPrice,
		source.round, source.code, minDigits, maxDigits, true );
	if ( ! source.price )
		PSYSTEMLOG ( Info, "no price" );
	source.euPrice = getPrice ( realDigits, pp -> euPrice, pp -> euDefPrice, pp -> euDiscount,
		source.euConnectPrice, source.euRound, source.euCode, minDigits, maxDigits, true );
	if ( ! source.euPrice )
		PSYSTEMLOG ( Info, "no euprice" );
	source.amortise.clear ( );
	for ( IntIntMap :: const_iterator i = pp -> amortise.begin ( ); i != pp -> amortise.end ( ); i ++ )
		source.amortise [ i -> first * 60 ] += i -> second / 100000.0;
	source.valuteRate = card -> valuteRate;
	source.tarif = pp -> tarif;
	source.euTarif = pp -> euTarif;
	if ( card -> responsibleBalance ) {
		const ResponsibleInfo * ri = pp -> parent -> parent;
		double balance = getBalance ( pp -> uname, ccalls );
		if ( ri -> balance <= balance && source.price > 0 ) {
			PSYSTEMLOG ( Info, "Balance for " << ri -> uname << " is " << ri -> balance - balance );
			return false;
		}
	}
	if ( card -> customerBalance ) {
		const CustomerInfo * ci = pp -> parent;
		double balance = getBalance ( pp -> uname, ccalls );
		if ( ci -> balance <= balance && source.price > 0 ) {
			PSYSTEMLOG ( Info, "Balance for " << pp -> uname << " is " << ci -> balance - balance );
			return false;
		}
	}
	if ( ! card -> credit && card -> cardBalance && source.euPrice > 0 &&
		source.euRound.roundDown ( int ( ( card -> cost * 100000.0 - source.euConnectPrice ) * 60 /
		abs ( source.euPrice ) ) ) <= 0 ) {
		errn = 25;
		PSYSTEMLOG ( Info, "not enough money on card" );
		return false;
	}
	if ( source.price && source.euPrice ) {
		source.peer = pp -> id;
		source.name = pp -> name;
		source.rname = pp -> rname;
		return true;
	}
	return false;
}

bool ConfData :: getCallInfoPrepaidChecked ( SourceData & source, const StringCustomerMoneySpoolMap & ccalls,
	const ss :: string & dialedDigits, ss :: string & realDigits, int & errn, bool & allRoutes,
	const RoutesMap * & allowedRoutes ) {
	const CardInfo * card = cardByAcctn ( source.acctn );
	if ( ! card ) {
		PSYSTEMLOG ( Info, "no card " << source.acctn );
		return false;
	}
	realDigits = dialedDigits;
	return getCallInfoPrepaidInt ( card, source, ccalls, realDigits, errn, allRoutes, allowedRoutes );
}

template < typename _Set1, typename _Set2 > static bool intersectionIsEmpty ( const _Set1 & s1, const _Set2 & s2 ) {
	typename _Set1 :: const_iterator i1 = s1.begin ( );
	typename _Set2 :: const_iterator i2 = s2.begin ( );
	while ( i1 != s1.end ( ) && i2 != s2.end ( ) ) {
		if ( * i1 < * i2 )
			++ i1;
		else if ( * i2 < * i1 )
			++ i2;
		else
			return false;
	}
	return true;
}

int ConfData :: getInPeer ( const ss :: string & ip, const ss :: string & callingDigits,
	const ss :: string & dialedDigits, ss :: string & realDigits, int & fromNat, ss :: string & inPrefix,
	int & retCode ) const {
	return ipToPeer.getInPeer ( ip, callingDigits, dialedDigits, realDigits, fromNat, inPrefix, retCode );
}

OutCardDetails ConfData :: getOutCardDetails ( const CardInfo * c, const ss :: string & digits, bool redirect ) const {
	const PrepaidInfo * p = c -> parent;
	TarifRound r, er;
	ss :: string code, euCode;
	unsigned minDigits, maxDigits; //dummy
	int connectPrice;
	int price = getPrice ( digits, Pointer < PriceElement > ( new PriceElement ), p -> cbDefPrice,
		p -> cbDiscount, connectPrice, r, code, minDigits, maxDigits, true );
	if ( ! price )
		throw std :: logic_error ( "no cbprice" );
	int euConnectPrice;
	int euPrice = getPrice ( digits, Pointer < PriceElement > ( new PriceElement ),
		p -> cbEuDefPrice, p -> cbEuDiscount, euConnectPrice, er, euCode, minDigits, maxDigits, true );
	if ( ! euPrice )
		throw std :: logic_error ( "no cbeuprice" );
	if ( price == - 1 )
		price = 0;
	if ( euPrice == - 1 )
		euPrice = 0;
	return OutCardDetails ( digits, code, euCode, redirect, price, connectPrice, euPrice, euConnectPrice,
		p -> cbTarif, p -> cbEuTarif, r, er, p -> amortise );
}

static bool hasRecodes ( const StringStringSetMap & recodes, const CodecInfoSet & from,
	const CodecInfoSet & to ) {
	for ( CodecInfoSet :: const_iterator i = from.begin ( ); i != from.end ( ); ++ i ) {
		StringStringSetMap :: const_iterator r = recodes.find ( i -> getCodec ( ) );
		if ( r != recodes.end ( ) && ! intersectionIsEmpty ( to, r -> second ) )
			return true;
	}
	return false;
}

bool hasRecodes ( const CodecInfoSet & inAllowedCodecs, const CodecInfoSet & outAllowedCodecs ) {
	const StringStringSetMap & recodes = conf -> getRecodes ( );
	ss :: for_each ( recodes, SLMapPrint ( "recodes: " ) );
	return hasRecodes ( recodes, inAllowedCodecs, outAllowedCodecs ) &&
		hasRecodes ( recodes, outAllowedCodecs, inAllowedCodecs );
}

struct OutChoicePeer {
	int codeLen;
	int price;
	int connectPrice;
	bool enabled;
	char prio;
	TarifRound round;
	OutChoicePeer ( ) { }
	OutChoicePeer ( const OutChoiceInt & c, int cl ) : codeLen ( cl ), price ( c.price ),
		connectPrice ( c.connectPrice ), enabled ( c.enabled ), prio ( c.prio ), round ( c.round ) { }
};

bool ConfData :: getCallInfoPreliminary ( SourceData & source, const StringCustomerMoneySpoolMap & ccalls,
	const ss :: string & dialedDigits, ss :: string & realDigits, int & errn, bool & allPrice, bool & inAllRoutes,
	const RoutesMap * & allowedRoutes, unsigned & minDigits, unsigned & maxDigits, bool & onlyToMaster ) {
	if ( source.type == SourceData :: card )
		return getCallInfoPrepaidChecked ( source, ccalls, dialedDigits, realDigits, errn, inAllRoutes, allowedRoutes );
	if ( isMaster ) {
		const InPeerInfo * ip = getInPeerInfo ( source.peer );
		if ( ! ip || ip -> remotePrice )
			if ( getCallInfoPrepaid ( source, ccalls, dialedDigits, realDigits, errn, inAllRoutes,
				allowedRoutes ) )
				return true;
		if ( source.type == SourceData :: inbound )
			return getCallInfoIn ( source, ccalls, realDigits, allPrice, inAllRoutes, allowedRoutes,
				minDigits, maxDigits );
		return false;
	}
	if ( source.type != SourceData :: inbound ) {
		if ( ! usePrefixAuth )
			return false;
	 	onlyToMaster = true;
	 	allPrice = true;
	 	realDigits = dialedDigits;
	 	return true;
	}
	return getCallInfoIn ( source, ccalls, realDigits, allPrice, inAllRoutes, allowedRoutes, minDigits, maxDigits );
}

bool ConfData :: getCallInfo ( SourceData & source, const ss :: string & dialedDigits,
	CodecInfoVector & incomingCodecsVector, const StringCustomerMoneySpoolMap & ccalls,
	const RegCardsMap & registeredCards, const RegisteredOutPeersMap & registeredOutPeers, OutChoiceSetVector & vv,
	StringVector & outAcctns, ss :: string & realDigits, int & errn, bool h323, bool sip, bool mgcp, bool & inAllCodecs,
	CodecInfoSet & inAllowedCodecs, bool & externalRouting ) {

//	PSYSTEMLOG(Info, "ConfData :: getCallInfo");

	bool allPrice = false;
	bool onlyToMaster = false;
	bool inAllRoutes = false;
	const RoutesMap * allowedRoutes = 0;
	unsigned minDigits = 0, maxDigits = 0;
	if ( ! getCallInfoPreliminary ( source, ccalls, dialedDigits, realDigits, errn, allPrice, inAllRoutes,
		allowedRoutes, minDigits, maxDigits, onlyToMaster ) )
		return false;
	if ( source.price == - 1 )
		source.price = 0;
	if ( source.euPrice == - 1 )
		source.euPrice = 0;
	if ( source.type != SourceData :: inbound )
		inAllCodecs = true;
	else if ( const InPeerInfo * ip = getInPeerInfo ( source.peer ) ) {
		if ( minDigits < ip -> minDigits)
			minDigits = ip -> minDigits;
		if ( maxDigits > ip -> maxDigits)
			maxDigits = ip -> maxDigits;
		if ( minDigits > realDigits.size ( ) ) {
			PSYSTEMLOG ( Error, "too little digits" );
			errn = 28;
			return false;
		}
		if ( maxDigits < realDigits.size ( ) ) {
			PSYSTEMLOG ( Error, "too many digits" );
			errn = 28;
			return false;
		}
		if ( ip -> allCodecs )
			inAllCodecs = true;
		else
			inAllowedCodecs = ip -> allowedCodecs;
		externalRouting = ip -> externalRouting;
	}
	CodecInfoContainer & incomingCodecs = incomingCodecsVector.c;
	if ( ! incomingCodecs.empty ( ) ) {
		if ( inAllCodecs ) {
			inAllCodecs = false;
			inAllowedCodecs.insert ( incomingCodecs.begin ( ), incomingCodecs.end ( ) );
			inAllowedCodecs.insert ( "t38fax" );
		} else {
			typedef CodecInfoContainer :: index < Codec > :: type ByCodec;
			ByCodec & byCodec = incomingCodecs.get < Codec > ( );
			for ( CodecInfoSet :: iterator i = inAllowedCodecs.begin ( );
				i != inAllowedCodecs.end ( ); ) {
				if ( i -> getCodec ( ) != "t38fax" && ! byCodec.count ( * i ) ) {
					CodecInfoSet :: iterator t = i;
					++ i;
					inAllowedCodecs.erase ( t );
				} else
					++ i;
			}
			//tut navernoe mogno i po spisku a ne po mapu idti
			for ( CodecInfoContainer :: iterator i = incomingCodecs.begin ( ); i != incomingCodecs.end ( ); ) {
				CodecInfoSet :: const_iterator j = inAllowedCodecs.find ( * i );
				if ( j == inAllowedCodecs.end ( ) ) {
					CodecInfoContainer :: iterator t = i;
					++ i;
					incomingCodecs.erase ( t );
				} else {
					if ( int f = j -> getFrames ( ) )
						incomingCodecs.modify ( i, std :: tr1 :: bind ( 
							& CodecInfo :: setFrames, std :: tr1 :: placeholders :: _1,
							f ) );
					++ i;
				}
			}
		}
	}
	if ( ! inAllCodecs && inAllowedCodecs.size ( ) == 1 && inAllowedCodecs.begin ( ) -> getCodec ( ) == "t38fax" ) {
		PSYSTEMLOG ( Error, "no allowed codecs on inbound" );
		return false;
	}
	typedef std :: multimap < int, OutCardDetails, std :: less < int >,
		__SS_ALLOCATOR < std :: pair < const int, OutCardDetails > > > NumberChoicesMultimap;
	typedef std :: vector < NumberChoicesMultimap, __SS_ALLOCATOR < NumberChoicesMultimap > > NumberChoicesVector;
	NumberChoicesVector numberChoicesVector;
	StringVector forks;
	if ( const CardInfo * card = cardByNumber ( realDigits ) ) {
		if ( card -> status == 'c' ) {
			PSYSTEMLOG ( Error, "attempted redirect to closed card " << card -> acctn );
			return false;
		}
		StringStringSetMap :: const_iterator i = card -> forks.find ( realDigits );
		if ( i == card -> forks.end ( ) ) {
			forks.push_back ( realDigits );
			numberChoicesVector.push_back ( NumberChoicesMultimap ( ) );
		} else {
			for ( StringSet :: const_iterator j = i -> second.begin ( ); j != i -> second.end ( ); ++ j ) {
				forks.push_back ( * j );
				numberChoicesVector.push_back ( NumberChoicesMultimap ( ) );
			}
		}
	} else {
		forks.push_back ( realDigits );
		numberChoicesVector.push_back ( NumberChoicesMultimap ( ) );
	}
	bool hasForkingRoute = false;
	source.allPrice = allPrice;
	vv.reserve ( forks.size ( ) );
	for ( unsigned i = 0; i < forks.size ( ); ++ i ) {
		allPrice = source.allPrice;
		OutChoiceContainer v;
		ss :: string outAcctn;
		realDigits = forks [ i ];
		NumberChoicesMultimap & numberChoices = numberChoicesVector [ i ];
		if ( const CardInfo * card = cardByNumber ( realDigits ) ) {
			bool needZeroPrice = false, needZeroEuPrice = false;
			if ( card -> cardBalance && card -> cost <= 0 && ! card -> credit )
				needZeroEuPrice = true;
			if ( card -> customerBalance && getBalance ( card -> parent -> uname, ccalls )
				>= card -> parent -> parent -> balance )
				needZeroPrice = true;
			if ( card -> responsibleBalance && getBalance ( card -> parent -> parent -> parent -> uname, ccalls )
				>= card -> parent -> parent -> parent -> balance )
				needZeroPrice = true;
			inAllRoutes = true;
			allPrice = false;
			outAcctn = card -> acctn;
			RegCardsMap :: const_iterator rc = registeredCards.find ( outAcctn );
			if ( rc != registeredCards.end ( ) && ( ( rc -> second.h323 && h323 ) ||
				( ! rc -> second.h323 && sip ) ) ) {
				bool fn = rc -> second.fromNat;
				const RegisteredCard & c = rc -> second;
				try {
					OutCardDetails ocd ( getOutCardDetails ( card, realDigits, false ) );
					if ( ( needZeroPrice && ocd.getPrice ( ) ) || ( needZeroEuPrice && ocd.getEuPrice ( ) ) )
						PSYSTEMLOG ( Error, "attempt to redirect to empty card " << card -> acctn );
					else
						v.insert ( OutChoice ( 0, 0, 0, ( fn ? c.rip : c.ip ),
							( fn ? c.rport : c.port ), realDigits, 0, 0, ocd.getCode ( ), 0,
							false, IntVector ( 1, INADDR_ANY ), card -> getSigOptions ( ), - 1,
							ocd, 0, fn, rc -> second.rasSock, rc -> second.h323 ? ptH323 : ptSip,
							0, TarifRound ( ), realDigits, realDigits, ss :: string ( ),
							ss :: string ( ) ) );
				} catch ( std :: logic_error & e ) {
					PSYSTEMLOG ( Error, "exception: " << e.what ( ) );
				}
			}
			for ( IntStringMultiMap :: const_iterator i = card -> redirects.begin ( );
				i != card -> redirects.end ( ); ++ i ) {
				try {
					OutCardDetails ocd ( getOutCardDetails ( card, i -> second, true ) );
					if ( ( needZeroPrice && ocd.getPrice ( ) ) || ( needZeroEuPrice && ocd.getEuPrice ( ) ) )
						PSYSTEMLOG ( Error, "attempt to redirect to empty card " << card -> acctn );
					else
						numberChoices.insert ( std :: make_pair ( i -> first, ocd ) );
				} catch ( std :: logic_error & e ) {
					PSYSTEMLOG ( Error, "exception: " << e.what ( ) );
				}
			}
		} else
			numberChoices.insert ( std :: make_pair ( 0, OutCardDetails ( realDigits, source.price ) ) );

		for ( NumberChoicesMultimap :: const_iterator numberChoice = numberChoices.begin ( );
			numberChoice != numberChoices.end ( ); ++ numberChoice ) {
			ss :: string tmp = numberChoice -> second.getDigits ( );
			ss :: string rdigs = tmp;
			ss :: string :: size_type rds = tmp.size ( );
			const OutPriceElement * root = outPrices;
			typedef std :: map < int, OutChoicePeer, std :: less < int >,
				__SS_ALLOCATOR < std :: pair < const int, OutChoicePeer > > > PeerChoicesMap;
			PeerChoicesMap peerChoices;
			while ( true ) {
				const OutPriceElement :: ChoicesSet & choices = root -> getChoices ( );
				for ( OutPriceElement :: ChoicesSet :: const_iterator i = choices.begin ( );
					i != choices.end ( ); ++ i )
					peerChoices [ i -> pid ] = OutChoicePeer ( * i, int ( rds - tmp.size ( ) ) );
				if ( tmp.empty ( ) || ! root -> getAt ( tmp [ 0 ] ) )
					break;
				root = root -> getAt ( tmp [ 0 ] );
				tmp.erase ( 0, 1 );
			}
			for ( PeerChoicesMap :: const_iterator i = peerChoices.begin ( ); i != peerChoices.end ( ); ++ i ) {
				if ( ! i -> second.enabled )
					continue;
				int peer = i -> first;
				const OutPeerInfo * op = getOutPeerInfo ( peer );
				if ( ! op )
					continue;
				RoutesMap :: const_iterator ri;
				bool allRoutes = ! allowedRoutes || ( inAllRoutes && op -> allRoutes );
				if ( ! allRoutes ) {
					ri = allowedRoutes -> find ( peer );
					if ( ri == allowedRoutes -> end ( ) )
						continue;
					if ( forks.size ( ) == 1 && ri -> second.forking )
						hasForkingRoute = true;
				}
				if ( onlyToMaster ) {
					if ( ! op -> remotePrice )
						continue;
				} else {
					if ( op -> minDigits > rds || op -> maxDigits < rds )
						continue;
				}
				int price = i -> second.price;
				// checking Profits
				bool profitGuardEnabled = false;
				if ( const InPeerInfo * ip = getInPeerInfo ( source.peer ) ) {
					if ( ! ip -> profitGuard.isProfitable ( numberChoice -> second.getPrice ( ), price ) &&
						! allowLosses ) {
						PSYSTEMLOG ( Info, "Choice " << peer << " is not profitable (InPeer). inPrice="
							<< numberChoice -> second.getPrice ( ) << ", outPrice=" <<
							price << " ( " << ip -> profitGuard.getMinProfitAbs ( ) <<
							", " << ip -> profitGuard.getMinProfitRel ( ) << "% )" );
						continue;
					}
					profitGuardEnabled = ip -> profitGuard.isEnabled ( );
				}
				if ( ! op -> profitGuard.isProfitable ( numberChoice -> second.getPrice ( ), price ) &&
					! allowLosses ) {
					PSYSTEMLOG ( Info, "Choice " << peer << " is not profitable (OutPeer). inPrice="
						<< numberChoice -> second.getPrice ( ) << ", outPrice=" << price <<
						" ( " << op -> profitGuard.getMinProfitAbs ( ) << ", " <<
						op -> profitGuard.getMinProfitRel ( ) << "% )" );
					continue;
				}
				if ( op -> profitGuard.isEnabled ( ) )
					profitGuardEnabled = true;
				if ( ! profitGuardEnabled && ! allowLosses && ! allPrice && price >=
					numberChoice -> second.getPrice ( ) /*&& price != 0*/ && outAcctn.empty ( ) )
					continue;
				if ( ! inAllCodecs && ! op -> allCodecs &&
					intersectionIsEmpty ( inAllowedCodecs, op -> allowedCodecs ) &&
					! hasRecodes ( inAllowedCodecs, op -> allowedCodecs ) )
					continue;
				int prio = op -> prio;
				if ( ! allRoutes && ri -> second.prio >= 0 )
					prio = ri -> second.prio;
				if ( i -> second.prio >= 0 )
					prio = i -> second.prio;
				OutPeerInfo :: IpsMap registeredIps;
				const OutPeerInfo :: IpsMap * ips = & registeredIps;
				if ( op -> ips.empty ( ) ) {
					const RegisteredOutPeer * outPeer = registeredOutPeers.getPeer ( peer );
					if ( ! outPeer )
						continue;
					registeredIps.insert ( std :: make_pair ( IpPort ( outPeer -> getIpAddr ( ),
						outPeer -> getPort ( ) ), IpPortInfo ( outPeer -> getTimeout ( ),
						outPeer -> getSuffix ( ), outPeer -> getH323 ( ) ? ptH323 : ptSip ) ) );
				} else
					ips = & op -> ips;
				ss :: string pre;
				pre.reserve ( 16 );
				pre.append ( op -> prefix );
				pre.append ( rdigs, op -> depth, ss :: string :: npos );
				ss :: string code ( rdigs, 0, i -> second.codeLen );
				OutPeerInfo :: IpsMap :: const_iterator ipse = ips -> end ( );
				for ( OutPeerInfo :: IpsMap :: const_iterator it = ips -> begin ( );
					it != ipse; ++ it ) {
					if ( ( disableSIP || ! sip ) && it -> second.type == ptSip )
						continue;
					if ( ! h323 && it -> second.type == ptH323 )
						continue;
					if ( ! mgcp && it -> second.type == ptMgcp )
						continue;
					int port = it -> first.port;
					const ss :: string & ip = it -> first.ip;
					OutChoice o ( price, i -> second.connectPrice, prio, ip, port,
						pre, op -> depth, peer,
						code, allRoutes ? 0 : ri -> second.lim,
						allRoutes ? op -> directRTP : ri -> second.directRTP,
						allRoutes ? op -> interfaces : ri -> second.interfaces,
						allRoutes ? op -> sigOptions : ri -> second.sigOptions,
						numberChoice -> first, numberChoice -> second, it -> second.timeout, false,
						0, it -> second.type, op -> tarif, i -> second.round, rdigs,
						getSentDigits ( rdigs, op, it -> second.suffix ), op -> mgcpGw,
						op -> mgcpEp );
					if ( deleteIp ( v, o ) )
						v.insert ( o );
				}
			}
		}
		if ( ! v.empty ( ) ) {
			vv.push_back ( OutChoiceSet ( ) );
			OutChoiceSet & t = * -- vv.end ( );
			std :: copy ( v.begin ( ), v.end ( ), std :: inserter ( t, t.begin ( ) ) );
			if ( outAcctns.empty ( ) )
				source.outAcctn = outAcctn;
			outAcctns.push_back ( outAcctn );
		}
	}

	if ( hasForkingRoute ) {
		OutChoiceSetVector t;
		std :: swap ( t, vv );
		StringVector o;
		std :: swap ( o, outAcctns );
		for ( OutChoiceSet :: const_iterator i = t [ 0 ].begin ( ); i != t [ 0 ].end ( ); ++ i ) {
			OutChoiceSet s;
			s.insert ( * i );
			vv.push_back ( s );
			outAcctns.push_back ( source.acctn );
		}
	}

	if ( vv.empty ( ) )
		PSYSTEMLOG ( Info, "no choices" );
	return ! vv.empty ( );
}

bool ConfData :: getCallInfo ( SourceData & source, const ss :: string & dialedDigits,
	CodecInfoVector & incomingCodecsVector, const StringCustomerMoneySpoolMap & ccalls,
	const RegCardsMap & registeredCards, const RegisteredOutPeersMap & registeredOutPeers, OutChoiceSetVector & vv,
	StringVector & outAcctns, ss :: string & realDigits, int & errn, bool h323, bool sip, bool mgcp, bool & inAllCodecs,
	CodecInfoSet & inAllowedCodecs,
	std :: tr1 :: function < void ( int peer, const ss :: string & rdigs, IntVector * outPeers ) > externalRoute ) {

	bool allPrice = false;
	bool onlyToMaster = false;
	bool inAllRoutes = false;
	const RoutesMap * allowedRoutes = 0;
	unsigned minDigits = 0, maxDigits = 0;
	bool externalRouting = false;
	if ( ! getCallInfoPreliminary ( source, ccalls, dialedDigits, realDigits, errn, allPrice, inAllRoutes,
		allowedRoutes, minDigits, maxDigits, onlyToMaster ) )
		return false;
	if ( source.price == - 1 )
		source.price = 0;
	if ( source.euPrice == - 1 )
		source.euPrice = 0;
	if ( source.type != SourceData :: inbound )
		inAllCodecs = true;
	else if ( const InPeerInfo * ip = getInPeerInfo ( source.peer ) ) {
		if ( maxDigits == 0 ) {
			minDigits = ip -> minDigits;
			maxDigits = ip -> maxDigits;
		}
		if ( minDigits > realDigits.size ( ) ) {
			PSYSTEMLOG ( Error, "too little digits" );
			errn = 28;
			return false;
		}
		if ( maxDigits < realDigits.size ( ) ) {
			PSYSTEMLOG ( Error, "too many digits" );
			errn = 28;
			return false;
		}
		if ( ip -> allCodecs )
			inAllCodecs = true;
		else
			inAllowedCodecs = ip -> allowedCodecs;
		externalRouting = ip -> externalRouting;
	}
	CodecInfoContainer & incomingCodecs = incomingCodecsVector.c;
	if ( ! incomingCodecs.empty ( ) ) {
		if ( inAllCodecs ) {
			inAllCodecs = false;
			inAllowedCodecs.insert ( incomingCodecs.begin ( ), incomingCodecs.end ( ) );
			inAllowedCodecs.insert ( "t38fax" );
		} else {
			typedef CodecInfoContainer :: index < Codec > :: type ByCodec;
			ByCodec & byCodec = incomingCodecs.get < Codec > ( );
			for ( CodecInfoSet :: iterator i = inAllowedCodecs.begin ( );
				i != inAllowedCodecs.end ( ); ) {
				if ( i -> getCodec ( ) != "t38fax" && ! byCodec.count ( * i ) ) {
					CodecInfoSet :: iterator t = i;
					++ i;
					inAllowedCodecs.erase ( t );
				} else
					++ i;
			}
			//tut navernoe mogno i po spisku a ne po mapu idti
			for ( CodecInfoContainer :: iterator i = incomingCodecs.begin ( ); i != incomingCodecs.end ( ); ) {
				CodecInfoSet :: const_iterator j = inAllowedCodecs.find ( * i );
				if ( j == inAllowedCodecs.end ( ) ) {
					CodecInfoContainer :: iterator t = i;
					++ i;
					incomingCodecs.erase ( t );
				} else {
					if ( int f = j -> getFrames ( ) )
						incomingCodecs.modify ( i, std :: tr1 :: bind (
							& CodecInfo :: setFrames,
							std :: tr1 :: placeholders :: _1, f ) );
					++ i;
				}
			}
		}
	}
	if ( ! inAllCodecs && inAllowedCodecs.size ( ) == 1 && inAllowedCodecs.begin ( ) -> getCodec ( ) == "t38fax" ) {
		PSYSTEMLOG ( Error, "no allowed codecs on inbound" );
		return false;
	}
	typedef std :: multimap < int, OutCardDetails, std :: less < int >,
		__SS_ALLOCATOR < std :: pair < const int, OutCardDetails > > > NumberChoicesMultimap;
	typedef std :: vector < NumberChoicesMultimap, __SS_ALLOCATOR < NumberChoicesMultimap > > NumberChoicesVector;
	NumberChoicesVector numberChoicesVector;
	StringVector forks;
	if ( const CardInfo * card = cardByNumber ( realDigits ) ) {
		if ( card -> status == 'c' ) {
			PSYSTEMLOG ( Error, "attempted redirect to closed card " << card -> acctn );
			return false;
		}
		StringStringSetMap :: const_iterator i = card -> forks.find ( realDigits );
		if ( i == card -> forks.end ( ) ) {
			forks.push_back ( realDigits );
			numberChoicesVector.push_back ( NumberChoicesMultimap ( ) );
		} else {
			for ( StringSet :: const_iterator j = i -> second.begin ( ); j != i -> second.end ( ); ++ j ) {
				forks.push_back ( * j );
				numberChoicesVector.push_back ( NumberChoicesMultimap ( ) );
			}
		}
	} else {
		forks.push_back ( realDigits );
		numberChoicesVector.push_back ( NumberChoicesMultimap ( ) );
	}
	bool hasForkingRoute = false;
	source.allPrice = allPrice;
	vv.reserve ( forks.size ( ) );
	for ( unsigned i = 0; i < forks.size ( ); ++ i ) {
		allPrice = source.allPrice;
		OutChoiceContainer v;
		ss :: string outAcctn;
		realDigits = forks [ i ];
		NumberChoicesMultimap & numberChoices = numberChoicesVector [ i ];
		if ( const CardInfo * card = cardByNumber ( realDigits ) ) {
			bool needZeroPrice = false, needZeroEuPrice = false;
			if ( card -> cardBalance && card -> cost <= 0 && ! card -> credit )
				needZeroEuPrice = true;
			if ( card -> customerBalance && getBalance ( card -> parent -> uname, ccalls )
				>= card -> parent -> parent -> balance )
				needZeroPrice = true;
			if ( card -> responsibleBalance && getBalance ( card -> parent -> parent -> parent -> uname, ccalls )
				>= card -> parent -> parent -> parent -> balance )
				needZeroPrice = true;
			inAllRoutes = true;
			allPrice = false;
			outAcctn = card -> acctn;
			RegCardsMap :: const_iterator rc = registeredCards.find ( card -> acctn );
			if ( rc != registeredCards.end ( ) && ( ( rc -> second.h323 && h323 ) ||
				( ! rc -> second.h323 && sip ) ) ) {
				bool fn = rc -> second.fromNat;
				const RegisteredCard & c = rc -> second;
				try {
					OutCardDetails ocd ( getOutCardDetails ( card, realDigits, false ) );
					if ( ( needZeroPrice && ocd.getPrice ( ) ) || ( needZeroEuPrice && ocd.getEuPrice ( ) ) )
						PSYSTEMLOG ( Error, "attempt to redirect to empty card " << card -> acctn );
					else
						v.insert ( OutChoice ( 0, 0, 0, ( fn ? c.rip : c.ip ),
							( fn ? c.rport : c.port ), realDigits, 0, 0, ocd.getCode ( ), 0,
							false, IntVector ( 1, INADDR_ANY ), card -> getSigOptions ( ), - 1,
							ocd, 0, fn, rc -> second.rasSock, rc -> second.h323 ? ptH323 : ptSip,
							0, TarifRound ( ), realDigits, realDigits, ss :: string ( ),
							ss :: string ( ) ) );
				} catch ( std :: logic_error & e ) {
					PSYSTEMLOG ( Error, "exception: " << e.what ( ) );
				}
			}
			for ( IntStringMultiMap :: const_iterator i = card -> redirects.begin ( );
				i != card -> redirects.end ( ); ++ i ) {
				try {
					OutCardDetails ocd ( getOutCardDetails ( card, i -> second, true ) );
					if ( ( needZeroPrice && ocd.getPrice ( ) ) || ( needZeroEuPrice && ocd.getEuPrice ( ) ) )
						PSYSTEMLOG ( Error, "attempt to redirect to empty card " << card -> acctn );
					else
						numberChoices.insert ( std :: make_pair ( i -> first, ocd ) );
				} catch ( std :: logic_error & e ) {
					PSYSTEMLOG ( Error, "exception: " << e.what ( ) );
				}
			}
		} else
			numberChoices.insert ( std :: make_pair ( 0, OutCardDetails ( realDigits, source.price ) ) );

		for ( NumberChoicesMultimap :: const_iterator numberChoice = numberChoices.begin ( );
			numberChoice != numberChoices.end ( ); ++ numberChoice ) {
			ss :: string tmp = numberChoice -> second.getDigits ( );
			ss :: string rdigs = tmp;
			IntIntMap outPeersMap;
			if ( externalRouting ) {
				IntVector outPeers;
				externalRoute ( source.peer, rdigs, & outPeers );
				for ( IntVector :: size_type i = 0, sz = outPeers.size ( ); i < sz; i ++ )
					outPeersMap [ outPeers [ i ] ] = int ( i );
			}
			ss :: string :: size_type rds = tmp.size ( );
			const OutPriceElement * root = outPrices;
			typedef std :: map < int, OutChoicePeer, std :: less < int >,
				__SS_ALLOCATOR < std :: pair < const int, OutChoicePeer > > > PeerChoicesMap;
			PeerChoicesMap peerChoices;

			while ( true ) {
				const OutPriceElement :: ChoicesSet & choices = root -> getChoices ( );
				OutPriceElement :: ChoicesSet :: const_iterator e = choices.end ( );
				for ( OutPriceElement :: ChoicesSet :: const_iterator i = choices.begin ( );
					i != e; ++ i )
					if ( ! externalRouting || outPeersMap.count ( i -> pid ) )
						peerChoices [ i -> pid ] = OutChoicePeer ( * i, int ( rds - tmp.size ( ) ) );
				if ( tmp.empty ( ) || ! root -> getAt ( tmp [ 0 ] ) )
					break;
				root = root -> getAt ( tmp [ 0 ] );
				tmp.erase ( 0, 1 );
			}

			for ( PeerChoicesMap :: const_iterator i = peerChoices.begin ( ); i != peerChoices.end ( ); ++ i ) {
				if ( ! i -> second.enabled )
					continue;
				int peer = i -> first;
				const OutPeerInfo * op = getOutPeerInfo ( peer );
				if ( ! op )
					continue;
				RoutesMap :: const_iterator ri;
				bool allRoutes = ! allowedRoutes || ( inAllRoutes && op -> allRoutes );
				if ( ! allRoutes ) {
					ri = allowedRoutes -> find ( peer );
					if ( ri == allowedRoutes -> end ( ) )
						continue;
					if ( forks.size ( ) == 1 && ri -> second.forking )
						hasForkingRoute = true;
				}

				if ( onlyToMaster ) {
					if ( ! op -> remotePrice )
						continue;
				} else {
					if ( op -> minDigits > rds || op -> maxDigits < rds )
						continue;
				}
				int price = i -> second.price;

				// checking Profits
				bool profitGuardEnabled = false;
				if ( const InPeerInfo * ip = getInPeerInfo ( source.peer ) ) {
					if ( ! ip -> profitGuard.isProfitable ( numberChoice -> second.getPrice ( ), price ) &&
						! allowLosses ) {
						PSYSTEMLOG ( Info, "Choice " << peer << " is not profitable (InPeer). inPrice="
							<< numberChoice -> second.getPrice ( ) << ", outPrice=" <<
							price << " ( " << ip -> profitGuard.getMinProfitAbs ( ) <<
							", " << ip -> profitGuard.getMinProfitRel ( ) << "% )" );
						continue;
					}
					profitGuardEnabled = ip -> profitGuard.isEnabled ( );
				}
				if ( ! op -> profitGuard.isProfitable ( numberChoice -> second.getPrice ( ), price ) &&
					! allowLosses ) {
					PSYSTEMLOG ( Info, "Choice " << peer << " is not profitable (OutPeer). inPrice="
						<< numberChoice -> second.getPrice ( ) << ", outPrice=" << price <<
						" ( " << op -> profitGuard.getMinProfitAbs ( ) << ", " <<
						op -> profitGuard.getMinProfitRel ( ) << "% )" );
					continue;
				}
				if ( op -> profitGuard.isEnabled ( ) )
					profitGuardEnabled = true;
				if ( ! profitGuardEnabled && ! allowLosses && ! allPrice && price >=
					numberChoice -> second.getPrice ( ) && price != 0 && outAcctn.empty ( ) )
					continue;
				if ( ! inAllCodecs && ! op -> allCodecs &&
					intersectionIsEmpty ( inAllowedCodecs, op -> allowedCodecs ) &&
					! hasRecodes ( inAllowedCodecs, op -> allowedCodecs ) )
					continue;
				int prio = op -> prio;
				if ( ! allRoutes && ri -> second.prio >= 0 )
					prio = ri -> second.prio;
				if ( i -> second.prio >= 0 )
					prio = i -> second.prio;
				OutPeerInfo :: IpsMap registeredIps;
				const OutPeerInfo :: IpsMap * ips = & registeredIps;
				if ( op -> ips.empty ( ) ) {
					const RegisteredOutPeer * outPeer = registeredOutPeers.getPeer ( peer );
					if ( ! outPeer )
						continue;
					registeredIps.insert ( std :: make_pair ( IpPort ( outPeer -> getIpAddr ( ),
						outPeer -> getPort ( ) ), IpPortInfo ( outPeer -> getTimeout ( ),
						outPeer -> getSuffix ( ), outPeer -> getH323 ( ) ? ptH323 : ptSip ) ) );
				} else
					ips = & op -> ips;
				if ( externalRouting )
					prio = outPeersMap [ peer ];
				ss :: string pre;
				pre.reserve ( 16 );
				pre.append ( op -> prefix );
				pre.append ( rdigs, op -> depth, ss :: string :: npos );
				ss :: string code ( rdigs, 0, i -> second.codeLen );
				OutPeerInfo :: IpsMap :: const_iterator ipse = ips -> end ( );
				for ( OutPeerInfo :: IpsMap :: const_iterator it = ips -> begin ( );
					it != ipse; ++ it ) {
					if ( ( disableSIP || ! sip ) && it -> second.type == ptSip )
						continue;
					if ( ! h323 && it -> second.type == ptH323 )
						continue;
					if ( ! mgcp && it -> second.type == ptMgcp )
						continue;
					int port = it -> first.port;
					const ss :: string & ip = it -> first.ip;
					OutChoice o ( price, i -> second.connectPrice, prio, ip, port,
						pre, op -> depth, peer,
						code, allRoutes ? 0 : ri -> second.lim,
						allRoutes ? op -> directRTP : ri -> second.directRTP,
						allRoutes ? op -> interfaces : ri -> second.interfaces,
						allRoutes ? op -> sigOptions : ri -> second.sigOptions,
						numberChoice -> first, numberChoice -> second, it -> second.timeout, false,
						0, it -> second.type, op -> tarif, i -> second.round, rdigs,
						getSentDigits ( rdigs, op, it -> second.suffix ), op -> mgcpGw,
						op -> mgcpEp );
					if ( deleteIp ( v, o ) )
						v.insert ( o );
				}
			}
		}
		if ( ! v.empty ( ) ) {
			vv.push_back ( OutChoiceSet ( ) );
			OutChoiceSet & t = * -- vv.end ( );
			std :: copy ( v.begin ( ), v.end ( ), std :: inserter ( t, t.begin ( ) ) );
			if ( outAcctns.empty ( ) )
				source.outAcctn = outAcctn;
			outAcctns.push_back ( outAcctn );
		}
	}

	if ( hasForkingRoute ) {
		OutChoiceSetVector t;
		std :: swap ( t, vv );
		StringVector o;
		std :: swap ( o, outAcctns );
		for ( OutChoiceSet :: const_iterator i = t [ 0 ].begin ( ); i != t [ 0 ].end ( ); ++ i ) {
			OutChoiceSet s;
			s.insert ( * i );
			vv.push_back ( s );
			outAcctns.push_back ( source.acctn );
		}
	}

	if ( vv.empty ( ) )
		PSYSTEMLOG ( Info, "no choices" );
	return ! vv.empty ( );
}

static void loadH323ToSIPErrorResponse(MySQL & m, int id, ConvertErrorResponseFromH323ToSIP* response)
{
		MyResult r ( m.getQuery ( "select textSIP from H323ToSIPErrorResponse where id = %d", id ) );
		if ( const char * * p = r.fetchRow ( ) )
			response->strTextSIP_ = *p;
		r.reset ( m.getQuery ( "select errorH323 from H323ToSIPErrorResponse where id = %d", id ) );
		if ( const char * * p = r.fetchRow ( ) )
			response->errorH323_ = std :: atoi ( * p );
		r.reset ( m.getQuery ( "select errorSIP from H323ToSIPErrorResponse where id = %d", id ) );
		if ( const char * * p = r.fetchRow ( ) )
			response->errorSIP_ = std :: atoi ( * p );
}

ss :: string ConfData :: getH323Text(int errorH323) const
{
	PSYSTEMLOG(Info, "getH323Text: " << errorH323);
	MapConvResponseSIPToH323 :: const_iterator cit = mapConvResponseSIPToH323_.begin();
	for(;cit != mapConvResponseSIPToH323_.end();++cit)
	{
		if(cit->second->errorH323_ == errorH323)
		{
			return cit -> second -> strTextH323_;
		}
	}

	return "";
}

ss :: string ConfData :: getSIPText(int errorSIP) const
{
	MapConvResponseH323ToSIP :: const_iterator cit = mapConvResponseH323ToSIP_.begin();
	for(;cit != mapConvResponseH323ToSIP_.end();++cit)
	{
		if(cit->second->errorSIP_ == errorSIP)
		{
			return cit -> second -> strTextSIP_;
		}
	}

	return "";
}

ResponsibleInfo * ConfData :: getResponsibleInfo ( const ss :: string & uname ) const {
	ResponsiblesMap :: const_iterator it = responsibles.find ( uname );
	if ( it == responsibles.end ( ) )
		return 0;
	return it -> second;
}

CustomerInfo * ConfData :: getCustomerInfo ( const ss :: string & uname ) const {
	CustomersMap :: const_iterator it = customers.find ( uname );
	if ( it == customers.end ( ) )
		return 0;
	return it -> second;
}

const InPeerInfo * ConfData :: getInPeerInfo ( int peer ) const {
	InPeersMap :: const_iterator it = inPeers.find ( peer );
	if ( it == inPeers.end ( ) )
		return 0;
	return it -> second;
}

const OutPeerInfo * ConfData :: getOutPeerInfo ( int peer ) const {
	OutPeersMap :: const_iterator opit = outPeers.find ( peer );
	if ( opit == outPeers.end ( ) )
		return 0;
	return opit -> second;
}

bool ConfData :: isDebugInIp ( const ss :: string & ip ) const {
	return debugInIps.count ( ip ) == 1;
}

const ConfData :: PrepaidInfo * ConfData :: getPrepaidInfo ( int id ) const {
	PrepaidIdsMap :: const_iterator pit = prepaidIds.find ( id );
	if ( pit == prepaidIds.end ( ) )
		return 0;
	return pit -> second;
}

int ConfData :: getSlaveId ( const ss :: string & ip ) const throw ( ) {
	StringIntMap :: const_iterator it = slaves.find ( ip );
	if ( it == slaves.end ( ) )
		return nasId;
	return it -> second;
}

const ss :: string & ConfData :: getTimeToChange ( ) const {
	return timeToChange;
}

const TarifInfo * ConfData :: getTarifInfo ( int id ) const {
	TarifsMap :: const_iterator i = tarifs.find ( id );
	if ( i == tarifs.end ( ) )
		return & defaultTarif;
	return i -> second;
}

const ConfData :: DNISInfo * ConfData :: getDNISInfo ( const ss :: string & s ) const {
	DNISMap :: const_iterator i = dnis.find ( s );
	if ( i == dnis.end ( ) )
		return 0;
	return i -> second;
}

void ConfData :: uncacheCard ( const CardInfo * c ) {
	for ( CardInfo :: CallBackInfoMap :: const_iterator i = c -> ani.begin ( ); i != c -> ani.end ( ); ++ i )
		aniToCard.erase ( i -> first );
	pinToCard.erase ( c -> pin );
	for ( StringStringSetMap :: const_iterator i = c -> forks.begin ( ); i != c -> forks.end ( ); ++ i )
		numberToCard.erase ( i -> first );
	for ( StringIntMap :: const_iterator i = c -> numbers.begin ( ); i != c -> numbers.end ( ); ++ i )
		numberToCard.erase ( i -> first );
	acctnToCard.erase ( c -> acctn );
}

CardInfo * ConfData :: cardByAni ( const ss :: string & ani ) {
	CardMap :: const_iterator i = aniToCard.find ( ani );
	if ( i != aniToCard.end ( ) )
		return i -> second;
	ss :: ostringstream os;
	os << "select card from Ani where ani = '" << MySQL :: escape ( ani ) << '\'';
	int id = cm.getInt ( os.str ( ) );
	if ( id != - 1 )
		addCardToCache ( id );
	return aniToCard [ ani ];
}

CardInfo * ConfData :: cardByAcctn ( const ss :: string & acctn ) {
	CardMap :: const_iterator i = acctnToCard.find ( acctn );
	if ( i != acctnToCard.end ( ) )
		return i -> second;
	ss :: ostringstream os;
	os << "select id from Cards where acctn = '" << MySQL :: escape ( acctn ) << '\'';
	int id = cm.getInt ( os.str ( ) );
	if ( id != - 1 )
		addCardToCache ( id );
	return acctnToCard [ acctn ];
}

CardInfo * ConfData :: cachedCardByAcctn ( const ss :: string & acctn ) {
	CardMap :: const_iterator i = acctnToCard.find ( acctn );
	if ( i != acctnToCard.end ( ) )
		return i -> second;
	return 0;
}

CardInfo * ConfData :: cardByPin ( const ss :: string & pin ) {
	CardMap :: const_iterator i = pinToCard.find ( pin );
	if ( i != pinToCard.end ( ) )
		return i -> second;
	ss :: ostringstream os;
	os << "select id from Cards where pin = '" << MySQL :: escape ( pin ) << '\'';
	int id = cm.getInt ( os.str ( ) );
	if ( id != - 1 )
		addCardToCache ( id );
	return pinToCard [ pin ];
}

CardInfo * ConfData :: cardByNumber ( const ss :: string & number ) {
	CardMap :: const_iterator i = numberToCard.find ( number );
	if ( i != numberToCard.end ( ) )
		return i -> second;
	ss :: ostringstream os;
	os << "select card from CardNumbers where number = '" << MySQL :: escape ( number ) << "' and redirect = 'n'";
	int id = cm.getInt ( os.str ( ) );
	if ( id != - 1 )
		addCardToCache ( id );
	return numberToCard [ number ];
}

class CardLoaded {
	CardLoaded ( const CardLoaded & );
	CardLoaded & operator= ( const CardLoaded & );
public:
	CardLoaded ( const CardInfo * c ) {
		conf -> cardLoaded ( c );
	}
};

bool ConfData :: addCardToCache ( int id ) {
	class TableLocker {
		MySQL & m;
	public:
		TableLocker ( MySQL & mm ) : m ( mm ) {
			if ( m.query ( "lock tables Ani read, Cards read, CardNumbers read, CardRadiusClasses read,"
				"ANumReplaceGroups read, ANumReplaceLists read, ANumReplacePrefixes read, ANumReplaceRanges read" ) )
				throw MySQLError ( m );
		}
		~TableLocker ( ) {
			m.unLockTables ( );
		}
	};
	TableLocker tl ( cm );

	ss :: ostringstream os;
	os << "select id, acctn, pin, cost, credit = 'y', valute, status, accRights, unix_timestamp( expDate ), expTimeFUse,"
		" options, aNumReplaceClass, language, callback, requireCallbackAuth, radiusClass, clearPass, isMonitored, "
		"sigOptions, singleAni, uid from Cards where id = " << id;
	MyResult r ( cm.getQuery ( os.str ( ) ) );
	struct CardsData {
		const char * id;
		const char * acctn;
		const char * pin;
		const char * cost;
		const char * credit;
		const char * valute;
		const char * status;
		const char * accRights;
		const char * expDate;
		const char * expFUse;
		const char * options;
		const char * aNumReplaceClass;
		const char * lang;
		const char * callback;
		const char * requireCallbackAuth;
		const char * radiusClass;
		const char * clearPass;
		const char * isMonitored;
		const char * sigOptions;
		const char * singleAni;
		const char * uid;
	};
	CardsData * p = reinterpret_cast < CardsData * > ( r.fetchRow ( ) );
	if ( ! p )
		return false;
	const PrepaidInfo * pi = getPrepaidInfo ( std :: atoi ( p -> uid ) );
	if ( ! pi )
		return false;
	Pointer < CardInfo > ci = new CardInfo;
	memcpy ( ci -> md5, p -> pin, 32 );
	ci -> acctn = p -> acctn;
	ci -> status = p -> status [ 0 ];
	ci -> parent = const_cast < PrepaidInfo * > ( pi );
	ci -> clearPass = p -> clearPass;
	char ct = p -> callback [ 0 ];
	if ( ct == 'D' )
		ct = pi -> callBack;
	ci -> callbackEnabled = ct == 'y';
	char ac = p -> accRights [ 0 ];
	if ( ac == 'D' )
		ac = pi -> accRights;
	switch ( ac ) {
		case 'I':
			ci -> accRightsInet = true;
			ci -> accRightsVoice = false;
			break;
		case 'V':
			ci -> accRightsInet = false;
			ci -> accRightsVoice = true;
			break;
		case 'B':
			ci -> accRightsInet = true;
			ci -> accRightsVoice = true;
			break;
	}
	ci -> cost = std :: atof ( p -> cost );
	ci -> credit = pi -> cardBalance && std :: atoi ( p -> credit );
	ci -> customerBalance = pi -> parent -> customerBalance;
	ci -> cardBalance = pi -> cardBalance;
	ci -> responsibleBalance = pi -> parent -> parent -> useBalance;
	int valute = std :: atoi ( p -> valute );
	if ( valute <= 0 )
		ci -> valuteRate = pi -> valuteRate;
	else {
		ci -> valuteRate = valuteRates [ valute ];
		if ( ci -> valuteRate <= 0 )
			ci -> valuteRate = 1;
	}
	ci -> expires = std :: atoi ( p -> expDate );
	ci -> expTimeFUse = std :: atoi ( p -> expFUse );
	ci -> radiusClass = std :: atoi ( p -> radiusClass );
	if ( p -> options [ 0 ] )
		ci -> options = p -> options;
	else
		ci -> options = pi -> options;
	if ( p -> lang [ 0 ] )
		ci -> lang = p -> lang;
	else
		ci -> lang = pi -> lang;
	acctnToCard.insert ( std :: make_pair ( ss :: string ( p -> acctn ), ci ) );
	pinToCard.insert ( std :: make_pair ( ci -> pin = p -> pin, ci ) );
	char t = p -> requireCallbackAuth [ 0 ];
	if ( t == 'D' )
		t = pi -> requireCallbackAuth;
	ci -> requireCallbackAuth = t == 'y';
	t = p -> singleAni [ 0 ];
	if ( t == 'D' )
		ci -> singleAni = pi -> singleAni;
	else
		ci -> singleAni = t == 'y';
	int repClass = std :: atoi ( p -> aNumReplaceClass );
	AnumTranslatorIn & replaces = anumClasses [ repClass ];
	if ( replaces.empty ( ) )
		loadAnumClass ( cm, repClass, replaces );
	ci -> at = & replaces;
	ci -> lastIp = "";
	ci -> isMonitored = ( p -> isMonitored [ 0 ] == 'y' );
	ci -> _sigOptions = _sigOptionsMap.get ( std :: atoi ( p -> sigOptions ) );
	PSYSTEMLOG ( Info, "Adding card " << p -> acctn );
	MyResult r1 ( cm.getQuery ( "select ani, callback, callbackNumber from Ani where card = %i", id ) );
	struct AcnData {
		const char * ani;
		const char * callback;
		const char * callbackNumber;
	};
	while ( const AcnData * vi = reinterpret_cast < const AcnData * > ( r1.fetchRow ( ) ) ) {
		aniToCard.insert ( std :: make_pair ( ss :: string ( vi -> ani ), ci ) );
		PSYSTEMLOG ( Info, "Adding ani " << vi -> ani );
		CallBackInfo cbi;
		cbi.number = vi -> callbackNumber;
		char t = vi -> callback [ 0 ];
		if ( t == 'D' )
			t = p -> callback [ 0 ];
		if ( t == 'D' )
			t = pi -> callBack;
		cbi.enabled = t == 'y';
		ci -> ani.insert ( std :: make_pair ( ss :: string ( vi -> ani ), cbi ) );
	}
	r1.reset ( cm.getQuery ( "select number, redirect = 'y', prio, outGroup, forking = 'y' from CardNumbers where "
		"card = %i", id ) );
	struct NrpData {
		const char * number;
		const char * redirect;
		const char * prio;
		const char * outGroup;
		const char * forking;
	};
	while ( const NrpData * vi = reinterpret_cast < const NrpData * > ( r1.fetchRow ( ) ) ) {
		if ( std :: atoi ( vi -> forking ) ) {
			PSYSTEMLOG ( Info, "Adding fork from " << vi -> number );
			MyResult r2 ( cm.getQuery ( "select ton from Forking where "
				"fromn = '%s'", vi -> number ) );
			while ( const char * * p2 = r2.fetchRow ( ) )
				ci -> forks [ vi -> number ].insert ( * p2 );
			numberToCard.insert ( std :: make_pair ( ss :: string ( vi -> number ), ci ) );
		} else if ( std :: atoi ( vi -> redirect ) ) {
			PSYSTEMLOG ( Info, "Adding redirect " << vi -> number );
			ci -> redirects.insert ( std :: make_pair ( std :: atoi ( vi -> prio ), vi -> number ) );
		} else {
			PSYSTEMLOG ( Info, "Adding number " << vi -> number );
			int outGroup = std :: atoi ( vi -> outGroup );
			ss :: string number = vi -> number;
			ci -> numbers.insert ( std :: make_pair ( number, outGroup ) );
			numberToCard.insert ( std :: make_pair ( number, ci ) );

		}
	}
	r1.reset ( cm.getQuery ( "select nasId, dnisId, ip, mask from CardRadiusClasses where cardId = %i", id ) );
	struct NdimData {
		const char * nasId;
		const char * dnisId;
		const char * ip;
		const char * mask;
	};
	while ( const NdimData * vi = reinterpret_cast < const NdimData * > ( r1.fetchRow ( ) ) ) {
		CardInfo :: FramedInfo fi;
		fi.ip = vi -> ip;
		fi.mask = vi -> mask;
		ci -> framedInfo [ std :: atoi ( vi -> nasId ) ].insert ( std :: make_pair ( std :: atoi ( vi -> dnisId ), fi ) );
	}
	( CardLoaded ( ci ) );
	return true;
}


void ConfData :: getSecret ( const ss :: string & ip, RadGWInfo & g ) const {
	SecretsMap :: const_iterator i = secrets.find ( ip );
	if ( i == secrets.end ( ) ) {
		g.useAni = true;
		g.useDnis = true;
		g.useAcode = false;
		g.defaultServiceType = Radius :: sLogin;
		return;
	}
	g = i -> second;
}

void ConfData :: getRadiusClass ( const CardInfo * card, int nasId, const DNISInfo * dnis, StringStringMap & rc ) const {
	RadiusClassesMap :: const_iterator i = radiusClasses.find ( dnis -> radiusClass );
	if ( i != radiusClasses.end ( ) ) {
		for ( StringStringMap :: const_iterator j = i -> second.begin ( ); j != i -> second.end ( ); j ++ )
			rc [ j -> first ] = j -> second;
	}
	const NASInfo * gw = getNASInfo ( nasId );
	i = radiusClasses.find ( gw -> radiusClass );
	if ( i != radiusClasses.end ( ) ) {
		for ( StringStringMap :: const_iterator j = i -> second.begin ( ); j != i -> second.end ( ); j ++ )
			rc [ j -> first ] = j -> second;
	}
	i = radiusClasses.find ( card -> parent -> radiusClass );
	if ( i != radiusClasses.end ( ) ) {
		for ( StringStringMap :: const_iterator j = i -> second.begin ( ); j != i -> second.end ( ); j ++ )
			rc [ j -> first ] = j -> second;
	}
	i = radiusClasses.find ( card -> radiusClass );
	if ( i != radiusClasses.end ( ) ) {
		for ( StringStringMap :: const_iterator j = i -> second.begin ( ); j != i -> second.end ( ); j ++ )
			rc [ j -> first ] = j -> second;
	}
	CardInfo :: FramedInfoMap :: const_iterator j = card -> framedInfo.find ( nasId );
	if ( j == card -> framedInfo.end ( ) )
		return;
	CardInfo :: IntFramedInfoMap :: const_iterator k = j -> second.find ( dnis -> id );
	if ( k == j -> second.end ( ) )
		k = j -> second.find ( 0 );
	if ( k == j -> second.end ( ) )
		return;
	if ( k -> second.ip.empty ( ) )
		return;
	rc [ "Framed-IP-Address" ] = k -> second.ip;
	rc [ "Framed-IP-Netmask" ] = k -> second.mask;
}

const ConfData :: NASInfo * ConfData :: getNASInfo ( int gw ) const {
	GwsMap :: const_iterator i = gws.find ( gw );
	if ( i == gws.end ( ) )
		return 0;
	return i -> second;
}

const StringSet & ConfData :: getLocalIps ( ) const {
	return localIps;
}

const ConfData :: GateKeepersMap & ConfData :: getGateKeepers ( ) const {
	return gateKeepers;
}

const ConfData :: LoginInfo * ConfData :: getLoginInfo ( const ss :: string & login ) const {
	LoginInfoMap :: const_iterator it = loginToPeer.find ( login );
	if ( it == loginToPeer.end ( ) )
		return 0;
	return & it -> second;
}

const RecodersMap & ConfData :: getRecoders ( ) const {
	return recoders;
}

const IntVector & ConfData :: getLocalIntIps ( ) const {
	return localIntIps;
}

bool ConfData :: registerAni ( AfterTaskVector & que, CardInfo * card, const ss :: string & ani,
	const ss :: string & lang ) {
	if ( card -> singleAni && ! card -> ani.empty ( ) )
		return false;
	CardMap :: const_iterator i = acctnToCard.find ( card -> acctn );
	if ( i == acctnToCard.end ( ) ) {
		PSYSTEMLOG ( Error, "card disapeared while registering " << card -> acctn );
		return 0;
	}
	aniToCard [ ani ] = i -> second;
	if ( ! lang.empty ( ) )
		card -> lang = lang;
	CallBackInfo cbi;
	cbi.number = ani;
	cbi.enabled = card -> callbackEnabled;
	card -> ani [ ani ] = cbi;
	que.push_back ( new Conf :: AniAfterTask ( card -> acctn, ani, lang ) );
	return true;
}

void CardInfo :: logRegistration ( const ss :: string & ip, int port, int h323, PTime endLease, long lockCount ) {
	if ( isMonitored ) {
		ss :: ostringstream os;
		os << "insert into CardsState ( acctn, state, expires, ip, port, eventTime, proto, locks ) "
			"values ( " << acctn << ", 'REGISTERED', sec_to_time(" << ( endLease - PTime ( ) ).GetSeconds ( ) <<
			"), '" << ip << "', " << port << ", from_unixtime( " << PTime ( ).GetTimeInSeconds ( ) <<
			" ), " << h323 << ", " << lockCount << " )";
		sqlOut -> add ( os.str ( ) );

		if ( lastIp == ip ) return;
		if ( lastIp != "" ) { // first registration, not logging
			PSYSTEMLOG ( Info, "WARNING! Card " << acctn << " migrated from " << lastIp <<
				" to " << ip );
			ss :: ostringstream os;
			os << "insert into CardMigrations ( eventTime, acctn, ip ) values ( from_unixtime( " <<
				PTime ( ).GetTimeInSeconds ( ) << " ), " << acctn << ", '" << ip << "')";
			sqlOut -> add ( os.str ( ) );
		}
		lastIp = ip;
	}
}

void CardInfo :: logUnregistration ( ) const {
	if ( isMonitored ) {
		ss :: ostringstream os;
		os << "delete from CardsState where acctn = '" << acctn << '\'';
		sqlOut -> add ( os.str ( ) );
	}
}

void CardInfo :: logLocking ( long lockCount ) const {
	if ( isMonitored ) {
		ss :: ostringstream os;
		os << "update CardsState set locks = " << lockCount << " where acctn = " << acctn;
		sqlOut -> add ( os.str ( ) );
	}
}

void ConfData :: SigOptionsMap :: load ( MySQL & m ) {
	MyResult r ( m.getQuery ( "select * from SignallingOptions" ) );
	while ( const char * * p = r.fetchRow ( ) ) {
//		PSYSTEMLOG ( Info, "roman 1b" << p [ 10 ] );
		_data.insert ( std :: make_pair ( std :: atoi ( p [ 10 ] ), SigOptionsPeer ( p ) ) );
//		PSYSTEMLOG ( Info, "roman 1c" );
	}
}

const SigOptionsPeer ConfData :: SigOptionsMap :: get ( int id ) {
	SigOptionsData :: const_iterator it = _data.find ( id );
	if ( it != _data.end ( ) )
		return it -> second;
	return SigOptionsPeer ( );
}

namespace boost {
template < class ST, class SA, class traits, class charT >
	std :: basic_string < charT, ST, SA > regex_replace (
	const std :: basic_string < charT, ST, SA > & s,
	const basic_regex < charT, traits > & e,
	const charT * fmt,
	match_flag_type flags = match_default ) {
	std :: basic_string < charT, ST, SA > result;
	re_detail :: string_out_iterator < std :: basic_string < charT, ST, SA > > i ( result );
	regex_replace ( i, s.begin ( ), s.end ( ), e, fmt, flags );
	return result;
}
template < class ST, class SA, class traits, class charT >
	std :: basic_string < charT, ST, SA > regex_replace (
	const std :: basic_string < charT, ST, SA > & s,
	const basic_regex < charT, traits > & e,
	const std :: basic_string < charT, ST, SA > & fmt,
	match_flag_type flags = match_default ) {
	return regex_replace ( s, e, fmt.c_str ( ), flags );
}
}

ss :: string ConfData :: getSentDigits ( const ss :: string & realDigits, const OutPeerInfo * op,
	const ss :: string & suffix ) const {
	if ( op -> rxReplaces.empty ( ) ) {
		ss :: string t;
		t.reserve ( 16 );
		t.append ( op -> prefix );
		t.append ( realDigits, op -> depth, ss :: string :: npos );
		t += suffix;
		return t;
	}
	const OutPeerInfo :: RXReplaceVector & v = op -> rxReplaces;
	for ( unsigned i = 0; i < v.size ( ); i ++ ) {
		try {
			if ( boost :: regex_match ( realDigits, v [ i ].rx ) )
				return boost :: regex_replace ( realDigits, v [ i ].rx,
					v [ i ].replace ) + suffix;
		} catch ( std :: exception & e ) {
			PSYSTEMLOG ( Error, "exception when matching regex: " << e.what ( ) );
		}
	}
	return realDigits + suffix;
}

bool ConfData :: getSupportVia ( ) const {
	return supportVia;
}

bool ConfData :: getIsNoChoiceCode ( ) const {
	return isNoChoiceCode;
}

static void loadSIPToH323ErrorResponse(MySQL & m, int id, ConvertErrorResponseFromSIPToH323* response)
{
        MyResult r ( m.getQuery ( "select textH323 from SIPToH323ErrorResponse where id = %d", id ) );
        if ( const char * * p = r.fetchRow ( ) )
            response->strTextH323_ = *p;
        r.reset ( m.getQuery ( "select errorH323 from SIPToH323ErrorResponse where id = %d", id ) );
        if ( const char * * p = r.fetchRow ( ) )
            response->errorH323_ = std :: atoi ( * p );
        r.reset ( m.getQuery ( "select errorSIP from SIPToH323ErrorResponse where id = %d", id ) );
        if ( const char * * p = r.fetchRow ( ) )
            response->errorSIP_ = std :: atoi ( * p );
}

bool ConfData :: loadErrorResponses ( MySQL & m )
{

    PSYSTEMLOG(Info, "Load Error response.");
        MyResult rID ( m.getQuery ( "select id from SIPToH323ErrorResponse" ) );
        PSYSTEMLOG(Info, "SIP->H323 count = " << rID.rowCount ( ) );
        while ( const char * * p = rID.fetchRow ( ) ) {
            ConvertErrorResponseFromSIPToH323* response = new ConvertErrorResponseFromSIPToH323;
            loadSIPToH323ErrorResponse(m, std :: atoi(*p), response);

            mapConvResponseSIPToH323_.insert ( make_pair ( response->errorSIP_, response ));
        }
        rID.reset ( m.getQuery ( "select id from H323ToSIPErrorResponse" ) );
        PSYSTEMLOG(Info, "H323->SIP count = " << rID.rowCount ( ) );
        while ( const char * * p = rID.fetchRow ( ) ) {
            ConvertErrorResponseFromH323ToSIP* response = new ConvertErrorResponseFromH323ToSIP;
            loadH323ToSIPErrorResponse(m, std :: atoi(*p), response);

            mapConvResponseH323ToSIP_.insert ( make_pair ( response->errorH323_, response ));
        }

    return true;
}

bool ConfData :: getSIPToH323ErrorResponse(int err, int* response, ss :: string* textResponse) const
{
    MapConvResponseSIPToH323 :: const_iterator cit = mapConvResponseSIPToH323_.find(err);

    if (cit != mapConvResponseSIPToH323_.end())
    {
        if(response)
            *response = cit -> second -> errorH323_;
        if(textResponse)
            *textResponse = cit -> second -> strTextH323_;
        return true;
    }
    else
    {
        if(getIsNoChoiceCode())
        {
            if(response)
                *response = int(Q931 :: cvNoCircuitChannelAvailable);
            if(textResponse)
                *textResponse = "No circuit channel available";
        }
        else
        {
            if(response)
                *response = int(Q931 :: cvNoRouteToDestination);
            if(textResponse)
                *textResponse = "No route to destination.";
        }
    }
    return false;
}

bool ConfData :: getH323ToSIPErrorResponse(int err, int* response, ss :: string* textResponse) const
{
    MapConvResponseH323ToSIP :: const_iterator cit = mapConvResponseH323ToSIP_.find(err);

    if (cit != mapConvResponseH323ToSIP_.end())
    {
        if(response)
            *response = cit -> second->errorSIP_;
        if(textResponse)
            *textResponse = cit -> second->strTextSIP_;
//        PSYSTEMLOG(Info, "ConfData :: getH323ToSIPErrorResponse1. err = " << err << "; response = " << *response << "; text = " << *textResponse);
        return true;
    }
    else
    {
        if(response)
            *response = SIP :: PDU :: scFaiureServiceUnavailable;
        if(textResponse)
            *textResponse = "Service Unavailable.";
    }

//    PSYSTEMLOG(Info, "ConfData :: getH323ToSIPErrorResponse2. err = " << err << "; response = " << *response << "; text = " << *textResponse);

    return false;
}

bool ConfData :: getMgcpConf ( MgcpGatewayInfoVector & v ) {
	if ( mgcpReloaded )
		return false;
	mgcpReloaded = true;
	mgcpGws.swap ( v );
	return true;
}

bool ConfData :: getRportAgents ( StringVector & v ) {
	if ( rportAgentsReloaded )
		return false;
	rportAgentsReloaded = true;
	rportAgents.swap ( v );
	return true;
}
