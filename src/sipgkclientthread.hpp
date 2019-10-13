#ifndef __SIPGKCLIENTTHREAD_HPP
#define __SIPGKCLIENTTHREAD_HPP
#pragma interface

#define HASHLEN 16
typedef char HASH [ HASHLEN ];
typedef char HASHHEX [ HASHLEN * 2 + 1 ];
#undef HASHLEN

namespace SIP {
	class PDU;
}

class SipGkClientThread : public BaseGkClientThread {
	PCLASSINFO ( SipGkClientThread, BaseGkClientThread )
public:
	SipGkClientThread ( const ss :: string & name, const ss :: string & localName, int regPeriod,
		const IntVector & localIps, const IntVector & bindIps, const ss :: string & ip,
		const ss :: string & pswd, int port, const ss :: string & gkName );
private:
	void updateState ( const ss :: string & n, const ss :: string & ln, int rs );
	bool getAddr ( const ss :: string & digits, PIPSocket :: Address & destAddr, int & destPort );
	void Main ( );
	void Close ( );
	void tryToRegister ( );
	bool _sendRRQ ( );
	void _buildRRQ ( int cSeq, const ss :: string & authLine, SIP :: PDU & mesg );
	bool _receiveMesg ( SIP :: PDU & mesg );
	bool _handleMesg ( SIP :: PDU & mesg );
	bool _handleOK ( SIP :: PDU & mesg );
	bool _handleError ( SIP :: PDU & mesg );
	static int getSeqNum ( );
	void CvtHex ( const HASH Bin, HASHHEX Hex );
	ss :: string _generateUID ( );
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
	ss :: string _callId, _authLine, _pswd;
	SIPAuthenticateValidator _validator;
	ss :: string _localIp;
};
#endif
