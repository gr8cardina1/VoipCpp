#pragma implementation
#pragma implementation "ss.hpp"
#pragma implementation "allocatable.hpp"
#pragma implementation "ipport.hpp"
#pragma implementation "pointer.hpp"
#pragma implementation "radgwinfo.hpp"
#pragma implementation "icqcontact.hpp"
#pragma implementation "calldetails.hpp"
#pragma implementation "sourcedata.hpp"
#pragma implementation "callcontrol.hpp"
#pragma implementation "ssconf.hpp"
#pragma implementation "latencylimits.hpp"
#pragma implementation "slprint.hpp"
#pragma implementation "serializestring.hpp"
#pragma implementation "registeredcard.hpp"
#pragma implementation "requestinfo.hpp"
#pragma implementation "findprefix.hpp"
#pragma implementation "findmapelement.hpp"
#pragma implementation "accountinginfo.hpp"
#pragma implementation "scopeguard.hpp"
#include "ss.hpp"
#include "allocatable.hpp"
#include <ptlib.h>
#include <ptlib/sockets.h>
#include "aftertask.hpp"
#include "ipport.hpp"
#include "icqcontact.hpp"
#include "radgwinfo.hpp"
#include "Conf.hpp"

#include <stdexcept>
#include <limits>
#include <cstring>
#include "asn.hpp"
#include "h225.hpp"
#include "condvar.hpp"
#include "tarifinfo.hpp"
#include "tarifround.hpp"
#include "pricedata.hpp"
#include "pointer.hpp"
#include "moneyspool.hpp"
#include "customercalls.hpp"
#include "mysql.hpp"
#include "dbconf.hpp"
#include "priceprios.hpp"
#include "registeredcard.hpp"
#include <queue>
#include "geganet.hpp"
#include "ixcudpsocket.hpp"
#include "basegkclientthread.hpp"
#include "RegisteredOutPeersMap.hpp"
//--------------------------
#include <boost/archive/text_iarchive.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/sequenced_index.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include "codecinfo.hpp"
#include "latencylimits.hpp"
#include "profitguard.hpp"
#include "priceelement.hpp"
#include "outpriceelement.hpp"
#include "signallingoptions.hpp"
#include <boost/regex.hpp>
#include <boost/multi_index/member.hpp>
#include "callbuffer.hpp"
#include "mgcpconf.hpp"
#include <tr1/functional>
#include "confdata.hpp"
//----------------------


#include <ptlib/svcproc.h>
#include "outcarddetails.hpp"
#include "outchoicedetails.hpp"
#include "outchoice.hpp"
#include "callcontrol.hpp"
#include "ssconf.hpp"
#include "automutex.hpp"
#include "sqloutthread.hpp"
#include "requestinfo.hpp"
#include "radius.hpp"
#include "accountinginfo.hpp"
#include "findmapelement.hpp"
#include "setthreeway.hpp"
#include "firstiterator.hpp"
#include "h323gkclientthread.hpp"
#include "SIPAuthenticateValidator.hpp"
#include "sip2gkclientthread.hpp"
#include "sourcedata.hpp"
#include "smartguard.hpp"
#include <tr1/memory>
#include "commoncalldetails.hpp"
#include "q931.hpp"
#include "calldetails.hpp"
#include "rtpstat.hpp"
#include "session.hpp"
#include "rtpsession.hpp"
#include <boost/multi_index/mem_fun.hpp>
#include "sip.hpp"
#include "sipcalldetails.hpp"
#include "h323call.hpp"
#include <tr1/cstdint>
#include "md5.hpp"
#include "findprefix.hpp"
#include <ptclib/pxml.h>
#include <cmath>
#include <boost/serialization/map.hpp>
#include <boost/serialization/vector.hpp>
#include "serializestring.hpp"
#include "radiuscommon.hpp"
#include "h323common.hpp"
#include <boost/range.hpp>
#include "slprint.hpp"
#include "scopeguard.hpp"
#include "mgcpthread.hpp"
#include <bitset>

extern void printIfaces ( PChannel & channel );

class Conf :: Impl : public Allocatable < __SS_ALLOCATOR >, boost :: noncopyable {
	friend class Conf :: AniAfterTask;
	struct GRQAnswer : public Allocatable < __SS_ALLOCATOR > {
		ss :: string gkid;
		PCondVar c;
		int reason;
		bool confirmed;
		explicit GRQAnswer ( PMutex & m ) : c ( m ), reason ( - 1 ), confirmed ( false ) { }
	};
	struct RRQAnswer : public Allocatable < __SS_ALLOCATOR > {
		ss :: string eid;
		PCondVar c;
		int reason;
		explicit RRQAnswer ( PMutex & m ) : c ( m ), reason ( - 1 ) { }
	};
	struct ARQAnswer : public Allocatable < __SS_ALLOCATOR > {
		H225 :: TransportAddress destAddr;
		PCondVar c;
		bool inited;
		explicit ARQAnswer ( PMutex & m ) : c ( m ), inited ( false ) { }
	};
	class RadiusCardMoneyEater;
	struct RadiusCall {
		ss :: string acctn;
		ss :: string calledStationId;
		ss :: string realNumber;
		ss :: string h323ConfId;
		PriceData price, cbprice, inPrice, inEuPrice;
		ss :: string code;
		ss :: string ip;
		ss :: string lastOV;
		ss :: string cbRealNumber;
		ss :: string cbcode;
		bool m_isOV;
		Pointer < RadiusCardMoneyEater > inCardEater, inEater, inDealerEater, cbCardEater, cbEater,
			cbDealerEater;
		Pointer < TrafficEater > inCardTrafEater, outCardTrafEater, inTrafEater, outTrafEater,
			inDealerTrafEater, outDealerTrafEater;
		PTime start;
		int creditTime;
		int ovSavedTime;
		bool callback;
		bool startCame;
		bool telephonyEnded;
		RadiusCall ( ) : m_isOV(false), creditTime ( 0 ), ovSavedTime ( 0 ), callback ( false ), startCame ( false ),
			telephonyEnded ( false ) { }
	};
	struct TimeReaction {
		long long min;
		long long max;
		long long sum;
		long cnt;
		TimeReaction ( ) : min ( 0 ), max ( 0 ), sum ( 0 ), cnt ( 0 ) { }
		void add ( long long t ) {
			if ( min > t )
				min = t;
			if ( max < t )
				max = t;
			sum += t;
			cnt ++;
		}
	};
	struct DestAtom {
		int peer;
		ss :: string ip;
		int port;
		DestAtom ( );
		DestAtom ( const OutChoiceDetails & c );
		bool operator< ( const DestAtom & d ) const;
	};
	class CardAfterTask : public AfterTask {
		double realMoney;
		ss :: string acctn;
	public:
		CardAfterTask ( double rm, const ss :: string & a );
		~CardAfterTask ( );
	};
	friend class CardAfterTask;
	class ExpAfterTask : public AfterTask {
		std :: time_t now;
		ss :: string acctn;
	public:
		ExpAfterTask ( std :: time_t n, const ss :: string & a );
		~ExpAfterTask ( );
	};
	friend class ExpAfterTask;
	class CardMoneySpool : public MoneySpool {
		ss :: string acctn;
		double valuteRate;
		void destroy ( );
		void propagateBalanceChange ( );
		Pointer < AfterTask > getAfterTask ( double realMoney ) const;
	public:
		CardMoneySpool ( const ss :: string & a, double v );
	};
	friend class CardMoneySpool;
	class H323CardMoneyEater : public FullMoneyEater {
		CallControl * ctrl;
		int getRealSecs ( ) const;
		void stop ( );
		H323CardMoneyEater ( const H323CardMoneyEater & );
		H323CardMoneyEater & operator= ( const H323CardMoneyEater & );
	public:
		H323CardMoneyEater ( MoneySpool & o, double p, const TarifInfo & t,
			const TarifRound & r, const PriceData :: AmortiseMap & am, CallControl * c );
	};
	friend class CustomerMoneySpool;
	class RadiusCardMoneyEater : protected FullMoneyEater {
		ss :: string h323Conf;
		int acctSessionTime;
		int getRealSecs ( ) const;
		void stop ( );
		bool tickable ( ) const;
	public:
		using FullMoneyEater :: operator new;
		using FullMoneyEater :: operator delete;
		RadiusCardMoneyEater ( MoneySpool & o, const PriceData & p, const ss :: string & conf );
		Pointer < AfterTask > detach ( int ast );
	};
	class RegisteredInPeer {
		ss :: string prefix;
		ss :: string realPrefix;
		int uid;
		ss :: string acctn;
		bool fromNat;
		PTime endLease;
	public:
		RegisteredInPeer ( const ss :: string & p, const ss :: string & r, int u, bool f, PTime e ) :
			prefix ( p ), realPrefix ( r ), uid ( u ), fromNat ( f ), endLease ( e ) { }
		RegisteredInPeer ( const ss :: string & p, const ss :: string & r, const ss :: string & a, bool f,
			PTime e ) : prefix ( p ), realPrefix ( r ), uid ( 0 ), acctn ( a ), fromNat ( f ),
			endLease ( e ) { }
		RegisteredInPeer ( ) : uid ( 0 ), fromNat ( false ), endLease ( 0 ) { }
		const ss :: string & getPrefix ( ) const {
			return prefix;
		}
		const ss :: string & getRealPrefix ( ) const {
			return realPrefix;
		}
		int getUid ( ) const {
			return uid;
		}
		const ss :: string & getAcctn ( ) const {
			return acctn;
		}
		bool getFromNat ( ) const {
			return fromNat;
		}
		PTime getEndLease ( ) const {
			return endLease;
		}
		void setEndLease ( PTime t ) {
			endLease = t;
		}
	};
	ConfData * data;
	MySQL m, lm;
	DBConf dbc;
	int updateId;
	mutable PMutex mut;
	PMutex confMut;
	bool inShuttingDown;
	PSemaphore callsSem, gwCallsSem, radiusCallsSem, httpSem;
	PIntCondMutex callsCount, gwCallsCount, radiusCallsCount, httpCount;
	ss :: string name;
	int logLevel;
	IntIntMap inLimits, outLimits;
	IntIntIntMap inOutLimits;
	StringCustomerMoneySpoolMap customerCalls;
	StringMultiSet cardLocks;
	PTime lastBalance;
	typedef std :: map < ss :: string, Pointer < CardMoneySpool >, std :: less < ss :: string >,
		__SS_ALLOCATOR < std :: pair < const ss :: string, Pointer < CardMoneySpool > > > > CardCallsMap;
	CardCallsMap cardCalls;
	StringSet sipCallsToRemove;
	PricePrios prios;
	PMutex rasMutex;
	typedef std :: map < int, GRQAnswer *, std :: less < int >,
		__SS_ALLOCATOR < std :: pair < const int, GRQAnswer * > > > GrqAnswersMap;
	GrqAnswersMap grqAnswers;
	typedef std :: map < int, RRQAnswer *, std :: less < int >,
		__SS_ALLOCATOR < std :: pair < const int, RRQAnswer * > > > RrqAnswersMap;
	RrqAnswersMap rrqAnswers;
	typedef std :: map < int, ARQAnswer *, std :: less < int >,
		__SS_ALLOCATOR < std :: pair < const int, ARQAnswer * > > > ArqAnswersMap;
	ArqAnswersMap arqAnswers;
	void loadPrios ( );
	ss :: string radiusIp;
	int radiusPort;
	ss :: string radiusSecret;
	StringStringMultiMap radiusAcctnToConf;
	StringStringMap radiusConfToAcctn;
	typedef std :: map < ss :: string, RadiusCall, std :: less < ss :: string >,
		__SS_ALLOCATOR < std :: pair < const ss :: string, RadiusCall > > > RadiusCallsMap;
	RadiusCallsMap m_radiusCallsSpool;
	TimeReaction tr, fulltr;
	int noStartRadiusCallThreshold;
	int bytesPerMb;
	typedef std :: map < ss :: string, StringSet, std :: less < ss :: string >,
		__SS_ALLOCATOR < std :: pair < const ss :: string, StringSet > > > InterestedMap;
	StringSet onlineNumbers;
	InterestedMap interestedAccounts;
	RegCardsMap registeredCards;
	typedef std :: set < H323Call *, std :: less < H323Call * >, __SS_ALLOCATOR < H323Call * > > SmsCallsSet;
	SmsCallsSet smsCalls;
	typedef std :: map < DestAtom, Geganet, std :: less < DestAtom >,
		__SS_ALLOCATOR < std :: pair < const DestAtom, Geganet > > > LastReleaseCompleteMap;
	LastReleaseCompleteMap lastReleaseComplete;
	typedef std :: map < int, CallControl *, std :: less < int >,
		__SS_ALLOCATOR < std :: pair < const int, CallControl * > > > ActiveCallsMap;
	ActiveCallsMap activeCalls;
	typedef std :: map < ss :: string, BaseGkClientThread *, std :: less < ss :: string >,
		__SS_ALLOCATOR < std :: pair < const ss :: string, BaseGkClientThread * > > > GateKeepersMap;
	GateKeepersMap gateKeepers;
	typedef std :: map < int, RegisteredInPeer, std :: less < int >,
		__SS_ALLOCATOR < std :: pair < const int, RegisteredInPeer > > > IntRegisteredInPeerMap;
	typedef std :: map < PIPSocket :: Address, IntRegisteredInPeerMap, std :: less < PIPSocket :: Address >,
		__SS_ALLOCATOR < std :: pair < const PIPSocket :: Address, IntRegisteredInPeerMap > > > RegisteredInPeersMap;
	RegisteredInPeersMap registeredInPeers;
	typedef std :: map < ss :: string, RegisteredInPeer, std :: less < ss :: string >,
		__SS_ALLOCATOR < std :: pair < const ss :: string, RegisteredInPeer > > > AdmissedCallsMap;
	AdmissedCallsMap admissedCalls;
	InterestedMap wantedLogins; // acctn -> list of acctns which have it in contact lists
	typedef std :: vector < std :: tr1 :: function < void ( ) > > ReloadNotifierVector;
	ReloadNotifierVector reloadNotifiers;
	bool gateKeepersStarted;
public:
	typedef std :: map < ss :: string, StringBoolMap, std :: less < ss :: string >,
		__SS_ALLOCATOR < std :: pair < const ss :: string, StringBoolMap > > > UnregisteredContactsMap;
private:
	UnregisteredContactsMap internalUnregisteredContacts; // acctn -> list of -numbers to send
	bool smsParallelStart;
	StringStringSetMap recodes, backRecodes, fullRecodes;
	int externalRoutingTimeout;
	RadiusCallsMap :: iterator findRadiusCall ( const ss :: string & h323Conf, const ss :: string & ipSession );
	void eraseCardLock ( const ss :: string & acctn );
	void eraseRadiusLock ( const ss :: string & confId, ss :: string & userName, int acctSessionTime,
		AfterTaskVector & q );
	void clearRadiusSpool ( const ss :: string & confId, const ss :: string & h323Conf, int acctSessionTime,
		ss :: string & userName, int acctInputoctets, int acctOutputOctets, AfterTaskVector & q );
	void addH323Call ( const ss :: string & uname, const SourceData & source, EaterDetails & eaters, CallControl * ctrl );
	void eraseRadiusSpool ( const RadiusCallsMap :: iterator & ci, int acctSessionTime,
		AfterTaskVector & q );
	void eraseStalledCardLocks ( const ss :: string & h323Conf );
	void checkStalledCardCalls ( );
	void checkCommands ( );
	void doCommand ( const ss :: string & s );
	void eraseStalledCardCall ( const RadiusCallsMap :: iterator & i,
		AfterTaskVector & q );
	int checkPinInt ( RequestInfo & ri, AfterTaskVector & q );
	int checkPinIVR ( const ss :: string & acctn, const ss :: string & pass,
		double & retString, ss :: string & lang ) const;
	int handleSmsCallback1 ( RequestInfo & ri );
	int handleSmsCallback2 ( RequestInfo & ri );
	CardMoneySpool * getCardMoneySpool ( const CardInfo * card );
	CustomerMoneySpool * getCustomerMoneySpool ( const ss :: string & uname );
	const CustomerMoneySpool * hasCustomerMoneySpool ( const ss :: string & uname ) const;
	int getCardPrice ( bool eu, const ss :: string & realDigits, const CardInfo * card, int priceAdd, bool canZero,
		PriceData & priceData, ss :: string & code, bool & forceCredit ) const;
	int getCallBackPrice ( const ss :: string & realDigits, const CardInfo * card, ss :: string & code,
		PriceData & priceData ) const;
	void rejectRasLocks ( );
	void checkStalledPeerRegistrations ( );
	void reloadGateKeepers ( );
	static void setAdmissedCard ( CommonCallDetailsBase & call, const RegisteredInPeer & p );
	void eraseCardRegistration ( ss :: string login /*cant be ref*/, UnregisteredContactsMap & unregisteredContacts );
	static void removeCbEaters ( RadiusCall & rc, AfterTaskVector & q, int acctSessionTime );
	static void removeInEaters ( RadiusCall & rc, AfterTaskVector & q, int acctSessionTime );
	static void removeTrafEaters ( RadiusCall & rc, AfterTaskVector & q );
	void addCbEaters ( RadiusCall & rc, const CardInfo * card );
	void addInEaters ( RadiusCall & rc, const CardInfo * card );
	void addTrafEaters ( RadiusCall & rc, const CardInfo * card );
	ss :: string translateAnumber ( const SourceData & source, int outPeer, const ss :: string & anum );
	RegisteredOutPeersMap _registeredOutPeers;
	bool roundSecondsUp;
	int sipInviteRegistrationPeriod;
	unsigned sipDefaultRegistrationPeriod;
	bool randomEqualPeers;
	int crashedTimeStamp;
	void _registerOutPeer ( int peerId, const PIPSocket :: Address & addr, int port, int timeout,
		const ss :: string & suffix, PTime endLease, bool h323 );
	void _registerInPeer ( int outPeerId, const PIPSocket :: Address & addr, int port, bool fromNat, PTime endLease,
		UnregisteredContactsMap & unregisteredContacts, const ss :: string & prefix,
		const ss :: string & realPrefix );
	bool isGkIp ( const ss :: string & ip ) const;
/*
	void refreshCard ( const CardInfo * c );
	int applyCodeSql ( const RequestInfo & ri );
*/
	void refreshCard ( const CardInfo * c );
	void refreshCardByAcctn ( const ss :: string & acctn );
	int applyCodeSql ( const RequestInfo & ri );
	void externalRouteWrapper ( const ss :: string & callingDigitsIn, int peer, const ss :: string & realDigits,
		IntVector * outPeers );
	void fireReloadNotifiers ( );
public:
	explicit Impl ( Impl * & );
	~Impl ( );
	void reloadConf ( );
	bool getCallInfo ( OutChoiceDetailsVectorVector & forks, StringVector & forkOutAcctns, CommonCallDetailsBase & call,
		bool h323, bool sip, bool mgcp );
	bool isValidInPeerAddress ( const ss :: string & ip );
	bool isValidOutPeerAddress ( const ss :: string & ip, int port );
	void setAllowedRouteRTP( CommonCallDetailsBaseOut & common, int inPeerId );
	bool take ( CommonCallDetailsBaseOut & call, int & cause, bool * gwTaken );
	void release ( const CommonCallDetailsBaseOut & call, bool rc, bool gwTaken );
	bool shuttingDown ( ) const;
	void setShuttingDown ( );
	void take ( );
	bool tryTake ( );
	void release ( int inPeer );
	bool tryGwCall ( ) __attribute__ ( ( warn_unused_result ) );
	void releaseGwCall ( );
	void takeRadiusCall ( );
	void releaseRadiusCall ( );
	void takeHttpCall ( );
	void releaseHttpCall ( );
	void waitForSignallingChannels ( );
	int getLogLevel ( ) const;
	const ss :: string & getName ( ) const;
	bool isDebugInIp ( const ss :: string & ip );
	void addCall ( const CommonCallDetailsBaseOut & common, EaterDetails & eaters, CallControl * ctrl );
	void removeCall ( SourceData & source, EaterDetails & eaters );
	void balanceSecond ( );
	void balanceCards ( );
	void balanceSipCards ( StringSet & calls );
	double getMinutesFromSeconds ( int tarif, int seconds ) const;
	const PricePrios & getPrios ( ) const;
	bool validInRtp ( int peer, const ss :: string & ip ) const;
	bool validOutRtp ( int peer, const ss :: string & ip ) const;
	void lockRas ( );
	void unLockRas ( );
	bool waitGrq ( int id, ss :: string & gkid, int & reason );
	void rejectGrq ( int id, const ss :: string & gkid, int reason );
	void confirmGrq ( int id, const ss :: string & gkid );
	void waitRrq ( int id, ss :: string & eid, int & reason );
	void rejectRrq ( int id, int reason );
	void confirmRrq ( int id, const ss :: string & eid );
	bool waitArq ( int id, H225 :: TransportAddress & addr );
	void rejectArq ( int id );
	void confirmArq ( int id, const H225 :: TransportAddress & addr );
	void translateProtoClass ( int peer, const Q931 & from, Q931 & to ) const;
	ss :: string getRadiusIp ( );
	int getRadiusPort ( ) const;
	ss :: string getRadiusSecret ( );
	void getSecret ( const PIPSocket :: Address & ip, RadGWInfo & ri );
	int checkPin ( RequestInfo & ri );
	int getCreditTime ( RequestInfo & ri );
	int getCreditTimeIVR(RequestInfo & ri);
	void handleAccounting ( AccountingInfo & ai );
	int handleSmsCallback ( RequestInfo & ri );
	int applyCode ( RequestInfo & ri );
	void addActiveCall ( int id, CallControl * c );
	void removeActiveCall ( int id );
	bool isLocalIp ( const ss :: string & s ) const;
	bool getAddrFromGk ( const PIPSocket :: Address & gk, const ss :: string & digits, PIPSocket :: Address & ip,
		int & port ) const;
	int isCardPresentAndRegister( const ss :: string & number );
	void registerPeer ( const ss :: string & login, const PIPSocket :: Address & addr, int port, bool h323, bool fromNat,
		PTime endLease, bool isCard, const PIPSocket :: Address & sigAddr, int sigPort,
		const StringSet & neededNumbers, StringSet & onlineNumbers,
		UnregisteredContactsMap * unregisteredContacts, PUDPSocket & sock,
		IcqContactsVector & icqContacts );
	void unregisterInPeer ( const PIPSocket :: Address & addr, int port,
		UnregisteredContactsMap & unregisteredContacts );
	bool checkKeepAlive ( const PIPSocket :: Address & addr, int port, PTime endLease,
		UnregisteredContactsMap & unregisteredContacts );
	void startGateKeepers ( );
	bool getPassword ( const PIPSocket :: Address & localIp, const IpPortSet & addresses,
		const ss :: string & login, ss :: string & password, bool & isCard ) const;
	bool admissCall ( const PIPSocket :: Address & addr, int port, const ss :: string & confId );
	void disengageCall ( const ss :: string & confId );
	RecodersMap getRecoders ( ) const;
	void addSetupTime ( long long ms );
	bool getRegisteredCardRasAddr ( const ss :: string & login, PIPSocket :: Address & addr, int & port,
		PUDPSocket * & sock ) const;
	IntVector getLocalIntIps ( ) const;
	void remove ( H323Call * c );
	void addRelease ( int outPeer, const ss :: string & digits, int code );
	void replaceReleaseCompleteCause ( CommonCallDetails & common, bool fromCaller );
	int getReleaseCompleteCause ( CommonCallDetails & common, bool fromCaller );
	const LatencyLimits getMesgLatency ( int outPeerId ) const;
	void checkNewContacts ( const ss :: string & login, const StringSet & newContactList,
		UnregisteredContactsMap & unregisteredContacts );
	bool isRegisteredCardAddr ( const ss :: string & ip, const WORD port, ss :: string & acctn );
	bool getGkInfo ( const ss :: string & ip, ss :: string & login, ss :: string & pswd, int & port );
	int getSipInviteRegistrationPeriod ( ) const { return sipInviteRegistrationPeriod; };
	int getSipDefaultRegistrationPeriod ( ) const { return sipDefaultRegistrationPeriod; };
	bool getSupportVia ( ) const;
	bool getIsNoChoiceCode ( ) const;
	Q931 :: CauseValues getDefaultDisconnectCause ( ) const;

	bool getSIPToH323ErrorResponse(int err, int* response, ss :: string* textResponse) const;
	bool getH323ToSIPErrorResponse(int err, int* response, ss :: string* textResponse) const;

	ss :: string getH323Text(int errorH323) const;
	ss :: string getSIPText(int errorSIP) const;

	const StringStringSetMap & getRecodes ( ) const;
	const StringStringSetMap & getBackRecodes ( ) const;
	const StringStringSetMap & getFullRecodes ( ) const;
	void checkCrashedActiveCalls ( );
	int getRounded ( double s ) const;
	void printActiveCalls ( std :: ostream & os ) const;
	void printActiveCallsCSV ( std :: ostream & os ) const;
	void printActiveCallsStats ( std :: ostream & os ) const;
	int loadIVRData ( const ss :: string & key, const ss :: string & value,
		double & strCost, ss :: string & lang ) const;
	bool checkAdminsPass ( const ss :: string & uname, const ss :: string & pass ) const;
	void cardLoaded ( const CardInfo * card );
	void addSipCallToRemove ( const ss :: string & confId );
	void destroyCustomerSpool ( const ss :: string & uname );
	void propagateCustomerBalanceChange ( const ss :: string & uname, double money );
	bool getMgcpConf ( MgcpGatewayInfoVector & v ) ;
	void registerReloadNotifier ( const std :: tr1 :: function < void ( ) > & f );
	bool getRportAgents ( StringVector & v ) const;
};

const int arqDelay = 3000;

Conf :: Impl * Conf :: impl = 0;

Conf :: Impl :: DestAtom :: DestAtom ( ) : peer ( 0 ), port ( 0 ) { }

Conf :: Impl :: DestAtom :: DestAtom ( const OutChoiceDetails & choice ) : peer ( choice.getPeer ( ) ),
	ip ( choice.getIp ( ) ), port ( choice.getPort ( ) ) { }

bool Conf :: Impl :: DestAtom :: operator< ( const DestAtom & d ) const {
	if ( peer < d.peer )
		return true;
	if ( peer > d.peer )
		return false;
	if ( ip < d.ip )
		return true;
	if ( ip > d.ip )
		return false;
	return port < d.port;
}

