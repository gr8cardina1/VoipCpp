#ifndef __CONF_HPP
#define __CONF_HPP
#pragma interface
#include <tr1/functional>
#include "q931.hpp"

class OutChoiceDetails;
typedef std :: vector < OutChoiceDetails, __SS_ALLOCATOR < OutChoiceDetails > > OutChoiceDetailsVector;
typedef std :: vector < OutChoiceDetailsVector, __SS_ALLOCATOR < OutChoiceDetailsVector > > OutChoiceDetailsVectorVector;
class CommonCallDetailsBase;
class CodecInfoVector;
class CommonCallDetailsBaseOut;
class SourceData;
class CallDetails;
class CallControl;
class SipCallDetails;
class EaterDetails;
class PricePrios;
class RequestInfo;
class AccountingInfo;
class PUDPSocket;
class IpPort;
typedef std :: map < ss :: string, IpPort, std :: less < ss :: string >,
	__SS_ALLOCATOR < std :: pair < const ss :: string, IpPort > > > IpPortMap;
typedef std :: map < ss :: string, IpPortMap, std :: less < ss :: string >,
	__SS_ALLOCATOR < std :: pair < const ss :: string, IpPortMap > > > RecodersMap;
class H323Call;
class CommonCallDetails;
class LatencyLimits;
namespace H225 {
	class TransportAddress;
}
class Q931;
class SigOptionsPeer;
class CardInfo;
class MgcpGatewayInfoVector;
class Conf : public Allocatable < __SS_ALLOCATOR >, boost :: noncopyable {
	void cardLoaded ( const CardInfo * card );
	friend class CardLoaded;
	void addSipCallToRemove ( const ss :: string & confId );
	friend class SipCallDetails;
	void destroyCustomerSpool ( const ss :: string & uname );
	void propagateCustomerBalanceChange ( const ss :: string & uname, double money );
	friend class CustomerMoneySpool;
	class Impl;
	static Impl * impl;
public:
	static PSemaphore * TerminationSema; // Main will wait on this semaphore for termination
	static PSemaphore * TerminatedSema; // OnStop then waits for this until finishing....
	typedef std :: map < ss :: string, StringBoolMap, std :: less < ss :: string >,
		__SS_ALLOCATOR < std :: pair < const ss :: string, StringBoolMap > > > UnregisteredContactsMap;
	Conf ( );
	~Conf ( );
	void reloadConf ( );
	bool getCallInfo ( OutChoiceDetailsVectorVector & forks, StringVector & forkOutAcctns, CommonCallDetailsBase & call,
		bool h323, bool sip, bool mgcp );
	bool isValidInPeerAddress ( const ss :: string & ip );
	bool isValidOutPeerAddress ( const ss :: string & ip, int port );
	bool take ( CommonCallDetailsBaseOut & call, int & cause, bool * gwTaken );
	void release ( const CommonCallDetailsBaseOut & call, bool rc, bool gwTaken );
	bool shuttingDown ( ) const;
	void setShuttingDown ( );
	void take ( );
	bool tryTake ( ) __attribute__ ( ( warn_unused_result ) );
	void release ( int inPeer = 0 );
	void takeRadiusCall ( );
	void releaseRadiusCall ( );
	void takeHttpCall ( );
	void releaseHttpCall ( );
	void waitForSignallingChannels ( );
	int getLogLevel ( ) const;
	const ss :: string & getName ( ) const;
	bool isDebugInIp ( const ss :: string & ip );
	void addCall ( CallDetails & call, CallControl * ctrl );
	void addCall ( SipCallDetails * call );
	void removeCall ( CallDetails & call );
	void removeCall ( SipCallDetails * call );
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
	void getSecret ( const PIPSocket :: Address & ip, RadGWInfo & g );
	int checkPin ( RequestInfo & ri );
	int getCreditTime ( RequestInfo & ri );
	int getCreditTimeIVR( RequestInfo & ri );
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
		const StringSet & neededNumbers, StringSet & onlineNumbers, UnregisteredContactsMap * unregisteredContacts,
		PUDPSocket & sock, IcqContactsVector & icqContacts );
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
	int getSipInviteRegistrationPeriod ( ) const;
	int getSipDefaultRegistrationPeriod ( ) const;
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
	int loadIVRData ( const ss :: string& account, const ss :: string & password,
		double & strCost, ss :: string & lang ) const;
	bool checkAdminsPass ( const ss :: string & uname, const ss :: string & pass ) const;
	bool getMgcpConf ( MgcpGatewayInfoVector & v );
	void registerReloadNotifier ( const std :: tr1 :: function < void ( ) > & f );
	bool getRportAgents ( StringVector & v ) const;
	class AniAfterTask : public AfterTask {
		ss :: string acctn;
		ss :: string ani;
		ss :: string lang;
	public:
		AniAfterTask ( const ss :: string & ac, const ss :: string & a, const ss :: string & l );
		~AniAfterTask ( );
	};
};
extern Conf * conf;
#endif
