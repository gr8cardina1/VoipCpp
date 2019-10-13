#ifndef __CALLCONTROL_HPP
#define __CALLCONTROL_HPP
#pragma interface

class CallControl {
public:
	virtual void beginShutDown ( ) = 0;
	virtual int getCallSeconds ( ) const = 0;
	virtual bool getCallConnected ( ) const = 0;
	virtual ~CallControl ( ) { }
	virtual int getCallRef ( ) const = 0;
	virtual ss :: string getInIp ( ) const = 0;
	virtual int getInPeerId ( ) const = 0;
	virtual ss :: string getInPeerName ( ) const = 0;
	virtual ss :: string getInResponsibleName ( ) const = 0;
	virtual ss :: string getSetupTime ( ) const = 0;
	virtual ss :: string getConnectTime ( ) const = 0;
	virtual ss :: string getOutIp ( ) const = 0;
	virtual int getOutPeerId ( ) const { return 0; }
	virtual ss :: string getCallId ( ) const = 0;
	virtual ss :: string getOutPeerName ( ) const = 0;
	virtual ss :: string getOutResponsibleName ( ) const = 0;
	virtual ss :: string getCallingDigits ( ) const = 0;
	virtual ss :: string getSentDigits ( ) const = 0;
	virtual ss :: string getDialedDigits ( ) const = 0;
	virtual ss :: string getInCode ( ) const = 0;
	virtual ss :: string getOutCode ( ) const = 0;
	virtual ss :: string getInAcctn ( ) const = 0;
	virtual ss :: string getOutAcctn ( ) const = 0;
	virtual ss :: string getPdd ( ) const = 0;
	virtual bool getDifferentProtocol ( ) const = 0;
	virtual bool getDifferentCodec ( ) const = 0;
};

#endif
