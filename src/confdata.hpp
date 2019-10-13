#ifndef __CONFDATA_HPP
#define __CONFDATA_HPP
#pragma interface
#include "prototype.hpp"
#include "ipinpeers.hpp"
struct CardInfo;

struct IpPortInfo {
	int timeout;
	ss :: string suffix;
	ProtoType type;
	IpPortInfo ( int t, const ss :: string & s, ProtoType ty ) : timeout ( t ), suffix ( s ), type ( ty ) { }
};

class AnumTranslatorOut : public Allocatable < __SS_ALLOCATOR > {
public:
	virtual ~AnumTranslatorOut ( ) { };
	virtual void translate ( ss :: string & num ) const = 0;
};

class AnumTranslatorIn {
	typedef std :: map < int, Pointer < AnumTranslatorOut >, std :: less < int >,
		__SS_ALLOCATOR < std :: pair < const int, Pointer < AnumTranslatorOut > > > > OutMap;
	OutMap outMap;
public:
	void translate ( int outgrp, ss :: string & num ) const;
	void add ( int outgrp, Pointer < AnumTranslatorOut > tr );
	const AnumTranslatorOut * getOut ( ) const;
	bool empty ( ) const;
};

struct OutPeerInfo : public Allocatable < __SS_ALLOCATOR > {
	typedef std :: map < IpPort, IpPortInfo, std :: less < IpPort >,
		__SS_ALLOCATOR < std :: pair < const IpPort, IpPortInfo > > > IpsMap;
	IpsMap ips;
	typedef std :: map < ss :: string, IntIntMap, std :: less < ss :: string >,
		__SS_ALLOCATOR < std :: pair < const ss :: string, IntIntMap > > > ProtoClassMap;
	ProtoClassMap protoClass;
	struct RXReplace {
		boost :: regex rx;
		ss :: string replace;
		RXReplace ( const char * x, const char * p ) : rx ( x ), replace ( p ) { }
	};
	typedef std :: vector < RXReplace, __SS_ALLOCATOR < RXReplace > > RXReplaceVector;
	RXReplaceVector rxReplaces;
	CodecInfoSet allowedCodecs;
	StringSet rtpIps;
	IntIntMap releaseCompleteReplaceMap;
	IntVector interfaces;
	ss :: string prefix, realPrefix;
	ss :: string name, rname;
	ss :: string mgcpGw, mgcpEp;
	SigOptionsPeer sigOptions;
	LatencyLimits latencyLimits;
	ProfitGuard profitGuard;
	double valuteRate;
	CallBuffer * callBuffer;
	int limit;
	int depth;
	int prio;
	int defPrice;
	int discount;
	int tarif;
	unsigned minDigits;
	unsigned maxDigits;
	int outGroupId;
	bool remotePrice;
	bool isGk;
	bool directRTP;
	bool allCodecs;
	bool allRoutes;
	void translateAnum ( ss :: string & num ) const;
private:
	friend class ConfData;
	const AnumTranslatorOut * at;
}
;
union RouteInfoData {
	const char * p [ 7 ];
	struct {
		const char * outId;
		const char * prio;
		const char * lim;
		const char * direct;
		const char * interfaces;
		const char * sigOptions;
		const char * forking;
	} n;
};
struct RouteInfo {
	IntVector interfaces;
	SigOptionsPeer sigOptions;
	int prio;
	int lim;
	bool directRTP;
	bool forking;
	bool inDefaultDirectRTP;
	RouteInfo ( MySQL & m, int nas, const OutPeerInfo * o, const RouteInfoData * p );
};
typedef std :: map < int, RouteInfo, std :: less < int >,
	__SS_ALLOCATOR < std :: pair < const int, RouteInfo > > > RoutesMap;

struct ResponsibleInfo : public Allocatable < __SS_ALLOCATOR > {
	bool useBalance;
	double balance;
	int rule;
	int outGroup;
	ss :: string uname;
	ss :: string pass;
};
struct CustomerInfo : public Allocatable < __SS_ALLOCATOR > {
	ResponsibleInfo * parent;
	bool cardBalance, customerBalance;
	double balance;
};
struct InPeerInfo : public Allocatable < __SS_ALLOCATOR > {
	int id;
	ss :: string name, uname;
	int limit;
	bool allRoutes;
	RoutesMap allowedRoutes;
	bool allPrice;
	bool remotePrice;
	Pointer < PriceElement > price, euPrice;
	int defPrice, euDefPrice;
	int discount, euDiscount;
	bool directRTP;
	bool allCodecs;
	CodecInfoSet allowedCodecs;
	int tarif, euTarif;
	StringSet rtpIps;
	double valuteRate;
	unsigned minDigits;
	unsigned maxDigits;
	SigOptionsPeer sigOptions;
	CustomerInfo * parent;
	IntIntMap releaseCompleteReplaceMap;
	ProfitGuard profitGuard;
	bool externalRouting;
	void translateAnum ( int grp, ss :: string & num ) const;
private:
	friend class ConfData;
	const AnumTranslatorIn * at;
};