Conf :: Impl :: CardAfterTask :: CardAfterTask ( double rm, const ss :: string & a ) : realMoney ( rm ), acctn ( a ) {
	if ( Conf :: impl -> data -> cm.doQuery ( "update Cards set cost = cost - %f, status = if( "
		"status != 'closed', 'open', 'closed' ) where acctn = '%s'",
		realMoney, acctn.c_str ( ) ) )
		PSYSTEMLOG ( Error, "Query error: " << Conf :: impl -> data -> cm.error ( ) << ", query: " <<
			Conf :: impl -> data -> cm.getLastQuery ( ) );
	else
		PSYSTEMLOG ( Error, "card " << acctn << " -= " << realMoney );
}

Conf :: Impl :: CardAfterTask :: ~CardAfterTask ( ) { }

Conf :: Impl :: ExpAfterTask :: ExpAfterTask ( std :: time_t n, const ss :: string & a ) : now ( n ), acctn ( a ) {
	ss :: ostringstream os;
	os << "update Cards set expDate = from_days( to_days( from_unixtime( " << now <<
			" ) ) + expTimeFUse ) where acctn = '" << acctn << '\'';
	if ( Conf :: impl -> data -> cm.query ( os.str ( ) ) )
		PSYSTEMLOG ( Error, "Query error: " << Conf :: impl -> data -> cm.error ( ) << ", query: " <<
			Conf :: impl -> data -> cm.getLastQuery ( ) );
}

Conf :: Impl :: ExpAfterTask :: ~ExpAfterTask ( ) { }

class TakeRadiusCallTask : public AfterTask {
	~TakeRadiusCallTask ( ) {
		conf -> takeRadiusCall ( );
	}
};

void Conf :: Impl :: CardMoneySpool :: destroy ( ) {
	Conf :: impl -> cardCalls.erase ( acctn );
}

void Conf :: Impl :: CardMoneySpool :: propagateBalanceChange ( ) {
	if ( CardInfo * ci = Conf :: impl -> data -> cardByAcctn ( acctn ) )
		ci -> cost = getTotalMoney ( ) * valuteRate;
}

Pointer < AfterTask > Conf :: Impl :: CardMoneySpool :: getAfterTask ( double realMoney ) const {
	return new CardAfterTask ( realMoney * valuteRate, acctn );
}

Conf :: Impl :: CardMoneySpool :: CardMoneySpool ( const ss :: string & a, double v ) : acctn ( a ), valuteRate ( v ) { }

int Conf :: Impl :: H323CardMoneyEater :: getRealSecs ( ) const {
	return ctrl -> getCallSeconds ( );
}

void Conf :: Impl :: H323CardMoneyEater :: stop ( ) {
	ctrl -> beginShutDown ( );
}

Conf :: Impl :: H323CardMoneyEater :: H323CardMoneyEater ( MoneySpool & o, double p, const TarifInfo & t,
	const TarifRound & r, const PriceData :: AmortiseMap & am, CallControl * c ) :
	FullMoneyEater ( o, p, t, r, am ), ctrl ( c ) { }

int Conf :: Impl :: RadiusCardMoneyEater :: getRealSecs ( ) const {
	return acctSessionTime;
}

void Conf :: Impl :: RadiusCardMoneyEater :: stop ( ) {
	// we can't stop radius call...
}

bool Conf :: Impl :: RadiusCardMoneyEater :: tickable ( ) const {
	return false;
}

Conf :: Impl :: RadiusCardMoneyEater :: RadiusCardMoneyEater ( MoneySpool & o, const PriceData & p, const ss :: string & conf ) :
	FullMoneyEater ( o, p.getPrice ( ), p.getTarif ( ), p.getRound ( ), p.getAmortise ( ) ),
	h323Conf ( conf ), acctSessionTime ( 0 ) { }

Pointer < AfterTask > Conf :: Impl :: RadiusCardMoneyEater :: detach ( int ast ) {
	acctSessionTime = ast;
	return MoneyEater :: detach ( );
}

static ss :: string md5pin ( const ss :: string & s ) {
	ss :: string md5pin = md5Sum ( s );
	const char * table = "0123456789abcdef";
	ss :: string r ( 32, ' ' );
	for ( int i = 0; i < 16; i ++ ) {
		r [ i * 2 ] = table [ static_cast < unsigned char > ( md5pin [ i ] ) / 16 ];
		r [ i * 2 + 1 ] = table [ static_cast < unsigned char > ( md5pin [ i ] ) % 16 ];
	}
	return r;
}

Conf :: Impl :: Impl ( Impl * & impl ) : data ( 0 ), m STD_MYSQL_PARMS, lm ( "", "psbc", "", "radius", 0, "/opt/psbc/var/db_3336.sock" ),
	dbc ( m, "radius.RadConf" ), updateId ( - 2 ), inShuttingDown ( false ), callsSem ( totalCallsLimit,
#ifdef _DEBUG
//	200
	std :: min ( 2800, totalCallsLimit )
#else
	totalCallsLimit
#endif
	), gwCallsSem ( gwTotalCallsLimit, gwTotalCallsLimit ),
	radiusCallsSem ( radiusTotalCallsLimit, radiusTotalCallsLimit + 1 ), httpSem ( 200, 200 ), logLevel (
#ifdef _DEBUG
	2
#else
	1
#endif
	), noStartRadiusCallThreshold ( 180 ), gateKeepersStarted ( false ), smsParallelStart ( true ),
	externalRoutingTimeout ( 5 ), roundSecondsUp ( false ), sipInviteRegistrationPeriod ( 1 ),
	sipDefaultRegistrationPeriod ( 0 ), randomEqualPeers ( false ), crashedTimeStamp ( - 1 ) {
	impl = this;
	PIPSocket :: Address addr;
	PIPSocket :: GetHostAddress ( addr );
	if ( m.getInt ( "select get_lock( 'ssLock_%s', 10 )", static_cast < const char * > ( addr.AsString ( ) ) ) != 1 ) {
		std :: ostringstream os;
		os << "can't get lock for " << addr.AsString ( ) << ": " << m.error ( );
		throw std :: runtime_error ( os.str ( ) );
	}
	reloadConf ( );
	AutoMutex am ( confMut );
	MyResult r ( m.getQuery ( "select id, name from Codecs" ) );
	IntStringMap cm;
	while ( const char * * p = r.fetchRow ( ) )
		cm [ std :: atoi ( p [ 0 ] ) ] = p [ 1 ];
	r.reset ( m.getQuery ( "select fromCodec, toCodec from Recoders" ) );
	while ( const char * * p = r.fetchRow ( ) ) {
		const ss :: string & fname = cm [ std :: atoi ( p [ 0 ] ) ];
		const ss :: string & tname = cm [ std :: atoi ( p [ 1 ] ) ];
		recodes [ fname ].insert ( tname );
		backRecodes [ tname ].insert ( fname );
	}
	for ( StringStringSetMap :: const_iterator i = recodes.begin ( ); i != recodes.end ( ); ++ i ) {
		for ( StringSet :: const_iterator j = i -> second.begin ( ); j != i -> second.end ( ); ++ j ) {
			StringStringSetMap :: const_iterator k = recodes.find ( * j );
			if ( k == recodes.end ( ) )
				continue;
			if ( k -> second.find ( i -> first ) == k -> second.end ( ) )
				continue;
			fullRecodes [ i -> first ].insert ( * j );
		}
	}
	ss :: for_each ( recodes, SLMapPrint ( "recodes: " ) );
	ss :: for_each ( backRecodes, SLMapPrint ( "backRecodes: " ) );
	ss :: for_each ( fullRecodes, SLMapPrint ( "fullRecodes: " ) );
	PSYSTEMLOG ( Info, "Conf :: Impl :: Impl ( ) done" );
}

void Conf :: Impl :: startGateKeepers ( ) {
	const ConfData :: GateKeepersMap & gks = data -> getGateKeepers ( );
	for ( ConfData :: GateKeepersMap :: const_iterator i = gks.begin ( );
		i != gks.end ( ); i ++ ) {
		switch ( i -> second -> gkType ) {
			case ConfData :: H323GK: {
				Pointer < BaseGkClientThread > p1 ( new H323GkClientThread ( i -> second -> identifier,
					i -> second -> login.empty ( ) ? name : i -> second -> login,
					i -> second -> registerPeriod, i -> second -> localIps, i -> second -> bindIps,
					i -> second -> ip, i -> second -> port, i -> second -> name ),
					Pointer < BaseGkClientThread > :: notDel );
				gateKeepers [ i -> first ] = p1;
				break;
			}
			case ConfData :: SipRegistrar: {
//				Pointer < BaseGkClientThread > p2 ( new SipGkClientThread ( i -> second -> identifier,
//					i -> second -> login.empty ( ) ? name : i -> second -> login,
//					i -> second -> registerPeriod, i -> second -> localIps, i -> second -> bindIps,
//					i -> second -> ip, i -> second -> pswd, i -> second -> port, i -> second -> name ),
//					Pointer < BaseGkClientThread > :: notDel );
				Pointer < BaseGkClientThread > p2 ( new SIP2 :: GkClientThread ( i -> second -> login.empty ( ) ? name :
					i -> second -> login, i -> second -> registerPeriod, i -> second -> ip, i -> second -> localIps,
					i -> second -> pswd, i -> second -> port, i -> second -> name ),
					Pointer < BaseGkClientThread > :: notDel );
				gateKeepers [ i -> first ] = p2;
				break;
			}
			case ConfData :: SilentSipRegistrar : // for LOGIN and PSWD only, no thread
				break;
			default:
				PSYSTEMLOG ( Info, "StartGateKeepers. Unknown GKType=" << i -> second -> gkType );
		}
	}
	gateKeepersStarted = true;
}

Conf :: Impl :: ~Impl ( ) {
	delete data;
	for ( GateKeepersMap :: iterator i = gateKeepers.begin ( ); i != gateKeepers.end ( ); ++ i ) {
		if ( i -> second -> IsTerminated ( ) )
			delete i -> second;
		else
			PSYSTEMLOG ( Error, "GATEKEEPER CLIENT THREAD DID NOT TERMINATE !!!!!!!!!!!!!!!" );
	}
	callsCount.Signal ( );
	httpCount.Signal ( );
}

template < class Container > static void printOutChoices ( const Container & choices, bool force = false ) {
	if ( force )
		PSYSTEMLOG ( Error, "getOutChoices:" );
	else
		PSYSTEMLOG ( Info, "getOutChoices:" );
	for ( typename Container :: const_iterator i = choices.begin ( ); i != choices.end ( ); ++ i ) {
		if ( force )
			PSYSTEMLOG ( Error, "choice: " << * i );
		else
			PSYSTEMLOG ( Info, "choice: " << * i );
	}
}

bool Conf :: Impl :: shuttingDown ( ) const {
	return inShuttingDown;
}

void Conf :: Impl :: setShuttingDown ( ) {
	AutoMutex am ( mut );
	inShuttingDown = true;
	for ( GateKeepersMap :: iterator i = gateKeepers.begin ( );
		i != gateKeepers.end ( ); i ++ )
		i -> second -> Close ( );
	rejectRasLocks ( );
}

void Conf :: Impl :: reloadGateKeepers ( ) {
	if ( ! gateKeepersStarted )
		return;
	const ConfData :: GateKeepersMap & ngks = data -> getGateKeepers ( );
	StringList o, s, n;
	set_threeway ( firster ( gateKeepers.begin ( ) ), firster ( gateKeepers.end ( ) ),
		firster ( ngks.begin ( ) ), firster ( ngks.end ( ) ), std :: back_inserter ( o ),
		std :: back_inserter ( s ), std :: back_inserter ( n ) );
	for ( StringList :: iterator i = s.begin ( ); i != s.end ( ); ++ i ) {
		const ConfData :: GateKeeperInfo * g = ngks.find ( * i ) -> second;
		gateKeepers [ * i ] -> updateState ( g -> identifier, g -> login.empty ( ) ? name : g -> login,
			g -> registerPeriod );
	}
	for ( StringList :: iterator i = n.begin ( ); i != n.end ( ); ++ i ) {
		const ConfData :: GateKeeperInfo * g = ngks.find ( * i ) -> second;
		switch ( g -> gkType ) {
			case ConfData :: H323GK: {
				Pointer < BaseGkClientThread > p1 ( new H323GkClientThread ( g -> identifier,
					g -> login.empty ( ) ? name : g -> login, g -> registerPeriod, g -> localIps,
					g -> bindIps, g -> ip, g -> port, g -> name ),
					Pointer < BaseGkClientThread > :: notDel );
				gateKeepers [ * i ] = p1;
				break;
			}
			case ConfData :: SipRegistrar: {
//				Pointer < BaseGkClientThread > p2 ( new SipGkClientThread ( g -> identifier,
//					g -> login.empty ( ) ? name : g -> login, g -> registerPeriod, g -> localIps,
//					g -> bindIps, g -> ip, g -> pswd, g -> port, g -> name ),
//					Pointer < baseGkClientThread > :: notDel );
				Pointer < BaseGkClientThread > p2 ( new SIP2 :: GkClientThread ( g -> login.empty ( ) ? name : g -> login,
					g -> registerPeriod, g -> ip, g -> localIps, g -> pswd, g -> port, g -> name ),
					Pointer < BaseGkClientThread > :: notDel );
				gateKeepers [ * i ] = p2;
				break;
			}
			case ConfData :: SilentSipRegistrar: // for LOGIN and PSWD only, no thread
				break;
			default:
				PSYSTEMLOG ( Info, "Unknown GKType=" << g -> gkType );
		}
	}
	for ( StringList :: iterator i = o.begin ( ); i != o.end ( ); ++ i ) {
		gateKeepers [ * i ] -> Close ( );
	}
	if ( ! o.empty ( ) ) {
		AntiMutex am ( mut );
		PThread :: Sleep ( 10000 );
	}
	for ( StringList :: iterator i = o.begin ( ); i != o.end ( ); ++ i ) {
		if ( gateKeepers [ * i ] -> IsTerminated ( ) )
			delete gateKeepers [ * i ];
		else
			PSYSTEMLOG ( Error, "GATEKEEPER CLIENT THREAD DID NOT TERMINATE !!!!!!!!!!!!!!!" );
		gateKeepers.erase ( * i );
	}
}

void Conf :: Impl :: reloadConf ( ) {
	static int c = 0;
	if ( shuttingDown ( ) )
		return;
	int shutDown = - 1;
	if ( c % 6 == 0 )
		checkStalledCardCalls ( );
	if ( data )
		checkCommands ( );
	loadPrios ( );
	PTime now;
	long secsToBalance = now.GetTimeInSeconds ( ) - lastBalance.GetTimeInSeconds ( );
	if ( secsToBalance ) {
		balanceSecond ( );
		lastBalance = now;
		checkStalledPeerRegistrations ( );
	}
	ss :: string tmpRadiusIp = "";
	int tmpRadiusPort = 1812;
	ss :: string tmpRadiusSecret = "";
	int peer = - 1;
	ss :: string digits;
	int inp;
	ConfData * tmp2 = 0, * tmp = 0; //eto vse ne exception-safe
	{
		AutoMutex cam ( confMut );
		dbc.get ( "shutDown", shutDown );
		if ( shutDown == 1 ) {
			inShuttingDown = true;
			dbc.set ( "shutDown", 0 );
			setShuttingDown ( );
			PThread :: Sleep ( 1000 );
			Conf :: TerminationSema -> Signal ( );
			return;
		}
		PIPSocket :: Address addr;
		PIPSocket :: GetHostAddress ( addr );
		c ++;
		if ( name == "" ) {
			name = m.getString ( "select name from gws where ip = '%s'",
				( const char * ) addr.AsString ( ) );
			if ( name == "" )
				throw MySQLError ( m );
			PSYSTEMLOG ( Error, "name: " << name );
		}
		ss :: string nm = "shutDown_" + name;
		dbc.get ( nm, shutDown );
		if ( shutDown == 1 ) {
			inShuttingDown = true;
			dbc.set ( nm, 0 );
			Conf :: TerminationSema -> Signal ( );
			return;
		}
		int tmpLogLevel = - 1;
		dbc.get ( "logLevel_" + name, tmpLogLevel );
		if ( tmpLogLevel != - 1 && tmpLogLevel != logLevel ) {
			if ( tmpLogLevel == 1 ) {
				PServiceProcess :: Current ( ).SetLogLevel ( PSystemLog :: Error );
				logLevel = 1;
			} else {
				PServiceProcess :: Current ( ).SetLogLevel ( PSystemLog :: Info );
				logLevel = 2;
			}
			PTrace :: SetLevel ( 1 + tmpLogLevel );
		}
		int tmpStart = - 1;
		dbc.get ( "radiusStartTime", tmpStart );
		if ( tmpStart != - 1 )
			noStartRadiusCallThreshold = tmpStart;
		int tmpSmsParallelStart = - 1;
		dbc.get ( "smsParallelStart", tmpSmsParallelStart );
		if ( tmpSmsParallelStart != - 1 )
			smsParallelStart = tmpSmsParallelStart;
		int bytesPerKb = - 1;
		dbc.get ( "bytesPerKb", bytesPerKb );
		if ( bytesPerKb <= 0 )
			bytesPerKb = 1024;
		bytesPerMb = bytesPerKb * bytesPerKb;
		int tmpExternalRoutingTimeout = - 1;
		dbc.get ( "externalRoutingTimeout", tmpExternalRoutingTimeout );
		if ( tmpExternalRoutingTimeout != - 1 )
			externalRoutingTimeout = tmpExternalRoutingTimeout;
		int tmpRoundSecondsUp = - 1;
		dbc.get ( "roundSecondsUp", tmpRoundSecondsUp );
		if ( tmpRoundSecondsUp != - 1 )
			roundSecondsUp = tmpRoundSecondsUp;
		// Default value, used when EXPIRES not found in SIP.REGISTER
		int tmpSipDefaultRegistrationPeriod = -1;
		dbc.get ( "sipDefaultRegistrationPeriod", tmpSipDefaultRegistrationPeriod );
		if ( tmpSipDefaultRegistrationPeriod != -1 )
			sipDefaultRegistrationPeriod = tmpSipDefaultRegistrationPeriod;
		// Default value, used when SIP.INVITE comes w/o SIP.REGISTER. Used for GKLogins in hunting
		int tmpSipInviteRegistrationPeriod = -1;
		dbc.get ( "sipInviteRegistrationPeriod", tmpSipInviteRegistrationPeriod );
		if ( tmpSipInviteRegistrationPeriod != -1 )
			sipInviteRegistrationPeriod = tmpSipInviteRegistrationPeriod;

		int tmpRandomEqualPeers = -1;
		dbc.get ( "randomEqualPeers", tmpRandomEqualPeers );
		if ( tmpRandomEqualPeers != -1 )
			randomEqualPeers = tmpRandomEqualPeers;
		static bool crashedTimeStampInited = false;


		if ( ! crashedTimeStampInited ) {
			crashedTimeStampInited = true;
			dbc.get ( "timeStamp_" + name, crashedTimeStamp );
		}
		else
			dbc.set ( "timeStamp_" + name, m.getInt ( "select UNIX_TIMESTAMP()" ) );
		int tmpUpdateId = updateId - 1;
		dbc.get ( "updateId", tmpUpdateId );
		if ( updateId == tmpUpdateId && m.getBool ( "select now() < '%s'", data -> getTimeToChange ( ).c_str ( ) ) )
			return;
		updateId = tmpUpdateId;
		PSYSTEMLOG ( Info, "config changed, reloading" );
		peer = m.getInt ( "select peer from DebugHunt" );
		digits = m.getString ( "select digits from DebugHunt" );
		inp = m.getInt ( "select price from DebugHunt" );
		dbc.get ( "radiusIp", tmpRadiusIp );
		dbc.get ( "radiusPort", tmpRadiusPort );
		dbc.get ( "radiusSecret", tmpRadiusSecret );
		int gwId = m.getInt ( "select id from gws where name = '%s'", name.c_str ( ) );
		if ( gwId == - 1 )
			throw MySQLError ( m );
PSYSTEMLOG ( Info, "A1" );
		tmp = new ConfData ( m, gwId, bytesPerKb );
PSYSTEMLOG ( Info, "A2" );
		dbc.update("updateId", tmp->m_reloadConfigTime, "fval");
	}
	PSYSTEMLOG ( Info, "new data ready" );
	{
		AutoMutex am ( mut );
		tmp2 = data;
		data = tmp;
		radiusIp = tmpRadiusIp;
		radiusPort = tmpRadiusPort;
		radiusSecret = tmpRadiusSecret;
		for ( CardCallsMap :: iterator i = cardCalls.begin ( );
			i != cardCalls.end ( ); ++ i ) {
			if ( CardInfo * card = data -> cardByAcctn ( i -> first ) ) {
				i -> second -> setTotalMoney ( card -> cost / card -> valuteRate );
				i -> second -> setCredit ( card -> credit );
			}
		}
		for ( StringCustomerMoneySpoolMap :: iterator i = customerCalls.begin ( );
			i != customerCalls.end ( ); ++ i ) {
			if ( ResponsibleInfo * ri = data -> getResponsibleInfo ( i -> first ) )
				i -> second -> setTotalMoney ( ri -> balance );
			else if ( CustomerInfo * ci = data -> getCustomerInfo ( i -> first ) )
				i -> second -> setTotalMoney ( ci -> balance );
		}
		reloadGateKeepers ( );
		fireReloadNotifiers ( );
		if ( tr.cnt )
			PSYSTEMLOG ( Error, "reaction to Setup: min " << tr.min << " max " << tr.max <<
				" avg " << tr.sum / tr.cnt << " milliseconds" );
		tr = TimeReaction ( );
		if ( fulltr.cnt )
			PSYSTEMLOG ( Error, "full reaction to Setup: min " << fulltr.min << " max " << fulltr.max <<
				" avg " << fulltr.sum / fulltr.cnt << " milliseconds" );
		fulltr = TimeReaction ( );
	}
	delete tmp2;
	PSYSTEMLOG ( Info, "old data deleted" );
	if ( mgcpThread )
		mgcpThread -> reloadState ( );
	if ( peer == - 1 )
		return;
	OutChoiceSetVector choices;
	SourceData sd;
	sd.type = SourceData :: inbound;
	sd.peer = peer;
	sd.price = inp;
	int errn = 0;
	bool allc;
	CodecInfoSet allowedc;
	CodecInfoVector incomingc;
	StringVector forkOutAcctns;
	try {
		AutoMutex am ( & mut );
		bool externalRoutes;
		data -> getCallInfo ( sd, digits, incomingc, customerCalls, registeredCards, _registeredOutPeers,
			choices, forkOutAcctns, digits , errn, true, false, false, allc, allowedc, externalRoutes );
	} catch ( std :: exception & e ) {
		PSYSTEMLOG ( Error, "getCallInfo exception: " << e.what ( ) );
	}
	if ( ! choices.empty ( ) )
		printOutChoices ( choices [ 0 ], true );
	if ( errn )
		PSYSTEMLOG ( Info, "result code: " << errn );
}

void Conf :: Impl :: setAdmissedCard ( CommonCallDetailsBase & call, const RegisteredInPeer & p ) {
	PSYSTEMLOG ( Info, "using admissed card call from acctn " << p.getAcctn ( ) );
	call.source ( ).acctn = p.getAcctn ( );
	call.source ( ).type = SourceData :: card;
	call.setFromNat ( p.getFromNat ( ) );
}

void Conf :: Impl :: externalRouteWrapper ( const ss :: string & callingDigitsIn, int peer, const ss :: string & realDigits,
	IntVector * outPeers ) {
	ss :: string replaceAni;
	AntiMutex am ( mut );
	externalRoute ( externalRoutingTimeout, radiusSecret, callingDigitsIn, peer, realDigits, * outPeers, replaceAni );
}

