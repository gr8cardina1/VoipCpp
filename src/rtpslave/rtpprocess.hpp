#include <sys/poll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include "../ss.hpp"
#include "../allocatable.hpp"
#include "../rtpstat.hpp"
class RTPRequest;
struct RepData;
class RTPSource {
	unsigned ssrc;
	unsigned short maxSeq;
	unsigned cycles;
	unsigned baseSeq;
	unsigned badSeq;
	unsigned probation;
	unsigned received;
	unsigned expectedPrior;
	unsigned receivedPrior;
	long long transit;
	unsigned jitter;
	void init ( unsigned short seq );
	void initFirst ( unsigned short seq, unsigned ts, unsigned s );
public:
	void reset ( );
	bool update ( unsigned short seq, unsigned ts, unsigned s );
	void printStats ( ) const;
	unsigned getFracLost ( int count ) const;
	unsigned getJitter ( ) const;
};
class RTPProcess {
public:
	RTPProcess ( );
	~RTPProcess ( );
	void pollIteration ( );
private:
	enum {
#ifdef linux
		maxCalls = 2000,
#else
		maxCalls = 2000,
#endif
		maxSockets = maxCalls * 4,
		portBase = 15000,
		portEnd = portBase + maxSockets * 3 / 2 + 1000
	};
	pollfd pollTable [ maxSockets + 1 ];
	struct sockaddr_in destAddrs [ maxSockets ];
	bool usedTable [ maxCalls ];
	char fromNat [ maxCalls ];
	bool toNat [ maxCalls ];
	RTPStat statTable [ maxCalls * 4 ];
	RTPSource rtpSources [ maxCalls * 2 ];
	std :: time_t lastActiveTimes [ maxCalls * 2 ];
	bool log [ maxCalls ];
	int allocateChannel ( int from, int to );
	bool bindSockets ( int base, int from, int to );
	static int makeSocket ( );
	static void bind ( int & s1, int & s2, int addr, int port = 0 );
	static bool listen ( int fd, int port, int addr );
	bool setSendAddress ( int i, int addr, int port );
	bool getLocalAddress ( int fd, int & addr, int & port );
	void startChannel ( int i );
	void stopChannel ( int i );
	void processRequest ( );
	void doInit ( );
	void initialize ( );
	void doSetSendAddress ( const RTPRequest & r );
	void freeChannel ( int i );
	void doFree ( const RTPRequest & r );
	void doAlloc ( const RTPRequest & r );
	void doGetTimeout ( const RTPRequest & r );
	void doProxy ( int i );
	void getStats ( int i, const RepData & rd, unsigned ssrc );
	void doEnableLog ( const RTPRequest & r );
	void doSetTo ( const RTPRequest & r );
	void doSetFrom ( const RTPRequest & r );
};
