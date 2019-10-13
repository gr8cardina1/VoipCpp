#include <sys/poll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>

class RTPRequest;
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
	bool usedTable [ maxCalls ];
	int sockTable [ maxSockets ];
	int sockRecodeTable [ maxCalls * 2 ];
	pollfd pollTable;
	int commandSocket;
	int allocateChannel ( int from, int to, int fromNat, bool toNat );
	bool bindSockets ( int base, int from, int to );
	static int makeSocket ( );
	static void bind ( int & s1, int & s2, int addr, int port = 0 );
	static bool listen ( int fd, int port, int addr );
	bool setSendAddress ( int i, int addr, int port, int codec, int frames, bool sendCodec, int recodePort,
		int recodeAddr );
	void setTelephoneEventsPayloadType(unsigned i, unsigned telephoneEventsPayloadType);
	void doSetTelephoneEventsPayloadType(const RTPRequest & r);
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
	void doSetTo ( const RTPRequest & r );
	void doSetFrom ( const RTPRequest & r );
	void doEnableLog ( const RTPRequest & r );
	void getStat ( int i, RTPStat * st );
	void setLocal ( int i, int l, int addr, bool nat, int ret [ 2 ] );
};
