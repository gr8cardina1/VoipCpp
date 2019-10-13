#ifndef __H323GKCLIENTTHREAD_HPP
#define __H323GKCLIENTTHREAD_HPP
#pragma interface

namespace H225 {
	class RasMessage;
	class TransportAddress;
};

class H323GkClientThread : public BaseGkClientThread {
	PCLASSINFO ( H323GkClientThread, BaseGkClientThread )
	ss :: string _ip, _name, _localName, _terminalAlias, _gkName;
	PMutex mut;
	PCondVar regDone;
	IxcUDPSocket sock;
	IntVector _localIps;
	IntVector _bindIps;
	PTime lastRegistration;
	int _port;
	int _regPeriod;
	int regTries;
	bool registered;
	bool _discoveryComplete;
public:
	H323GkClientThread ( const ss :: string & n, const ss :: string & ln, int rs, const IntVector & l,
		const IntVector & b, const ss :: string & i, int p, const ss :: string & gkName );
	void gotURQ ( );
private:
	void updateState ( const ss :: string & n, const ss :: string & ln, int rs );
	bool getAddr ( const ss :: string & digits, PIPSocket :: Address & destAddr, int & destPort );
	void Main ( );
	void Close ( );
	ss :: string getLocalName ( );
	ss :: string getName ( );
	void tryToRegister ( );
	int buildGRQ ( H225 :: RasMessage & mesg );
	bool sendGRQ ( int & n );
	bool discover ( );
	int buildRRQ ( H225 :: RasMessage & mesg );
	bool sendRRQ ( int & n );
	bool receiveRCF ( int seqNum );
	void buildLRQ ( PUDPSocket & socket, const ss :: string & digits, H225 :: RasMessage & mesg, int & seqNum );
	bool sendLRQ ( PUDPSocket & socket, const ss :: string & digits, int & seqNum );
	bool decodeLCF ( const H225 :: RasMessage & mesg, H225 :: TransportAddress & destAddr, bool & retry );
	bool receiveLCF ( int seqNum, PUDPSocket & socket, H225 :: TransportAddress & destAddr, bool & retry );
	static int getSeqNum ( );
};
#endif