bool Conf :: Impl :: getCallInfo ( OutChoiceDetailsVectorVector & forks, StringVector & forkOutAcctns,
	CommonCallDetailsBase & call, bool h323, bool sip, bool mgcp ) {

#define LOGINFO( variables ) do { if ( call.getForceDebug ( ) ) { PSYSTEMLOG ( Error, variables ); } \
	else PSYSTEMLOG ( Info, variables ); } while ( 0 )

	PTime ciStart;
	OutChoiceSetVector choices;
	SourceData & source = call.source ( );
	bool externalRouting = false;
	ss :: string callingDigitsIn = call.getCallingDigitsIn ( );
	AutoMutex am ( mut );
	try {
		ss :: string ip = call.getCallerIp ( );
		int port = call.getCallerPort ( );
		ss :: string dialedDigits = call.getDialedDigits ( );
		LOGINFO ( "getPeerForDigits: " << ip << ':' << port << ", "
			<< callingDigitsIn << ", " << dialedDigits << ", " << source.price );
		bool r = false;
		ss :: string realDigits;
		if ( source.type == SourceData :: inbound ) {
			AdmissedCallsMap :: const_iterator i = admissedCalls.find ( call.getConfId ( ) );
			if ( i != admissedCalls.end ( ) ) { // admissed h323 call
				const RegisteredInPeer & p = i -> second;
				if ( p.getAcctn ( ).empty ( ) ) {
					if ( p.getPrefix ( ) != dialedDigits.substr ( 0, p.getPrefix ( ).size ( ) ) ) {
						PSYSTEMLOG ( Error, "bad prefix for registered inpeer" );
						return false;
					}
					realDigits = p.getRealPrefix ( ) + dialedDigits.substr ( p.getPrefix ( ).size ( ) );
					source.peer = p.getUid ( );
					call.setFromNat ( p.getFromNat ( ) );
					source.inPrefix = p.getPrefix ( );
				} else {
					setAdmissedCard ( call, p );
				}
			} else if ( const RegisteredInPeer * ri = findMapElement ( registeredInPeers,
				PIPSocket :: Address ( ip.c_str ( ) ), port ) ) { // authorized by GKLogins SIP call
				if ( ri -> getAcctn ( ).empty ( ) ) {
					if ( ri -> getPrefix ( ) != dialedDigits.substr ( 0, ri -> getPrefix ( ).size ( ) ) ) {
						PSYSTEMLOG ( Error, "bad prefix for registered inpeer" );
						return false;
					}
					realDigits = ri -> getRealPrefix ( ) + dialedDigits.substr ( ri -> getPrefix ( ).size ( ) );
					source.peer = ri -> getUid ( );
					call.setFromNat ( ri -> getFromNat ( ) );
					source.inPrefix = ri -> getPrefix ( );
				}
			} else {
				int fromNat = 0;
				int retCode = call.getDisconnectCause ( );
				source.peer = data -> getInPeer ( ip, callingDigitsIn, dialedDigits,
					realDigits, fromNat, source.inPrefix, retCode );
				call.setDisconnectCause ( retCode );	
				call.setFromNat ( fromNat );
			}
			if ( const InPeerInfo * ip = data -> getInPeerInfo ( source.peer ) )
				call.sigOptions ( ).setIn ( ip -> sigOptions );
		} else if ( h323 ) {
			if ( const RegisteredInPeer * p = findMapElement ( admissedCalls, call.getConfId ( ) ) ) {
				if ( ! p -> getAcctn ( ).empty ( ) )
					setAdmissedCard ( call, * p );
			}
		}
		int errn = 0;
//		r = data -> getCallInfo ( source, dialedDigits, call.incomingCodecs ( ), customerCalls, registeredCards,
//			_registeredOutPeers, choices, forkOutAcctns, realDigits, errn, h323, sip, mgcp, call.inAllCodecs ( ),
//			call.inAllowedCodecs ( ), externalRouting );
		r = data -> getCallInfo ( source, dialedDigits, call.incomingCodecs ( ), customerCalls, registeredCards,
			_registeredOutPeers, choices, forkOutAcctns, realDigits, errn, h323, sip, mgcp, call.inAllCodecs ( ),
			call.inAllowedCodecs ( ),
			std :: tr1 :: bind ( & Conf :: Impl :: externalRouteWrapper, this, callingDigitsIn,
			std :: tr1 :: placeholders :: _1, std :: tr1 :: placeholders :: _2, std :: tr1 :: placeholders :: _3 ) );
		if ( source.type == SourceData :: card ) {
			if ( CardInfo * card = data -> cardByAcctn ( source.acctn ) ) {
				call.sigOptions ( ).setIn ( card -> getSigOptions ( ) );
			} else {
				PSYSTEMLOG ( Info, "Adding SigOptions. Card " << source.acctn << " not found" );
			}
		}
		if ( errn )
			call.setDisconnectCause ( errn );
		call.setRealDigits ( realDigits );
		if ( ! source.inLocked && isMaster && r && source.type == SourceData :: card ) {
			PSYSTEMLOG ( Info, "~~~~~~~~~~~~~~~~~~~ Card " << source.acctn );
			const ConfData :: PrepaidInfo * pi = data -> getPrepaidInfo ( source.peer );
			if ( pi && pi -> lines ) {
				if ( int ( cardLocks.count ( source.acctn ) ) < pi -> lines ) {
					PSYSTEMLOG ( Info, "locking card " << source.acctn );
					cardLocks.insert ( source.acctn );
					source.inLocked = true;
					if ( CardInfo * card = data -> cardByAcctn ( source.acctn ) ) {
						card -> logLocking ( cardLocks.count ( source.acctn ) );
					}
				} else {
					PSYSTEMLOG ( Error, "card already locked" );
					r = false;
				}
			} else
				PSYSTEMLOG ( Info, "~~~~~~~~~~~~~~~~~~~ Card error pi" );
		}
		if ( ! source.outLocked && isMaster && r && ! source.outAcctn.empty ( ) ) {
			const CardInfo * c = data -> cardByAcctn ( source.outAcctn );
			if ( c && c -> parent -> lines ) {
				PSYSTEMLOG ( Info, "cardLocks [ " << source.outAcctn << " ] = " <<
					cardLocks.count ( source.outAcctn ) );
				if ( int ( cardLocks.count ( source.outAcctn ) ) < c -> parent -> lines ) {
					PSYSTEMLOG ( Info, "locking out card " << source.outAcctn );
					source.outLocked = true;
					cardLocks.insert ( source.outAcctn );
					if ( CardInfo * card = data -> cardByAcctn ( source.outAcctn ) ) {
						card -> logLocking ( cardLocks.count ( source.outAcctn ) );
					}
				} else {
					PSYSTEMLOG ( Error, "out card already locked" );
					r = false;
				}
			}
		}
		for ( unsigned i = 0; i < choices.size ( ); i ++ )
			printOutChoices ( choices [ i ] );
		if ( r && source.type == SourceData :: inbound ) {
			const InPeerInfo * i = data -> getInPeerInfo ( source.peer );
			if ( ! i || ( i -> limit && inLimits [ source.peer ] >= i -> limit ) ) {
				PSYSTEMLOG ( Error, "limit reached for inpeer " << source.peer );
				call.setDisconnectCause ( conf -> getDefaultDisconnectCause ( ) /*Q931 :: cvNoCircuitChannelAvailable*/ );
				return false;
			}
			inLimits [ source.peer ] ++;
			call.setDirectIn ( i -> directRTP );
			call.setInTaken ( );
		}
		if ( ! r ) {
			PSYSTEMLOG ( Error, "can't get call info for " << dialedDigits << '@' << ip << ':' << port << '-' <<
				source.peer );
			return false;
		}
	} catch ( std :: exception & e ) {
		PSYSTEMLOG ( Error, "getPeerForDigits exception: " << e.what ( ) );
		return false;
	}
	for ( unsigned fn = 0; fn < choices.size ( ); fn ++ ) {
		ss :: string replaceAni;
		OutChoiceDetailsVector fch;
		if ( externalRouting ) {
			IntVector outPeers;
			replaceAni = callingDigitsIn;
			{
				AntiMutex am ( mut );
				externalRoute ( externalRoutingTimeout, radiusSecret, callingDigitsIn, source.peer,
					call.getRealDigits ( ), outPeers, replaceAni );
			}
			for ( std :: size_t i = 0; i < outPeers.size ( ); i ++ ) {
				PSYSTEMLOG ( Info, "externalRouting outpeer " << outPeers [ i ] );
				if ( i == 0 && ! outPeers [ i ] )
					goto justAni;
				for ( OutChoiceSet :: const_iterator j = choices [ fn ].begin ( );
					j != choices [ fn ].end ( ); ++ j ) {
					if ( j -> getPid ( ) == outPeers [ i ] ) {
						ss :: string anum = replaceAni;
						fch.push_back ( OutChoiceDetails ( * j, anum ) );
					}
				}
			}
		} else {
			justAni:
			while ( ! choices [ fn ].empty ( ) ) {
				OutChoiceSet :: const_iterator i = choices [ fn ].begin ( );
				if ( choices [ fn ].count ( * i ) < 2 ) {
					ss :: string anum = replaceAni.empty ( ) ?
						translateAnumber ( source, i -> getPid ( ), callingDigitsIn ) : replaceAni;
					fch.push_back ( OutChoiceDetails ( * i, anum ) );
					choices [ fn ].erase ( i );
					continue;
				}
				std :: pair < OutChoiceSet :: const_iterator, OutChoiceSet :: const_iterator > range =
					choices [ fn ].equal_range ( * i );
				typedef std :: multimap < int, OutChoiceSet :: const_iterator, std :: less < int >,
					__SS_ALLOCATOR < std :: pair < const int, OutChoiceSet :: const_iterator > > >
					EqualPeersMultimap;
				EqualPeersMultimap equalPeers;
				if ( randomEqualPeers ) {
					for ( ; i != range.second; i ++ )
						equalPeers.insert ( std :: make_pair ( rand ( ), i ) );
				} else {
					for ( ; i != range.second; i ++ )
						equalPeers.insert ( std :: make_pair ( outLimits [ i -> getPid ( ) ], i ) );
				}
				for ( EqualPeersMultimap :: const_iterator y = equalPeers.begin ( ); y != equalPeers.end ( ); y ++ ) {
					i = y -> second;
					ss :: string anum = replaceAni.empty ( ) ?
						translateAnumber ( source, i -> getPid ( ), callingDigitsIn ) : replaceAni;
					fch.push_back ( OutChoiceDetails ( * i, anum ) );
				}
				choices [ fn ].erase ( range.first, range.second );
			}
		}
		forks.push_back ( fch );
		if ( fn == 0 ) {
			call.initChoices ( int ( fch.size ( ) ) );
			for ( OutChoiceDetailsVector :: size_type i = 0, sz = fch.size ( ); i < sz; i ++ )
				call.addChoice ( fch [ i ] );
		}
	}
	if ( source.type == SourceData :: card )
		call.setFromNat ( 2 );
	printOutChoices ( call.getChoices ( ) );
	tr.add ( ( PTime ( ) - ciStart ).GetMilliSeconds ( ) );
	return ! call.getChoices ( ).empty ( );
}

bool Conf :: Impl :: isValidInPeerAddress ( const ss :: string & ip ) {
	PSYSTEMLOG ( Info, "Checking inip: " << ip );
	AutoMutex am ( & mut );
	if ( const IntRegisteredInPeerMap * i =
		findMapElement ( registeredInPeers, PIPSocket :: Address ( ip.c_str ( ) ) ) ) {
		for ( IntRegisteredInPeerMap :: const_iterator j = i -> begin ( ); j != i -> end ( ); j ++ )
			if ( j -> second.getAcctn ( ).empty ( ) )
				return true;
	}
	return data -> isValidInPeerAddress ( ip );
}

bool Conf :: Impl :: isValidOutPeerAddress ( const ss :: string & ip, int port ) {
	AutoMutex am ( & mut );
	return _registeredOutPeers.getPeer ( IpPort ( ip, port ) );
}

void Conf :: Impl :: take ( ) {
	if ( callsSem.WillBlock ( ) )
		PSYSTEMLOG ( Error, "total call limit reached" );
	callsSem.Wait ( );
	++ callsCount;
}

bool Conf :: Impl :: tryTake ( ) {
	if ( ! callsSem.Wait ( 0 ) )
		return false;
	++ callsCount;
	return true;
}

void Conf :: Impl :: release ( int inPeer ) {
	if ( inPeer ) {
		AutoMutex am ( & mut );
		inLimits [ inPeer ] --;
		if ( inLimits [ inPeer ] < 0 )
#ifndef _DEBUG
			* ( int * ) 0 = 1;
#else
			PSYSTEMLOG ( Error, "inLimits [ " << inPeer << " ] == " << inLimits [ inPeer ] );
#endif
	}
	callsSem.Signal ( );
	-- callsCount;
}

bool Conf :: Impl :: tryGwCall ( ) {
	if ( gwCallsSem.Wait ( 0 ) ) {
		++ gwCallsCount;
		return true;
	}
	PSYSTEMLOG ( Error, "total gw call limit reached" );
	return false;
}

void Conf :: Impl :: releaseGwCall ( ) {
	gwCallsSem.Signal ( );
	-- gwCallsCount;
//	PSYSTEMLOG ( Info, "gwCallsCount: " << int ( gwCallsCount ) );
}

void Conf :: Impl :: takeRadiusCall ( ) {
	if ( radiusCallsSem.WillBlock ( ) )
		PSYSTEMLOG ( Error, "total radius call limit reached" );
	radiusCallsSem.Wait ( );
	++ radiusCallsCount;
}

void Conf :: Impl :: releaseRadiusCall ( ) {
	radiusCallsSem.Signal ( );
	-- radiusCallsCount;
}

void Conf :: Impl :: takeHttpCall ( ) {
	if ( httpSem.WillBlock ( ) )
		PSYSTEMLOG ( Error, "total http call limit reached" );
	httpSem.Wait ( );
	++ httpCount;
}

void Conf :: Impl :: releaseHttpCall ( ) {
	httpSem.Signal ( );
	-- httpCount;
}

void Conf :: Impl :: setAllowedRouteRTP ( CommonCallDetailsBaseOut & common, int inPeerId ) {
	const InPeerInfo * inPeer = data -> getInPeerInfo ( inPeerId );
	if ( ! inPeer)
		return;
	const RouteInfo * r = findMapElement ( inPeer -> allowedRoutes, inPeerId );
	if ( ! r )
		return;
	if ( r -> inDefaultDirectRTP )
		return;
	common.setDirectIn ( r -> directRTP );
}

bool Conf :: Impl :: take ( CommonCallDetailsBaseOut & call, int & cause, bool * gwTaken ) {
	if ( gwTaken ) {
		if ( ! tryGwCall ( ) ) {
			* gwTaken = false;
			return false;
		}
		* gwTaken = true;
	}
	const OutChoiceDetails & choice = * call.curChoice ( );
	int peer = choice.getPeer ( );
	int inPeer = call.getSource ( ).type == SourceData :: inbound ? call.getSource ( ).peer : 0;
	setAllowedRouteRTP ( call, inPeer );
	if ( ! peer ) {
		call.setOutAllCodecs ( true );
		call.setOutRemotePrice ( false );
		call.setIsGk ( false );
		const ss :: string & outAcctn = call.getSource ( ).outAcctn;
		AutoMutex am ( mut );
		call.setOutName ( "card " + outAcctn ); // must be protected by mut for printActiveCalls
		if ( const CardInfo * card = data -> cardByAcctn ( outAcctn ) )
			call.setOutRname ( card -> parent -> rname );
		return true;
	}
	AutoMutex am ( mut );
	const OutPeerInfo * o = data -> getOutPeerInfo ( peer );
	if ( ! o )
		return false;
	if ( o -> callBuffer ) {
		int t = o -> callBuffer -> getCode ( call.getRealDigits ( ) );
		if ( t ) {
			cause = t;
			PSYSTEMLOG ( Info, "disconnect cause from buffer " << t );
			return false;
		}
	}
	if ( o -> limit && outLimits [ peer ] >= o -> limit )
		return false;
	int lim = choice.getLim ( );
	if ( inPeer && lim && inOutLimits [ inPeer ] [ peer ] >= lim )
		return false;
	if ( int timeout = choice.getTimeout ( ) ) {
		if ( ! lastReleaseComplete [ choice ].take ( timeout, o -> limit ) ) {
			PSYSTEMLOG ( Info, "not using " << choice.getIp ( ) << " due to release complete timeout" );
			return false;
		}
	}
	outLimits [ peer ] ++;
	if ( inPeer )
		inOutLimits [ inPeer ] [ peer ] ++;
	call.setIsGk ( isGkIp ( choice.getIp ( ) ) );
	call.setOutName ( o -> name );
	call.setOutRname ( o -> rname );
	call.setOutRemotePrice ( o -> remotePrice );
	call.setOutAllCodecs ( o -> allCodecs );
	call.setOutAllowedCodecs ( o -> allowedCodecs );
	PSYSTEMLOG ( Info, "taking " << peer );
	return true;
}

void Conf :: Impl :: release ( const CommonCallDetailsBaseOut & call, bool rc, bool gwTaken ) {
	if ( gwTaken )
		releaseGwCall ( );
	const OutChoiceDetails * pChoice = call.curChoice ( );
	if ( ! pChoice )
		return;
	const OutChoiceDetails & choice = * pChoice;
	int peer = choice.getPeer ( );
	if ( ! peer )
		return;
	PSYSTEMLOG ( Info, "releasing " << peer );
	AutoMutex am ( & mut );
	outLimits [ peer ] --;
	if ( outLimits [ peer ] < 0 )
#ifdef _DEBUG
		* ( int * ) 0 = 1;
#else
		PSYSTEMLOG ( Error, "outLimits [ " << peer << " ] == " << outLimits [ peer ] );
#endif
	int inPeer = call.getSource ( ).type == SourceData :: inbound ? call.getSource ( ).peer : 0;
	if ( inPeer ) {
		inOutLimits [ inPeer ] [ peer ] --;
		if ( inOutLimits [ inPeer ] [ peer ] < 0 )
#ifdef _DEBUG
			* ( int * ) 0 = 1;
#else
			PSYSTEMLOG ( Error, "inOutLimits [ " << inPeer << " ] [ " << peer << " ] == " <<
				inOutLimits [ inPeer ] [ peer ] );
#endif
	}
	if ( Geganet * g = findMapElement ( lastReleaseComplete, DestAtom ( choice ) ) )
		g -> release ( rc );
}

void Conf :: Impl :: waitForSignallingChannels ( ) {
	callsCount.WaitCondition ( );
	httpCount.WaitCondition ( );
	for ( GateKeepersMap :: iterator i = gateKeepers.begin ( ); i != gateKeepers.end ( ); ) {
		if ( i -> second -> IsTerminated ( ) ) {
			delete i -> second;
			gateKeepers.erase ( i ++ );
		} else {
			PSYSTEMLOG ( Error, "GATEKEEPER CLIENT THREAD DID NOT TERMINATE !!!!!!!!!!!!!!!" );
			++ i;
		}
	}
}

int Conf :: Impl :: getLogLevel ( ) const {
	return logLevel;
}

bool Conf :: Impl :: isDebugInIp ( const ss :: string & ip ) {
	AutoMutex am ( mut );
	return data -> isDebugInIp ( ip );
}

const ss :: string & Conf :: Impl :: getName ( ) const {
	return name;
}

CustomerMoneySpool * Conf :: Impl :: getCustomerMoneySpool ( const ss :: string & uname ) {
	if ( Pointer < CustomerMoneySpool > * i = findMapElement ( customerCalls, uname ) )
		return * i;
	else {
		Pointer < CustomerMoneySpool > s = new CustomerMoneySpool ( uname );
		i = & customerCalls.insert ( std :: make_pair ( uname, s ) ).first -> second;
		if ( const ResponsibleInfo * ri = data -> getResponsibleInfo ( uname ) )
			( * i ) -> setTotalMoney ( ri -> balance );
		else if ( const CustomerInfo * ci = data -> getCustomerInfo ( uname ) )
			( * i ) -> setTotalMoney ( ci -> balance );
		return * i;
	}
}

const CustomerMoneySpool * Conf :: Impl :: hasCustomerMoneySpool ( const ss :: string & uname ) const {
	if ( const Pointer < CustomerMoneySpool > * p = findMapElement ( customerCalls, uname ) )
		return * p;
	return 0;
}

static void addConnectPriceToAmortise ( PriceData :: AmortiseMap & amortise, int connectPrice ) {
	if ( connectPrice )
		amortise [ 1 ] += connectPrice / 100000.0;
}

void Conf :: Impl :: addH323Call ( const ss :: string & uname, const SourceData & source, EaterDetails & eaters,
	CallControl * ctrl ) {
	CustomerInfo * ci = data -> getCustomerInfo ( uname );
	if ( ! ci )
		return;
	if ( ci -> parent -> useBalance ) {
		MoneySpool * spool = getCustomerMoneySpool ( ci -> parent -> uname );
		PriceData :: AmortiseMap amortise = source.amortise;
		addConnectPriceToAmortise ( amortise, source.connectPrice );
		eaters.inDealerEater = new H323CardMoneyEater ( * spool, source.price / 100000.0,
			* data -> getTarifInfo ( source.tarif ), source.round, amortise, ctrl );
	}
	if ( ! ci -> customerBalance )
		return;
	MoneySpool * spool = getCustomerMoneySpool ( uname );
	if ( ci -> parent -> useBalance ) {
		PriceData :: AmortiseMap amortise = source.amortise;
		addConnectPriceToAmortise ( amortise, source.euConnectPrice );
		eaters.inEater = new H323CardMoneyEater ( * spool, source.euPrice / 100000.0,
			* data -> getTarifInfo ( source.euTarif ), source.euRound, amortise, ctrl );
	} else {
		PriceData :: AmortiseMap amortise = source.amortise;
		addConnectPriceToAmortise ( amortise, source.connectPrice );
		eaters.inEater = new H323CardMoneyEater ( * spool, source.price / 100000.0,
			* data -> getTarifInfo ( source.tarif ), source.round, amortise, ctrl );
	}
}

Conf :: Impl :: CardMoneySpool * Conf :: Impl :: getCardMoneySpool ( const CardInfo * card ) {
	if ( Pointer < CardMoneySpool > * i = findMapElement ( cardCalls, card -> acctn ) )
		return * i;
	else {
		Pointer < CardMoneySpool > s = new CardMoneySpool ( card -> acctn, card -> valuteRate );
		i = & cardCalls.insert ( std :: make_pair ( card -> acctn, s ) ).first -> second;
		( * i ) -> setTotalMoney ( card -> cost / card -> valuteRate );
		( * i ) -> setCredit ( card -> credit );
		return * i;
	}
}

void Conf :: Impl :: addCall ( const CommonCallDetailsBaseOut & common, EaterDetails & eaters, CallControl * ctrl ) {
	if ( ! eaters.empty ( ) ) {
		PSYSTEMLOG ( Error, "second add call" );
		return;
	}
	PSYSTEMLOG ( Info, "Conf::Impl::addCall. Digits=" << common.getDialedDigits ( ) << " from " << common.getCallerIp ( )
		<< ':' << common.getCallerPort ( ) );
	const SourceData & source = common.getSource ( );
	const ss :: string & outAcctn = source.outAcctn;
	AutoMutex am ( mut );
	if ( ! outAcctn.empty ( ) ) {
		if ( CardInfo * ci = data -> cardByAcctn ( outAcctn ) ) {
			const OutCardDetails & ocd = common.curChoice ( ) -> getOutCardDetails ( );
			if ( ci -> responsibleBalance ) {
				MoneySpool * spool = getCustomerMoneySpool ( ci -> parent -> parent -> parent -> uname );
				PriceData :: AmortiseMap amortise = ocd.getAmortise ( );
				addConnectPriceToAmortise ( amortise, ocd.getConnectPrice ( ) );
				eaters.outResponsibleEater = new H323CardMoneyEater ( * spool,
					ocd.getPrice ( ) / 100000.0, * data -> getTarifInfo ( ocd.getTarif ( ) ),
					ocd.getRound ( ), amortise, ctrl );
			}
			if ( ci -> customerBalance ) {
				MoneySpool * spool = getCustomerMoneySpool ( ci -> parent -> uname );
				if ( ci -> responsibleBalance ) {
					PriceData :: AmortiseMap amortise = ocd.getAmortise ( );
					addConnectPriceToAmortise ( amortise, ocd.getEuConnectPrice ( ) );
					eaters.outCustomerEater = new H323CardMoneyEater ( * spool,
						ocd.getEuPrice ( ) / 100000.0,
						* data -> getTarifInfo ( ocd.getEuTarif ( ) ),
						ocd.getEuRound ( ), amortise, ctrl );
				} else {
					PriceData :: AmortiseMap amortise = ocd.getAmortise ( );
					addConnectPriceToAmortise ( amortise, ocd.getConnectPrice ( ) );
					eaters.outCustomerEater = new H323CardMoneyEater ( * spool,
						ocd.getPrice ( ) / 100000.0,
						* data -> getTarifInfo ( ocd.getTarif ( ) ),
						ocd.getRound ( ), amortise, ctrl );
				}
			}
			if ( ci -> cardBalance ) {
				MoneySpool * spool = getCardMoneySpool ( ci );
				PriceData :: AmortiseMap amortise = ocd.getAmortise ( );
				addConnectPriceToAmortise ( amortise, ocd.getEuConnectPrice ( ) );
				eaters.outCardEater = new H323CardMoneyEater ( * spool,
					ocd.getEuPrice ( ) / 100000.0,
					* data -> getTarifInfo ( ocd.getEuTarif ( ) ),
					ocd.getEuRound ( ), amortise, ctrl );
			}
		}
	}
	if ( source.type == SourceData :: unknown )
		return;
	ss :: string uname;
	if ( source.type == SourceData :: card ) {
		CardInfo * ci = data -> cardByAcctn ( source.acctn );
		if ( ! ci )
			return;
		uname = ci -> parent -> uname;
		if ( ci -> cardBalance ) {
			CardMoneySpool * spool = getCardMoneySpool ( ci );
			PriceData :: AmortiseMap amortise = source.amortise;
			addConnectPriceToAmortise ( amortise, source.euConnectPrice );
			eaters.cardEater = new H323CardMoneyEater ( * spool, source.euPrice / 100000.0,
				* data -> getTarifInfo ( source.euTarif ), source.euRound, amortise, ctrl );
		}
	} else {
		const InPeerInfo * ip = data -> getInPeerInfo ( source.peer );
		if ( ! ip )
			return;
		uname = ip -> uname;
	}
	addH323Call ( uname, source, eaters, ctrl );
}

void Conf :: Impl :: removeCall ( SourceData & source, EaterDetails & eaters ) {
	PSYSTEMLOG ( Info, "Conf::Impl::removeCall. " << source.acctn << '@' << source.peer );
	AfterTaskVector v;
	AutoMutex am ( mut );
	if ( const MoneyEater * e = eaters.outCardEater ) {
		v.push_back ( e -> detach ( ) );
		eaters.outCardEater = 0;
	}
	if ( const MoneyEater * e = eaters.outCustomerEater ) {
		v.push_back ( e -> detach ( ) );
		eaters.outCustomerEater = 0;
	}
	if ( const MoneyEater * e = eaters.outResponsibleEater ) {
		v.push_back ( e -> detach ( ) );
		eaters.outResponsibleEater = 0;
	}
	if ( source.type == SourceData :: card && source.inLocked ) {
		eraseCardLock ( source.acctn );
		source.inLocked = false;
	}
	if ( ! source.outAcctn.empty ( ) && source.outLocked ) {
		eraseCardLock ( source.outAcctn );
		source.outLocked = false;
	}
	if ( const MoneyEater * e = eaters.cardEater ) {
		v.push_back ( e -> detach ( ) );
		eaters.cardEater = 0;
	}
	if ( const MoneyEater * e = eaters.inEater ) {
		v.push_back ( e -> detach ( ) );
		eaters.inEater = 0;
	}
	if ( const MoneyEater * e = eaters.inDealerEater ) {
		v.push_back ( e -> detach ( ) );
		eaters.inDealerEater = 0;
	}
}

void Conf :: Impl :: balanceSecond ( ) {
	AutoMutex am ( mut );
	for ( StringCustomerMoneySpoolMap :: iterator i = customerCalls.begin ( );
		i != customerCalls.end ( ); ++ i )
		i -> second -> tick ( );
}

void Conf :: Impl :: balanceCards ( ) {
	AutoMutex am ( mut );
	for ( CardCallsMap :: iterator i = cardCalls.begin ( ); i != cardCalls.end ( ); ++ i )
		i -> second -> tick ( );
}

void Conf :: Impl :: balanceSipCards ( StringSet & calls ) {
	AutoMutex am ( mut );
	calls.swap ( sipCallsToRemove );
}

double Conf :: Impl :: getMinutesFromSeconds ( int tarif, int seconds ) const {
	AutoMutex am ( mut );
	const TarifInfo * t = data -> getTarifInfo ( tarif );
	return t -> getMinutes ( seconds );
}

const PricePrios & Conf :: Impl :: getPrios ( ) const {
	return prios;
}

