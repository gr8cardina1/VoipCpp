#pragma implementation
#include "ss.hpp"
#include "q931.hpp"
#include "allocatable.hpp"
#include "namespair.hpp"
#include "printnumbers.hpp"
#include <stdexcept>
#include <iomanip>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/sequenced_index.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/member.hpp>

struct Discriminator { };

class Q931 :: Impl : public Allocatable < __SS_ALLOCATOR > {
	std :: size_t getTotalBytes ( ) const;
	void printOn ( ss :: string & s ) const;
	Impl & operator= ( const Impl & );
public:
	typedef std :: pair < unsigned char, ss :: string > InformationElement;
	typedef boost :: multi_index :: multi_index_container < InformationElement,
		boost :: multi_index :: indexed_by < boost :: multi_index :: sequenced < >,
		boost :: multi_index :: ordered_non_unique < boost :: multi_index :: tag < Discriminator >,
		boost :: multi_index :: member < InformationElement, InformationElement :: first_type,
		& InformationElement :: first > > >, __SS_ALLOCATOR < InformationElement > > ContainerType;
	typedef ContainerType :: index < Discriminator > :: type IEByDiscriminator;

	enum CallStates {
		csNull			= 0,
		csCallInitiated		= 1,
		csOverlapSending	= 2,
		csOutgoingCallProceeding= 3,
		csCallDelivered		= 4,
		csCallPresent		= 6,
		csCallReceived		= 7,
		csConnectRequest	= 8,
		csIncomingCallProceeding= 9,
		csActive		= 10,
		csDisconnectRequest	= 11,
		csDisconnectIndication	= 12,
		csSuspendRequest	= 15,
		csResumeRequest		= 17,
		csReleaseRequest	= 19,
		csOverlapReceiving	= 25,
		csError			= 0x100
	};
	enum ProgressIndication {
		piNotEndToEndISDN	= 1,// Call is not end-to-end ISDN;
						 // further call progress information may be available in-band
		piDestinationNonISDN	= 2,// Destination address is non ISDN
		piOriginNotISDN		= 3,// Origination address is non ISDN
		piReturnedToISDN	= 4,// Call has returned to the ISDN
		piServiceChange		= 5,// Interworking has occurred and has
						 // resulted in a telecommunication service change
		piInbandInformationAvailable = 8 // In-band information or an appropriate pattern is now available.
	};
	enum NumberingPlanCodes {
		npUnknown		= 0x00,
		npISDN			= 0x01,
		npData			= 0x03,
		npTelex			= 0x04,
		npNationalStandard	= 0x08,
		npPrivate		= 0x09,
		npReserved		= 0x0f
	};
	enum TypeOfNumberCodes {
		tnUnknown		= 0x00,
		tnInternational		= 0x01,
		tnNational		= 0x02,
		tnNetworkSpecific	= 0x03,
		tnSubscriber		= 0x04,
		tnAbbreviated		= 0x06,
		tnReserved		= 0x07
	};
	explicit Impl ( const ss :: string & data );
	Impl ( MsgTypes mt, unsigned cr, bool fd, bool dcr );
	Impl ( ) { }
	ss :: string tpkt ( ) const;
	const ss :: string & getIE ( InformationElementCodes ie, std :: size_t n = 0 ) const;
	IEByDiscriminator :: size_type hasIE ( InformationElementCodes ie ) const;
	void removeIE ( InformationElementCodes ie );
	void setIE ( InformationElementCodes ie, const ss :: string & s );
	void addIE ( InformationElementCodes ie, const ss :: string & s );
	void setCause ( CauseValues cause, unsigned standard = 0, unsigned location = 0 );
	CauseValues getCause ( unsigned * standard = 0, unsigned * location = 0 ) const;
	void setFromDest ( bool v );
	bool getFromDest ( ) const;
	unsigned getCallReference ( ) const;
	bool getDummyCallReference ( ) const;
	void setBearerCapabilities ( InformationTransferCapability capability,
		unsigned transferRate, CodingStandard codingStandard, UserInfoLayer1 userInfoLayer1 );
	bool getBearerCapabilities ( InformationTransferCapability & capability, unsigned & transferRate,
		CodingStandard * codingStandard = 0, UserInfoLayer1 * userInfoLayer1 = 0 ) const;
	void setProgressIndicator ( ProgressDescription descr, CodingStandard codingStandard, Location loc );
	void addProgressIndicator ( ProgressDescription descr, CodingStandard codingStandard, Location loc );
	bool getProgressIndicator ( ProgressDescription & descr, std :: size_t n, CodingStandard * codingStandard,
		Location * location ) const;
	void setSignal ( SignalInfo sig );
	bool getSignal ( SignalInfo & sig ) const;
	void setNotificationDescription ( NotificationDescription nd );
	bool getNotificationDescription ( NotificationDescription & nd ) const;
	ss :: string getCalledPartyNumber ( unsigned * plan, unsigned * type ) const;
	void setCalledPartyNumber ( const ss :: string & number, unsigned plan, unsigned type );
	ss :: string getCallingPartyNumber ( unsigned * plan, unsigned * type, unsigned * presentation, unsigned * screening,
		unsigned defPresentation, unsigned defScreening ) const;
	void setCallingPartyNumber ( const ss :: string & number, unsigned plan, unsigned type, int presentation,
		int screening );
	MsgTypes getMessageType ( ) const;
	void setMessageType ( MsgTypes t );
	ss :: string getMessageTypeName ( ) const;
	void printOn ( std :: ostream & os ) const;
private:
	ContainerType informationElements;
	unsigned callReference;
	MsgTypes messageType;
	bool fromDestination;
	bool dummyCallReference;
	unsigned char protocolDiscriminator;
};

