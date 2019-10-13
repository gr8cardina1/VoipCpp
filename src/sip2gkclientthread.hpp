#ifndef SIP2GKCLIENTTHREAD_HPP_
#define SIP2GKCLIENTTHREAD_HPP_
#pragma interface
namespace SIP2 {
class GkClientThread : public BaseGkClientThread {
	PCLASSINFO ( GkClientThread, BaseGkClientThread )
	class Impl;
	Impl * impl;
	~GkClientThread ( );
	void Main ( );
	void Close ( );
	void updateState ( const ss :: string & n, const ss :: string & ln, int rs );
	bool getAddr ( const ss :: string & /*digits*/, PIPSocket :: Address & /*destAddr*/, int & /*destPort*/ ) {
		return false;
		// eto nado location service
	}
public:
	GkClientThread ( const ss :: string & username, int regPeriod, const ss :: string & ip, const IntVector & localIps,
		const ss :: string & pswd, int port, const ss :: string & gkName );

};
}

#endif /*SIP2GKCLIENTTHREAD_HPP_*/