void Conf :: Impl :: loadPrios ( ) {
	int depth = 0, prio = 0, price = 0;
	{
		AutoMutex cam ( confMut );
		dbc.get ( "prioDepth", depth );
		dbc.get ( "prioPrio", prio );
		dbc.get ( "prioPrice", price );
	}
	AutoMutex am ( mut );
	if ( depth < prio ) {
		if ( depth < price ) {
			prios.f1 = & OutChoice :: getMinusDepth;
			if ( prio < price ) {
				prios.f2 = & OutChoice :: getPrio;
				prios.f3 = & OutChoice :: getPrice;
			} else {
				prios.f3 = & OutChoice :: getPrio;
				prios.f2 = & OutChoice :: getPrice;
			}
		} else {
			prios.f1 = & OutChoice :: getPrice;
			prios.f2 = & OutChoice :: getMinusDepth;
			prios.f3 = & OutChoice :: getPrio;
		}
	} else {
		if ( prio < price ) {
			prios.f1 = & OutChoice :: getPrio;
			if ( depth < price ) {
				prios.f2 = & OutChoice :: getMinusDepth;
				prios.f3 = & OutChoice :: getPrice;
			} else {
				prios.f3 = & OutChoice :: getMinusDepth;
				prios.f2 = & OutChoice :: getPrice;
			}
		} else {
			prios.f1 = & OutChoice :: getPrice;
			prios.f2 = & OutChoice :: getPrio;
			prios.f3 = & OutChoice :: getMinusDepth;
		}
	}
}

bool Conf :: Impl :: validInRtp ( int peer, const ss :: string & ip ) const {
	AutoMutex am ( mut );
	if ( const InPeerInfo * i = data -> getInPeerInfo ( peer ) )
		if ( i -> rtpIps.size ( ) && ! i -> rtpIps.count ( ip ) )
			return false;
	return true;
}

bool Conf :: Impl :: validOutRtp ( int peer, const ss :: string & ip ) const {
	AutoMutex am ( mut );
	if ( const OutPeerInfo * o = data -> getOutPeerInfo ( peer ) )
		if ( o -> rtpIps.size ( ) && ! o -> rtpIps.count ( ip ) )
			return false;
	return true;
}

void Conf :: Impl :: lockRas ( ) {
	PSYSTEMLOG ( Info, "lockRas" );
	rasMutex.Wait ( );
}

void Conf :: Impl :: unLockRas ( ) {
	PSYSTEMLOG ( Info, "unlockRas" );
	rasMutex.Signal ( );
}

void Conf :: Impl :: rejectRasLocks ( ) {
	PSYSTEMLOG ( Info, "amraslock" );
	for ( GrqAnswersMap :: iterator i = grqAnswers.begin ( ); i != grqAnswers.end ( ); ++ i ) {
		i -> second -> confirmed = false;
		i -> second -> c.Signal ( );
	}
	for ( RrqAnswersMap :: iterator i = rrqAnswers.begin ( ); i != rrqAnswers.end ( ); ++ i ) {
		i -> second -> reason = - 1;
		i -> second -> c.Signal ( );
	}
	for ( ArqAnswersMap :: iterator i = arqAnswers.begin ( ); i != arqAnswers.end ( ); ++ i )
		i -> second -> c.Signal ( );
	PSYSTEMLOG ( Info, "amrasunlock" );
}

bool Conf :: Impl :: waitGrq ( int id, ss :: string & gkid, int & reason ) {
	ScopeGuard g1 = makeGuard ( std :: tr1 :: bind ( & Conf :: Impl :: unLockRas, this ) );
	if ( grqAnswers.count ( id ) ) {
		PSYSTEMLOG ( Error, "grq id already in map" );
		gkid.clear ( );
		reason = - 1;
		return false;
	}
	GRQAnswer a ( rasMutex );
	grqAnswers.insert ( std :: make_pair ( id, & a ) );
	ScopeGuard g2 = makeGuard ( std :: tr1 :: bind
		( static_cast < GrqAnswersMap :: size_type ( GrqAnswersMap :: * ) ( const int & ) >
		( & GrqAnswersMap :: erase ), grqAnswers, id ) );
	a.c.Wait ( 3000 );
	gkid.swap ( a.gkid );
	reason = a.reason;
	return a.confirmed;
}

void Conf :: Impl :: rejectGrq ( int id, const ss :: string & gkid, int reason ) {
	PSYSTEMLOG ( Info, "amraslock" );
	AutoMutex am ( rasMutex );
	if ( GRQAnswer * * i = findMapElement ( grqAnswers, id ) ) {
		( * i ) -> confirmed = false;
		( * i ) -> gkid = gkid;
		( * i ) -> reason = reason;
		( * i ) -> c.Signal ( );
	}
	PSYSTEMLOG ( Info, "amrasunlock" );
}

void Conf :: Impl :: confirmGrq ( int id, const ss :: string & gkid ) {
	PSYSTEMLOG ( Info, "amraslock" );
	AutoMutex am ( rasMutex );
	if ( GRQAnswer * * i = findMapElement ( grqAnswers, id ) ) {
		( * i ) -> confirmed = true;
		( * i ) -> gkid = gkid;
		( * i ) -> c.Signal ( );
	}
	PSYSTEMLOG ( Info, "amrasunlock" );
}

void Conf :: Impl :: waitRrq ( int id, ss :: string & eid, int & reason ) {
	ScopeGuard g1 = makeGuard ( std :: tr1 :: bind ( & Conf :: Impl :: unLockRas, this ) );
	if ( rrqAnswers.count ( id ) ) {
		PSYSTEMLOG ( Error, "rrq id already in map" );
		reason = - 1;
		eid.clear ( );
		return;
	}
	RRQAnswer a ( rasMutex );
	rrqAnswers.insert ( std :: make_pair ( id, & a ) );
	ScopeGuard g2 = makeGuard ( std :: tr1 :: bind
		( static_cast < RrqAnswersMap :: size_type ( RrqAnswersMap :: * ) ( const int & ) >
		( & RrqAnswersMap :: erase ), rrqAnswers, id ) );
	a.c.Wait ( 3000 );
	eid.swap ( a.eid );
	reason = a.reason;
}

void Conf :: Impl :: rejectRrq ( int id, int reason ) {
	PSYSTEMLOG ( Info, "amraslock" );
	AutoMutex am ( rasMutex );
	if ( RRQAnswer * * i = findMapElement ( rrqAnswers, id ) ) {
		( * i ) -> reason = reason;
		( * i ) -> c.Signal ( );
	}
	PSYSTEMLOG ( Info, "amrasunlock" );
}

void Conf :: Impl :: confirmRrq ( int id, const ss :: string & eid ) {
	PSYSTEMLOG ( Info, "amraslock" );
	AutoMutex am ( rasMutex );
	if ( RRQAnswer * * i = findMapElement ( rrqAnswers, id ) ) {
		( * i ) -> eid = eid;
		( * i ) -> c.Signal ( );
	}
	PSYSTEMLOG ( Info, "amrasunlock" );
}

bool Conf :: Impl :: waitArq ( int id, H225 :: TransportAddress & addr ) {
	ScopeGuard g1 = makeGuard ( std :: tr1 :: bind ( & Conf :: Impl :: unLockRas, this ) );
	if ( arqAnswers.count ( id ) ) {
		PSYSTEMLOG ( Error, "arq id already in map" );
		return false;
	}
	ARQAnswer a ( rasMutex );
	arqAnswers.insert ( std :: make_pair ( id, & a ) );
	ScopeGuard g2 = makeGuard ( std :: tr1 :: bind
		( static_cast < ArqAnswersMap :: size_type ( ArqAnswersMap :: * ) ( const int & ) >
		( & ArqAnswersMap :: erase ), arqAnswers, id ) );
	if ( a.c.Wait ( arqDelay ) && a.inited ) {
		addr = a.destAddr;
		return true;
	}
	return false;
}

void Conf :: Impl :: rejectArq ( int id ) {
	PSYSTEMLOG ( Info, "amraslock" );
	AutoMutex am ( rasMutex );
	if ( ARQAnswer * * i = findMapElement ( arqAnswers, id ) )
		( * i ) -> c.Signal ( );
	PSYSTEMLOG ( Info, "amrasunlock" );
}

void Conf :: Impl :: confirmArq ( int id, const H225 :: TransportAddress & addr ) {
	PSYSTEMLOG ( Info, "amraslock" );
	AutoMutex am ( rasMutex );
	if ( ARQAnswer * * i = findMapElement ( arqAnswers, id ) ) {
		( * i ) -> destAddr = addr;
		( * i ) -> inited = true;
		( * i ) -> c.Signal ( );
	}
	PSYSTEMLOG ( Info, "amrasunlock" );
}

static int translateProtoValue ( const OutPeerInfo :: ProtoClassMap & protoClass, const ss :: string & name, int value ) {
	OutPeerInfo :: ProtoClassMap :: const_iterator it = protoClass.find ( name );
	if ( it == protoClass.end ( ) )
		return value;
	const IntIntMap & v = it -> second;
	IntIntMap :: const_iterator i = v.find ( value );
	if ( i == v.end ( ) )
		i = v.find ( - 2 );
	if ( i == v.end ( ) || i -> second == - 2 )
		return value;
	return i -> second;
}

void Conf :: Impl :: translateProtoClass ( int peer, const Q931 & from, Q931 & to ) const {
	AutoMutex am ( mut );
	const OutPeerInfo * o = data -> getOutPeerInfo ( peer );
	if ( ! o )
		return;
	const OutPeerInfo :: ProtoClassMap & protoClass = o -> protoClass;
	Q931 :: InformationTransferCapability capability;
	unsigned transferRate;
	Q931 :: CodingStandard codingStandard;
	Q931 :: UserInfoLayer1 userInfoLayer1 = Q931 :: UserInfoLayer1 ( - 1 );
	if ( from.getBearerCapabilities ( capability, transferRate, & codingStandard, & userInfoLayer1 ) ) {
		capability = Q931 :: InformationTransferCapability ( translateProtoValue ( protoClass,
			"BearerCapability.capability", capability ) );
		transferRate = translateProtoValue ( protoClass, "BearerCapability.transferRate", transferRate );
		codingStandard = Q931 :: CodingStandard ( translateProtoValue ( protoClass,
			"BearerCapability.codingStandard", codingStandard ) );
		userInfoLayer1 = Q931 :: UserInfoLayer1 ( translateProtoValue ( protoClass,
			"BearerCapability.userInfoLayer1", userInfoLayer1 ) );
		to.setBearerCapabilities ( capability, transferRate, codingStandard, userInfoLayer1 );
	}
	unsigned plan, type, presentation, screening;
	if ( to.hasIE ( Q931 :: ieCallingPartyNumber ) && ! from.getCallingPartyNumber ( & plan, & type, & presentation,
		& screening, unsigned ( - 1 ), unsigned ( - 1 ) ).empty ( ) ) {
		plan = translateProtoValue ( protoClass, "CallingPartyNumber.plan", plan );
		type = translateProtoValue ( protoClass, "CallingPartyNumber.type", type );
		presentation = translateProtoValue ( protoClass, "CallingPartyNumber.presentation", presentation );
		screening = translateProtoValue ( protoClass, "CallingPartyNumber.screening", screening );
		to.setCallingPartyNumber ( to.getCallingPartyNumber ( ), plan, type, presentation, screening );
	}
	if ( ! from.getCalledPartyNumber ( & plan, & type ).empty ( ) ) {
		plan = translateProtoValue ( protoClass, "CalledPartyNumber.plan", plan );
		type = translateProtoValue ( protoClass, "CalledPartyNumber.type", type );
		to.setCalledPartyNumber ( to.getCalledPartyNumber ( ), plan, type );
	}
}

ss :: string Conf :: Impl :: getRadiusIp ( ) {
	AutoMutex am ( mut );
	return radiusIp;
}

int Conf :: Impl :: getRadiusPort ( ) const {
	return radiusPort;
}

ss :: string Conf :: Impl :: getRadiusSecret ( ) {
	AutoMutex am ( mut );
	return radiusSecret;
}

void Conf :: Impl :: getSecret ( const PIPSocket :: Address & ip, RadGWInfo & ri ) {
	ss :: string s = static_cast < const char * > ( ip.AsString ( ) );
	AutoMutex am ( mut );
	data -> getSecret ( s, ri );
}

int Conf :: Impl :: checkPin ( RequestInfo & ri ) {
	AfterTaskVector q;
	return checkPinInt ( ri, q );
}

void Conf :: Impl :: refreshCard ( const CardInfo * c ) {
	ss :: string acctn = c -> acctn;
	data -> uncacheCard ( c );
	CardCallsMap :: iterator i = cardCalls.find ( acctn );
	if ( i == cardCalls.end ( ) )
		return;
	if ( ! data -> cardByAcctn ( acctn ) ) { // tut lishniy raz vipolnitsya cardCalls.find ( acctn )
		i -> second -> setTotalMoney ( 0 );
		i -> second -> setCredit ( 0 );
	}
}

void Conf :: Impl :: refreshCardByAcctn ( const ss :: string & acctn ) {
	if ( const CardInfo * c = data -> cachedCardByAcctn ( acctn ) )
		data -> uncacheCard ( c );
	CardCallsMap :: iterator i = cardCalls.find ( acctn );
	if ( i == cardCalls.end ( ) )
		return;
	if ( ! data -> cardByAcctn ( acctn ) ) { // tut lishniy raz vipolnitsya cardCalls.find ( acctn )
		i -> second -> setTotalMoney ( 0 );
		i -> second -> setCredit ( 0 );
	}
}

int Conf :: Impl :: applyCodeSql ( const RequestInfo & ri ) {
	class TableLocker {
		MySQL & m;
	public:
		TableLocker ( MySQL & mm ) : m ( mm ) {
			if ( m.query ( "lock tables radius.Cards write, radius.GrossBook write, "
				"radius.RadConf read, radius.PrepaidIds read, radius.Users read" ) )
				throw MySQLError ( m );
		}
		~TableLocker ( ) {
			m.unLockTables ( );
		}
	};
	AutoMutex cam ( confMut );
	TableLocker tl ( m );
	int applyAccountId = -1;
	dbc.get ( "applyAccountId", applyAccountId );
	if ( applyAccountId == -1 ) {
		PSYSTEMLOG ( Error, "no applyAccountId" );
		return -2;
	}
	int applyServiceId = -1;
	dbc.get ( "applyServiceId", applyServiceId );
	if ( applyServiceId == -1 ) {
		PSYSTEMLOG ( Error, "no applyServiceId" );
		return -2;
	}
	MyResult rb ( m.getQuery ( "select acctn, cost, radius.Users.id, radius.Users.uname from "
		"radius.Cards left join radius.PrepaidIds on radius.Cards.uid = radius.PrepaidIds.id left join "
		"radius.Users using ( uname ) where pin = '%s' and expDate > now() and status != 'closed' and "
		"radius.Users.id is not null", md5pin ( ri.applyCode ).c_str ( ) ) );
	const char * * pb = rb.fetchRow ( );
	if ( ! pb || pb [ 0 ] == ri.acctn ) {
		PSYSTEMLOG ( Error, "not a valid card for applyCode" );
		return -2;
	}
	MyResult ra ( m.getQuery ( "select radius.Users.id, radius.Users.uname from radius.Cards left join "
		"radius.PrepaidIds on radius.Cards.uid = radius.PrepaidIds.id left join radius.Users "
		"using ( uname ) where acctn = '%s' and radius.Users.id is not null", ri.acctn.c_str ( ) ) );
	const char * * pa = ra.fetchRow ( );
	if ( ! pa ) {
		PSYSTEMLOG ( Error, "card a disappeared" );
		return -2;
	}
	if ( m.doQuery ( "update radius.Cards set cost = cost + %s where acctn = %s", pb [ 1 ],
		ri.acctn.c_str ( ) ) )
		throw MySQLError ( m );
	if ( m.doQuery ( "update radius.Cards set cost = 0, status = 'closed' where acctn = %s", pb [ 0 ] ) )
		throw MySQLError ( m );
	if ( m.doQuery ( "insert into GrossBook set oDate = now(), sDate = now(), direction = 'In', what = 'Pay', "
		"fromAccount = 0, toAccount = %i, service = %i, correspondent = %s, defSum = %s, account = %s, "
		"fromSum = %s, toSum = %s", applyAccountId, applyServiceId, pa [ 0 ], pb [ 1 ], ri.acctn.c_str ( ),
		pb [ 1 ], pb [ 1 ] ) )
		throw MySQLError ( m );
	if ( m.doQuery ( "insert into GrossBook set oDate = now(), sDate = now(), direction = 'Out', what = 'Pay', "
		"fromAccount = %i, toAccount = 0, service = %i, correspondent = %s, defSum = %s, account = %s, "
		"fromSum = %s, toSum = %s", applyAccountId, applyServiceId, pb [ 2 ], pb [ 1 ], pb [ 0 ],
		pb [ 1 ], pb [ 1 ] ) )
		throw MySQLError ( m );
	return 0;
}

int Conf :: Impl :: applyCode ( RequestInfo & ri ) {
	if ( int r = checkPin ( ri ) )
		return r;
	if ( int r = applyCodeSql ( ri ) )	// luchshe vse sdelat v pamyati a potom bazu popravit
		return r;			// race ne budet t.k. cards zagrugayutsya pod etim ge mutexom
						// pravda budet race esli kto-to snimet dengi s carti mimo ss
						// no tak race budet dage pri obichnoy rabote
						// a dlya grossbook mutex dolgen bit sovobogden
						// no nugen lock tables. v ideale v otdelnom thread
						// t.k. lock tables prospit ves reload.
						// eshe odin sqloutthread, prinimayushiy neskolko query srazu ?
						// ili rasshirit api confthread
	{
		AutoMutex am ( mut );
		if ( const CardInfo * c = data -> cardByPin ( md5pin ( ri.applyCode ) ) )
			refreshCard ( c );
		refreshCardByAcctn ( ri.acctn );
	}
	return checkPin ( ri );
}

void Conf :: Impl :: addCbEaters ( RadiusCall & rc, const CardInfo * card ) {
	if ( card -> responsibleBalance && ! rc.cbDealerEater ) {
		MoneySpool * spool = getCustomerMoneySpool ( card -> parent -> parent -> parent -> uname );
		rc.cbDealerEater = new RadiusCardMoneyEater ( * spool, rc.cbprice, rc.h323ConfId );
	}
	if ( card -> customerBalance && ! rc.cbEater ) {
		MoneySpool * spool = getCustomerMoneySpool ( card -> parent -> uname );
		rc.cbEater = new RadiusCardMoneyEater ( * spool, rc.cbprice, rc.h323ConfId );
	}
	if ( card -> cardBalance && ! rc.cbCardEater ) {
		MoneySpool * spool = getCardMoneySpool ( card );
		rc.cbDealerEater = new RadiusCardMoneyEater ( * spool, rc.cbprice, rc.h323ConfId );
	}
}

static bool hasPrefix ( const StringSet & p, const ss :: string & s ) {
	for ( StringSet :: const_iterator i = p.begin ( ); i != p.end ( ); ++ i ) {
		const ss :: string & t = * i;
		if ( t == s.substr ( 0, t.size ( ) ) )
			return true;
	}
	return false;
}

int Conf :: Impl :: checkPinInt ( RequestInfo & ri, AfterTaskVector & que ) {
	AutoMutex am ( mut );
	const ConfData :: DNISInfo * dnis = data -> getDNISInfo ( ri.dnis );
	if ( ! dnis ) {
		PSYSTEMLOG ( Error, "unknown dnis " << ri.dnis );
		return - 2;
	}
	if ( hasPrefix ( dnis -> disabledAniPrefixes, ri.ani ) ) {
		PSYSTEMLOG ( Error, "ani " << ri.ani << " disabled by prefix" );
		return - 2;
	}
	ri.inet = dnis -> inet;
	ri.md5 = md5pin ( ri.pass );
	CardInfo * card = 0;
	switch ( dnis -> access ) {
		case 'p':
			card = data -> cardByAni ( ri.ani );
			if ( card && ri.callback2 && card -> requireCallbackAuth )
				card = 0;
			if ( ! card )
				card = data -> cardByPin ( ri.md5 );
			break;
		case 'a':
			card = data -> cardByAni ( ri.ani );
			if ( card && ri.callback2 && card -> requireCallbackAuth )
				card = 0;
			if ( ! card )
				card = data -> cardByAcctn ( ri.acctn );
			break;
		case 'b':
			card = data -> cardByAcctn ( ri.acctn );
			if ( ! card || ss :: string ( card -> md5, 32 ) != ri.md5 ) {
				card = data -> cardByAni ( ri.ani );
				if ( card && ri.callback2 && card -> requireCallbackAuth )
					card = 0;
			}
			break;
	}
	if ( ! card ) {
		PSYSTEMLOG ( Error, "unknown card ( " << dnis -> access << ", " << ri.ani << ", " <<
			ri.acctn << ", " << ri.md5 << " )" );
		return - 2;
	}
	if ( ri.registerAni && ! data -> cardByAni ( ri.ani ) && ! data -> registerAni ( que, card, ri.ani, ri.lang ) ) {
		PSYSTEMLOG ( Error, "singleAni: " << ri.acctn );
		return - 3;
	}
	ri.acctn = card -> acctn;
	ri.priceAdd = dnis -> price;
	if ( card -> status == 'c' ) {
		PSYSTEMLOG ( Error, "account is closed: " << ri.acctn );
		return - 7;
	}
	if ( ! card -> parent -> allowedDnis.empty ( ) && ! card -> parent -> allowedDnis.count ( dnis -> id ) ) {
		PSYSTEMLOG ( Error, "DNIS " << ri.dnis << " is not allowed for " << card -> parent -> name );
		return - 8;
	}
	int nasId = data -> getSlaveId ( ri.ip );
	if ( ! card -> parent -> slaves.count ( nasId ) ) {
		PSYSTEMLOG ( Error, "NAS " << ri.ip << " is not allowed for " << card -> parent -> name );
		return - 8;
	}
	if ( ri.inet && ! card -> accRightsInet ) {
		PSYSTEMLOG ( Error, "inet is not allowed for " << card -> acctn );
		return - 8;
	}
	if ( ! ri.inet && ! card -> accRightsVoice ) {
		PSYSTEMLOG ( Error, "voice is not allowed for " << card -> acctn );
		return - 8;
	}
	std :: time_t now = std :: time ( 0 );
	if ( card -> status == 'n' && card -> expTimeFUse && card -> expires > now + card -> expTimeFUse * 86400 ) {
		que.push_back ( new ExpAfterTask ( now, card -> acctn ) );
		card -> expires = now + card -> expTimeFUse * 86400;
		card -> status = 'o';
	}
	if ( card -> expires <= now ) {
		PSYSTEMLOG ( Error, "card " << card -> acctn << " is expired" );
		return - 5;
	}
	data -> getRadiusClass ( card, nasId, dnis, ri.radiusClass );
	ri.options = card -> options;
	ri.lang = card -> lang;
	ri.creditAmount = ri.cardCreditAmount = card -> cost / card -> valuteRate;
	ri.customerCreditAmount = card -> parent -> parent -> balance;
	PSYSTEMLOG ( Info, "set customerCreditAmount to " << ri.customerCreditAmount << " fromm balance" );
	if ( const CustomerMoneySpool * spool = hasCustomerMoneySpool ( card -> parent -> uname ) ) {
		ri.customerCreditAmount = spool -> getMoneyRemaining ( );
		PSYSTEMLOG ( Info, "set customerCreditAmount to " << ri.customerCreditAmount << " fromm spool" );
	}
	ri.responsibleCreditAmount = card -> parent -> parent -> parent -> balance;
	if ( const CustomerMoneySpool * spool = hasCustomerMoneySpool ( card -> parent -> parent -> parent -> uname ) )
		ri.responsibleCreditAmount = spool -> getMoneyRemaining ( );
	ri.valuteRate = card -> valuteRate;
	ri.credit = card -> credit;
	if ( ri.creditAmount <= 0 ) {
		if ( ri.credit )
			ri.creditAmount = 10000000;
		else
			return - 4;
	}
	if ( card -> customerBalance && ! card -> cardBalance )
		ri.creditAmount = ri.customerCreditAmount;
	if ( ! ri.callback )
		return 0;
	if ( ! ( ri.cbNumber.empty ( ) || ri.callback15 ) ) {
		ri.cbRealNumber = ri.cbNumber;
		return 0;
	}
	ss :: string number;
	if ( ! ri.callback2 && ! ri.callback3 && ! ri.callback4 ) {
		CardInfo :: CallBackInfoMap :: const_iterator ai = card -> ani.find ( ri.ani );
		if ( ai == card -> ani.end ( ) || ! ai -> second.enabled ) {
			PSYSTEMLOG ( Error, "no callback for card " << card -> acctn << " and ani " << ri.ani );
			return 0; // ???? -7
		}
		number = ai -> second.number;
	}
	if ( number.empty ( ) )
		number = ri.ani;
	ConfData :: PrepaidInfo :: CallBackPrefixesMap :: const_iterator pi =
		findPrefix ( card -> parent -> callbackPrefixes, number );
	if ( pi == card -> parent -> callbackPrefixes.end ( ) ) {
		PSYSTEMLOG ( Error, "no callback prefix" );
		return - 9;
	}
	if ( ri.cbNumber.empty ( ) )
		ri.cbRealNumber = pi -> second.realPrefix + number.substr ( pi -> first.size ( ) );
	else
		ri.cbRealNumber = ri.cbNumber;
	ri.cbDialNumber = pi -> second.dialPrefix + number.substr ( pi -> first.size ( ) );
	ss :: string cbcode;

	if ( int r = getCallBackPrice ( ri.cbRealNumber, card, ri.cbCode, ri.cbPrice ) )
		return r;

	if ( card -> requireCallbackAuth )
		ri.requireCallbackAuth = true;
	PriceData :: SecondsVector cbmap;
	cbmap.push_back ( std :: make_pair ( & ri.cbPrice, 0 ) );
	ri.creditTime = PriceData :: getSeconds ( cbmap, ri.creditAmount );
	if ( ! ri.callback15 )
		return 0;
	RadiusCallsMap :: iterator ci = m_radiusCallsSpool.find ( ri.h323Conf );
	if ( ci != m_radiusCallsSpool.end ( ) ) {
		if ( ci -> second.ovSavedTime ) {
			ci -> second.lastOV.clear ( );
			ss :: string un;
			PSYSTEMLOG ( Error, "erasing previous call " << ri.h323Conf );
			eraseRadiusLock ( ri.h323Conf, un, 0, que );
		}
		que.push_back ( new TakeRadiusCallTask );
	}
	PSYSTEMLOG ( Info, "adding to radius spool " << ri.h323Conf );
	RadiusCall & rc = m_radiusCallsSpool [ ri.h323Conf ];
	if ( ! ri.hasStart ) {
		PSYSTEMLOG ( Info, "begin of ! hasStart call" );
		rc.startCame = true;
	}
	rc.acctn = ri.acctn;
	rc.h323ConfId = ri.h323Conf;
	rc.cbcode = cbcode;
	rc.creditTime = ri.creditTime;
	rc.ip = ri.ip;
	rc.callback = ri.callback;
	rc.cbRealNumber = ri.cbRealNumber;
	rc.cbprice = ri.cbPrice;
	addCbEaters ( rc, card );
	ss :: ostringstream os;
	os << "replace into RadiusActiveCalls set id = '" << ri.h323Conf << "', acctn = '" << ri.acctn
		<< "', h323ConfId = '" << ri.h323Conf << "', creditTime = " << ri.creditTime;
	sqlOut -> add ( os.str ( ) );
	return 0;
}