static ss :: string getNumberIE ( const ss :: string & bytes, unsigned * plan, unsigned * type,
	unsigned * presentation, unsigned * screening, unsigned * reason,
	unsigned defPresentation, unsigned defScreening, unsigned defReason ) {
	if ( bytes.empty ( ) )
		return bytes;
	if ( plan )
		* plan = bytes [ 0 ] & 15;
	if ( type )
		* type = ( bytes [ 0 ] >> 4 ) & 7;
	unsigned offset;
	if ( bytes [ 0 ] & 0x80 ) { // Octet 3a not provided, set defaults
		if ( presentation )
			* presentation = defPresentation;
		if ( screening )
			* screening = defScreening;
		if ( reason )
			* reason = defReason;
		offset = 1;
	} else {
		static ss :: string empty;
		if ( bytes.size ( ) < 2 )
			return empty;
		if ( presentation )
			* presentation = ( ( bytes [ 1 ] >> 5 ) & 3 );
		if ( screening )
			* screening = bytes [ 1 ] & 3;
		if ( bytes [ 1 ] & 0x80 ) { // Octet 3b not provided, set defaults
			if ( reason )
				* reason = defReason;
			offset = 2;
		} else {
			if ( bytes.size ( ) < 3 )
				return empty;
			if ( reason )
				* reason = bytes [ 2 ] & 15;
			offset = 3;
		}
	}
	return bytes.substr ( offset );
}

static ss :: string setNumberIE ( const ss :: string & number, unsigned plan, unsigned type,
	int presentation, int screening, int reason ) {
	ss :: string bytes;
	if ( reason == - 1 ) {
		if ( presentation == - 1 || screening == - 1 )
			bytes.push_back ( char ( 0x80 | ( ( type & 7 ) << 4 ) | ( plan & 15 ) ) );
		else {
			bytes.push_back ( char ( ( ( type & 7 ) << 4 ) | ( plan & 15 ) ) );
			bytes.push_back ( char ( 0x80 | ( ( presentation & 3 ) << 5 ) | ( screening & 3 ) ) );
		}
	} else {
		// If octet 3b is present, then octet 3a must also be present!
		if ( presentation == - 1 || screening == - 1 )
			// This situation should never occur!!!
			bytes.push_back ( char ( 0x80 | ( ( type & 7 ) << 4 ) | ( plan & 15 ) ) );
		else {
			bytes.push_back ( char ( 0x80 | ( ( type & 7 ) << 4 ) | ( plan & 15 ) ) );
			bytes.push_back ( char ( 0x80 | ( ( presentation & 3 ) << 5 ) | ( screening & 3 ) ) );
			bytes.push_back ( char ( 0x80 | ( reason & 15 ) ) );
		}
	}
	bytes += number;
	return bytes;
}

std :: size_t Q931 :: Impl :: getTotalBytes ( ) const {
	std :: size_t totalBytes = 5 - dummyCallReference * 2;
	for ( ContainerType :: const_iterator i = informationElements.begin ( );
		i != informationElements.end ( ); ++ i ) {
		if ( i -> first < 128 )
			totalBytes += i -> second.size ( ) + ( i -> first == ieUserUser ? 4 : 2 );
		else
			totalBytes ++;
	}
	return totalBytes;
}

void Q931 :: Impl :: printOn ( ss :: string & s ) const {
	s.push_back ( protocolDiscriminator );
	if ( dummyCallReference )
		s.push_back ( 0 );
	else {
		s.push_back ( 2 );
		if ( fromDestination )
			s.push_back ( char ( ( callReference >> 8 ) | 0x80 ) );
		else
			s.push_back ( char ( callReference >> 8 ) );
		s.push_back ( char ( callReference ) );
	}
	s.push_back ( messageType );
	for ( ContainerType :: const_iterator i = informationElements.begin ( );
		i != informationElements.end ( ); ++ i ) {
		s.push_back ( i -> first );
		if ( i -> first < 128 ) {
			ss :: string :: size_type len = i -> second.size ( );
			if ( i -> first != ieUserUser )
				s.push_back ( char ( len ) );
			else {
				len ++;
				s.push_back ( char ( len >> 8 ) );
				s.push_back ( char ( len ) );
				s.push_back ( 5 );
			}
			s += i -> second;
		}
	}
}