/////////////////////////////////////////////////////////////////////////////////
	struct ConvertErrorResponseFromH323ToSIP : public Allocatable < __SS_ALLOCATOR >
	{
		int errorH323_;
		ss :: string strTextSIP_;
		int errorSIP_;
	};

	struct ConvertErrorResponseFromSIPToH323 : public Allocatable < __SS_ALLOCATOR >
	{
		int errorSIP_;
		ss :: string strTextH323_;
		int errorH323_;
	};
/////////////////////////////////////////////////////////////////////////////////

typedef std :: multiset < OutChoice, std :: less < OutChoice >, __SS_ALLOCATOR < OutChoice > > OutChoiceSet;
typedef std :: vector < OutChoiceSet, __SS_ALLOCATOR < OutChoiceSet > > OutChoiceSetVector;
class OutCardDetails;
class ConfData {
	class TableLocker {
		MySQL & m;
	public:
		TableLocker ( MySQL & mm, bool useIPAcct );
		~TableLocker ( );
	};
public:
	enum GKType {
		H323GK = 1,
		SipRegistrar = 2,
		SilentSipRegistrar = 3
	};
	struct GateKeeperInfo : public Allocatable < __SS_ALLOCATOR> {
		ss :: string ip;
		int port;
		ss :: string name;
		ss :: string identifier;
		int registerPeriod;
		ss :: string login;
		IntVector bindIps;
		IntVector localIps;
		int gkType;
		ss :: string pswd;
//		bool isCNonce_;
	};
	typedef std :: map < ss :: string, Pointer < GateKeeperInfo >, std :: less < ss :: string >,
		__SS_ALLOCATOR < std :: pair < const ss :: string, Pointer < GateKeeperInfo > > > > GateKeepersMap;
	struct CallBackInfo {
		bool enabled;
		ss :: string number;
	};
	struct CallBackPrefix {
		ss :: string realPrefix;
		ss :: string dialPrefix;
	};
	typedef std :: map < ss :: string, Pointer < CardInfo >, std :: less < ss :: string >,
		__SS_ALLOCATOR < std :: pair < const ss :: string, Pointer < CardInfo > > > > CardMap;