int Conf :: Impl :: checkPinIVR ( const ss :: string & acctn, const ss :: string & pass, double & retString,
	ss :: string & lang ) const {
	AutoMutex am ( mut );
	CardInfo * card = data -> cardByAcctn ( acctn );
	if ( ! card ) {
		retString = 0; //"Account is wrong";
		return -1;
	}
	if ( card -> clearPass == pass ) {
		retString = card -> cost;
		lang = card -> lang;
		return 0;
	}
	retString = 0; // "Password is wrong";
	return -2;
}


void Conf :: Impl :: eraseStalledCardLocks ( const ss :: string & h323Conf ) {
	StringStringMap :: iterator i = radiusConfToAcctn.find ( h323Conf );
	if ( i != radiusConfToAcctn.end ( ) ) {
		pair < StringStringMultiMap :: iterator, StringStringMultiMap :: iterator > er =
			radiusAcctnToConf.equal_range ( i -> second );
		for ( StringStringMultiMap :: iterator it = er.first; it != er.second; ) {
			StringStringMultiMap :: iterator t = it;
			++ it;
			if ( t -> second == h323Conf ) {
				radiusAcctnToConf.erase ( t );
				break;
			}

		}
		eraseCardLock ( i -> second );
		radiusConfToAcctn.erase ( i );
	}
}

void Conf :: Impl :: removeCbEaters ( RadiusCall & rc, AfterTaskVector & q, int acctSessionTime ) {
	if ( RadiusCardMoneyEater * e = rc.cbDealerEater ) {
		q.push_back ( e -> detach ( acctSessionTime ) );
		rc.cbDealerEater = 0;
	}
	if ( RadiusCardMoneyEater * e = rc.cbEater ) {
		q.push_back ( e -> detach ( acctSessionTime ) );
		rc.cbEater = 0;
	}
	if ( RadiusCardMoneyEater * e = rc.cbCardEater ) {
		q.push_back ( e -> detach ( acctSessionTime ) );
		rc.cbCardEater = 0;
	}
}

void Conf :: Impl :: eraseStalledCardCall ( const RadiusCallsMap :: iterator & i,
	AfterTaskVector & q ) {
	eraseStalledCardLocks ( i -> first );
	removeCbEaters ( i -> second, q, 0 );
	eraseRadiusSpool ( i, 0, q );
}

void Conf :: Impl :: checkStalledCardCalls ( ) {
	PTime now;
	AfterTaskVector q;
	AutoMutex am ( mut );
	for ( RadiusCallsMap :: iterator i = m_radiusCallsSpool.begin ( );
		i != m_radiusCallsSpool.end ( ); ) {
		const RadiusCall & rc = i -> second;
		if ( ! rc.startCame && ( now - rc.start ).GetSeconds ( ) > noStartRadiusCallThreshold ) {
			PSYSTEMLOG ( Error, "no start for " << i -> first << " in 3 minutes - erasing" );
			RadiusCallsMap :: iterator t = i;
			i ++;
			eraseStalledCardCall ( t, q );
		} else
			i ++;
	}
}

static bool getRealDigits ( const ConfData :: PrepaidInfo * p, const ss :: string & number, ss :: string & realDigits ) {
	StringStringMap :: const_iterator pi = findPrefix ( p -> prefixes, number );
	if ( pi == p -> prefixes.end ( ) ) {
		PSYSTEMLOG ( Error, "no prefix for " << number );
		return false;
	}
	realDigits = pi -> second + number.substr ( pi -> first.size ( ) );
	return true;
}

int Conf :: Impl :: getCardPrice ( bool eu, const ss :: string & realDigits, const CardInfo * card, int priceAdd,
	bool canZero, PriceData & priceData, ss :: string & code, bool & forceCredit ) const {
	TarifRound round;
	unsigned minDigits, maxDigits; //dummy
	int price, connectPrice, tarif;
	const ConfData :: PrepaidInfo * p = card -> parent;
	if ( ! eu ) {
		price = data -> getPrice ( realDigits, p -> price, p -> defPrice, p -> discount,
			connectPrice, round, code, minDigits, maxDigits, canZero );
		tarif = p -> tarif;
	} else {
		price = data -> getPrice ( realDigits, p -> euPrice, p -> euDefPrice, p -> euDiscount,
			connectPrice, round, code, minDigits, maxDigits, canZero );
		tarif = p -> euTarif;
	}
	if ( ! price ) {
		PSYSTEMLOG ( Error, "no price" );
		return - 9;
	}
	if ( price < 0 )
		price = 0;
	price += priceAdd;
	forceCredit = canZero && price == 0;
	priceData = PriceData ( price / 100000.0, round, data -> getTarifInfo ( tarif ), & p -> amortise,
		connectPrice );
	return 0;
}

void Conf :: Impl :: addInEaters ( RadiusCall & rc, const CardInfo * card ) {
	if ( card -> responsibleBalance && ! rc.inDealerEater ) {
		MoneySpool * spool = getCustomerMoneySpool ( card -> parent -> parent -> parent -> uname );
		rc.inDealerEater = new RadiusCardMoneyEater ( * spool, rc.inPrice, rc.h323ConfId );
	}
	if ( card -> customerBalance && ! rc.inEater ) {
		MoneySpool * spool = getCustomerMoneySpool ( card -> parent -> uname );
		rc.inEater = new RadiusCardMoneyEater ( * spool, card -> responsibleBalance ? rc.inEuPrice : rc.inPrice,
			rc.h323ConfId );
	}
	if ( card -> cardBalance && ! rc.inCardEater ) {
		MoneySpool * spool = getCardMoneySpool ( card );
		rc.inCardEater = new RadiusCardMoneyEater ( * spool, rc.inEuPrice, rc.h323ConfId );
	}
}

void Conf :: Impl :: addTrafEaters ( RadiusCall & rc, const CardInfo * card ) {
	int inPrice, inEuPrice, outPrice, outEuPrice, dummyConnectPrice;
	ss :: string code;
	TarifRound round;
	unsigned minDigits, maxDigits;
	const ConfData :: PrepaidInfo * p = card -> parent;
	inPrice = data -> getPrice ( "inetmbi", p -> price, p -> defPrice, p -> discount,
		dummyConnectPrice, round, code, minDigits, maxDigits, false );
	outPrice = data -> getPrice ( "inetmbo", p -> price, p -> defPrice, p -> discount,
		dummyConnectPrice, round, code, minDigits, maxDigits, false );
	inEuPrice = data -> getPrice ( "inetmbi", p -> euPrice, p -> euDefPrice, p -> euDiscount,
		dummyConnectPrice, round, code, minDigits, maxDigits, false );
	outEuPrice = data -> getPrice ( "inetmbo", p -> euPrice, p -> euDefPrice, p -> euDiscount,
		dummyConnectPrice, round, code, minDigits, maxDigits, false );
	if ( card -> responsibleBalance && ! rc.inDealerTrafEater && inPrice > 0 ) {
		MoneySpool * spool = getCustomerMoneySpool ( card -> parent -> parent -> parent -> uname );
		rc.inDealerTrafEater = new TrafficEater ( * spool, inPrice / ( 100000.0 * bytesPerMb ) );
	}
	if ( card -> customerBalance && ! rc.inTrafEater && inPrice > 0 ) {
		MoneySpool * spool = getCustomerMoneySpool ( card -> parent -> uname );
		rc.inTrafEater = new TrafficEater ( * spool, inPrice / ( 100000.0 * bytesPerMb ) );
	}
	if ( card -> cardBalance && ! rc.inCardTrafEater && inEuPrice > 0 ) {
		MoneySpool * spool = getCustomerMoneySpool ( card -> parent -> uname );
		rc.inCardTrafEater = new TrafficEater ( * spool, inEuPrice / ( 100000.0 * bytesPerMb ) );
	}
	if ( card -> responsibleBalance && ! rc.outDealerTrafEater && outPrice > 0 ) {
		MoneySpool * spool = getCustomerMoneySpool ( card -> parent -> parent -> parent -> uname );
		rc.outDealerTrafEater = new TrafficEater ( * spool, outPrice / ( 100000.0 * bytesPerMb ) );
	}
	if ( card -> customerBalance && ! rc.outTrafEater && outPrice > 0 ) {
		MoneySpool * spool = getCustomerMoneySpool ( card -> parent -> uname );
		rc.outTrafEater = new TrafficEater ( * spool, outPrice / ( 100000.0 * bytesPerMb ) );
	}
	if ( card -> cardBalance && ! rc.outCardTrafEater && outEuPrice > 0 ) {
		MoneySpool * spool = getCardMoneySpool ( card );
		rc.outCardTrafEater = new TrafficEater ( * spool, outEuPrice / ( 100000.0 * bytesPerMb ) );
	}
}

int Conf :: Impl :: getCreditTime ( RequestInfo & ri ) {
	int r = checkPin ( ri );
	if ( r )
		return r;
	vector < Pointer < AfterTask > > q;
	AutoMutex am ( mut );
	const CardInfo * card = data -> cardByAcctn ( ri.acctn );
	if ( ! card ) {
		PSYSTEMLOG ( Error, "card disappeared" );
		return - 2;
	}
	const ConfData :: PrepaidInfo * p = card -> parent;
	if ( ri.inet )
	{
	    ri.calledStationId = "internet";
	    if ( card->callbackEnabled )
	    // callback option is handled by RadiusThread::handleAuth to tell cisco that this is dialup-callback
		ri.callback = true;
	}
	ss :: string realDigits;
	if ( ! getRealDigits ( p, ri.calledStationId, realDigits ) )
		return - 9;
	ss :: string code;
	bool forceCredit1, forceCredit2, forceCredit;
	PriceData priceData, euPriceData;
	if ( int r = getCardPrice ( true, realDigits, card, ri.priceAdd, ri.inet, euPriceData, code, forceCredit1 ) )
		return r;
	if ( int r = getCardPrice ( false, realDigits, card, ri.priceAdd, ri.inet, priceData, code, forceCredit2 ) )
		return r;
	forceCredit = forceCredit1 && forceCredit2;
	if ( forceCredit )
		ri.creditTime = ri.creditRealTime = 1000000000;
	else {
		if ( card -> responsibleBalance && ri.responsibleCreditAmount <= 0 ) {
			PSYSTEMLOG ( Error, "no responsible credit" );
			return - 12;
		}
		if ( card -> customerBalance && ri.customerCreditAmount <= 0 ) {
			PSYSTEMLOG ( Error, "no customer credit" );
			return - 12;
		}
		if ( ri.credit )
			ri.creditTime = ri.creditRealTime = 1000000000;
		else {
			PriceData :: SecondsVector pmap;
			pmap.reserve ( 2 );
			pmap.push_back ( std :: make_pair ( ! card -> cardBalance ? & priceData : & euPriceData, 0 ) );
			if ( ri.callback )
				pmap.push_back ( std :: make_pair ( & ri.cbPrice, ri.cbTime ) );
			ri.creditTime = PriceData :: getSeconds ( pmap, ri.creditAmount );
			ri.creditRealTime = PriceData :: getRealSeconds ( pmap, ri.creditAmount );
		}
	}
	if ( ri.creditTime <= 0 ) {
		PSYSTEMLOG ( Error, "no credittime" );
		return - 12;
	}
	if ( ri.callback4 ) {
		if ( RadiusCall * rc = findMapElement ( m_radiusCallsSpool, ri.h323Conf ) ) {
			rc -> creditTime = ri.creditTime;
			return 0;
		} else {
			PSYSTEMLOG ( Error, "callback4 for unknown call " << ri.h323Conf );
			return - 9;
		}
	}
	eraseStalledCardLocks ( ri.h323Conf );
	if ( p -> lines ) {
		if ( int ( cardLocks.count ( ri.acctn ) ) < p -> lines ) {
			PSYSTEMLOG ( Info, "locking card " << ri.acctn );
			cardLocks.insert ( ri.acctn );
			if ( CardInfo * card = data -> cardByAcctn ( ri.acctn ) ) {
				card -> logLocking ( cardLocks.count ( ri.acctn ) );
			}
		} else {
			PSYSTEMLOG ( Error, "card " << ri.acctn << " already locked" );
			return - 3;
		}
	}
	radiusAcctnToConf.insert ( std :: make_pair ( ri.acctn, ri.h323Conf ) );
	radiusConfToAcctn.insert ( std :: make_pair ( ri.h323Conf, ri.acctn ) );
	if ( ! m_radiusCallsSpool.count ( ri.h323Conf ) )
		q.push_back ( new TakeRadiusCallTask );
	RadiusCall & rc = m_radiusCallsSpool [ ri.h323Conf ];
	if ( ! ri.hasStart ) {
		PSYSTEMLOG ( Info, "begin of ! hasStart call" );
		rc.startCame = true;
	}
	rc.acctn = ri.acctn;
	rc.calledStationId = ri.calledStationId;
	rc.realNumber = realDigits;
	rc.h323ConfId = ri.h323Conf;
	rc.price = ! card -> cardBalance ? priceData : euPriceData;
	rc.inPrice = priceData;
	rc.inEuPrice = euPriceData;
	rc.code = code;
	rc.creditTime = ri.creditTime;
	rc.ip = ri.ip;
	rc.callback = ri.callback;
	rc.cbRealNumber = ri.cbRealNumber;
	rc.cbprice = ri.cbPrice;
	rc.cbcode = ri.cbCode;
	if ( ! ri.callback4 ) {
		rc.lastOV = ri.ip + ' ' + ri.sessionId;
		PSYSTEMLOG ( Info, "set lastOV of " << ri.h323Conf << " to " << rc.lastOV );
	}
	PSYSTEMLOG ( Info, "adding to radius spool " << ri.h323Conf );
	addInEaters ( rc, card );
	if ( ri.inet )
		addTrafEaters ( rc, card );
	ss :: ostringstream os;
	os << "replace into RadiusActiveCalls set id = '" << ri.h323Conf << "', acctn = '" << ri.acctn
		<< "', h323ConfId = '" << ri.h323Conf << "', creditTime = " << ri.creditTime <<
		", CalledStationId = '" << ri.calledStationId << "', price = " << rc.price.getPrice ( ) <<
		", code = '" << code << '\'';
	sqlOut -> add ( os.str ( ) );
	return 0;
}

int Conf :: Impl :: getCreditTimeIVR ( RequestInfo & ri ) {
/*
    int r = checkPin ( ri );
	if ( r )
		return r;
*/
	vector < Pointer < AfterTask > > q;
	AutoMutex am ( mut );
	const CardInfo * card = data -> cardByAcctn ( ri.acctn );
	if ( ! card ) {
		PSYSTEMLOG ( Error, "card disappeared" );
		return - 2;
	}
	const ConfData :: PrepaidInfo * p = card -> parent;
	if ( ri.inet )
		ri.calledStationId = "internet";
	ss :: string realDigits;
	if ( ! getRealDigits ( p, ri.calledStationId, realDigits ) )
		return - 9;
/******************/
	ri.options = card -> options;
	ri.lang = card -> lang;
	ri.creditAmount = ri.cardCreditAmount = card -> cost / card -> valuteRate;
	ri.customerCreditAmount = card -> parent -> parent -> balance;
	PSYSTEMLOG ( Info, "1set customerCreditAmount to " << ri.customerCreditAmount << " from balance" );
	if ( const CustomerMoneySpool * spool = hasCustomerMoneySpool ( card -> parent -> uname ) ) {
		ri.customerCreditAmount = spool -> getMoneyRemaining ( );
		PSYSTEMLOG ( Info, "1set customerCreditAmount to " << ri.customerCreditAmount << " from spool" );
	}
	ri.responsibleCreditAmount = card -> parent -> parent -> parent -> balance;
	if ( const CustomerMoneySpool * spool = hasCustomerMoneySpool ( card -> parent -> parent -> parent -> uname ) )
		ri.responsibleCreditAmount = spool -> getMoneyRemaining ( );
	ri.valuteRate = card -> valuteRate;
	ri.credit = card -> credit;
	if ( ri.credit && ri.creditAmount <= 0 )
		ri.creditAmount = 10000000;
	if ( card -> customerBalance && ! card -> cardBalance )
		ri.creditAmount = ri.customerCreditAmount;
/**********************/
	ss :: string code;
	bool forceCredit1, forceCredit2, forceCredit;
	PriceData priceData, euPriceData;
	if ( int r = getCardPrice ( true, realDigits, card, ri.priceAdd, ri.inet, euPriceData, code, forceCredit1 ) )
		return r;
	if ( int r = getCardPrice ( false, realDigits, card, ri.priceAdd, ri.inet, priceData, code, forceCredit2 ) )
		return r;
	forceCredit = forceCredit1 && forceCredit2;
	if ( forceCredit )
		ri.creditTime = ri.creditRealTime = 1000000000;
	else {
		if ( card -> responsibleBalance && ri.responsibleCreditAmount <= 0 ) {
			PSYSTEMLOG ( Error, "no responsible credit" );
			return - 12;
		}
		if ( card -> customerBalance && ri.customerCreditAmount <= 0 ) {
			PSYSTEMLOG ( Error, "no customer credit" );
			return - 12;
		}
		if ( ri.credit )
			ri.creditTime = ri.creditRealTime = 1000000000;
		else {
			PriceData :: SecondsVector pmap;
			pmap.reserve ( 2 );
			pmap.push_back ( std :: make_pair ( ! card -> cardBalance ? & priceData : & euPriceData, 0 ) );
			if ( ri.callback )
				pmap.push_back ( std :: make_pair ( & ri.cbPrice, ri.cbTime ) );
			ri.creditTime = PriceData :: getSeconds ( pmap, ri.creditAmount );
			ri.creditRealTime = PriceData :: getRealSeconds ( pmap, ri.creditAmount );
		}
	}
	if ( ri.creditTime <= 0 ) {
		PSYSTEMLOG ( Error, "no credittime" );
		return - 12;
	}
	if ( ri.callback4 ) {
		if ( RadiusCall * rc = findMapElement ( m_radiusCallsSpool, ri.h323Conf ) ) {
			rc -> creditTime = ri.creditTime;
			return 0;
		} else {
			PSYSTEMLOG ( Error, "callback4 for unknown call " << ri.h323Conf );
			return - 9;
		}
	}
	eraseStalledCardLocks ( ri.h323Conf );
	if ( p -> lines ) {
		if ( int ( cardLocks.count ( ri.acctn ) ) < p -> lines ) {
			PSYSTEMLOG ( Info, "locking card " << ri.acctn );
			cardLocks.insert ( ri.acctn );
			if ( CardInfo * card = data -> cardByAcctn ( ri.acctn ) ) {
				card -> logLocking ( cardLocks.count ( ri.acctn ) );
			}
		} else {
			PSYSTEMLOG ( Error, "card " << ri.acctn << " already locked" );
			return - 3;
		}
	}
	radiusAcctnToConf.insert ( std :: make_pair ( ri.acctn, ri.h323Conf ) );
	radiusConfToAcctn.insert ( std :: make_pair ( ri.h323Conf, ri.acctn ) );
	if ( ! m_radiusCallsSpool.count ( ri.h323Conf ) )
		q.push_back ( new TakeRadiusCallTask );
	RadiusCall & rc = m_radiusCallsSpool [ ri.h323Conf ];
	if ( ! ri.hasStart ) {
		PSYSTEMLOG ( Info, "begin of ! hasStart call" );
		rc.startCame = true;
	}
	rc.acctn = ri.acctn;
	rc.calledStationId = ri.calledStationId;
	rc.realNumber = realDigits;
	rc.h323ConfId = ri.h323Conf;
	rc.price = ! card -> cardBalance ? priceData : euPriceData;
	rc.inPrice = priceData;
	rc.inEuPrice = euPriceData;
	rc.code = code;
	rc.creditTime = ri.creditTime;
	rc.ip = ri.ip;
	rc.callback = ri.callback;
	rc.cbRealNumber = ri.cbRealNumber;
	rc.cbprice = ri.cbPrice;
	rc.cbcode = ri.cbCode;
	if ( ! ri.callback4 ) {
		rc.lastOV = ri.ip + ' ' + ri.sessionId;
		PSYSTEMLOG ( Info, "set lastOV of " << ri.h323Conf << " to " << rc.lastOV );
	}
	PSYSTEMLOG ( Info, "adding to radius spool " << ri.h323Conf );
	addInEaters ( rc, card );
	if ( ri.inet )
		addTrafEaters ( rc, card );
	ss :: ostringstream os;
	os << "replace into RadiusActiveCalls set id = '" << ri.h323Conf << "', acctn = '" << ri.acctn
		<< "', h323ConfId = '" << ri.h323Conf << "', creditTime = " << ri.creditTime <<
		", CalledStationId = '" << ri.calledStationId << "', price = " << rc.price.getPrice ( ) <<
		", code = '" << code << '\'';
	sqlOut -> add ( os.str ( ) );
	return 0;
}

int Conf :: Impl :: getCallBackPrice ( const ss :: string & realDigits, const CardInfo * card, ss :: string & code,
	PriceData & priceData ) const {
	TarifRound round;
	unsigned minDigits, maxDigits; //dummy
	const ConfData :: PrepaidInfo * p = card -> parent;
	int connectPrice;
	int price = data -> getPrice ( realDigits, Pointer < PriceElement > ( new PriceElement ), p -> cbDefPrice,
		p -> cbDiscount, connectPrice, round, code, minDigits, maxDigits );
	int tarif = p -> cbTarif;
	if ( ! price ) {
		PSYSTEMLOG ( Error, "no callback price" );
		return - 9;
	}
	priceData = PriceData ( price / 100000.0, round, data -> getTarifInfo ( tarif ), & p -> amortise,
		connectPrice );
	return 0;
}

int Conf :: Impl :: handleSmsCallback1 ( RequestInfo & ri ) {
	int r = checkPin ( ri );
	if ( r )
		return r;
	AutoMutex am ( mut );
	const CardInfo * card = data -> cardByAcctn ( ri.acctn );
	if ( ! card ) {
		PSYSTEMLOG ( Error, "card disappeared" );
		return - 2;
	}
	const ConfData :: PrepaidInfo * p = card -> parent;
	ss :: string realDigits;
	if ( ! getRealDigits ( p, ri.number1, realDigits ) )
		return - 9;
	ss :: string code;
	PriceData priceData;
	if ( int r = getCallBackPrice ( realDigits, card, code, priceData ) )
		return r;
	if ( ri.credit )
		ri.creditTime = ri.creditRealTime = 1000000000;
	else {
		PriceData :: SecondsVector pmap;
		pmap.push_back ( std :: make_pair ( & priceData, 0 ) );
		ri.creditTime = PriceData :: getSeconds ( pmap, ri.creditAmount );
		ri.creditRealTime = PriceData :: getRealSeconds ( pmap, ri.creditAmount );
	}
	if ( ri.creditTime <= 0 ) {
		PSYSTEMLOG ( Error, "no credittime" );
		return - 12;
	}
	return 0;
}

int Conf :: Impl :: handleSmsCallback2 ( RequestInfo & ri ) {
	int r = checkPin ( ri );
	if ( r )
		return r;
	vector < Pointer < AfterTask > > q;
	AutoMutex am ( mut );
	const CardInfo * card = data -> cardByAcctn ( ri.acctn );
	if ( ! card ) {
		PSYSTEMLOG ( Error, "card disappeared" );
		return - 2;
	}
	const ConfData :: PrepaidInfo * p = card -> parent;
	ss :: string realDigits1, realDigits2;
	if ( ! getRealDigits ( p, ri.number1, realDigits1 ) )
		return - 9;
	if ( ! getRealDigits ( p, ri.number2, realDigits2 ) )
		return - 9;
	ss :: string code1, code2;
	PriceData cbPriceData, inPriceData, inEuPriceData;
	if ( int r = getCallBackPrice ( realDigits1, card, code1, cbPriceData ) )
		return r;
	bool forceCredit1, forceCredit2, forceCredit;
	if ( int r = getCardPrice ( true, realDigits2, card, ri.priceAdd, false, inEuPriceData, code2, forceCredit1 ) )
		return r;
	if ( int r = getCardPrice ( false, realDigits2, card, ri.priceAdd, false, inPriceData, code2, forceCredit2 ) )
		return r;
	forceCredit = forceCredit1 && forceCredit2;
	if ( forceCredit )
		ri.creditTime = ri.creditRealTime = 1000000000;
	else {
		if ( card -> responsibleBalance && ri.responsibleCreditAmount <= 0 ) {
			PSYSTEMLOG ( Error, "no responsible credit" );
			return - 12;
		}
		if ( card -> customerBalance && ri.customerCreditAmount <= 0 ) {
			PSYSTEMLOG ( Error, "no customer credit" );
			return - 12;
		}
		if ( ri.credit )
			ri.creditTime = ri.creditRealTime = 1000000000;
		else {
			PriceData :: SecondsVector pmap;
			pmap.push_back ( std :: make_pair ( & cbPriceData, ri.cbTime ) );
			pmap.push_back ( std :: make_pair ( ! card -> cardBalance ? & inPriceData : & inEuPriceData, 0 ) );
			ri.creditTime = PriceData :: getSeconds ( pmap, ri.creditAmount );
			ri.creditRealTime = PriceData :: getRealSeconds ( pmap, ri.creditAmount );
		}
		if ( ri.creditTime <= 0 ) {
			PSYSTEMLOG ( Error, "no credittime" );
			return - 12;
		}
	}
	if ( ri.smscallback2 ) {
		eraseStalledCardLocks ( ri.h323Conf );
		if ( p -> lines ) {
			if ( int ( cardLocks.count ( ri.acctn ) ) < p -> lines ) {
				PSYSTEMLOG ( Info, "locking card " << ri.acctn );
				cardLocks.insert ( ri.acctn );
				if ( CardInfo * card = data -> cardByAcctn ( ri.acctn ) ) {
					card -> logLocking ( cardLocks.count ( ri.acctn ) );
				}
			} else {
				PSYSTEMLOG ( Error, "card " << ri.acctn << " already locked" );
				return - 3;
			}
		}
		radiusAcctnToConf.insert ( std :: make_pair ( ri.acctn, ri.h323Conf ) );
		radiusConfToAcctn.insert ( std :: make_pair ( ri.h323Conf, ri.acctn ) );
	} else {
		if ( ! m_radiusCallsSpool.count ( ri.h323Conf ) )
			PSYSTEMLOG ( Error, "smscallback3 w/o 2" );
	}
	if ( ! m_radiusCallsSpool.count ( ri.h323Conf ) )
		q.push_back ( new TakeRadiusCallTask );
	RadiusCall & rc = m_radiusCallsSpool [ ri.h323Conf ];
	rc.acctn = ri.acctn;
	rc.calledStationId = ri.calledStationId;
	rc.realNumber = realDigits2;
	rc.h323ConfId = ri.h323Conf;
	rc.price = ! card -> cardBalance ? inPriceData : inEuPriceData;
	rc.inPrice = inPriceData;
	rc.inEuPrice = inEuPriceData;
	rc.code = code2;
	rc.creditTime = ri.creditTime;
	rc.ip = ri.ip;
	rc.callback = ri.callback;
	rc.cbRealNumber = ri.cbRealNumber;
	rc.cbprice = cbPriceData;
	rc.cbcode = code1;
	if ( ri.smscallback2 ) {
		rc.lastOV = ri.ip + ' ' + ri.sessionId;
		PSYSTEMLOG ( Info, "set lastOV of " << ri.h323Conf << " to " << rc.lastOV );
		PSYSTEMLOG ( Info, "adding to radius spool " << ri.h323Conf );
		addCbEaters ( rc, card );
	} else
		addInEaters ( rc, card );
	ss :: ostringstream os;
	os << "replace into RadiusActiveCalls set id = '" << ri.h323Conf << "', acctn = '" << ri.acctn
		<< "', h323ConfId = '" << ri.h323Conf << "', creditTime = " << ri.creditTime <<
		", CalledStationId = '" << ri.calledStationId << "', price = " << rc.price.getPrice ( ) <<
		", code = '" << code2 << '\'';
	sqlOut -> add ( os.str ( ) );
	return 0;
}