Q931 :: Impl :: Impl ( const ss :: string & data ) {
	protocolDiscriminator = data.at ( 0 );
	if ( protocolDiscriminator != 8 )
		throw std :: runtime_error ( "unknown protocol discriminator" );
	int callRefLen = data.at ( 1 );
	if ( callRefLen != 2 && callRefLen != 1 && callRefLen != 0 ) {
		std :: ostringstream os;
		os << "callref size must be 2 or 1, not " << callRefLen;
		throw std :: runtime_error ( os.str ( ) );
	}
	unsigned offset = 2;
	if ( callRefLen == 0 ) {
		callReference = 0;
		dummyCallReference = true;
		fromDestination = false;
	} else {
		fromDestination = ( data.at ( 2 ) & 0x80 ) != 0;
		dummyCallReference = false;
		callReference = data.at ( offset ++ ) & 0x7fu;
		if ( callRefLen == 2 )
			callReference = ( callReference << 8 ) | ( unsigned char ) data.at ( offset ++ );
	}
	messageType = MsgTypes ( static_cast < unsigned char > ( data.at ( offset ++ ) ) );
	while ( offset < data.size ( ) ) {
		unsigned char discriminator = data.at ( offset ++ );
		ss :: string item;
		// For discriminator with high bit set there is no data
		if ( ( discriminator & 0x80 ) == 0 ) {
			unsigned len = ( unsigned char ) data.at ( offset ++ );
			if ( discriminator == ieUserUser ) {
				// Special case of User-user field. See 7.2.2.31/H.225.0v4.
				len <<= 8;
				len |= ( unsigned char ) data.at ( offset ++ );
				// we also have a protocol discriminator, which we ignore
				if ( data.at ( offset ++ ) != 5 )
					throw std :: runtime_error ( "unknown user-user protocol" );
				// before decrementing the length, make sure it is not zero
				if ( len == 0 )
					throw std :: runtime_error ( "uuie length is -1" );
				// adjust for protocol discriminator
				len --;
			}
			if ( offset + len > data.size ( ) )
				throw std :: runtime_error ( "not complete ie" );
			item.assign ( data, offset, len );
			offset += len;
		}
		if ( discriminator ) {
			if ( ! informationElements.push_back ( InformationElement ( discriminator, item ) ).second ) {
				std :: ostringstream os;
				os << "error inserting ie " << unsigned ( discriminator );
				throw std :: runtime_error ( os.str ( ) );
			}
		}
	}
}

Q931 :: Impl :: Impl ( MsgTypes mt, unsigned cr, bool fd, bool dcr ) : callReference ( cr ), messageType ( mt ),
	fromDestination ( fd ), dummyCallReference ( dcr ), protocolDiscriminator ( 8 ) { }

ss :: string Q931 :: Impl :: tpkt ( ) const {
	char pkt [ 4 ];
	pkt [ 0 ] = 3;
	pkt [ 1 ] = 0;
	std :: size_t packetLength = getTotalBytes ( ) + 4;
	pkt [ 2 ] = char ( packetLength >> 8 );
	pkt [ 3 ] = char ( packetLength );
	ss :: string s;
	s.reserve ( packetLength );
	s.assign ( pkt, 4 );
	printOn ( s );
	return s;
}

const ss :: string & Q931 :: Impl :: getIE ( InformationElementCodes ie, std :: size_t n ) const {
	const IEByDiscriminator & byDisc = informationElements.get < Discriminator > ( );
	IEByDiscriminator :: const_iterator i = byDisc.find ( ie );
	while ( n && i != byDisc.end ( ) ) {
		n --;
		++ i;
	}
	if ( i == byDisc.end ( ) )
		throw std :: invalid_argument ( std :: string ( "not existing ie " ) + getIEName ( ie ).c_str ( ) );
	return i -> second;
}

Q931 :: Impl :: IEByDiscriminator :: size_type Q931 :: Impl :: hasIE ( InformationElementCodes ie ) const {
	return informationElements.get < Discriminator > ( ).count ( ie );
}

void Q931 :: Impl :: removeIE ( InformationElementCodes ie ) {
	informationElements.get < Discriminator > ( ).erase ( ie );
}

void Q931 :: Impl :: setIE ( InformationElementCodes ie, const ss :: string & s ) {
	IEByDiscriminator & byDisc = informationElements.get < Discriminator > ( );
	IEByDiscriminator :: const_iterator i = byDisc.find ( ie );
	if ( i == byDisc.end ( ) )
		byDisc.insert ( IEByDiscriminator :: value_type ( ie, s ) );
	else
		byDisc.replace ( i, IEByDiscriminator :: value_type ( ie, s ) );
}

void Q931 :: Impl :: addIE ( InformationElementCodes ie, const ss :: string & s ) {
	informationElements.push_back ( InformationElement ( ie, s ) );
}

void Q931 :: Impl :: setCause ( CauseValues cause, unsigned standard, unsigned location ) {
	ss :: string t;
	t.push_back ( char ( 0x80 | ( ( standard & 3 ) << 5 ) | ( location & 15 ) ) );
	t.push_back ( char ( 0x80 | cause ) );
	setIE ( ieCause, t );
}

Q931 :: CauseValues Q931 :: Impl :: getCause ( unsigned * standard, unsigned * location ) const {
	if ( ! hasIE ( ieCause ) )
		return cvError;
	const ss :: string & data = getIE ( ieCause );
	if ( data.size ( ) < 2 )
		return cvError;
	if ( standard != 0 )
		* standard = ( data [ 0 ] >> 5 ) & 3;
	if ( location != 0 )
		* location = data [ 0 ] & 15;
	if ( ( data [ 0 ] & 0x80 ) != 0 )
		return CauseValues ( data [ 1 ] & 0x7f );
	// Allow for optional octet
	if ( data.size ( ) < 3 )
		return cvError;
	return CauseValues ( data [ 2 ] & 0x7f );
}

void Q931 :: Impl :: setFromDest ( bool v ) {
	fromDestination = v;
}

bool Q931 :: Impl :: getFromDest ( ) const {
	return fromDestination;
}

unsigned Q931 :: Impl :: getCallReference ( ) const {
	return callReference;
}

bool Q931 :: Impl :: getDummyCallReference ( ) const {
	return dummyCallReference;
}

