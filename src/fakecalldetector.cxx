#pragma implementation
#include "ss.hpp"
#include "latencylimits.hpp"
#include <ptlib.h>
#include "fakecalldetector.hpp"
#include <ptlib/svcproc.h>
#include "allocatable.hpp"
#include "q931.hpp"
#include "sqloutthread.hpp"


FakeCallDetector :: FakeCallDetector ( ) {
	_standardMesgSeq.push_back ( Q931 :: msgCallProceeding );
	_standardMesgSeq.push_back ( Q931 :: msgAlerting );
	_standardMesgSeq.push_back ( Q931 :: msgConnect );
	_inited = false;
}

void FakeCallDetector :: registerMesg ( int mesgType ) {
	PTimeInterval tt = PTime ( ) - _callStartTime;
	PSYSTEMLOG ( Info, "FakeCallDetector :: registerMesg. MesgType=" << mesgType << ", timeshift=" << tt );
	switch ( mesgType ) {
		case Q931 :: msgCallProceeding :
			_procTime = tt.GetMilliSeconds ( );
			break;
		case Q931 :: msgAlerting :
			_alertTime = tt.GetMilliSeconds ( );
			break;
		case Q931 :: msgConnect :
			_connectTime = tt.GetMilliSeconds ( );
			_connected = true;
			break;
		case Q931 :: msgReleaseComplete :
			if ( _connected ) {
				_callDuration = tt.GetMilliSeconds ( );
			}
			_connected = false;
			return;
		default :
			return;
	}
	_curMesgSeq.push_back ( mesgType );
	if ( _curMesgSeq.size ( ) > _standardMesgSeq.size ( ) ) {
		_curMesgSeq.clear ( );
		return;
	}
}

void FakeCallDetector :: initialize ( int outPeerId, const LatencyLimits & latencyLimits ) {
	_connected = false;
	_procTime = _alertTime = _connectTime = _callDuration = 0;
	_motive = fNone;
	_curMesgSeq.clear ( );
	_latencyLimits = latencyLimits;
	_outPeerId = outPeerId;
	_callDuration = 0;
	_motive = fNone;
	_callStartTime = PTime ( );
	_rtpTimeout = false;
	_inited = true;
}

bool FakeCallDetector :: _isFakeCall ( ) {
	if ( ! _inited ) {
		PSYSTEMLOG ( Info, "FakeCallDetector: not inited" );
		return false;
	}
	if ( ! _latencyLimits.enabled ) {
		PSYSTEMLOG ( Info, "FakeCallDetector: disabled for outPeer " << _outPeerId );
		return false;
	}
	if ( _curMesgSeq.size ( ) == 0 ) {
		PSYSTEMLOG ( Info, "FakeCallDetector: seq len=0, no call at all :)" );
		return false;
	}
	if ( _callDuration > _latencyLimits.minCallDuration ) {
		PSYSTEMLOG ( Info, "FakeCallDetector: Call long enough (" << _callDuration << '/' <<
			_latencyLimits.minCallDuration << "), cannot be faked" );
		return false;
	}
	if ( _standardMesgSeq.size ( ) != _curMesgSeq.size ( ) ) {
		PSYSTEMLOG ( Info, "FakeCallDetector: seq length missmatch " << _curMesgSeq.size ( ) );
		_motive = fLen;
		return true;
	}
	if ( ! ( _standardMesgSeq == _curMesgSeq ) ) {
		PSYSTEMLOG ( Info, "FakeCallDetector: Seq differ" );
		_motive = fSeq;
		return true;
	}
	if ( _procTime < _latencyLimits.minCallProceedingTime ) {
		PSYSTEMLOG ( Info, "FakeCallDetector: too short CallProceeding " << _procTime << '/' << _latencyLimits.minCallProceedingTime );
		_motive = fShortCallProceeding;
		return true;
	}
	if ( _alertTime < _latencyLimits.minAlertingTime ) {
		PSYSTEMLOG ( Info, "FakeCallDetector: too short Alerting " << _alertTime << '/' << _latencyLimits.minAlertingTime );
		_motive = fShortAlerting;
		return true;
	}
	if ( _connectTime < _latencyLimits.minConnectTime ) {
		PSYSTEMLOG ( Info, "FakeCallDetector: too short Connect " << _connectTime << '/' << _latencyLimits.minConnectTime );
		_motive = fShortConnect;
		return true;
	}
	if ( _rtpTimeout ) {
		PSYSTEMLOG ( Info, "FakeCallDetector: RTP timeout" );
		_motive = fRtpTimeout;
		return true;
	}
	PSYSTEMLOG ( Info, "FakeCallDetector: Call is OK (" << _procTime << ", " << _alertTime << ", " <<
		 _connectTime << ", " << _callDuration << ')' );
	_motive = fNone;
	return false;
}

const ss :: string FakeCallDetector :: _composeLog ( ) {
	if ( _motive == fSeq )
		_procTime = _alertTime = _connectTime = 0;
	if ( _connected )
		_callDuration = ( PTime ( ) - _callStartTime ).GetMilliSeconds ( );
	ss :: ostringstream os;
	os << "insert into FakeCalls ( callDateTime, outPeerId, fakeMotive, callProceedingTime, "
		"alertingTime, connectTime, callDuration ) "
		"values ( from_unixtime( " << PTime ( ).GetTimeInSeconds ( ) << " ), " <<
		 _outPeerId << ", " << _motive << ", " << _procTime << ", " << _alertTime <<
		 ", " << _connectTime << ", " << _callDuration << " )";
	return os.str ( );
}

FakeCallDetector :: ~FakeCallDetector ( ) {
	PSYSTEMLOG ( Info, "FakeCallDetector: destructing..." );
	if ( ! _isFakeCall ( ) )
		return;
	ss :: string query = _composeLog ( );
	sqlOut -> add ( query );
}

void FakeCallDetector :: registerRTPtimeout ( ) {
	PSYSTEMLOG ( Info, "FakeCallDetector: RTP timeout registered" );
	_rtpTimeout = true;
}