int Conf :: Impl :: handleSmsCallback ( RequestInfo & ri ) {
	if ( ri.smscallback1 )
		return handleSmsCallback1 ( ri );
	return handleSmsCallback2 ( ri );
}

void Conf :: Impl :: removeInEaters ( RadiusCall & rc, AfterTaskVector & q, int acctSessionTime ) {
	if ( RadiusCardMoneyEater * e = rc.inDealerEater ) {
		q.push_back ( e -> detach ( acctSessionTime ) );
		rc.inDealerEater = 0;
	}
	if ( RadiusCardMoneyEater * e = rc.inEater ) {
		q.push_back ( e -> detach ( acctSessionTime ) );
		rc.inEater = 0;
	}
	if ( RadiusCardMoneyEater * e = rc.inCardEater ) {
		q.push_back ( e -> detach ( acctSessionTime ) );
		rc.inCardEater = 0;
	}
}

void Conf :: Impl :: removeTrafEaters ( RadiusCall & rc, AfterTaskVector & q ) {
	if ( TrafficEater * e = rc.inDealerTrafEater ) {
		q.push_back ( e -> detach ( ) );
		rc.inDealerTrafEater = 0;
	}
	if ( TrafficEater * e = rc.inTrafEater ) {
		q.push_back ( e -> detach ( ) );
		rc.inTrafEater = 0;
	}
	if ( TrafficEater * e = rc.inCardTrafEater ) {
		q.push_back ( e -> detach ( ) );
		rc.inCardTrafEater = 0;
	}
	if ( TrafficEater * e = rc.outDealerTrafEater ) {
		q.push_back ( e -> detach ( ) );
		rc.outDealerTrafEater = 0;
	}
	if ( TrafficEater * e = rc.outTrafEater ) {
		q.push_back ( e -> detach ( ) );
		rc.outTrafEater = 0;
	}
	if ( TrafficEater * e = rc.outCardTrafEater ) {
		q.push_back ( e -> detach ( ) );
		rc.outCardTrafEater = 0;
	}
}

void Conf :: Impl :: eraseRadiusSpool ( const RadiusCallsMap :: iterator & ci, int acctSessionTime,
	AfterTaskVector & q ) {
	removeInEaters ( ci -> second, q, acctSessionTime );
	removeTrafEaters ( ci -> second, q );
	const RadiusCall & rc = ci -> second;
	if ( rc.cbEater || rc.cbCardEater || rc.cbDealerEater )
		PSYSTEMLOG ( Error, "cbEater exist on delete !!!!!!!!!!!!!!!!!!!!!!! " << rc.acctn );
	ss :: ostringstream os;
	os << "delete from RadiusActiveCalls where id = '" << ci -> first << '\'';
	sqlOut -> add ( os.str ( ) );
	m_radiusCallsSpool.erase ( ci );
	releaseRadiusCall ( );
}

void Conf :: Impl :: eraseRadiusLock ( const ss :: string & confId, ss :: string & userName, int acctSessionTime,
	AfterTaskVector & q ) {
	StringStringMap :: iterator i = radiusConfToAcctn.find ( confId );
	if ( i != radiusConfToAcctn.end ( ) ) {
		userName = i -> second;
		radiusConfToAcctn.erase ( i );
	}
	pair < StringStringMultiMap :: iterator, StringStringMultiMap :: iterator > p =
		radiusAcctnToConf.equal_range ( userName );
	for ( StringStringMultiMap :: iterator i = p.first; i != p.second; i ++ ) {
		if ( i -> second == confId ) {
			radiusAcctnToConf.erase ( i );
			break;
		}
	}
	eraseCardLock ( userName );
	RadiusCallsMap :: iterator ci = m_radiusCallsSpool.find ( confId );
	if ( ci == m_radiusCallsSpool.end ( ) )
		return;
	RadiusCall & rc = ci -> second;

	if ( ! rc.m_isOV )
		acctSessionTime = 0;

	removeCbEaters ( rc, q, acctSessionTime );
	if ( rc.callback ) {
		userName = rc.acctn;
		int len = rc.cbprice.roundSeconds ( acctSessionTime );
		double mlen = len / 60.0;
		ss :: ostringstream os;
		os << "card " << userName << ' ' << setprecision ( 5 ) << setiosflags ( ios :: fixed ) <<
			rc.cbprice.getPrice ( ) << ' ' << len << ' ' << rc.cbprice.getConnectMoney ( acctSessionTime ) <<
			' ' << rc.cbcode << ' ' << mlen << " 1 " << rc.cbRealNumber;
		userName = os.str ( );
	}
	rc.telephonyEnded = true;
	if ( !rc.m_isOV/*rc.lastOV.empty ( )*/ ) {
		PSYSTEMLOG ( Error, "erasing stalled call " << confId );
		eraseRadiusSpool ( ci, acctSessionTime/*rc.ovSavedTime*/, q );
	}
}

Conf :: Impl :: RadiusCallsMap :: iterator Conf :: Impl :: findRadiusCall ( const ss :: string & h323Conf,
	const ss :: string & ipSession ) {
	RadiusCallsMap :: iterator i;
	for ( RadiusCallsMap :: iterator i = m_radiusCallsSpool.begin ( ); i != m_radiusCallsSpool.end ( ); ++ i )
		if ( i -> second.lastOV == ipSession )
			return i;
	return m_radiusCallsSpool.find ( h323Conf );
}

void Conf :: Impl :: clearRadiusSpool ( const ss :: string & ipSession, const ss :: string & h323Conf, int acctSessionTime,
	ss :: string & userName, int acctInputOctets, int acctOutputOctets, AfterTaskVector & q ) {
	RadiusCallsMap :: iterator ci = findRadiusCall ( h323Conf, ipSession );;
	if ( ci == m_radiusCallsSpool.end ( ) ) {
		PSYSTEMLOG ( Error, "stop for unknown call " << ipSession );
		return;
	}
	RadiusCall & rc = ci -> second;
	double trafCardCost = 0, trafCost = 0, trafDealerCost = 0;
	if ( TrafficEater * e = rc.inDealerTrafEater ) {
		e -> setOctets ( acctInputOctets );
		trafDealerCost += e -> getRealMoney ( );
	}
	if ( TrafficEater * e = rc.inTrafEater ) {
		e -> setOctets ( acctInputOctets );
		trafCost += e -> getRealMoney ( );
	}
	if ( TrafficEater * e = rc.inCardTrafEater ) {
		e -> setOctets ( acctInputOctets );
		trafCardCost += e -> getRealMoney ( );
	}
	if ( TrafficEater * e = rc.outDealerTrafEater ) {
		e -> setOctets ( acctOutputOctets );
		trafDealerCost += e -> getRealMoney ( );
	}
	if ( TrafficEater * e = rc.outTrafEater ) {
		e -> setOctets ( acctOutputOctets );
		trafCost += e -> getRealMoney ( );
	}
	if ( TrafficEater * e = rc.outCardTrafEater ) {
		e -> setOctets ( acctOutputOctets );
		trafCardCost += e -> getRealMoney ( );
	}
	userName = rc.acctn;
	int len = rc.price.roundSeconds ( acctSessionTime );
	double mlen = len / 60.0;
	//nado vibrat odin izcostov
	double connectPrice = rc.price.getConnectMoney ( acctSessionTime ) + trafCost + trafCardCost + trafDealerCost;
	double price = rc.price.getPrice ( );
	ss :: ostringstream os;
	os << "card " << userName << ' ' << setprecision ( 5 ) << setiosflags ( ios :: fixed ) <<
		price << ' ' << len << ' ' << connectPrice << ' ' << rc.code;
	if ( rc.code.empty ( ) )
		os << "empty";
	os << ' ' << mlen << " 0 " << rc.realNumber;
	userName = os.str ( );
	rc.lastOV.clear ( );
	if ( ! rc.telephonyEnded ) {
		PSYSTEMLOG ( Info, "not erasing, waiting for telephony" );
		rc.ovSavedTime = acctSessionTime;
		removeInEaters ( ci -> second, q, acctSessionTime );
		removeTrafEaters ( ci -> second, q );
		return;
	}
	PSYSTEMLOG ( Info, "erasing from radius spool " << ipSession << ", " << ci -> second.h323ConfId );
	eraseRadiusSpool ( ci, acctSessionTime, q );
}

void Conf :: Impl :: handleAccounting ( AccountingInfo & ai ) {
	AfterTaskVector q;
	AutoMutex am ( mut );
	using namespace Radius;

    bool isOV = ai.h323CallType == "VoIP" && ai.h323CallOrigin == "originate";
//    PSYSTEMLOG( Info, "handleAccounting. acctStatusType = " << ai.acctStatusType << "; serviceType = " << ai.serviceType
//                << "; ai.h323CallType = " << ai.h323CallType
//                << "; ai.h323CallOrigin = " << ai.h323CallOrigin << "; OV = " << isOV);
    if( isOV )
    {
        ss :: string ipSession = ai.nasIpAddress + " " + ai.acctSessionId;
        RadiusCallsMap :: iterator i = findRadiusCall ( ai.h323ConfId, ipSession );
        if ( i == m_radiusCallsSpool.end ( ) )
            PSYSTEMLOG ( Error, "handleAccounting: unknown call " << ai.h323ConfId );
        else {
            if(ai.h323CallType == "VoIP")
            {
                i -> second.m_isOV = isOV;
                PSYSTEMLOG ( Error, "handleAccounting: ai.h323CallType == 'VoIP', m_isOV = " << isOV );
            }
            else
            {
                PSYSTEMLOG ( Error, "handleAccounting: ai.h323CallType != 'VoIP', m_isOV = " << i -> second.m_isOV <<
                             ", isOV = " << isOV );
            }
        }
    }

	if ( ai.acctStatusType == sAccountingOn || ai.acctStatusType == sAccountingOff ) {
		radiusConfToAcctn.clear ( );
		for ( StringStringMultiMap :: const_iterator i = radiusAcctnToConf.begin ( ); i != radiusAcctnToConf.end ( ); i ++ )
			eraseCardLock ( i -> first );
		radiusAcctnToConf.clear ( );
		return;
	}
	if ( ai.acctStatusType == sStart ) {
		ss :: string ipSession = ai.nasIpAddress + ' ' + ai.acctSessionId;
		RadiusCallsMap :: iterator i = findRadiusCall ( ai.h323ConfId, ipSession );
		if ( i == m_radiusCallsSpool.end ( ) )
			PSYSTEMLOG ( Error, "start for unknown call " << ai.h323ConfId );
		else {
			ai.sessionTimeout = i -> second.creditTime;
			i -> second.startCame = true;
			ss :: ostringstream os;
			os << "update RadiusActiveCalls set startTime=now() where id = '" <<
				i -> first << '\'';
			sqlOut -> add ( os.str ( ) );
			PSYSTEMLOG ( Info, "registering start for " << ai.h323ConfId );
		}
		return;
	}
	if ( ai.acctStatusType != sStop )
		return;
	if ( ai.serviceType == sFramed ) {
		ss :: string un = ai.userName;
		clearRadiusSpool ( ai.h323ConfId, ai.h323ConfId, ai.acctSessionTime, ai.userName, ai.acctInputOctets,
			ai.acctOutputOctets, q );
		eraseRadiusLock ( ai.h323ConfId, un, ai.acctSessionTime, q );
		return;
	}
	bool callback = ai.h323CallOrigin == "callback";
	bool smscallback1 = ai.h323CallOrigin == "smscallback";
	bool smscallback2 = ai.h323CallOrigin == "smscallback2";
	bool answer = callback || smscallback1 || smscallback2 || ai.h323CallOrigin == "answer";
	bool voip = ai.h323CallType == "VoIP";
	if ( smscallback1 ) {
		eraseRadiusLock ( ai.h323ConfId, ai.userName, ai.acctSessionTime, q );
		return;
	}
	if ( smscallback2 ) {
		clearRadiusSpool ( ai.nasIpAddress + ' ' + ai.acctSessionId, ai.h323ConfId, ai.acctSessionTime,
			ai.userName, ai.acctInputOctets, ai.acctOutputOctets, q );
		return;
	}
	if ( answer == voip )
		return;// AV or OT
	if ( answer ) {// AT stop
        PSYSTEMLOG( Info, "handleAccounting. AT stop. userName = " << ai.userName );
        RadiusCallsMap :: iterator ci = m_radiusCallsSpool.find ( ai.h323ConfId );
        if ( ci == m_radiusCallsSpool.end ( ) )
        {
            PSYSTEMLOG(Info, "handleAccounting: " << ai.h323ConfId << "; ci == m_radiusCallsSpool.end");
            return;
        }

		eraseRadiusLock ( ai.h323ConfId, ai.userName, ai.acctSessionTime, q );
		return;
	}
    // OV stop
    PSYSTEMLOG(Info, "handleAccounting. OV stop. userName = " << ai.userName );
    RadiusCallsMap :: iterator ci = m_radiusCallsSpool.find ( ai.h323ConfId );
    if ( ci == m_radiusCallsSpool.end ( ) )
    {
        PSYSTEMLOG(Info, "handleAccounting: 1. " << ai.h323ConfId << "; ci == m_radiusCallsSpool.end");
        return;
    }
    RadiusCall & rc = ci -> second;
    rc.m_isOV = false;

	clearRadiusSpool ( ai.nasIpAddress + ' ' + ai.acctSessionId, ai.h323ConfId, ai.acctSessionTime, ai.userName,
		ai.acctInputOctets, ai.acctOutputOctets, q );
}

void Conf :: Impl :: eraseCardLock ( const ss :: string & acctn ) {
	PSYSTEMLOG ( Info, "cardLocks [ " << acctn << " ] = " << cardLocks.count ( acctn ) );
	StringMultiSet :: iterator it = cardLocks.find ( acctn );
	if ( it != cardLocks.end ( ) ) {
		PSYSTEMLOG ( Info, "unlocking card " << acctn << " succeeded" );
		cardLocks.erase ( it );
		if ( CardInfo * card = data -> cardByAcctn ( acctn ) ) {
			card -> logLocking ( cardLocks.count ( acctn ) );
		}
		return;
	}
	if ( const CardInfo * card = data -> cardByAcctn ( acctn ) )
		if ( ! card -> parent -> lines )
			return;
	PSYSTEMLOG ( Info, "unlocking card " << acctn << " failed" );
}

void Conf :: Impl :: checkCommands ( ) {
	MyResult r ( lm.getQuery ( "select * from Commands order by id" ) );
	while ( const char * * p = r.fetchRow ( ) ) {
		doCommand ( p [ 1 ] );
		lm.doQuery ( "delete from Commands where id = %s", p [ 0 ] );
	}
}

void Conf :: Impl :: doCommand ( const ss :: string & s ) {
	if ( s.substr ( 0, 18 ) == "delete radiuscall " ) {
		AfterTaskVector q;
		AutoMutex am ( mut );
		RadiusCallsMap :: iterator i = m_radiusCallsSpool.find ( s.substr ( 18 ) );
		if ( i != m_radiusCallsSpool.end ( ) )
			eraseStalledCardCall ( i, q );
		return;
	} else if ( s.substr ( 0, 23 ) == "delete radiuscardlocks " ) {
		AfterTaskVector q;
		ss :: string acctn = s.substr ( 23 );
		AutoMutex am ( mut );
		std :: pair < StringStringMultiMap :: iterator, StringStringMultiMap :: iterator > p =
			radiusAcctnToConf.equal_range ( acctn );
		StringVector confIds;
		while ( p.first != p.second ) {
			confIds.push_back ( p.first -> second );
			++ p.first;
		}
		for ( unsigned i = 0; i < confIds.size ( ); i ++ )
			eraseRadiusLock ( confIds [ i ], acctn, 0, q );
	} else if ( s.substr ( 0, 16 ) == "delete h323call " ) {
		int id = std :: atoi ( s.substr ( 16 ).c_str ( ) );
		{
			AutoMutex am ( mut );
			if ( CallControl * * i = findMapElement ( activeCalls, id ) ) {
				( * i ) -> beginShutDown ( );
				return;
			}
		}
		PSYSTEMLOG ( Error, "unknown h323 call " << id );
	} else if ( s.substr ( 0, 8 ) == "smscall " ) {
		ss :: istringstream is ( s );
		ss :: string dummy, ani, num1, num2;
		is >> dummy >> ani >> num1 >> num2;
		ss :: string acctn;
		{
			AutoMutex am ( mut );
			if ( const CardInfo * card = data -> cardByAni ( ani ) )
				acctn = card -> acctn;
		}
		if ( acctn.empty ( ) ) {
			PSYSTEMLOG ( Error, "smscallback: no ani " << ani );
			return;
		}
		if ( ! tryTake ( ) ) {
			PSYSTEMLOG ( Error, "tryTake failed" );
			return;
		}
		if ( ! tryTake ( ) ) {
			PSYSTEMLOG ( Error, "tryTake failed" );
			release ( 0 );
			return;
		}
		if ( ! tryGwCall ( ) ) {
			PSYSTEMLOG ( Error, "tryGwCall failed" );
			release ( 0 );
			release ( 0 );
			return;
		}
		AutoMutex am ( mut );
		static int callId = 0;
		smsCalls.insert ( new CallBackCall ( smsParallelStart, callId, ani, acctn, num1, num2 ) );
		callId += 2;
	}
}

void Conf :: Impl :: remove ( H323Call * c ) {
	AutoMutex am ( mut );
	SmsCallsSet :: iterator i = smsCalls.find ( c );
	if ( i != smsCalls.end ( ) ) {
		delete * i;
		smsCalls.erase ( i );
		releaseGwCall ( );
	}
}

void Conf :: Impl :: addActiveCall ( int id, CallControl * t ) {
	AutoMutex am ( mut );
	activeCalls [ id ] = t;
}

void Conf :: Impl :: removeActiveCall ( int id ) {
	AutoMutex am ( mut );
	if ( ! activeCalls.count ( id ) )
		PSYSTEMLOG ( Error, "removing unknown active call " << id );
	activeCalls.erase ( id );
}

bool Conf :: Impl :: isLocalIp ( const ss :: string & s ) const {
	AutoMutex am ( mut );
	return data -> getLocalIps ( ).count ( s );
}

bool Conf :: Impl :: isGkIp ( const ss :: string & ip ) const {
	return gateKeepers.count ( ip );
}

bool Conf :: Impl :: getAddrFromGk ( const PIPSocket :: Address & gk, const ss :: string & digits,
	PIPSocket :: Address & ip, int & port ) const {
	ss :: string gkip = static_cast < const char * > ( gk.AsString ( ) );
	{
		AutoMutex am ( mut );
		if ( BaseGkClientThread * const * i = findMapElement ( gateKeepers, gkip ) )
			return ( * i ) -> getAddr ( digits, ip, port );
	}
	return :: getAddrFromGK ( gk, digits, ip, port );
}

int Conf :: Impl :: isCardPresentAndRegister( const ss :: string & number )
{
    AutoMutex am ( mut );
    PSYSTEMLOG( Info, "isCardPresentAndRegister: " << number );
    CardInfo * card = data -> cardByNumber( number );
    if(card)
    { // Check to register
        PSYSTEMLOG( Info, "isCardPresentAndRegister: " << number << "; card is present");
        if ( registeredCards.count ( card -> acctn ) ) // card is ONLINE
        {
            PSYSTEMLOG( Info, "isCardPresentAndRegister: " << number << "; true");
            return 1;
        }
        else
        {
	    PSYSTEMLOG(Info, "isCardPresentAndRegister: Card is offline");
		return -1; // Card offline
    	}
    }

    PSYSTEMLOG( Info, "isCardPresentAndRegister: " << number << "; false");
    return 0;
}

void Conf :: Impl :: registerPeer ( const ss :: string & login, const PIPSocket :: Address & addr, int port, bool h323,
	bool fromNat, PTime endLease, bool isCard, const PIPSocket :: Address & sigAddr, int sigPort,
	const StringSet & neededNumbers, StringSet & onlineNumbers, UnregisteredContactsMap * unregisteredContacts,
	//unregistered contacts otdaetsya v h323/sip bez razbora
	PUDPSocket & sock, IcqContactsVector & icqContacts ) {
	AutoMutex am ( mut );
	if ( ! isCard ) {
		if ( const ConfData :: LoginInfo * l = data -> getLoginInfo ( login ) ) {
			if ( l -> inPeerId ) { // registering InPeer
				PSYSTEMLOG ( Info, "adding registeredInPeer " << login << '@' << addr << ':' << port <<
					". Expires in " << endLease - PTime ( ) );
				_registerInPeer ( l -> inPeerId, addr, port, fromNat, endLease, internalUnregisteredContacts,
					l -> prefix, l -> realPrefix );
			}
			if ( l -> outPeerId ) { // registering OutPeer
				PSYSTEMLOG ( Info, "adding registeredOutPeer " << login << '@' << addr << ':' << port <<
					". Expires in " << endLease - PTime ( ) );
				_registerOutPeer ( l -> outPeerId, addr, port, l -> timeout, l -> suffix, endLease, h323 );
			}
		}
	} else {
		if ( CardInfo * card = data -> cardByAcctn ( login ) ) {
			eraseCardRegistration ( login, internalUnregisteredContacts );
			eraseCardRegistration ( registeredInPeers [ addr ] [ port ].getAcctn ( ), internalUnregisteredContacts );
			PSYSTEMLOG ( Info, "adding registeredCard " << login << '@' << addr << ':' << port <<
				". Expires in " << endLease - PTime ( ) );
			registeredInPeers [ addr ] [ port ] = RegisteredInPeer ( "", "", login, fromNat, endLease );
			RegisteredCard rc ( h323, & sock, static_cast < const char * > ( sigAddr.AsString ( ) ),
				short ( sigPort ), static_cast < const char * > ( addr.AsString ( ) ), short ( port ), fromNat );
			for ( StringSet :: const_iterator i = neededNumbers.begin ( );
				i != neededNumbers.end ( ); ++ i ) {
				if ( const CardInfo * card = data -> cardByNumber ( * i ) ) {
					wantedLogins [ card -> acctn ].insert ( login );
					rc.contactList.insert ( card -> acctn );
					if ( registeredCards.count ( card -> acctn ) || // card is ONLINE
						( ! card -> redirects.empty ( ) ) ) 	// ... or has redirects
						onlineNumbers.insert ( * i );
				}
			}
			registeredCards [ login ] = rc;
			const StringSet & w = wantedLogins [ login ];
			for ( StringSet :: const_iterator i = w.begin ( ); i != w.end ( ); ++ i ) {
				StringBoolMap & ucs = internalUnregisteredContacts [ * i ];
				for ( StringIntMap :: const_iterator j = card -> numbers.begin ( );
					j != card -> numbers.end ( ); ++ j )
					ucs [ j -> first ] = true;
			}
			icqContacts = card -> parent -> icqContacts;
			card -> logRegistration ( ss :: string ( addr.AsString ( ) ), port, h323, endLease,
				cardLocks.count ( login ) );
		}
	}
	if ( unregisteredContacts )
		unregisteredContacts -> swap ( internalUnregisteredContacts );
}

void Conf :: Impl :: _registerOutPeer ( int peerId, const PIPSocket :: Address & addr, int port, int timeout,
	const ss :: string & suffix, PTime endLease, bool h323 ) {
	_registeredOutPeers.updatePeer ( peerId, ss :: string ( addr.AsString ( ) ), port, timeout, suffix, endLease, h323 );
}

void Conf :: Impl :: _registerInPeer ( int inPeerId, const PIPSocket :: Address & addr, int port, bool fromNat,
	PTime endLease, UnregisteredContactsMap & unregisteredContacts, const ss :: string & prefix,
	const ss :: string & realPrefix ) {
	eraseCardRegistration ( registeredInPeers [ addr ] [ port ].getAcctn ( ), unregisteredContacts );
	registeredInPeers [ addr ] [ port ] = RegisteredInPeer ( prefix, realPrefix, inPeerId, fromNat, endLease );
}

void Conf :: Impl :: unregisterInPeer ( const PIPSocket :: Address & addr, int port,
	UnregisteredContactsMap & unregisteredContacts ) {
	AutoMutex am ( mut );
	unregisteredContacts.swap ( internalUnregisteredContacts );
	eraseCardRegistration ( registeredInPeers [ addr ] [ port ].getAcctn ( ), unregisteredContacts );
	PSYSTEMLOG ( Info, "unregisterInPeer: removing registeredInPeer " << addr << ':' << port );
	registeredInPeers [ addr ].erase ( port );
	if ( registeredInPeers [ addr ].empty ( ) )
		registeredInPeers.erase ( addr );
}

bool Conf :: Impl :: checkKeepAlive ( const PIPSocket :: Address & addr, int port, PTime endLease,
	UnregisteredContactsMap & unregisteredContacts ) {
	AutoMutex am ( mut );
	unregisteredContacts.swap ( internalUnregisteredContacts );
	if ( RegisteredInPeer * r = findMapElement ( registeredInPeers, addr, port ) ) {
		r -> setEndLease ( endLease );
		return true;
	}
	return false;
}

