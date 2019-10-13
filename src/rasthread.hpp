#ifndef __RASTHREAD_HPP
#define __RASTHREAD_HPP
#pragma interface

namespace H225 {
	class RegistrationRequest;
	class UnregistrationRequest;
}
class RasThread : public PThread, public Allocatable < __SS_ALLOCATOR > {
	PCLASSINFO ( RasThread, PThread )
public:
	RasThread ( const WORD port = 1719 );
	virtual ~RasThread ( );
	void Close ( );
	bool writeRasReply ( PUDPSocket & sock, const H225 :: TransportAddress & replyTo, const H225 :: RasMessage & mesg );
protected:
	RasThread ( const RasThread & ); // ...or copy constructor
	bool handleRasRequest ( const H225 :: RasMessage & request, H225 :: RasMessage & reply,
		H225 :: TransportAddress & replyTo, PUDPSocket & sock );
	virtual void Main ( );
	bool readRasReq ( PUDPSocket & sock );
	H225 :: TransportAddress getGatekeeperAddr ( PUDPSocket & sock ) const;
	bool onRRQ ( const H225 :: RegistrationRequest & rrq, H225 :: RasMessage & reply,
		H225 :: TransportAddress & replyto, PUDPSocket & sock );
	bool onURQ ( const H225 :: UnregistrationRequest & urq, H225 :: RasMessage & reply,
		H225 :: TransportAddress & replyto );
	bool checkTokens ( const H225 :: RegistrationRequest & regReq, const PIPSocket :: Address & srcAddr, int srcPort,
		int & ttl, const PIPSocket :: Address & sigAddr, int sigPort, const StringSet & neededNumbers,
		StringSet & interestedLogins, Conf :: UnregisteredContactsMap & unregisteredContacts,
		ss :: string & login, PUDPSocket & sock, IcqContactsVector & icqContacts ) const;
	ss :: string getLoginFromRRQ ( const H225 :: RegistrationRequest & regReq );
//	void sendNotifications ( const Conf :: UnregisteredContactsMap & changes );
	vector < Pointer < PUDPSocket > > rasSockets;
	PMutex mut;
	WORD rasPort;
public:
	void sendNotifications ( const Conf :: UnregisteredContactsMap & changes );
};
extern RasThread * rasThread;
#endif
