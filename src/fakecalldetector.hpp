#ifndef _FAKECALLDETECTOR_HPP_
#define _FAKECALLDETECTOR_HPP_
#pragma interface

class FakeCallDetector {
public:
	enum FakeCallMotive {
		fNone			= 1,
		fLen			= 2,
		fSeq			= 3,
		fShortConnect		= 4,
		fShortCallProceeding	= 5,
		fShortAlerting		= 6,
		fRtpTimeout		= 7
	};
	FakeCallDetector ( );
	~FakeCallDetector ( );
	void registerMesg ( int mesgType );
	void registerRTPtimeout ( );
	void initialize ( int outPeerId, const LatencyLimits & latencyLimits );

private:
	const ss :: string _composeLog ( );
	bool _isFakeCall ( );

	IntVector _standardMesgSeq, _curMesgSeq;
	long long _procTime, _alertTime, _connectTime, _callDuration;//, _minCallDuration;
	PTime _callStartTime;
	LatencyLimits _latencyLimits;
	int _outPeerId;
	FakeCallMotive _motive;
	bool _connected, _rtpTimeout, _inited;
};

#endif