void Conf :: Impl :: eraseCardRegistration ( ss :: string login, UnregisteredContactsMap & unregisteredContacts ) {
// cant be const ss :: string & login because of eraseCardRegistration ( registeredInPeers [ addr ] [ port ].getAcctn ( ) )
// erases login parameter
	if ( login.empty ( ) )
		return;
	RegCardsMap :: iterator i = registeredCards.find ( login );
	if ( i == registeredCards.end ( ) )
		return;
	const RegisteredCard & rc = i -> second;
	if ( eraseMapElement ( registeredInPeers, PIPSocket :: Address ( rc.rip.c_str ( ) ), int ( rc.rport ) ) )
		PSYSTEMLOG ( Info, "eraseCardRegistration: removing registeredInPeer " << rc.rip << ':' << rc.rport );
	for ( StringSet :: const_iterator k = rc.contactList.begin ( ); k != rc.contactList.end ( ); ++ k )
		eraseMapElement ( wantedLogins, * k, login );
	if ( const CardInfo * card = data -> cardByAcctn ( login ) ) {
		for ( StringSet :: const_iterator k = wantedLogins [ login ].begin ( );
			k != wantedLogins [ login ].end ( ); ++ k ) {
			StringBoolMap & ucs = unregisteredContacts [ * k ];
			for ( StringIntMap :: const_iterator l = card -> numbers.begin ( );
				l != card -> numbers.end ( ); ++ l )
				ucs [ l -> first ] = false;
		}
		card -> logUnregistration ( );
	}
	registeredCards.erase ( i );
}

bool Conf :: Impl :: getPassword ( const PIPSocket :: Address & localIp, const IpPortSet & addresses,
	const ss :: string & login, ss :: string & password, bool & isCard ) const {
	Pointer < AfterTask > q;
	AutoMutex am ( mut );
	if ( const ConfData :: LoginInfo * l = data -> getLoginInfo ( login ) ) {
		if ( ! l -> ip.empty ( ) && ( addresses.size ( ) != 1 || addresses.begin ( ) -> ip != l -> ip ) ) {
			PSYSTEMLOG ( Error, "unauthorized ip for " << login );
			return false;
		}
		password = l -> pass;
		isCard = false;
		return true;
	}
	if ( CardInfo * card = data -> cardByAcctn ( login ) ) {
		PSYSTEMLOG ( Info, "localIp: " << localIp );
		for ( IntSet :: const_iterator i = card -> parent -> interfaces.begin ( );
			i != card -> parent -> interfaces.end ( ); ++ i )
			PSYSTEMLOG ( Info, "allowedInterfaces: " << PIPSocket :: Address ( * i ) );
		if ( ! card -> parent -> interfaces.empty ( ) && ! card -> parent -> interfaces.count ( localIp ) ) {
			PSYSTEMLOG ( Error, "not allowed local interface " << localIp << " for card " << login );
			return false;
		}
		if ( card -> clearPass.empty ( ) ) {
			PSYSTEMLOG ( Error, "clearPass is empty" );
			return false;
		}
		if ( card -> status == 'c' ) {
			PSYSTEMLOG ( Error, "card is closed" );
			return false;
		}
		if ( ! card -> accRightsVoice ) {
			PSYSTEMLOG ( Error, "no accRightsVoice" );
			return false;
		}
		std :: time_t now;
		if ( card -> expires <= ( now = std :: time ( 0 ) ) ) {
			PSYSTEMLOG ( Error, "card is expired" );
			return false;
		}
		if ( card -> status == 'n' && card -> expTimeFUse && card -> expires > now + card -> expTimeFUse * 86400 ) {
			q = new ExpAfterTask ( now, card -> acctn );
			card -> expires = now + card -> expTimeFUse * 86400;
			card -> status = 'o';
		}
		password = card -> clearPass;
		isCard = true;
		return true;
	}
	PSYSTEMLOG ( Error, "getPassword: unknown login " << login );
	return false;
}

void Conf :: Impl :: checkStalledPeerRegistrations ( ) {
	PTime now;
	StringList acctns;
	AutoMutex am ( mut );
	for ( RegisteredInPeersMap :: iterator i = registeredInPeers.begin ( );
		i != registeredInPeers.end ( ); ) {
		RegisteredInPeersMap :: iterator t = i;
		++ i;
		for ( IntRegisteredInPeerMap :: iterator j = t -> second.begin ( );
			j != t -> second.end ( ); ) {
			IntRegisteredInPeerMap :: iterator t2 = j;
			++ j;
			if ( t2 -> second.getEndLease ( ) <= now ) {
				if ( ! t2 -> second.getAcctn ( ).empty ( ) )
					acctns.push_back ( t2 -> second.getAcctn ( ) );
				PSYSTEMLOG ( Info, "checkStalledPeerRegistrations: removing registeredInPeer " << t -> first << ':' << t2 -> first );
				t -> second.erase ( t2 );
			}
		}
		if ( t -> second.empty ( ) )
			registeredInPeers.erase ( t );
	}
	for ( AdmissedCallsMap :: iterator i = admissedCalls.begin ( ); i != admissedCalls.end ( ); ) {
		AdmissedCallsMap :: iterator t = i;
		++ i;
		if ( t -> second.getEndLease ( ) <= now )
			admissedCalls.erase ( t );
	}
	for ( StringList :: const_iterator i = acctns.begin ( ); i != acctns.end ( ); ++ i )
		eraseCardRegistration ( * i, internalUnregisteredContacts );

	_registeredOutPeers.eraseExpired ( now );
}

bool Conf :: Impl :: admissCall ( const PIPSocket :: Address & addr, int port, const ss :: string & confId ) {
	AutoMutex am ( mut );
	if ( const RegisteredInPeer * i = findMapElement ( registeredInPeers, addr, port ) ) {
		RegisteredInPeer p = * i;
		p.setEndLease ( PTime ( ) + 10000 );
		PSYSTEMLOG ( Info, "admissing call " << confId );
		admissedCalls [ confId ] = p;
		return true;
	} else {
		PSYSTEMLOG ( Error, "no ip/port to admiss" );
		return false;
	}
}

void Conf :: Impl :: disengageCall ( const ss :: string & confId ) {
	PSYSTEMLOG ( Info, "disengaging call " << confId );
	AutoMutex am ( mut );
	if ( ! admissedCalls.erase ( confId ) )
		PSYSTEMLOG ( Info, "this call is not admissed (anymore)" );
}

RecodersMap Conf :: Impl :: getRecoders ( ) const {
	AutoMutex am ( mut );
	return data -> getRecoders ( );
}

void Conf :: Impl :: addSetupTime ( long long ms ) {
	AutoMutex am ( mut );
	fulltr.add ( ms );
}

bool Conf :: Impl :: getRegisteredCardRasAddr ( const ss :: string & login, PIPSocket :: Address & addr, int & port,
	PUDPSocket * & sock ) const {
	AutoMutex am ( mut );
	if ( const RegisteredCard * i = findMapElement ( registeredCards, login ) ) {
		addr = PIPSocket :: Address ( i -> rip.c_str ( ) );
		port = i -> rport;
		sock = i -> rasSock;
		return true;
	}
	return false;
}

IntVector Conf :: Impl :: getLocalIntIps ( ) const {
	AutoMutex am ( mut );
	return data -> getLocalIntIps ( );
}

int Conf :: Impl :: getReleaseCompleteCause ( CommonCallDetails & common, bool fromCaller ) {
	AutoMutex am ( mut );
	int cause = common.getDisconnectCause ();
	if ( fromCaller ) {
		if ( common.source ( ).type == SourceData :: card )
		{
			PSYSTEMLOG ( Info, "getReleaseCompleteCause 1");
			return -1;
		}
		const InPeerInfo * ip = data -> getInPeerInfo ( common.source ( ).peer );
		if ( ! ip ) {
			PSYSTEMLOG ( Error, "InPeer " << common.source ( ).peer << " not found" );
			return -1;
		}
		IntIntMap :: const_iterator i = ip -> releaseCompleteReplaceMap.find ( cause );
		if ( i == ip -> releaseCompleteReplaceMap.end ( ) ) {
			ss :: for_each ( ip -> releaseCompleteReplaceMap, SLPrint ( "releaseCompleteReplaceMap: " ) );
			return - 1;
		}
		PSYSTEMLOG ( Info, "Geting for InPeer " << ip -> name << " cause " << i -> second );
		return i -> second;
	} else {
		if ( common.getPeerIndex ( ) == -1 )
			return -1;
		const OutPeerInfo * op = data -> getOutPeerInfo ( common.curChoice ( ) -> getPeer ( ) );
		if ( ! op ) {
			PSYSTEMLOG ( Info, "OutPeer " << common.curChoice ( ) -> getPeer ( ) << "not found" );
			return -1;
		}
		IntIntMap :: const_iterator i = op -> releaseCompleteReplaceMap.find ( cause );
		if ( i == op -> releaseCompleteReplaceMap.end ( ) ) {
			ss :: for_each ( op -> releaseCompleteReplaceMap, SLPrint ( "releaseCompleteReplaceMap: " ) );
			return -1;
		}
		PSYSTEMLOG ( Info, "Geting for OutPeer" << op -> name << " cause " << i -> second );
		return i -> second;
	}

	return -1;
}

void Conf :: Impl :: replaceReleaseCompleteCause ( CommonCallDetails & common, bool fromCaller ) {
	AutoMutex am ( mut );
	int cause = common.getDisconnectCause ( );
	PSYSTEMLOG ( Info, "Conf :: Impl :: replaceReleaseCompleteCause: fromCaller = " << fromCaller << "; cause = " <<
		cause );
	if ( fromCaller ) {
		if ( common.source ( ).type == SourceData :: card )
		{
			PSYSTEMLOG ( Info, "replaceReleaseCompleteCause 1");
			return;
		}
		const InPeerInfo * ip = data -> getInPeerInfo ( common.source ( ).peer );
		if ( ! ip ) {
			PSYSTEMLOG ( Error, "InPeer " << common.source ( ).peer << " not found" );
			return;
		}
		IntIntMap :: const_iterator i = ip -> releaseCompleteReplaceMap.find ( cause );
		if ( i == ip -> releaseCompleteReplaceMap.end ( ) ) {
			ss :: for_each ( ip -> releaseCompleteReplaceMap, SLPrint ( "releaseCompleteReplaceMap: " ) );
			return;
		}
		common.setDisconnectCause ( i -> second );
		PSYSTEMLOG ( Info, "Replacing for InPeer " << ip -> name << " cause " << i -> second );
		return;
	} else {
		const OutPeerInfo * op = data -> getOutPeerInfo ( common.curChoice ( ) -> getPeer ( ) );
		if ( ! op ) {
			PSYSTEMLOG ( Info, "OutPeer " << common.curChoice ( ) -> getPeer ( ) << "not found" );
			return;
		}
		IntIntMap :: const_iterator i = op -> releaseCompleteReplaceMap.find ( cause );
		if ( i == op -> releaseCompleteReplaceMap.end ( ) ) {
			ss :: for_each ( op -> releaseCompleteReplaceMap, SLPrint ( "releaseCompleteReplaceMap: " ) );
			return;
		}
		PSYSTEMLOG ( Info, "Replacing for OutPeer" << op -> name << " cause " << i -> second );
		common.setDisconnectCause ( i -> second );
		return;
	}
}

const LatencyLimits Conf :: Impl :: getMesgLatency ( int outPeerId ) const {
	AutoMutex am ( mut );
	const OutPeerInfo * op = data -> getOutPeerInfo ( outPeerId );
	if ( ! op )
		return LatencyLimits ( );
	return op -> latencyLimits;
}

ss :: string Conf :: Impl :: translateAnumber ( const SourceData & source, int outPeer, const ss :: string & anum ) {
	PSYSTEMLOG ( Info, "translateAnumber: source.peer=" << source.peer <<
		", acctn=" << source.acctn << ", source.type=" << source.type <<
		", outPeer=" << outPeer );
	ss :: string newAnum = anum;
	const OutPeerInfo * op = data -> getOutPeerInfo ( outPeer );
	int grp = op ? op -> outGroupId : 0;
	if ( source.type == SourceData :: inbound ) {
		if ( const InPeerInfo * ip = data -> getInPeerInfo ( source.peer ) )
			ip -> translateAnum ( grp, newAnum );
	}
	if ( source.type == SourceData :: card ) {
		if ( const CardInfo * ci = data -> cardByAcctn ( source.acctn ) )
			ci -> translateAnum ( grp, newAnum );
	}
	if ( op )
		op -> translateAnum ( newAnum );
	return newAnum;
}

void Conf :: Impl :: checkNewContacts ( const ss :: string & login, const StringSet & newContactList,
	UnregisteredContactsMap & unregisteredContacts ) {
 	// if there are new contacts is user's RRQ they will be added to contactList,
 	// wantedLogins and unregisteredContacts for Notifying
	AutoMutex am ( mut );
	unregisteredContacts.swap ( internalUnregisteredContacts );
	RegCardsMap :: iterator i2 = registeredCards.find ( login );
	if ( i2 == registeredCards.end ( ) )
		return;
	RegisteredCard & rc = i2 -> second;

	StringSet newAcctnList;
	for ( StringSet :: const_iterator i = newContactList.begin ( ); i != newContactList.end ( ); ++ i ) {
		const CardInfo * card = data -> cardByNumber ( * i );
		if ( ! card ) // trying to add bad card number
			continue;
		newAcctnList.insert ( card -> acctn );
		if ( rc.contactList.count ( card -> acctn ) ) // card already in contactList
			continue;
		PSYSTEMLOG ( Info, "checkNewContacts: Adding contact " << card -> acctn << " to card " << login );
		wantedLogins [ card -> acctn ].insert ( login );
		rc.contactList.insert ( card -> acctn );
		if ( registeredCards.count ( card -> acctn ) || // need to notify, card is ONLINE
			( ! card -> redirects.empty ( ) ) ) // ... or card has redirects
			unregisteredContacts [ login ] [ * i ] = true;
	}
	// searching for removed contacts
	for ( StringSet :: const_iterator i = rc.contactList.begin ( ); i != rc.contactList.end ( ); ) {
		if ( ! newAcctnList.count ( * i ) ) { // removed
			PSYSTEMLOG ( Info, "checkNewContacts: Removing contact " << * i << " from card " << login );
			const CardInfo * card = data -> cardByNumber ( * i );
			if ( card )
				wantedLogins [ card -> acctn ].erase ( login );
			StringSet :: const_iterator i2 = i++;
			rc.contactList.erase ( i2 );
		} else {
			++i;
		}
	}
}

bool Conf :: Impl :: isRegisteredCardAddr ( const ss :: string & ip, const WORD port, ss :: string & acctn ) {
	acctn.clear ( );
	AutoMutex am ( mut );
    bool retVal = false;
//    PSYSTEMLOG(Info, "~~~Conf :: Impl :: isRegisteredCardAddr: card count = " << registeredCards.size() );
/*
    for ( RegCardsMap :: const_iterator rc = registeredCards.begin ( ); rc != registeredCards.end ( ); ++ rc ) {
		if ( ( rc -> second.ip == ip ) && ( port == rc -> second.port ) ) {
			PSYSTEMLOG ( Info, "(storm): Card from " << ip << ':' << port << " already registered" );
			acctn = rc -> first;
			return true;
		}
	}
	return false;
*/
    for ( RegCardsMap :: const_iterator rc = registeredCards.begin ( ); rc != registeredCards.end ( ); ++ rc ) {
        if ( ( rc -> second.ip == ip ) && ( port == rc -> second.port ) ) {
//            PSYSTEMLOG ( Info, "(storm): Card from " << ip << ':' << port << " already registered" );
            acctn = rc -> first;
            retVal =  true;
        }
        else
        {;
//            PSYSTEMLOG(Info, "~~~Registered card: " << rc -> second.ip << ':' << rc -> second.port  );
        }
    }

    return retVal;
}

const StringStringSetMap & Conf :: Impl :: getRecodes ( ) const {
	return recodes;
}

const StringStringSetMap & Conf :: Impl :: getBackRecodes ( ) const {
	return backRecodes;
}

const StringStringSetMap & Conf :: Impl :: getFullRecodes ( ) const {
	return fullRecodes;
}

bool Conf :: Impl :: getGkInfo ( const ss :: string & ip, ss :: string & login, ss :: string & pswd, int & port ) {
	AutoMutex am ( mut );
	const ConfData :: GateKeepersMap gkMap = data -> getGateKeepers ( );
	ConfData :: GateKeepersMap :: const_iterator i = gkMap.find ( ip );
	if ( i == gkMap.end ( ) )
		return false;
	login = i -> second -> login;
	pswd = i -> second -> pswd;
	port = i -> second -> port;
	return true;
}

void Conf :: Impl :: addRelease ( int outPeer, const ss :: string & digits, int code ) {
	AutoMutex am ( mut );
	const OutPeerInfo * p = data -> getOutPeerInfo ( outPeer );
	if ( ! p || ! p -> callBuffer )
		return;
	p -> callBuffer -> addCall ( digits, code );
}

void Conf :: Impl :: checkCrashedActiveCalls ( ) {
	if ( crashedTimeStamp < 0 )
	{
	    dbc.set ( "timeStamp_" + name, m.getInt ( "select UNIX_TIMESTAMP()" ) );
	    return;
	}
	try {
		MyResult r ( lm.getQuery ( "select id, unix_timestamp( connectTime ), inCallId, outCallId,"
			"priceCache from ActiveCalls where priceCache is not null" ) );
		struct ActiveData {
			const char * id;
			const char * begTime;
			const char * inCallId;
			const char * outCallId;
			const char * priceCache;
		};
		while ( ActiveData * p = reinterpret_cast < ActiveData * > ( r.fetchRow ( ) ) ) {
			ss :: istringstream is ( p -> priceCache );
			CommonCallDetails common;
			common.dismiss ( );
			OutTryVector tries;
			try {
				boost :: archive :: text_iarchive as ( is );
				as >> common >> tries;
			} catch ( boost :: archive :: archive_exception & e ) {
				PSYSTEMLOG ( Error, "crashed active callse exception: " << e.what ( ) );
				continue;
			}
			const SourceData & source = common.getSource ( );
			PTime begTime ( std :: atoi ( p -> begTime ) );
			PTime endTime ( crashedTimeStamp );
			std :: time_t timeInSecs = begTime.GetTimeInSeconds ( );
			int secs = getRounded ( double ( ( endTime - begTime ).GetMilliSeconds ( ) ) / 1000.0 );
			int secs2 = secs;
			CodecInfo codec;
			ss :: ostringstream os;
			os << "insert into radacctSS values ( 0, '" << common.getCallingDigitsIn ( ) << "', '";
			bool hasCurChoice = common.curChoice ( );
			if ( hasCurChoice )
				os << common.getCallingDigits ( );
			os << "', '";
			ss :: string outCode, inCode = source.code, inEuCode = source.euCode;
			if ( inEuCode.empty ( ) )
				inEuCode = '-';
			int tarifedSeconds = getRounded ( getMinutesFromSeconds ( source.tarif, secs ) * 60 ),
				euTarifedSeconds = getRounded ( getMinutesFromSeconds ( source.euTarif, secs ) * 60 );
			int outSecs = 0, inSecs = source.round.round ( tarifedSeconds ),
				inEuSecs = source.euRound.round ( euTarifedSeconds );
			double outPrice = 0, outConnectPrice = 0, inPrice = source.price / 100000.0,
				inConnectPrice = inSecs ? source.connectPrice / 100000.0 : 0,
				inEuPrice = source.euPrice / 100000.0,
				inEuConnectPrice = inEuSecs ? source.euConnectPrice / 100000.0 : 0;
			inConnectPrice += getAmortisedMoney ( source.amortise, tarifedSeconds );
			inEuConnectPrice += getAmortisedMoney ( source.amortise, euTarifedSeconds );
			if ( source.type == SourceData :: card )
				os << "card " << source.acctn << ' ' << inEuPrice * source.valuteRate << ' ' << secs << ' ' <<
					( inEuSecs ? inEuConnectPrice * source.valuteRate : 0.0 ) << ' '
					<< inEuCode << ' ' << common.getRealDigits ( ) << ' ' << inEuSecs / 60.0;
			else
				os << common.getDialedDigits ( );
			os << "', '";
			if ( hasCurChoice )
				os << common.getConvertedDigits ( );
			else
				os << '-';
			int ip;
			if ( source.type == SourceData :: inbound && source.peer > 0 )
				ip = source.peer;
			else
				ip = 0;
			os << ' ' << ip;
			ss :: string outDigits;
			int op = 0;
			if ( hasCurChoice ) {
				outDigits = common.curChoice ( ) -> getRealDigits ( ).substr ( common.curChoice ( ) -> getDepth ( ) );
				op = common.curChoice ( ) -> getPeer ( );
				outCode = common.curChoice ( ) -> getCode ( ).substr ( common.curChoice ( ) -> getDepth ( ) );
				outPrice = common.curChoice ( ) -> getPrice ( ) / 100000.0;
				int outTarifedSeconds = getRounded ( getMinutesFromSeconds ( common.curChoice ( ) -> getTarif ( ),
					secs2 ) * 60 );
				outSecs = common.curChoice ( ) -> getRound ( ).round ( outTarifedSeconds );
				outConnectPrice = outSecs ? common.curChoice ( ) -> getConnectPrice ( ) / 100000.0 : 0;
			}
			if ( inCode.empty ( ) )
				inCode = '-';
			if ( outCode.empty ( ) )
				outCode = '-';
			os << ' ' << op << ' ' << 0 << ' ' << 0 << ' ' << codec <<
				' ' << inCode << ' ' << inPrice << ' ' << inSecs << ' ' << inConnectPrice << ' ' << inEuCode << ' '
				<< inEuPrice << ' ' << inEuSecs << ' ' << inEuConnectPrice << ' ' << outCode << ' ' << outPrice << ' '
				<< outSecs << ' ' << outConnectPrice;
			if ( hasCurChoice && ! source.outAcctn.empty ( ) ) {
				const OutCardDetails & ocd = common.curChoice ( ) -> getOutCardDetails ( );
				int ocdTarifedSecs = getRounded ( getMinutesFromSeconds ( ocd.getEuTarif ( ), secs2 ) * 60 );
				int ocdSecs = ocd.getEuRound ( ).round ( ocdTarifedSecs ) ;
				os << " card " << source.outAcctn << ' ' << ocd.getEuPrice ( ) * source.valuteRate / 100000.0 << ' '
					<< secs2 << ' ' << ( ocdSecs ? ( ocd.getEuConnectPrice ( ) + getAmortisedMoney (
					ocd.getAmortise ( ), ocdTarifedSecs ) ) * source.valuteRate / 100000.0 : 0.0 )
					<< ' ' << ocd.getDigits ( ) << ' ' << ocd.getEuCode ( ) << ' ' << ocdSecs / 60.0;
			}
			os << "', from_unixtime( " << timeInSecs << " ), from_unixtime( " << endTime.GetTimeInSeconds ( ) <<
				" ), " << secs << ", " << secs2 << ", '" << common.getCallerIp ( ) << "', '";
			if ( hasCurChoice )
				os << common.getCalledIp ( );
			os << "', " << common.getDisconnectCause ( );
			for ( int i = 0; i < 4; i ++ )
				os << ", " << 0 << ", " << 0 << ", " << 0;
			os << ", '" << common.getRealDigits ( ) << "', '" << source.inPrefix << "', '" << outDigits << "', '";
			if ( p -> inCallId )
				os << p -> inCallId;
			os << "', '";
			if ( p -> outCallId )
				os << p -> outCallId;
			os << "' )";
			for ( unsigned i = 0; i + 1 < tries.size ( ); i ++ ) {
				int index = tries [ i ].choiceIndex;
				const OutChoiceDetails * curChoice = common.choice ( index );
				os << ", ( 0, '', '";
				if ( curChoice -> getCallingDigits ( ) != "-" )
					os << curChoice -> getCallingDigits ( );
				os << "', '" << curChoice -> getSentDigits ( ) << "', '" << curChoice -> getDigits ( ) << ' ' << ip << ' ';
				op = curChoice -> getPeer ( );
				os << op << " 0 0 - - 0 0 0 - 0 0 0 ";
				ss :: string outCode = curChoice -> getCode ( ).substr ( curChoice -> getDepth ( ) );
				if ( outCode.empty ( ) )
					outCode = '-';
				double outPrice = curChoice -> getPrice ( ) / 100000.0,
					outConnectPrice = 0;//curChoice -> getConnectPrice ( ) / 100000.0;
				os << outCode << ' ' << outPrice << " 0 " << outConnectPrice << "', from_unixtime( " <<
					timeInSecs << " ), from_unixtime( " << timeInSecs << " ), 0, 0, '', '" <<
					curChoice -> getIp ( ) << "', " << tries [ i ].cause <<
					", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '" << common.getRealDigits ( ) << "', '', '" <<
					common.getRealDigits ( ).substr ( curChoice -> getDepth ( ) ) << "', '', '" << tries [ i ].callId << "' )";
			}
			sqlOut -> add ( os.str ( ) );
			os.str ( "" );
			os << "delete from ActiveCalls where id = " << p -> id;
			sqlOut -> add ( os.str ( ) );
			dbc.set ( "timeStamp_" + name, m.getInt ( "select UNIX_TIMESTAMP()" ) );
		}
	} catch ( std :: exception & e ) {
		PSYSTEMLOG ( Error, "crashed active callse exception: " << e.what ( ) );
	}
}

int Conf :: Impl :: getRounded ( double s ) const {
	if ( roundSecondsUp )
		return int ( ceil ( s - 0.0001 ) + 0.0001 );
	else
		return int ( floor ( s + 0.0001 ) + 0.0001 );
}

void Conf :: Impl :: cardLoaded ( const CardInfo * card ) {
	CardCallsMap :: iterator i = cardCalls.find ( card -> acctn );
	if ( i == cardCalls.end ( ) )
		return;
	i -> second -> setTotalMoney ( card -> cost / card -> valuteRate );
	i -> second -> setCredit ( card -> credit );
}

static ss :: string toString ( int a ) {
	ss :: ostringstream os;
	os << a;
	return os.str ( );
}

static std :: bitset < 256 > initAlnumChars ( const char * t ) { // ftom sip2.cxx
	std :: bitset < 256 > s;
	for ( char c = '0'; c <= '9'; c ++ )
		s.set ( c );
	for ( char c = 'A'; c <= 'Z'; c ++ )
		s.set ( c );
	for ( char c = 'a'; c <= 'z'; c ++ )
		s.set ( c );
	while ( * t )
		s.set ( * t ++ );
	return s;
}