	struct PrepaidInfo : public Allocatable < __SS_ALLOCATOR > {
		int id;
		int lines;
		ss :: string name, uname, rname;
		Pointer < PriceElement > price, euPrice;
		int defPrice;
		int discount;
		int euDefPrice;
		int euDiscount;
		int cbDefPrice;
		int cbDiscount;
		int cbEuDefPrice;
		int cbEuDiscount;
		IntSet slaves;
		int tarif;
		int euTarif;
		int cbTarif;
		int cbEuTarif;
		double valuteRate;
		IntSet allowedDnis;
		IntSet interfaces;
		StringStringMap prefixes;
		IntIntMap amortise;
		typedef std :: map < ss :: string, CallBackPrefix, std :: less < ss :: string >,
			__SS_ALLOCATOR < std :: pair < const ss :: string, CallBackPrefix > > > CallBackPrefixesMap;
		CallBackPrefixesMap callbackPrefixes;
		bool singleAni;
		bool allRoutes;
		RoutesMap allowedRoutes;
		int radiusClass;
		IcqContactsVector icqContacts;
		CustomerInfo * parent;
		char callBack, accRights, requireCallbackAuth;
		bool cardBalance;
		ss :: string options, lang;
	};
	struct DNISInfo : public Allocatable < __SS_ALLOCATOR > {
		int id;
		int price;
		ss :: string name;
		bool inet;
		char access;
		int radiusClass;
		StringSet disabledAniPrefixes;
	};
	struct NASInfo : public Allocatable < __SS_ALLOCATOR> {
		int id;
		int radiusClass;
	};
	enum PeerType {
		InPeer = 1,
		OutPeer = 2,
	};
	struct LoginInfo {
		ss :: string ip;
		ss :: string pass;
		ss :: string prefix;
		ss :: string realPrefix;
		ss :: string suffix;
		int timeout;
		int inPeerId;
		int outPeerId;
		LoginInfo ( const ss :: string & i, const ss :: string & pa, const ss :: string & pr,
			const ss :: string & rp, const ss :: string & suf, int to, int inId, int outId ) :
			ip ( i ), pass ( pa ), prefix ( pr ), realPrefix ( rp ), suffix ( suf ), timeout ( to ),
			inPeerId ( inId ), outPeerId ( outId ) { }
	};
	MySQL cm;
private:
	int nasId;
	typedef std :: map < int, Pointer < InPeerInfo >, std :: less < int >,
		__SS_ALLOCATOR < std :: pair < const int, Pointer < InPeerInfo > > > > InPeersMap;
	InPeersMap inPeers;
	typedef std :: map < ss :: string, LoginInfo, std :: less < ss :: string >,
		__SS_ALLOCATOR < std :: pair < const ss :: string, LoginInfo > > > LoginInfoMap;
	LoginInfoMap loginToPeer;
	typedef std :: map < int, Pointer < PrepaidInfo >, std :: less < int >,
		__SS_ALLOCATOR < std :: pair < const int, Pointer < PrepaidInfo > > > > PrepaidIdsMap;
	PrepaidIdsMap prepaidIds;
	StringIntMultiMap prepaidPrefixes;
	typedef std :: map < int, Pointer < OutPeerInfo >, std :: less < int >,
		__SS_ALLOCATOR < std :: pair < const int, Pointer < OutPeerInfo > > > > OutPeersMap;
	OutPeersMap outPeers;
	StringSet debugInIps;
	typedef std :: map < int, Pointer < PriceElement >, std :: less < int >,
		__SS_ALLOCATOR < std :: pair < const int, Pointer < PriceElement > > > > DefaultPricesMap;
	DefaultPricesMap defaultPrices;
	typedef std :: map < ss :: string, Pointer < ResponsibleInfo >, std :: less < ss :: string >,
		__SS_ALLOCATOR < std :: pair < const ss :: string, Pointer < ResponsibleInfo > > > > ResponsiblesMap;
	ResponsiblesMap responsibles;
	typedef std :: map < ss :: string, Pointer < CustomerInfo >, std :: less < ss :: string >,
		__SS_ALLOCATOR < std :: pair < const ss :: string, Pointer < CustomerInfo > > > > CustomersMap;
	CustomersMap customers;
	Pointer < OutPriceElement > outPrices;
	StringIntMap slaves;
	ss :: string timeToChange;
	typedef std :: map < int, Pointer < TarifInfo >, std :: less < int >,
		__SS_ALLOCATOR < std :: pair < const int, Pointer < TarifInfo > > > > TarifsMap;
	TarifsMap tarifs;
	TarifInfo defaultTarif;
	typedef std :: map < ss :: string, RadGWInfo, std :: less < ss :: string >,
		__SS_ALLOCATOR < std :: pair < const ss :: string, RadGWInfo > > > SecretsMap;
	SecretsMap secrets;
	CardMap acctnToCard, pinToCard, aniToCard, numberToCard;
	typedef std :: map < ss :: string, Pointer < DNISInfo >, std :: less < ss :: string >,
		__SS_ALLOCATOR < std :: pair < const ss :: string, Pointer < DNISInfo > > > > DNISMap;
	DNISMap dnis;
	bool allowLosses;
	typedef std :: map < int, StringStringMap, std :: less < int >,
		__SS_ALLOCATOR < std :: pair < const int, StringStringMap > > > RadiusClassesMap;
	RadiusClassesMap radiusClasses;
	typedef std :: map < int, Pointer < NASInfo >, std :: less < int >,
		__SS_ALLOCATOR < std :: pair < const int, Pointer < NASInfo > > > > GwsMap;
	GwsMap gws;
	StringSet localIps;
	IntVector localIntIps;
	GateKeepersMap gateKeepers;
	RecodersMap recoders;
	StringSet monitoredCards;
	typedef std :: map < int, Pointer < CallBuffer >, std :: less < int >,
		__SS_ALLOCATOR < std :: pair < const int, Pointer < CallBuffer > > > > CallBufferMap;
	CallBufferMap callBuffers;
	IntDoubleMap valuteRates;
	bool getCallInfoIn ( SourceData & source, const StringCustomerMoneySpoolMap & ccalls,
		const ss :: string & realDigits, bool & allPrice, bool & allRoutes,
		const RoutesMap * & allowedRoutes, unsigned & minDigits, unsigned & maxDigits ) const;
	bool getCallInfoPrepaid ( SourceData & source, const StringCustomerMoneySpoolMap & ccalls,
		const ss :: string & dialedDigits, ss :: string & realDigits, int & errn,
		bool & allRoutes, const RoutesMap * & allowedRoutes );
	bool getCallInfoPrepaidChecked ( SourceData & source, const StringCustomerMoneySpoolMap & ccalls,
		const ss :: string & dialedDigits, ss :: string & realDigits, int & errn, bool & allRoutes,
		const RoutesMap * & allowedRoutes );
	bool getCallInfoPrepaidInt ( const CardInfo * card, SourceData & source,
		const StringCustomerMoneySpoolMap & ccalls, ss :: string & realDigits, int & errn, bool & allRoutes,
		const RoutesMap * & allowedRoutes ) const;
	bool getCallInfoPreliminary ( SourceData & source, const StringCustomerMoneySpoolMap & ccalls,
		const ss :: string & dialedDigits, ss :: string & realDigits, int & errn, bool & allPrice, bool & inAllRoutes,
		const RoutesMap * & allowedRoutes, unsigned & minDigits, unsigned & maxDigits, bool & onlyToMaster );
	IpInPeers ipToPeer;
	OutCardDetails getOutCardDetails ( const CardInfo * c, const ss :: string & digits, bool redirect ) const;
	const NASInfo * getNASInfo ( int gw ) const;