void Q931 :: Impl :: setBearerCapabilities ( InformationTransferCapability capability, unsigned transferRate,
	CodingStandard codingStandard, UserInfoLayer1 userInfoLayer1 ) {
	ss :: string data;
	data.push_back ( char ( 0x80 | ( ( codingStandard & 3 ) << 5 ) | ( capability & 31 ) ) );
	switch ( codingStandard ) {
		case csCCITT: // ITU-T standardized coding
			// Note this is always "Circuit Mode"
			switch ( transferRate ) {
				case 1:
					data.push_back ( '\x90' );
					break;
				case 2:
					data.push_back ( '\x91' );
					break;
				case 6:
					data.push_back ( '\x93' );
					break;
				case 24:
					data.push_back ( '\x95' );
					break;
				case 30:
					data.push_back ( '\x97' );
					break;
				default:
					data.push_back ( 0x18 );
					data.push_back ( char ( 0x80 | transferRate ) );
			}
			if ( userInfoLayer1 == - 1 )
				data.push_back ( '\xe0' );
			else
				data.push_back ( char ( 0x80 | ( 1 << 5 ) | ( userInfoLayer1 & 31 ) ) );
			break;
		case csISO: // Other international standard
			data.push_back ( '\x80' ); // Call independent signalling connection
			break;
		default :
			break;
	}
	setIE ( ieBearerCapability, data );
}

bool Q931 :: Impl :: getBearerCapabilities ( InformationTransferCapability & capability, unsigned & transferRate,
	CodingStandard * codingStandard, UserInfoLayer1 * userInfoLayer1 ) const {
	if ( ! hasIE ( ieBearerCapability ) )
		return false;
	const ss :: string & data = getIE ( ieBearerCapability );
	if ( data.size ( ) < 3 )
		return false;
	capability = InformationTransferCapability ( data [ 0 ] & 31 );
	if ( codingStandard )
		* codingStandard = CodingStandard ( ( data [ 0 ] >> 5 ) & 3 );
	unsigned nextByte = 2;
	switch ( data [ 1 ] & 31 ) {
		case 0x10:
			transferRate = 1;
			break;
		case 0x11:
			transferRate = 2;
			break;
		case 0x13:
			transferRate = 6;
			break;
		case 0x15:
			transferRate = 24;
			break;
		case 0x17:
			transferRate = 30;
			break;
		case 0x18:
			if ( data.size ( ) < 4)
				return false;
			transferRate = data [ 2 ] & 0x7f;
			nextByte = 3;
			break;
		default:
			return false;
	}
	if ( userInfoLayer1 && ( ( data [ nextByte ] >> 5 ) & 3 ) == 1 )
		* userInfoLayer1 = UserInfoLayer1 ( data [ nextByte ] & 31 );
	return true;
}

void Q931 :: Impl :: setProgressIndicator ( ProgressDescription descr, CodingStandard codingStandard, Location loc ) {
	ss :: string data;
	data.push_back ( char ( 0x80 | ( ( codingStandard & 3 ) << 5 ) | ( loc & 15 ) ) );
	data.push_back ( char ( 0x80 | descr ) );
	setIE ( ieProgressIndicator, data );
}

void Q931 :: Impl :: addProgressIndicator ( ProgressDescription descr, CodingStandard codingStandard, Location loc ) {
	ss :: string data;
	data.push_back ( char ( 0x80 | ( ( codingStandard & 3 ) << 5 ) | ( loc & 15 ) ) );
	data.push_back ( char ( 0x80 | descr ) );
	addIE ( ieProgressIndicator, data );
}

bool Q931 :: Impl :: getProgressIndicator ( ProgressDescription & descr, std :: size_t n, CodingStandard * codingStandard,
	Location * location ) const {
	if ( hasIE ( ieProgressIndicator ) <= n )
		return false;
	const ss :: string & data = getIE ( ieProgressIndicator, n );
	if ( data.size ( ) != 2 )
		return false;
	if ( codingStandard )
		* codingStandard = CodingStandard ( ( data [ 0 ] >> 5 ) & 3 );
	if ( location )
		* location = Location ( data [ 0 ] & 15 );
	descr = ProgressDescription ( data [ 1 ] & 127 );
	return true;
}

void Q931 :: Impl :: setSignal ( SignalInfo sig ) {
	ss :: string data;
	data.push_back ( sig );
	setIE ( ieSignal, data );
}

bool Q931 :: Impl :: getSignal ( SignalInfo & sig ) const {
	if ( ! hasIE ( ieSignal ) )
		return false;
	const ss :: string & data = getIE ( ieSignal );
	if ( data.size ( ) != 1 )
		return false;
	sig = SignalInfo ( data [ 0 ] & 0xff );
	return true;
}

void Q931 :: Impl :: setNotificationDescription ( NotificationDescription nd ) {
	ss :: string data;
	data.push_back ( char ( nd | 0x80 ) );
	setIE ( ieNotificationIndicator, data );
}

bool Q931 :: Impl :: getNotificationDescription ( NotificationDescription & nd ) const {
	if ( ! hasIE ( ieNotificationIndicator ) )
		return false;
	const ss :: string & data = getIE ( ieNotificationIndicator );
	if ( data.size ( ) != 1 )
		return false;
	nd = NotificationDescription ( data [ 0 ] & 0x7f );
	return true;
}

ss :: string Q931 :: Impl :: getCalledPartyNumber ( unsigned * plan = 0, unsigned * type = 0 ) const {
	try {
		return getNumberIE ( getIE ( ieCalledPartyNumber ), plan, type, 0, 0, 0, 0, 0, 0 );
	} catch ( ... ) {
		return ss :: string ( );
	}
}

