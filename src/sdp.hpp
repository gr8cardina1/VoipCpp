#ifndef SDP_HPP_
#define SDP_HPP_
#pragma interface

namespace SDP {

class MediaFormat {
	ss :: string fmt;
	ss :: string fmtp;
	ss :: string rtpmap;
public:
	explicit MediaFormat ( const ss :: string & f ) : fmt ( f ) { }
	const ss :: string & getFmt ( ) const {
		return fmt;
	}
	void setFmt ( const ss :: string & s ) {
		fmt = s;
	}
	const ss :: string & getFmtp ( ) const {
		return fmtp;
	}
	void setFmtp ( const ss :: string & s ) {
		fmtp = s;
	}
	const ss :: string & getRtpmap ( ) const {
		return rtpmap;
	}
	void setRtpmap ( const ss :: string & s ) {
		rtpmap = s;
	}
};

inline std :: ostream & operator<< ( std :: ostream & os, const MediaFormat & f ) {
	return os << f.getFmt ( ) << ' ' << f.getRtpmap ( ) << ' ' << f.getFmtp ( );
}

struct fmt { };
struct rtpmap { };
typedef boost :: multi_index :: multi_index_container < MediaFormat,
	boost :: multi_index :: indexed_by < boost :: multi_index :: sequenced < >,
	boost :: multi_index :: ordered_unique < boost :: multi_index :: tag < fmt >,
	boost :: multi_index :: const_mem_fun < MediaFormat, const ss :: string &, & MediaFormat :: getFmt > >,
	boost :: multi_index :: ordered_non_unique < boost :: multi_index :: tag < rtpmap >,
	boost :: multi_index :: const_mem_fun < MediaFormat, const ss :: string &, & MediaFormat :: getRtpmap > > >,
	__SS_ALLOCATOR < MediaFormat > > MediaFormatVector;

class MediaDescription {
	class Impl;
	Impl * impl;
public:
	MediaDescription ( const ss :: string & media, unsigned port, const ss :: string & proto );
	MediaDescription ( const MediaDescription & m );
	MediaDescription & operator= ( const MediaDescription & m );
	void swap ( MediaDescription & m );
	~MediaDescription ( );
	void printOn ( std :: ostream & os ) const;
	void addFmt ( const ss :: string & s );
	void setInformationField ( const ss :: string & s );
	void addConnectionField ( const ss :: string & ip );
	void clearConnectionField ( );
	void addBandwidthField ( const ss :: string & s );
	void setKeyField ( const ss :: string & s );
	void addAttributeField ( const ss :: string & key, const ss :: string & value );
	void delAttributeField ( const char * key );
	std :: size_t getFmtsSize ( ) const;
	const ss :: string & getMedia ( ) const;
	const ss :: string * getAttributeValue ( const char * key ) const;
	void getAttributeValues ( const char * key, StringVector & v ) const;
	const MediaFormatVector & getFmts ( ) const;
	const ss :: string * getConnectionAddress ( ) const;
	unsigned getPort ( ) const;
};

unsigned getPtime ( const MediaDescription * media );

extern const ss :: string mediaAudio;
extern const ss :: string mediaImage;
extern const ss :: string protoRtpAvp;
extern const ss :: string protoUdptl;

extern const ss :: string PCMU;
extern const ss :: string FS1016;
extern const ss :: string G721;
extern const ss :: string G726;
extern const ss :: string GSM;
extern const ss :: string G7231;
extern const ss :: string DVI4_8k;
extern const ss :: string DVI4_16k;
extern const ss :: string LPC;
extern const ss :: string PCMA;
extern const ss :: string G722;
extern const ss :: string L16_Stereo;
extern const ss :: string L16_Mono;
extern const ss :: string G723;
extern const ss :: string CN;
extern const ss :: string MPA;
extern const ss :: string G728;
extern const ss :: string DVI4_11k;
extern const ss :: string DVI4_22k;
extern const ss :: string G729;
extern const ss :: string Cisco_CN;
extern const ss :: string CelB;
extern const ss :: string JPEG;
extern const ss :: string H261;
extern const ss :: string MPV;
extern const ss :: string MP2T;
extern const ss :: string H263;
extern const ss :: string T38;
extern const ss :: string TelephoneEvent;

class SessionDescription : public Allocatable < __SS_ALLOCATOR > {
	class Impl;
	Impl * impl;
public:
	explicit SessionDescription ( const ss :: string & s );
	SessionDescription ( const ss :: string :: const_iterator & ib, const ss :: string :: const_iterator & ie );
	SessionDescription ( const SessionDescription & d );
	SessionDescription & operator= ( const SessionDescription & d );
	void printOn ( std :: ostream & os ) const;
	void swap ( SessionDescription & d );
	~SessionDescription ( );
	const ss :: string & getSessionName ( ) const;
	void setConnectionAddress ( const ss :: string & caddr );
	void addMediaDescription ( const MediaDescription & m );
	const MediaDescription * getMediaDescription ( const ss :: string & media ) const;
	const ss :: string * getAttributeValue ( const char * key ) const;
	void getAttributeValues ( const char * key, StringVector & v ) const;
	const ss :: string & getConnectionAddress ( ) const;
};

inline std :: ostream & operator<< ( std :: ostream & os, const SessionDescription & s ) {
	s.printOn ( os );
	return os;
}

inline void swap ( SessionDescription & s1, SessionDescription & s2 ) {
	s1.swap ( s2 );
}

}

#endif /*SDP_HPP_*/
