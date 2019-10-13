#ifndef __REQUESTINFO_HPP
#define __REQUESTINFO_HPP
#pragma interface

struct RequestInfo {
	bool inet, credit, callback, callback15, callback2, callback3, callback4, requireCallbackAuth, hasStart,
		smscallback, smscallback1, smscallback2, smscallback3, registerAni;
	double creditAmount, cardCreditAmount, customerCreditAmount, responsibleCreditAmount, valuteRate;
	int priceAdd, creditTime, creditRealTime, cbTime;
	PriceData cbPrice;
	ss :: string cbCode;
	ss :: string ip;
	ss :: string acctn;
	ss :: string pass;
	ss :: string md5;
	ss :: string ani;
	ss :: string calledStationId;
	ss :: string dnis;
	ss :: string h323Conf;
	ss :: string sessionId;
	ss :: string cbNumber;
	ss :: string options;
	ss :: string lang;
	ss :: string cbDialNumber;
	ss :: string cbRealNumber;
	ss :: string number1;
	ss :: string number2;
	ss :: string applyCode;
	StringStringMap radiusClass;
	StringVector proxyStates;
	RequestInfo ( ) : inet ( false ), credit ( false ), callback ( false ), callback15 ( false ), callback2 ( false ),
		callback3 ( false ), callback4 ( false ), requireCallbackAuth ( false ), hasStart ( true ), smscallback ( false ),
		smscallback1 ( false ), smscallback2 ( false ), smscallback3 ( false ), registerAni ( false ), creditAmount ( 0 ),
		cardCreditAmount ( 0 ), customerCreditAmount ( 0 ), responsibleCreditAmount ( 0 ), valuteRate ( 1 ),
		priceAdd ( 0 ), creditTime ( 0 ), creditRealTime ( 0 ), cbTime ( 0 ) { }
};
#endif