void Q931 :: Impl :: setCalledPartyNumber ( const ss :: string & number, unsigned plan, unsigned type ) {
	setIE ( ieCalledPartyNumber, setNumberIE ( number, plan, type, -1, -1, -1 ) );
}

ss :: string Q931 :: Impl :: getCallingPartyNumber ( unsigned * plan, unsigned * type, unsigned * presentation,
	unsigned * screening, unsigned defPresentation, unsigned defScreening ) const {
	try {
		return getNumberIE ( getIE ( ieCallingPartyNumber ), plan, type, presentation,
			screening, 0, defPresentation, defScreening, 0 );
	} catch ( ... ) {
		return ss :: string ( );
	}
}

void Q931 :: Impl :: setCallingPartyNumber ( const ss :: string & number, unsigned plan, unsigned type, int presentation,
	int screening ) {
	setIE ( ieCallingPartyNumber, setNumberIE ( number, plan, type, presentation, screening, -1 ) );
}

Q931 :: MsgTypes Q931 :: Impl :: getMessageType ( ) const {
	return messageType;
}

void Q931 :: Impl :: setMessageType ( MsgTypes t ) {
	messageType = t;
}

ss :: string Q931 :: Impl :: getMessageTypeName ( ) const {
	static const NamesPair namePairs [ ] = {
		{ msgNationalEscape, "NationalEscape" },
		{ msgAlerting, "Alerting" },
		{ msgCallProceeding, "CallProceeding" },
		{ msgConnect, "Connect" },
		{ msgConnectAck, "ConnectAck" },
		{ msgProgress, "Progress" },
		{ msgSetup, "Setup" },
		{ msgSetupAck, "SetupAck" },
		{ msgResume, "Resume" },
		{ msgResumeAck, "ResumeAck" },
		{ msgResumeReject, "ResumeReject" },
		{ msgSuspend, "Suspend" },
		{ msgSuspendAck, "SuspendAck" },
		{ msgSuspendReject, "SuspendReject" },
		{ msgUserInformation, "UserInformation" },
		{ msgDisconnect, "Disconnect" },
		{ msgRelease, "Release" },
		{ msgReleaseComplete, "ReleaseComplete" },
		{ msgRestart, "Restart" },
		{ msgRestartAck, "RestartAck" },
		{ msgSegment, "Segment" },
		{ msgCongestionCtrl, "CongestionCtrl" },
		{ msgInformation, "Information" },
		{ msgNotify, "Notify" },
		{ msgStatus, "Status" },
		{ msgStatusEnquiry, "StatusEnquiry" },
		{ msgFacility, "Facility" }
	};
	static const NamesMapType msgNamesMap ( namePairs, namePairs + sizeof ( namePairs ) / sizeof ( * namePairs ) );
	NamesMapType :: const_iterator i = msgNamesMap.find ( messageType );
	if ( i != msgNamesMap.end ( ) )
		return i -> second;
	ss :: ostringstream os;
	os << '<' << messageType << '>';
	return os.str ( );
}

class PrintIE {
	std :: ostream & os;
	int indent;
public:
	PrintIE ( std :: ostream & o, int i ) : os ( o ), indent ( i ) { }
	void operator() ( const std :: pair < unsigned char, ss :: string > & i ) {
		os << std :: setw ( indent + 4 ) << "IE: " <<
			Q931 :: getIEName ( Q931 :: InformationElementCodes ( i.first ) );
		if ( i.first > 127 )
			return;
		if ( i.first == Q931 :: ieCause && i.second.size ( ) > 1 ) {
 			ss :: string t = Q931 :: getCauseName ( ( unsigned char ) i.second [ 1 ] & 0x7f );
			if ( t [ 0 ] != '0' )
				os << " - " << t;
		}
		os << " = {\n"
		   << std :: hex << std :: setfill ( '0' ) << std :: resetiosflags ( std :: ios :: floatfield )
		   << std :: setprecision ( indent + 2 ) << std :: setw ( 16 );
		printNumbers ( os, i.second, indent + 5 );
		os << std :: setfill ( ' ' ) << std :: setw ( indent + 2 ) << "}\n";
	}
};

void Q931 :: Impl :: printOn ( std :: ostream & os ) const {
	int indent = int ( os.precision ( ) + 2 );
	std :: _Ios_Fmtflags flags = os.flags ( );
	os << "{\n"
	   << std :: setw ( indent + 24 ) << "protocolDiscriminator = " << unsigned ( protocolDiscriminator );
	os << '\n' << std :: setw ( indent + 16 ) << "callReference = " << callReference;
	os << ( dummyCallReference ? " (dummy)" : "" ) << '\n' << std :: setw ( indent + 7 ) << "from = "
	   << ( fromDestination ? "destination" : "originator" ) << '\n' << std :: setw ( indent + 14 )
	   << "messageType = " << getMessageTypeName ( ) << '\n';
	std :: for_each ( informationElements.begin ( ), informationElements.end ( ), PrintIE ( os, indent ) );
	os << std :: setw ( indent - 1 ) << '}';
	os.flags ( flags );
}

