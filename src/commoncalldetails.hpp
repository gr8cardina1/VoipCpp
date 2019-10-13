#ifndef __COMMONCALLDETAILS_HPP
#define __COMMONCALLDETAILS_HPP
#pragma interface

class CommonCallDetailsBase {
protected:
	virtual ~CommonCallDetailsBase ( ) { }
public:
	virtual const ss :: string & getCallerIp ( ) const = 0;
	virtual int getCallerPort ( ) const = 0;
	virtual const ss :: string & getCallingDigitsIn ( ) const = 0;
	virtual const ss :: string & getDialedDigits ( ) const = 0;
	virtual SourceData & source ( ) = 0;
	virtual const SourceData & getSource ( ) const = 0;
	virtual const ss :: string & getConfId ( ) const = 0;
	virtual void setFromNat ( int fn ) = 0;
	virtual int getFromNat ( ) const = 0;
	virtual SigOptionsCall & sigOptions ( ) = 0;
	virtual const SigOptionsCall & getSigOptions ( ) const = 0;
	virtual bool & inAllCodecs ( ) = 0;
	virtual bool getInAllCodecs ( ) const = 0;
	virtual CodecInfoSet & inAllowedCodecs ( ) = 0;
	virtual const CodecInfoSet & getInAllowedCodecs ( ) const = 0;
	virtual CodecInfoVector & incomingCodecs ( ) = 0;
	virtual const CodecInfoVector & getIncomingCodecs ( ) const = 0;
	virtual void setDisconnectCause ( int c ) = 0;
	virtual int getDisconnectCause ( ) const = 0;
	virtual void setRealDigits ( const ss :: string & s ) = 0;
	virtual const ss :: string & getRealDigits ( ) const = 0;
	virtual void initChoices ( int reserveSize ) = 0;
	virtual void addChoice ( const OutChoiceDetails & c ) = 0;
	virtual const OutChoiceDetailsVector & getChoices ( ) const = 0;
	virtual bool getForceDebug ( ) const = 0;
	virtual const ss :: string & getConvertedDigits ( ) const = 0;
	virtual void setTelephoneEventsPayloadType(unsigned newPayload) = 0;
	virtual unsigned getTelephoneEventsPayloadType() const = 0;
	virtual void setInTaken ( bool t = true ) = 0;
	virtual void setDirectIn ( bool d ) = 0;
	virtual bool getDirectIn ( ) const = 0;
};
class CommonCallDetailsBaseOut : public CommonCallDetailsBase {
protected:
	~CommonCallDetailsBaseOut ( ) { };
public:
	virtual void setIsGk ( bool g ) = 0;
	virtual void setOutRemotePrice ( bool r ) = 0;
	virtual void setOutAllCodecs ( bool a ) = 0;
	virtual bool getOutAllCodecs ( ) const = 0;
	virtual void setOutAllowedCodecs ( const CodecInfoSet & codecs ) = 0;
	virtual const CodecInfoSet & getOutAllowedCodecs ( ) const = 0;
	virtual void setOutName ( const ss :: string & n ) = 0;
	virtual void setOutRname ( const ss :: string & n ) = 0;
	virtual const OutChoiceDetails * curChoice ( ) const = 0;
};
class CommonCallDetails : public CommonCallDetailsBaseOut {
	friend class boost :: serialization :: access;
	std :: tr1 :: shared_ptr < SmartGuard > releaseGuard;
	ss :: string callerIp;
	ss :: string callingDigitsIn;
	ss :: string dialedDigits;
	ss :: string realDigits;
	SourceData _source;
	ss :: string confId;
	SigOptionsCall _sigOptions;
	CodecInfoSet _inAllowedCodecs;
	OutChoiceDetailsVector choices;
	CodecInfoSet outAllowedCodecs;
	ss :: string outName, outRname;
	CodecInfoVector _incomingCodecs;
	int callerPort;
	int fromNat;
	int disconnectCause;
	int peerIndex;
	unsigned uTelephoneEventsPayloadType_;
	bool _inAllCodecs;
	bool _disconnectCauseInited;
	bool forceDebug;
	bool isGk;
	bool outRemotePrice;
	bool outAllCodecs;
	bool isAnonymous_;
	bool inTaken, directIn;
	template < class Archive > void serialize ( Archive & ar, const unsigned int /*version*/ ) {
		ar & callerIp;
		ar & callerPort;
		ar & callingDigitsIn;
		ar & dialedDigits;
		ar & realDigits;
		ar & _source;
//		ar & confId;
		ar & fromNat;
//		ar & _sigOptions;
		ar & _inAllCodecs;
//		ar & _inAllowedCodecs;
		ar & _disconnectCauseInited;
		ar & disconnectCause;
		ar & choices;
		ar & peerIndex;
		ar & forceDebug;
		ar & isGk;
		ar & outRemotePrice;
		ar & outAllCodecs;
//		ar & outAllowedCodecs;
		ar & outName;
		ar & outRname;
		ar & isAnonymous_;
		ar & uTelephoneEventsPayloadType_;
	}
public:
	CommonCallDetails ( );
	void setCallerIp ( const ss :: string & i ) {
		callerIp = i;
	}
	const ss :: string & getCallerIp ( ) const {
		return callerIp;
	}
	void setCallerPort ( int p ) {
		callerPort = p;
	}
	int getCallerPort ( ) const {
		return callerPort;
	}
	/// A-number (tot, chtj nabrali)
	void setCallingDigitsIn ( const ss :: string & d ) {
		callingDigitsIn = d;
	}
	const ss :: string & getCallingDigitsIn ( ) const {
		return callingDigitsIn;
	}
	/// Modificirovannyj A-Number.
	void setDialedDigits ( const ss :: string & d ) {
		dialedDigits = d;
	}
	const ss :: string & getDialedDigits ( ) const {
		return dialedDigits;
	}
	SourceData & source ( ) {
		return _source;
	}
	const SourceData & getSource ( ) const {
		return _source;
	}
	void setConfId ( const ss :: string & s ) {
		confId = s;
	}
	const ss :: string & getConfId ( ) const {
		return confId;
	}
	void setFromNat ( int fn ) {
		fromNat = fn;
	}
	int getFromNat ( ) const {
		return fromNat;
	}
	SigOptionsCall & sigOptions ( ) {
		return _sigOptions;
	}
	const SigOptionsCall & getSigOptions ( ) const {
		return _sigOptions;
	}
	bool & inAllCodecs ( ) {
		return _inAllCodecs;
	}
	bool getInAllCodecs ( ) const {
		return _inAllCodecs;
	}
	CodecInfoSet & inAllowedCodecs ( ) {
		return _inAllowedCodecs;
	}
	const CodecInfoSet & getInAllowedCodecs ( ) const {
		return _inAllowedCodecs;
	}
	CodecInfoVector & incomingCodecs ( ) {
		return _incomingCodecs;
	}
	const CodecInfoVector & getIncomingCodecs ( ) const {
		return _incomingCodecs;
	}
	bool disconnectCauseInited ( ) const {
		return _disconnectCauseInited;
	}
	void setDisconnectCause ( int c ) {
		_disconnectCauseInited = true;
		disconnectCause = c;
	}
	int getDisconnectCause ( ) const {
		return disconnectCause;
	}
	void setDisconnectCauseWeak ( int c ) {
		if ( ! disconnectCauseInited ( ) )
			setDisconnectCause ( c );
	}
	void clearDisconnectCause ( ) {
		_disconnectCauseInited = false;
		disconnectCause = 0;
	}
	void setRealDigits ( const ss :: string & s ) {
		realDigits = s;
	}
	const ss :: string & getRealDigits ( ) const {
		return realDigits;
	}
	const ss :: string & getCallingDigits ( ) const {
		return curChoice ( ) -> getCallingDigits ( );
	}
	void initChoices ( int reserveSize ) {
		choices.clear ( );
		choices.reserve ( reserveSize );
		peerIndex = - 1;
	}
	void addChoice ( const OutChoiceDetails & c ) {
		choices.push_back ( c );
	}
	const OutChoiceDetailsVector & getChoices ( ) const {
		return choices;
	}
	const OutChoiceDetails * curChoice ( ) const {
		int i = getPeerIndex ( );
		if ( i == - 1 )
			return 0;
		return choice ( i );
	}
	const OutChoiceDetails * choice ( int i ) const {
		if ( i < getPeerCount ( ) && i >= 0 )
			return & choices [ i ];
		return 0;
	}
	int getPeerCount ( ) const {
		return int ( choices.size ( ) );
	}
	int getPeerIndex ( ) const {
		return peerIndex;
	}
	bool hasNextPeerIndex ( ) const {
		return getPeerIndex ( ) < getPeerCount ( ) - 1;
	}
	void nextPeerIndex ( ) {
		peerIndex ++;
		clearDisconnectCause ( );
		sigOptions ( ).setOut ( curChoice ( ) -> getSigOptions ( ) );
	}
	void setPeerIndex ( int i ) {
		peerIndex = i;
	}
	void setForceDebug ( bool fd ) {
		forceDebug = fd;
	}
	bool getForceDebug ( ) const {
		return forceDebug;
	}
	const ss :: string & getConvertedDigits ( ) const {
		return curChoice ( ) -> getDigits ( );
	}
	const ss :: string & getCalledIp ( ) const {
		return curChoice ( ) -> getIp ( );
	}
	int getCalledPort ( ) const {
		return curChoice ( ) -> getPort ( );
	}
	void setIsGk ( bool g ) {
		isGk = g;
	}
	bool getIsGk ( ) const {
		return isGk;
	}
	void setOutRemotePrice ( bool r ) {
		outRemotePrice = r;
	}
	bool getOutRemotePrice ( ) const {
		return outRemotePrice;
	}
	void setOutAllCodecs ( bool a ) {
		outAllCodecs = a;
	}
	bool getOutAllCodecs ( ) const {
		return outAllCodecs;
	}
	void setOutAllowedCodecs ( const CodecInfoSet & codecs ) {
		outAllowedCodecs = codecs;
	}
	const CodecInfoSet & getOutAllowedCodecs ( ) const {
		return outAllowedCodecs;
	}
	void setOutName ( const ss :: string & n ) {
		outName = n;
	}
	const ss :: string & getOutName ( ) const {
		return outName;
	}
	void setOutRname ( const ss :: string & n ) {
		outRname = n;
	}
	const ss :: string & getOutRname ( ) const {
		return outRname;
	}
	bool getUseNormalizer ( ) const {
		if ( _sigOptions.getUseNormalizer ( ) )
			return true;
		for ( OutChoiceDetailsVector :: const_iterator i = choices.begin ( ); i != choices.end ( ); ++ i )
			if ( i -> getSigOptions ( ).getUseNormalizer ( ) )
				return true;
		return false;
	}
	const ss :: string & getSentDigits ( ) const {
		return curChoice ( ) -> getSentDigits ( );
	}

	void setAnonymous(bool isAnonymous)
	{
		isAnonymous_ = isAnonymous;
	}
	bool isAnonymous() const
	{
		return isAnonymous_;
	}

	unsigned getTelephoneEventsPayloadType() const
	{
		return uTelephoneEventsPayloadType_;
	}

	void setTelephoneEventsPayloadType(unsigned newPayload)
	{
		uTelephoneEventsPayloadType_ = newPayload;
	}
	void setInTaken ( bool t ) {
		inTaken = t;
	}
	void setDirectIn ( bool d ) {
		directIn = d;
	}
	bool getDirectIn ( ) const {
		return directIn;
	}
	void release ( ) const;
	void dismiss ( ) { // nado bi prosto eshe odin constructor vmesto etogo
		releaseGuard -> dismiss ( );
	}
	void dropNonH323 ( );
	void dropNonSIP ( );
};

struct EaterDetails {
	Pointer < MoneyEater > inEater, inDealerEater, cardEater, outCardEater, outCustomerEater, outResponsibleEater;
	bool empty ( ) {
		return ! inEater && ! inDealerEater && ! cardEater && ! outCardEater && ! outCustomerEater && ! outResponsibleEater;
	}
};
#endif