static ss :: string xmlEscape ( const ss :: string & s ) {
	static std :: bitset < 256 > callIdChars = initAlnumChars ( "-.!%*_+`'~()<>:\\/[]?{}\"" );
	ss :: ostringstream os;
	for ( ss :: string :: const_iterator i = s.begin ( ); i != s.end ( ); ++ i ) {
		unsigned char c = * i;
		if ( callIdChars.test ( c ) )
			os << c;
		else
			os << "&#" << unsigned ( c ) << ';';
	}
	return os.str ( );
}

void Conf :: Impl :: printActiveCalls ( std :: ostream & os ) const {
	AutoMutex am ( mut );
	PXML xml ( PXML :: Indent | PXML :: NewLineAfterElement );
	PXMLElement * elem = new PXMLElement ( 0, "activecalls" );
	xml.SetRootElement ( elem );
	for ( ActiveCallsMap :: const_iterator i = activeCalls.begin ( ); i != activeCalls.end ( ); ++ i ) {
		PXMLElement * elem2 = new PXMLElement ( elem, "call" );
		elem -> AddChild ( elem2 );
		const CallControl * c = i -> second;
		elem2 -> SetAttribute ( "callId", xmlEscape ( c -> getCallId ( ) ).c_str ( ) );
		PXMLElement * elem3 = new PXMLElement ( elem2, "id", toString ( i -> first ).c_str ( ) );
		elem2 -> AddChild ( elem3 );
		elem3 = new PXMLElement ( elem2, "callRef", toString ( c -> getCallRef ( ) ).c_str ( ) );
		elem2 -> AddChild ( elem3 );
		elem3 = new PXMLElement ( elem2, "inIp", c -> getInIp ( ).c_str ( ) );
		elem2 -> AddChild ( elem3 );
		elem3 = new PXMLElement ( elem2, "inPeerId", toString ( c -> getInPeerId ( ) ).c_str ( ) );
		elem2 -> AddChild ( elem3 );
		elem3 = new PXMLElement ( elem2, "inPeerName", c -> getInPeerName ( ).c_str ( ) );
		elem2 -> AddChild ( elem3 );
		elem3 = new PXMLElement ( elem2, "inResponsibleName", c -> getInResponsibleName ( ).c_str ( ) );
		elem2 -> AddChild ( elem3 );
		elem3 = new PXMLElement ( elem2, "setupTime", c -> getSetupTime ( ).c_str ( ) );
		elem2 -> AddChild ( elem3 );
		elem3 = new PXMLElement ( elem2, "connectTime", c -> getConnectTime ( ).c_str ( ) );
		elem2 -> AddChild ( elem3 );
		elem3 = new PXMLElement ( elem2, "pdd", c -> getPdd ( ).c_str ( ) );
		elem2 -> AddChild ( elem3 );
		elem3 = new PXMLElement ( elem2, "outIp", c -> getOutIp ( ).c_str ( ) );
		elem2 -> AddChild ( elem3 );
		elem3 = new PXMLElement ( elem2, "outPeerId", toString ( c -> getOutPeerId ( ) ).c_str ( ) );
		elem2 -> AddChild ( elem3 );
		elem3 = new PXMLElement ( elem2, "outPeerName", c -> getOutPeerName ( ).c_str ( ) );
		elem2 -> AddChild ( elem3 );
		elem3 = new PXMLElement ( elem2, "outResponsibleName", c -> getOutResponsibleName ( ).c_str ( ) );
		elem2 -> AddChild ( elem3 );
		elem3 = new PXMLElement ( elem2, "callingDigits", c -> getCallingDigits ( ).c_str ( ) );
		elem2 -> AddChild ( elem3 );
		elem3 = new PXMLElement ( elem2, "sentDigits", c -> getSentDigits ( ).c_str ( ) );
		elem2 -> AddChild ( elem3 );
		elem3 = new PXMLElement ( elem2, "dialedDigits", c -> getDialedDigits ( ).c_str ( ) );
		elem2 -> AddChild ( elem3 );
		elem3 = new PXMLElement ( elem2, "inCode", c -> getInCode ( ).c_str ( ) );
		elem2 -> AddChild ( elem3 );
		elem3 = new PXMLElement ( elem2, "outCode", c -> getOutCode ( ).c_str ( ) );
		elem2 -> AddChild ( elem3 );
		elem3 = new PXMLElement ( elem2, "inAcctn", c -> getInAcctn ( ).c_str ( ) );
		elem2 -> AddChild ( elem3 );
		elem3 = new PXMLElement ( elem2, "outAcctn", c -> getInAcctn ( ).c_str ( ) );
		elem2 -> AddChild ( elem3 );
		elem3 = new PXMLElement ( elem2, "differentProtocol", c -> getDifferentProtocol ( ) ? "true" : "false" );
		elem2 -> AddChild ( elem3 );
		elem3 = new PXMLElement ( elem2, "differentCodec", c -> getDifferentCodec ( ) ? "true" : "false" );
		elem2 -> AddChild ( elem3 );
	}
	xml.PrintOn ( os );
}

void Conf :: Impl :: printActiveCallsCSV ( std :: ostream & os ) const {
	AutoMutex am ( mut );
	for ( ActiveCallsMap :: const_iterator i = activeCalls.begin ( ); i != activeCalls.end ( ); ++ i ) {
		const CallControl * c = i -> second;
		os << c -> getCallId ( ) << ';';
		os << i -> first << ';';
		os << c -> getCallRef ( ) << ';';
		os << c -> getInIp ( ) << ';';
		os << c -> getInPeerId ( ) << ';';
		os << c -> getInPeerName ( ) << ';';
		os << c -> getInResponsibleName ( ) << ';';
		os << c -> getSetupTime ( ) << ';';
		os << c -> getConnectTime ( ) << ';';
		os << c -> getOutIp ( ) << ';';
		os << c -> getOutPeerId ( ) << ';';
		os << c -> getOutPeerName ( ) << ';';
		os << c -> getOutResponsibleName ( ) << ';';
		os << c -> getCallingDigits ( ) << ';';
		os << c -> getSentDigits ( ) << ';';
		os << c -> getDialedDigits ( ) << ';';
		os << c -> getInCode ( ) << ';';
		os << c -> getOutCode ( ) << ';';
		os << c -> getInAcctn ( ) << ';';
		os << c -> getOutAcctn ( ) << ';';
		os << c -> getDifferentProtocol ( ) << ';';
		os << c -> getDifferentCodec ( ) << ';';
		os << std :: endl;
	}
}

void Conf :: Impl :: printActiveCallsStats ( std :: ostream & os ) const {
	AutoMutex am ( mut );
	int total = 0, connected = 0, difCodec = 0;
	for ( ActiveCallsMap :: const_iterator i = activeCalls.begin ( ); i != activeCalls.end ( ); ++ i ) {
		total ++;
		const CallControl * c = i -> second;
		connected += c -> getCallConnected ( );
		difCodec += c -> getDifferentCodec ( );
	}
	PXML xml ( PXML :: Indent | PXML :: NewLineAfterElement );
	PXMLElement * elem = new PXMLElement ( 0, "totalactivecalls" );
	xml.SetRootElement ( elem );
	PXMLElement * elem2 = new PXMLElement ( elem, "total", total );
	elem -> AddChild ( elem2 );
	elem2 = new PXMLElement ( elem, "connected", connected );
	elem -> AddChild ( elem2 );
	elem2 = new PXMLElement ( elem, "differentProtocol", int ( gwCallsCount ) );
	elem -> AddChild ( elem2 );
	elem2 = new PXMLElement ( elem, "differentCodec", difCodec );
	elem -> AddChild ( elem2 );
	xml.PrintOn ( os );
}

int Conf :: Impl :: loadIVRData ( const ss :: string & account, const ss :: string & password,
	double & strCost, ss :: string & lang ) const {
	return checkPinIVR ( account, password, strCost, lang );
}

bool Conf :: Impl :: checkAdminsPass ( const ss :: string & uname, const ss :: string & pass ) const {
	AutoMutex am ( mut );
	const ResponsibleInfo * r = data -> getResponsibleInfo ( uname );
	if ( ! r )
		return false;
	return r -> pass == pass;
}

bool Conf :: Impl :: getSupportVia ( ) const {
	AutoMutex am ( mut );
	return data -> getSupportVia ( );
}

bool Conf :: Impl :: getIsNoChoiceCode ( ) const {
	AutoMutex am ( mut );
	return data -> getIsNoChoiceCode();
}

Q931 :: CauseValues Conf :: Impl :: getDefaultDisconnectCause ( ) const {
	if ( getIsNoChoiceCode ( ) )
//roman		return Q931 :: cvNoCircuitChannelAvailable;
		return Q931 :: cvNormalCallClearing;
	else
		return Q931 :: cvNoRouteToDestination;
}

bool Conf :: Impl :: getSIPToH323ErrorResponse ( int err, int * response, ss :: string * textResponse ) const {
	AutoMutex am ( mut );
	return data -> getSIPToH323ErrorResponse ( err, response, textResponse );
}

bool Conf :: Impl :: getH323ToSIPErrorResponse ( int err, int * response, ss :: string * textResponse ) const {
	AutoMutex am ( mut );
	return data -> getH323ToSIPErrorResponse ( err, response, textResponse );
}

ss :: string Conf :: Impl :: getH323Text ( int errorH323 ) const {
	AutoMutex am ( mut );
	return data -> getH323Text ( errorH323 );
}

ss :: string Conf :: Impl :: getSIPText ( int errorSIP ) const {
	AutoMutex am ( mut );
	return data -> getSIPText ( errorSIP );
}

void Conf :: Impl :: addSipCallToRemove ( const ss :: string & confId ) {
	sipCallsToRemove.insert ( confId );
}

void Conf :: Impl :: destroyCustomerSpool ( const ss :: string & uname ) {
	customerCalls.erase ( uname );
}

void Conf :: Impl :: propagateCustomerBalanceChange ( const ss :: string & uname, double money ) {
	if ( CustomerInfo * ci = data -> getCustomerInfo ( uname ) ) {
		ci -> balance = money;
		PSYSTEMLOG ( Info, "balance of " << uname << " set to " << ci -> balance );
	}
}

bool Conf :: Impl :: getMgcpConf ( MgcpGatewayInfoVector & v ) {
	AutoMutex am ( mut );
	return data -> getMgcpConf ( v );
}

void Conf :: Impl :: registerReloadNotifier ( const std :: tr1 :: function < void ( ) > & f ) {
	AutoMutex am ( mut );
	reloadNotifiers.push_back ( f );
	f ( );
}

void Conf :: Impl :: fireReloadNotifiers ( ) {
	for ( ReloadNotifierVector :: const_iterator i = reloadNotifiers.begin ( ); i != reloadNotifiers.end ( ); ++ i )
		( * i ) ( );
}

bool Conf :: Impl :: getRportAgents ( StringVector & v ) const {
	AutoMutex am ( mut );
	return data -> getRportAgents ( v );
}

PSemaphore * Conf :: TerminationSema = new PSemaphore ( 0, 1 ); // Main will wait on this semaphore for termination
PSemaphore * Conf :: TerminatedSema = new PSemaphore ( 0, 1 ); // OnStop then waits for this until finishing....

Conf :: Conf ( ) {
	conf = this;
	try {
		new Impl ( impl );
	} catch ( ... ) {
		conf = 0;
		throw;
	}
}

void Conf :: startGateKeepers ( ) {
	impl -> startGateKeepers ( );
}

Conf :: ~Conf ( ) {
	safeDel ( impl );
}

bool Conf :: shuttingDown ( ) const {
	return impl -> shuttingDown ( );
}

void Conf :: setShuttingDown ( ) {
	impl -> setShuttingDown ( );
}

void Conf :: reloadConf ( ) {
	impl -> reloadConf ( );
}

bool Conf :: getCallInfo ( OutChoiceDetailsVectorVector & forks, StringVector & forkOutAcctns,
	CommonCallDetailsBase & call, bool h323, bool sip , bool mgcp) {
	return impl -> getCallInfo ( forks, forkOutAcctns, call, h323, sip, mgcp );
}

bool Conf :: isValidInPeerAddress ( const ss :: string & ip ) {
	return impl -> isValidInPeerAddress ( ip );
}

bool Conf :: isValidOutPeerAddress ( const ss :: string & ip, int port ) {
	return impl -> isValidOutPeerAddress ( ip, port );
}

void Conf :: take ( ) {
	impl -> take ( );
}

bool Conf :: tryTake ( ) {
	return impl -> tryTake ( );
}

void Conf :: release ( int inPeer ) {
	impl -> release ( inPeer );
}

void Conf :: takeRadiusCall ( ) {
	impl -> takeRadiusCall ( );
}

void Conf :: releaseRadiusCall ( ) {
	impl -> releaseRadiusCall ( );
}

void Conf :: takeHttpCall ( ) {
	impl -> takeHttpCall ( );
}

void Conf :: releaseHttpCall ( ) {
	impl -> releaseHttpCall ( );
}

bool Conf :: take ( CommonCallDetailsBaseOut & call, int & cause, bool * gwTaken ) {
	return impl -> take ( call, cause, gwTaken );
}

void Conf :: release ( const CommonCallDetailsBaseOut & call, bool rc, bool gwTaken ) {
	impl -> release ( call, rc, gwTaken );
}

void Conf :: waitForSignallingChannels ( ) {
	impl -> waitForSignallingChannels ( );
}

int Conf :: getLogLevel ( ) const {
	return impl -> getLogLevel ( );
}

bool Conf :: isDebugInIp ( const ss :: string & ip ) {
	return impl -> isDebugInIp ( ip );
}

const ss :: string & Conf :: getName ( ) const {
	return impl -> getName ( );
}

void Conf :: addCall ( const CommonCallDetailsBaseOut & common, EaterDetails & eaters, CallControl * ctrl ) {
	impl -> addCall ( common, eaters, ctrl );
}

void Conf :: addCall ( CallDetails & call, CallControl * ctrl ) {
	addCall ( call.common, call.eaters, ctrl );
}

void Conf :: addCall ( SipCallDetails * call ) {
	addCall ( call -> common, call -> eaters, call );
}

void Conf :: removeCall ( SourceData & source, EaterDetails & eaters ) {
	impl -> removeCall ( source, eaters );
}

void Conf :: removeCall ( CallDetails & call ) {
	removeCall ( call.common.source ( ), call.eaters );
}

void Conf :: removeCall ( SipCallDetails * call ) {
	if ( call -> removed )
		return;
	call -> removed = true;
	removeCall ( call -> common.source ( ), call -> eaters );
}

void Conf :: balanceSecond ( ) {
	impl -> balanceSecond ( );
}

void Conf :: balanceCards ( ) {
	impl -> balanceCards ( );
}

void Conf :: balanceSipCards ( StringSet & calls ) {
	impl -> balanceSipCards ( calls );
}

double Conf :: getMinutesFromSeconds ( int tarif, int seconds ) const {
	return impl -> getMinutesFromSeconds ( tarif, seconds );
}

const PricePrios & Conf :: getPrios ( ) const {
	return impl -> getPrios ( );
}

bool Conf :: validInRtp ( int peer, const ss :: string & ip ) const {
	return impl -> validInRtp ( peer, ip );
}

bool Conf :: validOutRtp ( int peer, const ss :: string & ip ) const {
	return impl -> validOutRtp ( peer, ip );
}

void Conf :: lockRas ( ) {
	impl -> lockRas ( );
}

void Conf :: unLockRas ( ) {
	impl -> unLockRas ( );
}

bool Conf :: waitGrq ( int id, ss :: string & gkid, int & reason ) {
	return impl -> waitGrq ( id, gkid, reason );
}

void Conf :: rejectGrq ( int id, const ss :: string & gkid, int reason ) {
	impl -> rejectGrq ( id, gkid, reason );
}

void Conf :: confirmGrq ( int id, const ss :: string & gkid ) {
	impl -> confirmGrq ( id, gkid );
}

void Conf :: waitRrq ( int id, ss :: string & eid, int & reason ) {
	impl -> waitRrq ( id, eid, reason );
}

void Conf :: rejectRrq ( int id, int reason ) {
	impl -> rejectRrq ( id, reason );
}

void Conf :: confirmRrq ( int id, const ss :: string & eid ) {
	impl -> confirmRrq ( id, eid );
}

bool Conf :: waitArq ( int id, H225 :: TransportAddress & addr ) {
	return impl -> waitArq ( id, addr );
}

void Conf :: rejectArq ( int id ) {
	impl -> rejectArq ( id );
}

void Conf :: confirmArq ( int id, const H225 :: TransportAddress & addr ) {
	impl -> confirmArq ( id, addr );
}

void Conf :: translateProtoClass ( int peer, const Q931 & from, Q931 & to ) const {
	impl -> translateProtoClass ( peer, from, to );
}

ss :: string Conf :: getRadiusIp ( ) {
	return impl -> getRadiusIp ( );
}

int Conf :: getRadiusPort ( ) const {
	return impl -> getRadiusPort ( );
}

ss :: string Conf :: getRadiusSecret ( ) {
	return impl -> getRadiusSecret ( );
}

void Conf :: getSecret ( const PIPSocket :: Address & ip, RadGWInfo & g ) {
	impl -> getSecret ( ip, g );
}

int Conf :: checkPin ( RequestInfo & ri ) {
	return impl -> checkPin ( ri );
}

int Conf :: getCreditTime ( RequestInfo & ri ) {
	return impl -> getCreditTime ( ri );
}

int Conf :: getCreditTimeIVR ( RequestInfo & ri ) {
	return impl -> getCreditTimeIVR ( ri );
}

int Conf :: handleSmsCallback ( RequestInfo & ri ) {
	return impl -> handleSmsCallback ( ri );
}

void Conf :: handleAccounting ( AccountingInfo & ai ) {
	impl -> handleAccounting ( ai );
}

void Conf :: remove ( H323Call * c ) {
	impl -> remove ( c );
}

void Conf :: addActiveCall ( int id, CallControl * t ) {
	impl -> addActiveCall ( id, t );
}

void Conf :: removeActiveCall ( int id ) {
	impl -> removeActiveCall ( id );
}

bool Conf :: isLocalIp ( const ss :: string & s ) const {
	return impl -> isLocalIp ( s );
}

bool Conf :: getAddrFromGk ( const PIPSocket :: Address & gk, const ss :: string & digits,
	PIPSocket :: Address & ip, int & port ) const {
	return impl -> getAddrFromGk ( gk, digits, ip, port );
}

int Conf :: isCardPresentAndRegister( const ss :: string & number )
{
    return impl -> isCardPresentAndRegister( number );
}

void Conf :: registerPeer ( const ss :: string & login, const PIPSocket :: Address & addr, int port, bool h323,
	bool fromNat, PTime endLease, bool isCard, const PIPSocket :: Address & sigAddr, int sigPort,
	const StringSet & neededNumbers, StringSet & onlineNumbers, UnregisteredContactsMap * unregisteredContacts,
	PUDPSocket & sock, IcqContactsVector & icqContacts ) {
	impl -> registerPeer ( login, addr, port, h323, fromNat, endLease, isCard, sigAddr, sigPort,
		neededNumbers, onlineNumbers, unregisteredContacts, sock, icqContacts );
}

void Conf :: unregisterInPeer ( const PIPSocket :: Address & addr, int port,
	UnregisteredContactsMap & unregisteredContacts ) {
	impl -> unregisterInPeer ( addr, port, unregisteredContacts );
}

bool Conf :: checkKeepAlive ( const PIPSocket :: Address & addr, int port, PTime endLease,
	UnregisteredContactsMap & unregisteredContacts ) {
	return impl -> checkKeepAlive ( addr, port, endLease, unregisteredContacts );
}

bool Conf :: getPassword ( const PIPSocket :: Address & localIp, const IpPortSet & addresses,
	const ss :: string & login, ss :: string & password, bool & isCard ) const {
	return impl -> getPassword ( localIp, addresses, login, password, isCard );
}

bool Conf :: admissCall ( const PIPSocket :: Address & addr, int port, const ss :: string & confId ) {
	return impl -> admissCall ( addr, port, confId );
}

void Conf :: disengageCall ( const ss :: string & confId ) {
	impl -> disengageCall ( confId );
}

RecodersMap Conf :: getRecoders ( ) const {
	return impl -> getRecoders ( );
}

void Conf :: addSetupTime ( long long ms ) {
	impl -> addSetupTime ( ms );
}

bool Conf :: getRegisteredCardRasAddr ( const ss :: string & login, PIPSocket :: Address & addr, int & port,
	PUDPSocket * & sock ) const {
	return impl -> getRegisteredCardRasAddr ( login, addr, port, sock );
}

IntVector Conf :: getLocalIntIps ( ) const {
	return impl -> getLocalIntIps ( );
}

int Conf :: getReleaseCompleteCause ( CommonCallDetails & common, bool fromCaller ) {
	return impl -> getReleaseCompleteCause ( common, fromCaller );
}

void Conf :: replaceReleaseCompleteCause ( CommonCallDetails & common, bool fromCaller ) {
	impl -> replaceReleaseCompleteCause ( common, fromCaller );
}

const LatencyLimits Conf :: getMesgLatency ( int outPeerId ) const {
	return impl -> getMesgLatency ( outPeerId );
}

void Conf :: checkNewContacts ( const ss :: string & login, const StringSet & newContactList,
	UnregisteredContactsMap & unregisteredContacts ) {
	impl -> checkNewContacts ( login, newContactList, unregisteredContacts );
}

bool Conf :: isRegisteredCardAddr ( const ss :: string & ip, const WORD port, ss :: string & acctn ) {
	return impl -> isRegisteredCardAddr ( ip, port, acctn );
}

const StringStringSetMap & Conf :: getRecodes ( ) const {
	return impl -> getRecodes ( );
}

const StringStringSetMap & Conf :: getBackRecodes ( ) const {
	return impl -> getBackRecodes ( );
}

const StringStringSetMap & Conf :: getFullRecodes ( ) const {
	return impl -> getFullRecodes ( );
}

bool Conf :: getGkInfo ( const ss :: string & ip, ss :: string & login, ss :: string & pswd, int & port ) {
	return impl -> getGkInfo ( ip, login, pswd, port );
}

void Conf :: addRelease ( int outPeer, const ss :: string & digits, int code ) {
	impl -> addRelease ( outPeer, digits, code );
}

void Conf :: checkCrashedActiveCalls ( ) {
	impl -> checkCrashedActiveCalls ( );
}

int Conf :: getRounded ( double s ) const {
	return impl -> getRounded ( s );
}

void Conf :: cardLoaded ( const CardInfo * card ) {
	impl -> cardLoaded ( card );
}

void Conf :: printActiveCalls ( std :: ostream & os ) const {
	impl -> printActiveCalls ( os );
}

void Conf :: printActiveCallsCSV ( std :: ostream & os ) const {
	impl -> printActiveCallsCSV ( os );
}

void Conf :: printActiveCallsStats ( std :: ostream & os ) const {
	impl -> printActiveCallsStats ( os );
}

int Conf :: loadIVRData ( const ss :: string & key, const ss :: string & value,
	double & strCost, ss :: string & lang ) const {
	return impl -> loadIVRData ( key, value, strCost, lang );
}

bool Conf :: checkAdminsPass ( const ss :: string & uname, const ss :: string & pass ) const {
	return impl -> checkAdminsPass ( uname, pass );
}

Conf :: AniAfterTask :: AniAfterTask ( const ss :: string & ac, const ss :: string & a, const ss :: string & l ) :
	acctn ( ac ), ani ( a ), lang ( l ) {
	ss :: ostringstream os;
	os << "replace into Ani select '" << ani << "', id, 'Default', '" << ani << "' from Cards where acctn = '"
		<< acctn << '\'';
	if ( Conf :: impl -> data -> cm.query ( os.str ( ) ) )
		PSYSTEMLOG ( Error, "Query error: " << Conf :: impl -> data -> cm.error ( ) << ", query: " <<
			Conf :: impl -> data -> cm.getLastQuery ( ) );
	if ( lang.empty ( ) )
		return;
	os.str ( ss :: string ( ) );
	os << "update Cards set language = '" << lang << "' where acctn = '" << acctn << '\'';
	if ( Conf :: impl -> data -> cm.query ( os.str ( ) ) )
		PSYSTEMLOG ( Error, "Query error: " << Conf :: impl -> data -> cm.error ( ) << ", query: " <<
			Conf :: impl -> data -> cm.getLastQuery ( ) );
}

Conf :: AniAfterTask :: ~AniAfterTask ( ) { }

void Conf :: addSipCallToRemove ( const ss :: string & confId ) {
	impl -> addSipCallToRemove ( confId );
}

void Conf :: destroyCustomerSpool ( const ss :: string & uname ) {
	impl -> destroyCustomerSpool ( uname );
}

void Conf :: propagateCustomerBalanceChange ( const ss :: string & uname, double money ) {
	impl -> propagateCustomerBalanceChange ( uname, money );
}

int Conf :: getSipInviteRegistrationPeriod ( ) const {
	return impl -> getSipInviteRegistrationPeriod ( );
}

int Conf :: getSipDefaultRegistrationPeriod ( ) const {
	return impl -> getSipDefaultRegistrationPeriod ( );
}

bool Conf :: getSupportVia ( ) const {
	return impl -> getSupportVia ( );
}

bool Conf :: getIsNoChoiceCode ( ) const {
	return impl -> getIsNoChoiceCode ( );
}

Q931 :: CauseValues Conf :: getDefaultDisconnectCause ( ) const {
	return impl -> getDefaultDisconnectCause ( );
}

bool Conf :: getSIPToH323ErrorResponse ( int err, int * response, ss :: string * textResponse ) const {
	return impl -> getSIPToH323ErrorResponse ( err, response, textResponse );
}

bool Conf :: getH323ToSIPErrorResponse ( int err, int * response, ss :: string * textResponse ) const {
	return impl -> getH323ToSIPErrorResponse ( err, response, textResponse );
}

ss :: string Conf :: getH323Text ( int errorH323 ) const {
	return impl -> getH323Text ( errorH323 );
}

ss :: string Conf :: getSIPText ( int errorSIP ) const {
	return impl -> getSIPText ( errorSIP );
}

int Conf :: applyCode ( RequestInfo & ri ) {
	return impl -> applyCode ( ri );
}

bool Conf :: getMgcpConf ( MgcpGatewayInfoVector & v ) {
	return impl -> getMgcpConf ( v );
}

void Conf :: registerReloadNotifier ( const std :: tr1 :: function < void ( ) > & f ) {
	impl -> registerReloadNotifier ( f );
}

bool Conf :: getRportAgents ( StringVector & v ) const {
	return impl -> getRportAgents ( v );
}