ss :: string Q931 :: getIEName ( InformationElementCodes ie ) {
	static const NamesPair namePairs [ ] = {
		{ ieSegmented, "Segmented-Message" },
		{ ieBearerCapability, "Bearer-Capability" },
		{ ieCause, "Cause" },
		{ ieFacility, "Facility" },
		{ ieProgressIndicator, "Progress-Indicator" },
		{ ieProgressIndicator2, "Progress-Indicator2" },
		{ ieCallState, "Call-State" },
		{ ieNotificationIndicator, "NotificationIndicator" },
		{ ieDisplay, "Display" },
		{ ieKeypad, "Keypad" },
		{ ieSignal, "Signal" },
		{ ieConnectedNumber, "Connected-Number" },
		{ ieCallingPartyNumber, "Calling-Party-Number" },
		{ ieCalledPartyNumber, "Called-Party-Number" },
		{ ieRedirectingNumber, "Redirecting-Number" },
		{ ieUserUser, "User-User" },
		{ ieSendingComplete, "Sending-Complete" }
	};
	static const NamesMapType ieNamesMap ( namePairs, namePairs + sizeof ( namePairs ) / sizeof ( * namePairs ) );
	NamesMapType :: const_iterator i = ieNamesMap.find ( ie );
	if ( i != ieNamesMap.end ( ) )
		return i -> second;
	ss :: ostringstream os;
	os << "0x" << std :: hex << ie;
	return os.str ( );
};

ss :: string Q931 :: getCauseName ( unsigned cause ) {
	static const NamesPair namePairs [ ] = {
		{ cvUnallocatedNumber, "Unallocated number" },
		{ cvNoRouteToNetwork, "No route to network" },
		{ cvNoRouteToDestination, "No route to destination" },
		{ cvSendSpecialTone, "Send special tone" },
		{ cvMisdialledTrunkPrefix, "Misdialled trunk prefix" },
		{ cvChannelUnacceptable, "Channel unacceptable" },
		{ cvDeliveredInEstablishedChannel, "Call awarded and being delivered in an established channel" },
		{ cvNormalCallClearing, "Normal call clearing" },
		{ cvUserBusy, "User busy" },
		{ cvNoResponse, "No response" },
		{ cvNoAnswer, "No answer" },
		{ cvSubscriberAbsent, "Subscriber absent" },
		{ cvCallRejected, "Call rejected" },
		{ cvNumberChanged, "Number changed" },
		{ cvRedirection, "Redirection" },
		{ cvExchangeRoutingError, "Exchange routing error" },
		{ cvNonSelectedUserClearing, "Non selected user clearing" },
		{ cvDestinationOutOfOrder, "Destination out of order" },
		{ cvInvalidNumberFormat, "Invalid number format" },
		{ cvFacilityRejected, "Facility rejected" },
		{ cvStatusEnquiryResponse, "Status enquiry response" },
		{ cvNormalUnspecified, "Normal unspecified" },
		{ cvNoCircuitChannelAvailable, "No circuit/channel available" },
		{ cvNetworkOutOfOrder, "Network out of order" },
		{ cvTemporaryFailure, "Temporary failure" },
		{ cvCongestion, "Congestion" },
		{ cvAccesInformationDiscarded, "Acces information discarded" },
		{ cvRequestedCircitChannelNotAvailable, "Requested circit/channel not available" },
		{ cvResourceUnavailable, "Resource unavailable" },
		{ cvBearerCapabilityNotImplemented, "Bearer capability not implemented" },
		{ cvInvalidCallReference, "Invalid call reference" },
		{ cvIncompatibleDestination, "Incompatible destination" },
		{ cvRecoveryOnTimerExpires, "Recovery on timer expires" },
		{ cvInterworkingUnspecified, "Interworking, unspecified" },
		{ cvNonStandardReason, "Non standard reason" }
	};
	static const NamesMapType causeNamesMap ( namePairs, namePairs + sizeof ( namePairs ) / sizeof ( * namePairs ) );
	NamesMapType :: const_iterator i = causeNamesMap.find ( cause );
	if ( i != causeNamesMap.end ( ) )
		return i -> second;
	ss :: ostringstream os;
	os << "0x" << std :: hex << cause;
	return os.str ( );
}

ss :: string Q931 :: getUil1Name ( UserInfoLayer1 uil1 ) {
	static const NamesPair namePairs [ ] = {
		{ uil1V110, "CCITT standardized rate adaption V.110 and X.30" },
		{ uil1G711m, "Recommendation G.711 m-law" },
		{ uil1G711A, "Recommendation G.711 A-law" },
		{ uil1G721, "Recommendation G.721 32 kbit/s ADPCM and Recommendation I.460" },
		{ uil1H221, "Recommendations H.221 and H.242" },
		{ uil1NonCCITT, "Non-CCITT standardized rate adaption" },
		{ uil1V120, "CCITT standardized rate adaption V.120" },
		{ uil1X31, "CCITT standardized rate adaption X.31" }
	};
	static const NamesMapType uil1NamesMap ( namePairs, namePairs + sizeof ( namePairs ) / sizeof ( * namePairs ) );
	NamesMapType :: const_iterator i = uil1NamesMap.find ( uil1 );
	if ( i != uil1NamesMap.end ( ) )
		return i -> second;
	ss :: ostringstream os;
	os << "0x" << std :: hex << uil1;
	return os.str ( );
};