	typedef std :: map < int, Pointer < ConvertErrorResponseFromH323ToSIP >, std :: less < int >,
		__SS_ALLOCATOR < std :: pair < const int, Pointer < ConvertErrorResponseFromH323ToSIP > > > > MapConvResponseH323ToSIP;
	typedef std :: map < int, Pointer < ConvertErrorResponseFromSIPToH323 >, std :: less < int >,
		__SS_ALLOCATOR < std :: pair < const int, Pointer < ConvertErrorResponseFromSIPToH323 > > > > MapConvResponseSIPToH323;

	MapConvResponseSIPToH323 mapConvResponseSIPToH323_;
	MapConvResponseH323ToSIP mapConvResponseH323ToSIP_;

	MgcpGatewayInfoVector mgcpGws;
	StringVector rportAgents;
	bool mgcpReloaded;
	bool rportAgentsReloaded;
	bool loadErrorResponses( MySQL & m );
public:
	ConfData ( MySQL & m, int nas, int bytesPerKb );
	bool isValidInPeerAddress ( const ss :: string & addr ) const;
	bool getCallInfo ( SourceData & source, const ss :: string & dialedDigits, CodecInfoVector & incomingCodecs,
		const StringCustomerMoneySpoolMap & ccalls, const RegCardsMap & registeredCards,
		const RegisteredOutPeersMap & registeredOutPeers, OutChoiceSetVector & vv, StringVector & outAcctns,
		ss :: string & realDigits, int & errn, bool h323, bool sip, bool mgcp, bool & inAllCodecs,
		CodecInfoSet & inAllowedCodecs, bool & externalRouting );
	bool getCallInfo ( SourceData & source, const ss :: string & dialedDigits,
		CodecInfoVector & incomingCodecsVector, const StringCustomerMoneySpoolMap & ccalls,
		const RegCardsMap & registeredCards, const RegisteredOutPeersMap & registeredOutPeers, OutChoiceSetVector & vv,
		StringVector & outAcctns, ss :: string & realDigits, int & errn, bool h323, bool sip, bool mgcp, bool & inAllCodecs,
		CodecInfoSet & inAllowedCodecs,
		std :: tr1 :: function < void ( int peer, const ss :: string & rdigs, IntVector * outPeers ) > externalRoute );
	ResponsibleInfo * getResponsibleInfo ( const ss :: string & uname ) const;
	CustomerInfo * getCustomerInfo ( const ss :: string & uname ) const;
	const InPeerInfo * getInPeerInfo ( int peer ) const;
	const OutPeerInfo * getOutPeerInfo ( int peer ) const;
	bool isDebugInIp ( const ss :: string & ip ) const;
	const PrepaidInfo * getPrepaidInfo ( int id ) const;
	const ss :: string & getTimeToChange ( ) const;
	const TarifInfo * getTarifInfo ( int id ) const;
	int getInPeer ( const ss :: string & ip, const ss :: string & callingDigits,
		const ss :: string & dialedDigits, ss :: string & realDigits, int & fromNat,
		ss :: string & inPrefix, int & retCode ) const;
	int getSlaveId ( const ss :: string & ip ) const throw ( );
	const DNISInfo * getDNISInfo ( const ss :: string & s ) const;
	CardInfo * cardByAni ( const ss :: string & ani );
	CardInfo * cardByAcctn ( const ss :: string & acctn );
	CardInfo * cachedCardByAcctn ( const ss :: string & acctn );
	CardInfo * cardByPin ( const ss :: string & pin );
	CardInfo * cardByNumber ( const ss :: string & number );
	void getSecret ( const ss :: string & ip, RadGWInfo & ri ) const;
	int getPrice ( const ss :: string & realDigits, const PriceElement * root, int defPrice, int discount,
		int & connectPrice, TarifRound & round, ss :: string & code, unsigned & minDigits, unsigned & maxDigits,
		bool canZero = false ) const;
	void getRadiusClass ( const CardInfo * card, int nasId, const DNISInfo * dnis, StringStringMap & rc ) const;
	const StringSet & getLocalIps ( ) const;
	const GateKeepersMap & getGateKeepers ( ) const;
	const LoginInfo * getLoginInfo ( const ss :: string & login ) const;
	const RecodersMap & getRecoders ( ) const;
	const IntVector & getLocalIntIps ( ) const;
	bool registerAni ( AfterTaskVector & que, CardInfo * c, const ss :: string & ani, const ss :: string & lang );
	bool addCardToCache ( int id );
	void uncacheCard ( const CardInfo * c );
	bool getSupportVia ( ) const;
    bool getIsNoChoiceCode() const;
	bool getSIPToH323ErrorResponse(int err, int* response, ss :: string* textResponse) const;
	bool getH323ToSIPErrorResponse(int err, int* response, ss :: string* textResponse) const;
	ss :: string getH323Text(int errorH323) const;
	ss :: string getSIPText(int errorSIP) const;
	bool getMgcpConf ( MgcpGatewayInfoVector & v );
	bool getRportAgents ( StringVector & v );
private:
	typedef std :: map < int, AnumTranslatorIn, std :: less < int >,
		__SS_ALLOCATOR < std :: pair < const int, AnumTranslatorIn > > > AnumReplaceClasses;
	AnumReplaceClasses anumClasses;
	class SigOptionsMap {
		typedef std :: map < int, SigOptionsPeer, std :: less < int >,
			__SS_ALLOCATOR < std :: pair < const int, SigOptionsPeer > > > SigOptionsData;
	public:
			void load ( MySQL & m );
			const SigOptionsPeer get ( int id );
	private:
			SigOptionsData _data;
	};
	SigOptionsMap _sigOptionsMap;
	ss :: string getSentDigits ( const ss :: string & realDigits, const OutPeerInfo * op, const ss :: string & suffix ) const;
	bool supportVia;
    bool isNoChoiceCode;
public:
    double m_reloadConfigTime;	
};
struct CardInfo : public Allocatable < __SS_ALLOCATOR > {
	struct FramedInfo {
		ss :: string ip;
		ss :: string mask;
	};
	char md5 [ 32 ];
	double cost;
	double valuteRate;
	ConfData :: PrepaidInfo * parent;
	ss :: string acctn;
	std :: time_t expires;
	int expTimeFUse;
	char status;
	bool credit;
	bool accRightsInet, accRightsVoice;
	ss :: string options, lang;
	StringIntMap numbers;
	typedef std :: map < ss :: string, ConfData :: CallBackInfo, std :: less < ss :: string >,
		__SS_ALLOCATOR < std :: pair < const ss :: string, ConfData :: CallBackInfo > > > CallBackInfoMap;
	CallBackInfoMap ani;
	bool requireCallbackAuth;
	bool cardBalance, customerBalance, responsibleBalance;
	IntStringMultiMap redirects;
	typedef std :: map < int, FramedInfo, std :: less < int >,
		__SS_ALLOCATOR < std :: pair < const int, FramedInfo > > > IntFramedInfoMap;
	typedef std :: map < int, IntFramedInfoMap, std :: less < int >,
		__SS_ALLOCATOR < std :: pair < const int, IntFramedInfoMap > > > FramedInfoMap;
	FramedInfoMap framedInfo;
	int radiusClass;
	ss :: string clearPass;

	ss :: string lastIp;
	StringStringSetMap forks;
	bool callbackEnabled;
	bool isMonitored;
	bool singleAni;
	void logRegistration ( const ss :: string & ip, int port, int h323, PTime endLease, long lockCount );
	void logUnregistration ( ) const;
	void logLocking ( long lockCount ) const;

	const SigOptionsPeer & getSigOptions ( ) const { return _sigOptions; };
	SigOptionsPeer _sigOptions;
	ss :: string pin;
	void translateAnum ( int grp, ss :: string & num ) const;
private:
	friend class ConfData;
	const AnumTranslatorIn * at;
};


#endif