ss :: string Q931 :: getLocationName ( Location loc ) {
	static const NamesPair namePairs [ ] = {
		{ lUser, "User" },
		{ lPrivateLocal, "Private network serving the local user" },
		{ lPublicLocal, "Public network serving the local user" },
		{ lTransit, "Transit network" },
		{ lPublicRemote, "Public network serving the remote user" },
		{ lPrivateRemote, "Private network serving the remote user" },
		{ lInternetworking, "Network beyond the interworking point" }
	};
	static const NamesMapType locNamesMap ( namePairs, namePairs + sizeof ( namePairs ) / sizeof ( * namePairs ) );
	NamesMapType :: const_iterator i = locNamesMap.find ( loc );
	if ( i != locNamesMap.end ( ) )
		return i -> second;
	ss :: ostringstream os;
	os << "0x" << std :: hex << loc;
	return os.str ( );
};

ss :: string Q931 :: getProgressDescriptionName ( ProgressDescription descr ) {
	static const NamesPair namePairs [ ] = {
		{ pdNotEndToEndISDN, "Call is not end-to-end ISDN; "
			"further call progress information may be available in‑band" },
		{ pdDestinationNonISDN, "Destination address is non ISDN" },
		{ pdOriginationNonISDN, "Origination address is non ISDN" },
		{ pdReturnedToISDN, "Call has returned to the ISDN" },
		{ pdInternetworking, "Interworking has occurred and has resulted in a telecommunication service change" },
		{ pdInBand, "In-band information or an appropriate pattern is now available" }
	};
	static const NamesMapType descrNamesMap ( namePairs, namePairs + sizeof ( namePairs ) / sizeof ( * namePairs ) );
	NamesMapType :: const_iterator i = descrNamesMap.find ( descr );
	if ( i != descrNamesMap.end ( ) )
		return i -> second;
	ss :: ostringstream os;
	os << "0x" << std :: hex << descr;
	return os.str ( );
};

ss :: string Q931 :: getCodingStandardName ( CodingStandard cs ) {
	static const NamesPair namePairs [ ] = {
		{ csCCITT, "CCITT standardized coding" },
		{ csISO, "ISO/IEC standard" },
		{ csNational, "National standard" },
		{ csLocal, "Standard specific to identified location" }
	};
	static const NamesMapType csNamesMap ( namePairs, namePairs + sizeof ( namePairs ) / sizeof ( * namePairs ) );
	NamesMapType :: const_iterator i = csNamesMap.find ( cs );
	if ( i != csNamesMap.end ( ) )
		return i -> second;
	ss :: ostringstream os;
	os << "0x" << std :: hex << cs;
	return os.str ( );
};

ss :: string Q931 :: getInformationTransferCapabilityName ( InformationTransferCapability itc ) {
	static const NamesPair namePairs [ ] = {
		{ tcSpeech, "Speech" },
		{ tcUnrestrictedDigital, "Unrestricted digital information" },
		{ tcRestrictedDigital, "Restricted digital information" },
		{ tc3_1kHzAudio, "3.1 kHz audio" },
		{ tcUnrestrictedDigitalWithTones, "Unrestricted digital information with tones/announcements" },
		{ tcVideo, "Video" }
	};
	static const NamesMapType itcNamesMap ( namePairs, namePairs + sizeof ( namePairs ) / sizeof ( * namePairs ) );
	NamesMapType :: const_iterator i = itcNamesMap.find ( itc );
	if ( i != itcNamesMap.end ( ) )
		return i -> second;
	ss :: ostringstream os;
	os << "0x" << std :: hex << itc;
	return os.str ( );
};

ss :: string Q931 :: getSignalName ( SignalInfo sig ) {
	static const NamesPair namePairs [ ] = {
		{ siDialToneOn, "Dial tone on" },
		{ siRingBackToneOn, "Ring back tone on" },
		{ siInterceptToneOn, "Intercept tone on" },
		{ siNetworkCongestionToneOn, "Network congestion tone on" },
		{ siBusyToneOn, "Busy tone on" },
		{ siConfirmToneOn, "Confirm tone on" },
		{ siAnswerToneOn, "Answer tone on" },
		{ siCallWaitingTone, "Call waiting tone" },
		{ siOffhookWarningTone, "Off-hook warning tone" },
		{ siPreemptionToneOn, "Preemption tone on" },
		{ siTonesOff, "Tones off" },
		{ siAlertingPattern0, "Alerting on – pattern 0" },
		{ siAlertingPattern1, "Alerting on – pattern 1" },
		{ siAlertingPattern2, "Alerting on – pattern 2" },
		{ siAlertingPattern3, "Alerting on – pattern 3" },
		{ siAlertingPattern4, "Alerting on – pattern 4" },
		{ siAlertingPattern5, "Alerting on – pattern 5" },
		{ siAlertingPattern6, "Alerting on – pattern 6" },
		{ siAlertingPattern7, "Alerting on – pattern 7" },
		{ siAlertingOff, "Alerting off" }
	};
	static const NamesMapType sigNamesMap ( namePairs, namePairs + sizeof ( namePairs ) / sizeof ( * namePairs ) );
	NamesMapType :: const_iterator i = sigNamesMap.find ( sig );
	if ( i != sigNamesMap.end ( ) )
		return i -> second;
	ss :: ostringstream os;
	os << "0x" << std :: hex << sig;
	return os.str ( );
};

ss :: string Q931 :: getNotificationDescriptionName ( NotificationDescription nd ) {
	static const NamesPair namePairs [ ] = {
		{ ndUserSuspended, "User suspended" },
		{ ndUserResumed, "User resumed" },
		{ ndBearerServiceChange, "Bearer service change" }
	};
	static const NamesMapType ndNamesMap ( namePairs, namePairs + sizeof ( namePairs ) / sizeof ( * namePairs ) );
	NamesMapType :: const_iterator i = ndNamesMap.find ( nd );
	if ( i != ndNamesMap.end ( ) )
		return i -> second;
	ss :: ostringstream os;
	os << "0x" << std :: hex << nd;
	return os.str ( );
};

Q931 :: Q931 ( ) : impl ( new Impl ( ) ) { }

Q931 :: Q931 ( const Q931 & o ) : impl ( new Impl ( * o.impl ) ) { }

Q931 & Q931 :: operator= ( const Q931 & o ) {
	Q931 ( o ).swap ( * this );
	return * this;
}

Q931 :: Q931 ( const ss :: string & data ) : impl ( new Impl ( data ) ) { }

Q931 :: Q931 ( MsgTypes mt, unsigned cr, bool fd, bool dcr ) : impl ( new Impl ( mt, cr, fd, dcr ) ) { }

Q931 :: ~Q931 ( ) {
	delete impl;
}

void Q931 :: swap ( Q931 & o ) {
	std :: swap ( impl, o.impl );
}

ss :: string Q931 :: tpkt ( ) const {
	return impl -> tpkt ( );
}

const ss :: string & Q931 :: getIE ( InformationElementCodes ie, std :: size_t n ) const {
	return impl -> getIE ( ie, n );
}

std :: size_t Q931 :: hasIE ( InformationElementCodes ie ) const {
	return impl -> hasIE ( ie );
}

void Q931 :: removeIE ( InformationElementCodes ie ) {
	impl -> removeIE ( ie );
}

void Q931 :: setIE ( InformationElementCodes ie, const ss :: string & s ) {
	impl -> setIE ( ie, s );
}

void Q931 :: addIE ( InformationElementCodes ie, const ss :: string & s ) {
	impl -> addIE ( ie, s );
}

void Q931 :: setCause ( CauseValues cause, unsigned standard, unsigned location ) {
	impl -> setCause ( cause, standard, location );
}

Q931 :: CauseValues Q931 :: getCause ( unsigned * standard, unsigned * location ) const {
	return impl -> getCause ( standard, location );
}

void Q931 :: setFromDest ( bool v ) {
	impl -> setFromDest ( v );
}

bool Q931 :: getFromDest ( ) const {
	return impl -> getFromDest ( );
}

unsigned Q931 :: getCallReference ( ) const {
	return impl -> getCallReference ( );
}

bool Q931 :: getDummyCallReference ( ) const {
	return impl -> getDummyCallReference ( );
}

void Q931 :: setBearerCapabilities ( InformationTransferCapability capability, unsigned transferRate,
	CodingStandard codingStandard, UserInfoLayer1 userInfoLayer1 ) {
	impl -> setBearerCapabilities ( capability, transferRate, codingStandard, userInfoLayer1 );
}

bool Q931 :: getBearerCapabilities ( InformationTransferCapability & capability, unsigned & transferRate,
	CodingStandard * codingStandard, UserInfoLayer1 * userInfoLayer1 ) const {
	return impl -> getBearerCapabilities ( capability, transferRate, codingStandard, userInfoLayer1 );
}

void Q931 :: setProgressIndicator ( ProgressDescription descr, CodingStandard codingStandard, Location loc ) {
	impl -> setProgressIndicator ( descr, codingStandard, loc );
}

void Q931 :: addProgressIndicator ( ProgressDescription descr, CodingStandard codingStandard, Location loc ) {
	impl -> addProgressIndicator ( descr, codingStandard, loc );
}

bool Q931 :: getProgressIndicator ( ProgressDescription & descr, std :: size_t n, CodingStandard * codingStandard,
	Location * location ) const {
	return impl -> getProgressIndicator ( descr, n, codingStandard, location );
}

void Q931 :: setSignal ( SignalInfo sig ) {
	impl -> setSignal ( sig );
}

bool Q931 :: getSignal ( SignalInfo & sig ) const {
	return impl -> getSignal ( sig );
}

void Q931 :: setNotificationDescription ( NotificationDescription nd ) {
	impl -> setNotificationDescription ( nd );
}

bool Q931 :: getNotificationDescription ( NotificationDescription & nd ) const {
	return impl -> getNotificationDescription ( nd );
}

ss :: string Q931 :: getCalledPartyNumber ( unsigned * plan, unsigned * type ) const {
	return impl -> getCalledPartyNumber ( plan, type );
}

void Q931 :: setCalledPartyNumber ( const ss :: string & number, unsigned plan, unsigned type ) {
	impl -> setCalledPartyNumber ( number, plan, type );
}

ss :: string Q931 :: getCallingPartyNumber ( unsigned * plan, unsigned * type, unsigned * presentation,
	unsigned * screening, unsigned defPresentation, unsigned defScreening ) const {
	return impl -> getCallingPartyNumber ( plan, type, presentation, screening, defPresentation, defScreening );
}

void Q931 :: setCallingPartyNumber ( const ss :: string & number, unsigned plan, unsigned type, int presentation,
	int screening ) {
	impl -> setCallingPartyNumber ( number, plan, type, presentation, screening );
}

Q931 :: MsgTypes Q931 :: getMessageType ( ) const {
	return impl -> getMessageType ( );
}

void Q931 :: setMessageType ( MsgTypes t ) {
	impl -> setMessageType ( t );
}

ss :: string Q931 :: getMessageTypeName ( ) const {
	return impl -> getMessageTypeName ( );
}

void Q931 :: printOn ( std :: ostream & os ) const {
	impl -> printOn ( os );
}
