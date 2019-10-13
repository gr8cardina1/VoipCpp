#ifndef __ASN_HPP
#define __ASN_HPP
#pragma interface

void printNumbers ( std :: ostream & os, const ss :: string & s, int indent );

namespace Asn {

#define __ASN_ALLOCATOR __SS_ALLOCATOR

inline unsigned countBitsChar ( unsigned range ) {
/*	unsigned nBits = 1;
	while ( range >= ( 1u << nBits ) )
		nBits ++;
	return nBits; //optimize via table*/
/*	static unsigned char table [ 255 ] = { 1, 1, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4,
		5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
		6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
		7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
		7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
		8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
		8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
		8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
		8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8 };
	return table [ range ];*/
	return unsigned ( sizeof ( unsigned ) ) * 8 - __builtin_clz ( range );
}

inline unsigned countBytes ( unsigned range ) {
	return range < 256 ? 1 : unsigned ( sizeof ( unsigned ) ) - __builtin_clz ( range ) / 8;
}

using ss :: string;
typedef std :: basic_string < unsigned short, std :: char_traits < unsigned short >,
	__ASN_ALLOCATOR < unsigned short > > ushort_string;
using ss :: ostringstream;
using ss :: istringstream;

typedef std :: map < unsigned, string, std :: less < unsigned >,
	__ASN_ALLOCATOR < std :: pair < const unsigned, string > > > NamesMapType;

class istream {
	const string s;
	string except;
	string :: size_type byteOffset;
	unsigned bitOffset;
public:
	istream ( const string & ss ) : s ( ss ), byteOffset ( 0 ), bitOffset ( 8 ) { }
	void setException ( const std :: exception & e ) {
		except = e.what ( );
	}
	const string & getException ( ) const {
		return except;
	}
	bool hasException ( ) const {
		return ! except.empty ( );
	}
	bool bitDecode ( ) {
		bool r = s.at ( byteOffset ) & ( 1 << -- bitOffset );
		if ( ! bitOffset ) {
			bitOffset = 8;
			byteOffset ++;
		}
		return r;
	}
	void byteAlign ( ) {
		if ( bitOffset != 8 ) {
			bitOffset = 8;
			byteOffset ++;
		}
	}
	unsigned char alignedByteDecode ( ) {
		return s.at ( byteOffset ++ );
	}
	unsigned lengthDecode ( unsigned lower, unsigned upper ) {
		if ( upper < 65536 )
			return unsignedDecode ( lower, upper );
		byteAlign ( );
		unsigned v = alignedByteDecode ( );
		if ( v < 128 )
			return v;
		if ( v & 64 )
			throw std :: runtime_error ( "not implemented" );
		return ( v & 63 ) << 8 | alignedByteDecode ( );
	}
	unsigned multiByteDecode ( unsigned nBytes ) {
		byteAlign ( );
		return alignedMultiByteDecode ( nBytes );
	}
	unsigned alignedMultiByteDecode ( unsigned nBytes ) {
		unsigned v = 0;
		switch ( nBytes ) {
			case 4:
				v = alignedByteDecode ( ) << 24;
			case 3:
				v |= alignedByteDecode ( ) << 16;
			case 2:
				v |= alignedByteDecode ( ) << 8;
			case 1:
				v |= alignedByteDecode ( );
				return v;
		}
		std :: ostringstream os;
		os << "unsupported multiByteDecode " << nBytes;
		throw std :: out_of_range ( os.str ( ) );
	}
	unsigned unsignedDecode ( unsigned lower, unsigned upper ) {
		unsigned range = upper - lower;
		if ( ! range )
			return lower;
		unsigned r = lower;
		if ( range >= 255 ) { //full byte or more
			if ( range >= 65536 ) { //3-4 bytes
				unsigned nBytes = lengthDecode ( 1, 3 + ( range >= 65536 * 256 ) );
				r += multiByteDecode ( nBytes );
			} else {
				byteAlign ( );
				if ( range >= 256 ) //2 bytes
					r += alignedByteDecode ( ) << 8;
				r += alignedByteDecode ( );
			}
		} else {
			unsigned nBits = countBitsChar ( range );
			r += multiBitDecode ( nBits );
		}
		if ( r > upper )
			throw std :: runtime_error ( "invalid unsigned decoded" );
		return r;
	}
	unsigned smallUnsignedDecode ( ) {
		if ( ! bitDecode ( ) )
			return multiBitDecode ( 6 );
		unsigned len = lengthDecode ( 0, std :: numeric_limits < int > :: max ( ) );
		return alignedMultiByteDecode ( len );
	}
	unsigned multiBitDecode ( unsigned nBits ) {
		if ( nBits == 0 )
			return 0;
		if ( nBits > sizeof ( unsigned ) * 8 )
			throw std :: domain_error ( "big multibits" );
		if ( nBits < bitOffset ) {
			bitOffset -= nBits;
			return ( s.at ( byteOffset ) >> bitOffset ) & ( ( 1 << nBits ) - 1 );
		}
		unsigned v = s.at ( byteOffset ) & ( ( 1 << bitOffset ) - 1 );
		nBits -= bitOffset;
		bitOffset = 8;
		byteOffset++;
		while ( nBits >= 8 ) {
			v = ( v << 8 ) | static_cast < unsigned char > ( s.at ( byteOffset ) );
			byteOffset ++;
			nBits -= 8;
		}
		if ( nBits > 0 ) {
			bitOffset = 8 - nBits;
			v = ( v << nBits ) | ( static_cast < unsigned char > ( s.at ( byteOffset ) ) >> bitOffset );
		}
		return v;
	}
	string alignedStringDecode ( unsigned l ) {
		string :: size_type oldOffset = byteOffset;
		byteOffset += l;
		return string ( s, oldOffset, l );
	}
	ushort_string alignedShortStringDecode ( unsigned l ) {
		ushort_string t;
		t.reserve ( l );
		while ( l -- > 0 )
			t.push_back ( static_cast < unsigned short > ( ( s.at ( byteOffset ++ ) << 8 ) | s.at ( byteOffset ++ ) ) );
		return t;
	}
	string :: size_type getPosition ( ) const {
		return byteOffset;
	}
	void setPosition ( string :: size_type p ) {
		byteOffset = p;
		bitOffset = 8;
	}
};

class ostream {
	string s;
	string :: size_type byteOffset;
	unsigned bitOffset;
public:
	ostream ( ) : byteOffset ( 0 ), bitOffset ( 8 ) { }
	void initTpkt ( ) {
		s.assign ( "\x03\0\0\0", 4 );
		byteOffset = 4;
	}
	void finishTpkt ( ) {
		std :: size_t packetSize = s.size ( );
		s [ 2 ] = char ( packetSize >> 8 );
		s [ 3 ] = char ( packetSize );
	}
	void clear ( ) {
		s.clear ( );
		byteOffset = 0;
		bitOffset = 8;
	}
	void bitEncode ( bool v ) {
		if ( byteOffset == s.size ( ) )
			s.push_back ( 0 );
		bitOffset --;
		if ( v ) {
			char & c = s.at ( byteOffset );
			c = char ( c | 1 << bitOffset );
		}
		if ( ! bitOffset ) {
			bitOffset = 8;
			byteOffset ++;
		}
	}
	void byteAlign ( ) {
		if ( bitOffset != 8 ) {
			bitOffset = 8;
			byteOffset ++;
		}
	}
	void alignedByteEncode ( unsigned char v ) {
		s.push_back ( v );
		byteOffset ++;
	}
	void lengthEncode ( unsigned len, unsigned lower, unsigned upper ) {
		if ( upper < 65536 ) {
			unsignedEncode ( len, lower, upper );
			return;
		}
		byteAlign ( );
		if ( len < 128 ) {
			alignedByteEncode ( char ( len ) );
			return;
		}
		if ( len < 0x4000 ) {
			alignedByteEncode ( char ( len >> 8 | 128 ) );
			alignedByteEncode ( char ( len ) );
			return;
		}
		throw std :: runtime_error ( "not implemented" );
	}
	void unsignedEncode ( unsigned v, unsigned lower, unsigned upper ) {
		unsigned range = upper - lower;
		if ( ! range )
			return;
		if ( v < lower )
			throw std :: runtime_error ( "value out of range in unsignedEncode" );
//			v = 0;
		else
			v -= lower;
		if ( range >= 255 ) { // not 10.5.6 and not 10.5.7.1
			if ( range >= 65536 ) { //3-4 bytes
				unsigned numBytes = countBytes ( v ); // not 10.5.7.4
				lengthEncode ( numBytes, 1, 3 + ( range >= 65536 * 256 ) ); // 12.2.6
				multiByteEncode ( v, numBytes );
			} else {
				byteAlign ( ); // 10.7.5.2 - 10.7.5.4
				if ( range >= 256 ) // not 10.5.7.2
					alignedByteEncode ( char ( v >> 8 ) ); // 10.5.7.3
				alignedByteEncode ( char ( v ) );
			}
		} else {
			unsigned nBits = countBitsChar ( range );
			multiBitEncode ( v, nBits );
		}
	}
	void multiBitEncode ( unsigned v, unsigned nBits ) {
		if ( nBits == 0 )
			return;
		// Make sure value is in bounds of bit available.
		if ( nBits < sizeof ( unsigned ) * 8 )
			v &= ( 1 << nBits ) - 1;
		if ( nBits < bitOffset ) {
			if ( bitOffset == 8 ) {
				s.push_back ( char ( v << ( bitOffset -= nBits ) ) );
			} else {
				char & c = s.at ( byteOffset );
				c = char ( c | v << ( bitOffset -= nBits ) );
			}
			return;
		}
		nBits -= bitOffset;
		if ( bitOffset == 8 )
			s.push_back ( char ( v >> nBits ) );
		else {
			char & c = s.at ( byteOffset );
			c = char ( c | v >> nBits );
		}
		bitOffset = 8;
		byteOffset ++;
		while ( nBits >= 8 ) {
			nBits -= 8;
			s.push_back ( char ( v >> nBits ) );
			byteOffset ++;
		}
		if ( nBits > 0 ) {
			bitOffset = 8 - nBits;
			s.push_back ( char ( ( v & ( ( 1 << nBits ) - 1 ) ) << bitOffset ) );
		}
	}
	void multiByteEncode ( unsigned v, unsigned nBytes ) {
		byteAlign ( );
		alignedMultiByteEncode ( v, nBytes );
	}
	void alignedMultiByteEncode ( unsigned v, unsigned nBytes ) {
		switch ( nBytes ) {
			case 4:
				alignedByteEncode ( char ( v >> 24 ) );
			case 3:
				alignedByteEncode ( char ( v >> 16 ) );
			case 2:
				alignedByteEncode ( char ( v >> 8 ) );
			case 1:
				alignedByteEncode ( char ( v ) );
				return;
		}
		std :: ostringstream os;
		os << "unsupported multiByteEncode " << nBytes;
		throw std :: out_of_range ( os.str ( ) );
	}
	void smallUnsignedEncode ( unsigned v ) {
		if ( v < 64 ) {
			multiBitEncode ( v, 7 );
			return;
		}
		bitEncode ( true );
		byteAlign ( );
		char t [ 8 ];
		t [ 7 ] = char ( v );
		t [ 6 ] = char ( v >> 8 );
		t [ 5 ] = char ( v >> 16 );
		t [ 4 ] = char ( v >> 24 );
		unsigned l = countBytes ( v );
		char * p = t + 7 - l;
		* p = char ( l );
		alignedStringEncode ( p, l + 1 );
	}
	void alignedStringEncode ( const char * t, std :: size_t l ) {
		this -> s.append ( t, l );
		byteOffset += l;
	}
	void alignedStringEncode ( const string & t ) {
		s.append ( t );
		byteOffset += t.size ( );
	}
	void alignedShortStringEncode ( const ushort_string & t ) {
		ushort_string :: size_type sz = t.size ( );
		for ( ushort_string :: size_type i = 0; i < sz; i ++ ) {
			s.push_back ( char ( t [ i ] >> 8 ) );
			s.push_back ( char ( t [ i ] ) );
		}
		byteOffset += sz * 2;
	}
	const string & str ( ) const {
		return s;
	}
};

class Object : public Allocatable < __ASN_ALLOCATOR > {
public:
	virtual Object * clone ( ) const = 0;
	virtual ~Object ( ) { }
	virtual void encode ( ostream & os ) const = 0;
	virtual void printOn ( std :: ostream & os ) const = 0;
	void anyEncode ( ostream & os ) const {
		ostream t;
		encode ( t );
		const string & s = t.str ( );
		os.lengthEncode ( unsigned ( s.size ( ) ), 0, std :: numeric_limits < int > :: max ( ) );
//		os.byteAlign ( ); aligned in lengthEncode
		os.alignedStringEncode ( s );
	}
};

inline std :: ostream & operator<< ( std :: ostream & os, const Object & ob ) {
	ob.printOn ( os );
	return os;
}

template < class T > void assignCloned ( T * & d, T * s ) {
	T * t = s ? s -> clone ( ) : 0;
	delete d;
	d = t;
}

template < class T > void assignCopy ( T * & d, T * s ) {
	T * t = s ? new T ( * s ) : 0;
	delete d;
	d = t;
}

template < class T > void assignCloned ( const T * & d, T * s ) {
	T * t = s ? s -> clone ( ) : 0;
	delete d;
	d = t;
}

template < class T > void assignNew ( T * & d, T * s ) {
	delete d;
	d = s;
}

class Null : public Object {
public:
	Null * clone ( ) const {
		return new Null ( * this );
	}
	explicit Null ( istream & ) { }
	void encode ( ostream & ) const { }
	void printOn ( std :: ostream & os ) const {
		os << "<<null>>";
	}
	Null ( ) { }
};

class Boolean : public Object {
protected:
	bool val;
public:
	Boolean * clone ( ) const {
		return new Boolean ( * this );
	}
	explicit Boolean ( istream & is ) : val ( is.bitDecode ( ) ) { }
	Boolean ( bool v = false ) : val ( v ) { }
	operator bool() const {
		return val;
	}
	void encode ( ostream & os ) const {
		os.bitEncode ( val );
	}
	void printOn ( std :: ostream & os ) const {
		os << ( val ? "TRUE" : "FALSE" );
	}
};

enum ConstraintType {
	unconstrained,
	partiallyConstrained,
	fixedConstraint,
	extendableConstraint
};

class ConstrainedObject : public Object {
protected:
	ConstraintType constraint;
	int lowerLimit;
	unsigned upperLimit;
	ConstrainedObject ( ConstraintType c /*= unconstrained*/, int l /*= 0*/,
		unsigned u /*= std :: numeric_limits < unsigned > :: max ( )*/ ) :
		constraint ( c ), lowerLimit ( c == unconstrained ? 0 : l ),
		upperLimit ( c == unconstrained ? std :: numeric_limits < unsigned > :: max ( ) : u ) {
		if ( ( lowerLimit < 0 && upperLimit >= unsigned ( std :: numeric_limits < int > :: max ( ) ) ) ||
			( lowerLimit >= 0 && unsigned ( lowerLimit ) > upperLimit ) )
			throw std :: invalid_argument ( "invalid constraints" );
	}
	unsigned constrainedLengthDecode ( istream & is ) const {
		if ( ( constraint == extendableConstraint && is.bitDecode ( ) ) || constraint == unconstrained )
			return is.lengthDecode ( 0, std :: numeric_limits < int > :: max ( ) );
		return is.lengthDecode ( lowerLimit, upperLimit );
	}
	void constrainedLengthEncode ( ostream & os, unsigned l ) const {
		if ( constraintEncode ( os, l ) )
			os.lengthEncode ( l, 0, std :: numeric_limits < int > :: max ( ) );
		else
			os.lengthEncode ( l, lowerLimit, upperLimit );
	}
	bool constraintEncode ( ostream & os, unsigned v ) const {
		if ( constraint != extendableConstraint )
			return constraint == unconstrained;
		bool needsExtending = v > upperLimit;
		if ( ! needsExtending ) {
			if ( lowerLimit < 0 ) {
				if ( int ( v ) < lowerLimit )
					needsExtending = true;
			} else {
				if ( v < unsigned ( lowerLimit ) )
					needsExtending = true;
			}
		}
		os.bitEncode ( needsExtending );
		return needsExtending;
	}
};

class Integer : public ConstrainedObject {
protected:
	unsigned val;
	bool isUnsigned ( ) const {
		return constraint != unconstrained && lowerLimit >= 0;
	}
public:
	Integer * clone ( ) const {
		return new Integer ( * this );
	}
	Integer ( istream & is, ConstraintType c, int l, unsigned u ) :
		ConstrainedObject ( c, l, u ) {
		switch ( constraint ) {
			case fixedConstraint:
				break;
			case extendableConstraint:
				if ( ! is.bitDecode ( ) )
					break;
			default:
				unsigned len = is.lengthDecode ( 0, std :: numeric_limits < int > :: max ( ) );
				val = is.alignedMultiByteDecode ( len );
				if ( ! isUnsigned ( ) ) {
					len *= 8;
					if ( ( val & ( 1 << ( len - 1 ) ) ) != 0 )
						val |= std :: numeric_limits < unsigned > :: max ( ) << len;
				}
				val += lowerLimit;
				return;
		}
		val = is.unsignedDecode ( lowerLimit, upperLimit );
	}
	Integer ( unsigned v = 0, ConstraintType c = unconstrained, int l = 0,
		unsigned u = std :: numeric_limits < unsigned > :: max ( ) ) :
		ConstrainedObject ( c, l, u ) {
		operator= ( v );
	}
	operator unsigned ( ) const {
		return val;
	}
	Integer & operator= ( unsigned v ) {
		if ( constraint == unconstrained )
			val = v;
		else if ( lowerLimit >= 0 ) { // Is unsigned integer
			if ( v < unsigned ( lowerLimit ) )
				val = lowerLimit;
			else if ( v > upperLimit )
				val = upperLimit;
			else
				val = v;
		} else {
			int ival = int ( v );
			if ( ival < lowerLimit )
				val = lowerLimit;
			else if ( upperLimit < unsigned ( std :: numeric_limits < int > :: max ( ) ) ) {
				if ( ival > int ( upperLimit ) )
					val = upperLimit;
				else
					val = v;
			} else
				throw std :: domain_error ( "negative lower limit but upper limit > INT_MAX" );
		}
		return * this;
	}
	void encode ( ostream & os ) const {
		if ( constraintEncode ( os, val ) ) {
			// 12.2.6
			unsigned adjusted_value = val - lowerLimit;
			unsigned t;
			if ( isUnsigned ( ) || int ( adjusted_value ) > 0 )
				t = adjusted_value;
			else
				t = - int ( adjusted_value );
			if ( ! isUnsigned ( ) )
				t *= 2;
			unsigned nBytes = countBytes ( t );
			os.lengthEncode ( nBytes, 0, std :: numeric_limits < int > :: max ( ) );
			os.alignedMultiByteEncode ( adjusted_value, nBytes );
			return;
		}
		// 12.2.2 which devolves to 10.5
		os.unsignedEncode ( val, lowerLimit, upperLimit);
	}
	void printOn ( std :: ostream & os ) const {
		if ( constraint == unconstrained || lowerLimit < 0 )
			os << int ( val );
		else
			os << val;
	}
};

class Enumeration : public Object {
protected:
	unsigned maxEnumValue;
	unsigned val;
	bool extendable;
	virtual const NamesMapType & getNames ( ) const = 0;
public:
	Enumeration ( istream & is, unsigned m, bool e ) : maxEnumValue ( m ), extendable ( e ) {
		if ( extendable && is.bitDecode ( ) ) {
			unsigned len = is.smallUnsignedDecode ( );
			if ( len == 0 )
				throw std :: domain_error ( "zero enum len" );
			val = is.unsignedDecode ( 0, len - 1 );
			return;
		}
		val = is.unsignedDecode ( 0, maxEnumValue );
	}
	Enumeration ( unsigned v, unsigned m, bool e ) : maxEnumValue ( m ), val ( v ), extendable ( e ) { }
	operator unsigned ( ) const {
		return val;
	}
	Enumeration & operator= ( unsigned v ) {
		val = v;
		return * this;
	}
	void encode ( ostream & os ) const {
		if ( extendable ) { // 13.3
			bool extended = val > maxEnumValue;
			os.bitEncode ( extended );
			if ( extended ) {
				os.smallUnsignedEncode ( 1 + val );
				os.unsignedEncode ( val, 0, val );
				return;
			}
		}
		os.unsignedEncode ( val, 0, maxEnumValue ); // 13.2
	}
	void printOn ( std :: ostream & os ) const {
		const NamesMapType & names = getNames ( );
		NamesMapType :: const_iterator i = names.find ( val );
		if ( __builtin_expect ( i != names.end ( ), true ) )
			os << i -> second;
		else
			os << '<' << val << '>';
	}
};

class ObjectId : public Object {
protected:
	typedef std :: vector < unsigned, __ASN_ALLOCATOR < unsigned > > vector_type;
	vector_type val;
public:
	ObjectId * clone ( ) const {
		return new ObjectId ( * this );
	}
	explicit ObjectId ( istream & is ) {
		unsigned l = is.lengthDecode ( 0, 255 );
//		is.byteAlign ( ); aligned in lengthDecode
		if ( l == 0 ) {
			val.push_back ( 0 );
			val.push_back ( 0 );
			return;
		}
		unsigned subId;
		val.reserve ( l + 1 );
		val.push_back ( 0 );
		while ( l > 0 ) {
			unsigned b;
			subId = 0;
			do {
				b = is.alignedByteDecode ( );
				subId = ( subId << 7 ) + ( b & 0x7f );
				l --;
			} while ( ( b & 0x80 ) != 0 );
			val.push_back ( subId );
		}
		if ( l != 0 )
			throw std :: runtime_error ( "invalid objectid decode" );
		subId = val [ 1 ];
		val [ 0 ] = subId / 40;
		val [ 1 ] = subId % 40;
	}
	explicit ObjectId ( const string & dotstr ) {
		operator= ( dotstr );
	}
	ObjectId ( ) { }
	ObjectId & operator= ( const string & dotstr ) {
		val.clear ( );
		istringstream is ( dotstr );
		unsigned a = 0;
		while ( is >> a ) {
			char c;
			is >> c;
			val.push_back ( a );
		}
		return * this;
	}
	string str ( ) const {
		ostringstream os;
		for ( vector_type :: size_type i = 0, sz = val.size ( ); i < sz; i ++ ) {
			if ( i )
				os << '.';
			os << val [ i ];
		}
		return os.str ( );
	}
	void encode ( ostream & os ) const {
		string s;
		s.reserve ( 32 );
		vector_type :: size_type l = val.size ( );
		if ( l < 2 ) {
			// Thise case is really illegal, but we have to do SOMETHING
			throw std :: runtime_error ( "invalid objectid length" );
		}
		const unsigned * objId = & val [ 0 ];
		unsigned subId = ( objId [ 0 ] * 40 ) + objId [ 1 ];
		objId += 2;
		while ( -- l > 0 ) {
			if ( subId < 128 )
				s.push_back ( char ( subId ) );
			else {
				unsigned mask = 0x7F; /* handle subid == 0 case */
				int bits = 0;
				/* testmask *MUST* !!!! be of an unsigned type */
				unsigned testmask = 0x7F;
				int testbits = 0;
				while ( testmask != 0 ) {
					if ( subId & testmask ) {  /* if any bits set */
						mask = testmask;
						bits = testbits;
					}
					testmask <<= 7;
					testbits += 7;
				}
				/* mask can't be zero here */
				while ( mask != 0x7F ) {
					/* fix a mask that got truncated above */
					if ( mask == 0x1E00000 )
						mask = 0xFE00000;
					s.push_back ( char ( ( ( subId & mask ) >> bits ) | 0x80 ) );
					mask >>= 7;
					bits -= 7;
				}
				s.push_back ( char ( subId & mask ) );
			}
			if ( l > 1 )
				subId = * objId ++;
		}
		os.lengthEncode ( unsigned ( s.size ( ) ), 0, 255 );
//		os.byteAlign ( ); aligned in lengthEncode
		os.alignedStringEncode ( s );
	}
	void printOn ( std :: ostream & os ) const {
		for ( vector_type :: size_type i = 0, sz = val.size ( ); i < sz; i ++ ) {
			if ( i )
				os << '.';
			os << val [ i ];
		}
	}
};

class BitString : public ConstrainedObject {
protected:
	unsigned totalBits;
	string val;
public:
	BitString * clone ( ) const {
		return new BitString ( * this );
	}
	BitString ( istream & is, ConstraintType c, int l, unsigned u ) :
		ConstrainedObject ( c, l, u ), totalBits ( constrainedLengthDecode ( is ) ) {
		if ( totalBits > 16 ) {
			is.byteAlign ( );
			val = is.alignedStringDecode ( ( totalBits + 7 ) / 8 );
		} else if ( totalBits > 8 ) {
			val.reserve ( 2 );
			val.push_back ( char ( is.multiBitDecode ( 8 ) ) );
			val.push_back ( char ( is.multiBitDecode ( totalBits - 8 ) << ( 16 - totalBits ) ) );
		} else {
			val.reserve ( 1 );
			val.push_back ( char ( is.multiBitDecode ( totalBits ) << ( 8 - totalBits ) ) );
		}
	}
	explicit BitString ( unsigned tb = 0, const string & v = string ( ), ConstraintType c = unconstrained,
		int l = 0, unsigned u = std :: numeric_limits < unsigned > :: max ( ) ) :
		ConstrainedObject ( c, l, u ) {
		setValue ( tb, v );
	}
	void setValue ( unsigned tb, const string & v ) {
		if ( int ( tb ) < lowerLimit )
			tb = lowerLimit;
		if ( tb > upperLimit )
			tb = upperLimit;
		totalBits = tb;
		val = v;
		val.resize ( ( tb + 7 ) / 8, '\0' );
	}
	void encode ( ostream & os ) const {
		constrainedLengthEncode ( os, totalBits );
		if ( totalBits == 0 )
			return;
		if ( totalBits > 16 ) {
			os.byteAlign ( );
			os.alignedStringEncode ( val ); // 15.9
		} else if ( totalBits <= 8 ) // 15.8
			os.multiBitEncode ( val [ 0 ] >> ( 8 - totalBits ), totalBits );
		else {
			os.multiBitEncode ( val [ 0 ], 8);
			os.multiBitEncode ( val [ 1 ] >> ( 16 - totalBits ), totalBits - 8 );
		}
	}
	void printOn ( std :: ostream & os ) const {
		std :: streamsize indent = os.precision ( ) + 2;
		std :: _Ios_Fmtflags flags = os.flags ( );
		os << ' ' << totalBits << " bits {\n"
		   << std :: hex << std :: setfill ( '0' ) << std :: resetiosflags ( std :: ios :: floatfield )
		   << std :: setprecision ( int ( indent ) ) << std :: setw ( 16 ); //wtf .precision() is streamsize but setprecision() is int ??
		printNumbers ( os, val, int ( indent + 4 ) ); // same as above
		os << std :: dec << std :: setfill ( ' ' ) << std :: setw ( int ( indent - 1 ) ) << '}';
		os.flags ( flags );
	}
	const string & str ( ) const {
		return val;
	}
	unsigned size ( ) const {
		return totalBits;
	}
};

class OctetString : public ConstrainedObject {
protected:
	string val;
public:
	OctetString * clone ( ) const {
		return new OctetString ( * this );
	}
	explicit OctetString ( istream & is, ConstraintType c = unconstrained, int l = 0,
		unsigned u = std :: numeric_limits < unsigned > :: max ( ) ) :
		ConstrainedObject ( c, l, u ) {
		unsigned nBytes = constrainedLengthDecode ( is );
		if ( l == int ( u ) ) {
			switch ( nBytes ) {
				case 2:
					val.push_back ( char ( is.multiBitDecode ( 8 ) ) );
				case 1:
					val.push_back ( char ( is.multiBitDecode ( 8 ) ) );
				case 0:
					return;
			}
		}
		is.byteAlign ( );
		val = is.alignedStringDecode ( nBytes );
	}
	OctetString ( const string & v = string ( ), ConstraintType c = unconstrained, int l = 0,
		unsigned u = std :: numeric_limits < unsigned > :: max ( ) ) : ConstrainedObject ( c, l, u ) {
		operator= ( v );
	}
	operator const string & ( ) const {
		return val;
	}
	char operator[] ( std :: size_t i ) const {
		return val [ i ];
	}
	std :: size_t size ( ) const {
		return val.size ( );
	}
	const char * data ( ) const {
		return val.data ( );
	}
	OctetString & operator= ( const string & v ) {
		if ( v.size ( ) > upperLimit )
			val.assign ( v, 0, upperLimit );
		else {
			val = v;
			if ( int ( val.size ( ) ) < lowerLimit )
				val.resize ( lowerLimit, '\0' );
		}
		return * this;
	}
	void encode ( ostream & os ) const {
		constrainedLengthEncode ( os, unsigned ( val.size ( ) ) );
		if ( int ( upperLimit ) != lowerLimit ) {
			os.byteAlign ( );
			os.alignedStringEncode ( val );
			return;
		}
		switch ( val.size ( ) ) {
			case 0:
				return;
			case 1:
				os.multiBitEncode ( val [ 0 ], 8 );
				return;
			case 2:
				os.multiBitEncode ( val [ 0 ], 8 );
				os.multiBitEncode ( val [ 1 ], 8 );
				return;
		}
		os.byteAlign ( );
		os.alignedStringEncode ( val );
	}
	void printOn ( std :: ostream & os ) const {
		std :: streamsize indent = os.precision ( ) + 2;
		std :: _Ios_Fmtflags flags = os.flags ( );
		os << ' ' << val.size ( ) << " octets {\n"
		   << std :: hex << std :: setfill ( '0' ) << std :: resetiosflags ( std :: ios :: floatfield )
		   << std :: setprecision ( int ( indent ) ) << std :: setw ( 16 );
		printNumbers ( os, val, int ( indent + 4 ) );
		os << std :: dec << std :: setfill ( ' ' ) << std :: setw ( int ( indent - 1 ) ) << '}';
		os.flags ( flags );
	}
};

class ConstrainedString : public ConstrainedObject {
	unsigned canonicalBits;
	const char * characterSet;
	unsigned charSetAlignedBits;
protected:
	string val;
	ConstrainedString ( istream & is, unsigned cb, const char * cs, unsigned ab,
		ConstraintType c /*= unconstrained*/, int l /*= 0*/, unsigned u /*= std :: numeric_limits < unsigned > :: max ( )*/ ) :
		ConstrainedObject ( c, l, u ), canonicalBits ( cb ), characterSet ( cs ),
		charSetAlignedBits ( ab ) {
		unsigned len = constrainedLengthDecode ( is );
		if ( len == 0 )
			return;
		if ( constraint == unconstrained || upperLimit > 16 ||
			upperLimit * charSetAlignedBits > 15 + ( lowerLimit == int ( upperLimit ) ) ) {
			is.byteAlign ( );
			if ( charSetAlignedBits == 8 ) {
				val = is.alignedStringDecode ( len );
				return;
			}
		}
		val.reserve ( len );
		if ( charSetAlignedBits >= canonicalBits && canonicalBits > 4 ) {
			for ( unsigned i = 0; i < len; i ++ )
				val.push_back ( char ( is.multiBitDecode ( charSetAlignedBits ) ) );
		} else {
			for ( unsigned i = 0; i < len; i ++ )
				val.push_back ( characterSet [ is.multiBitDecode ( charSetAlignedBits ) ] );
		}
	}
	ConstrainedString ( const string & v, unsigned cb, const char * cs, unsigned ab,
		ConstraintType c /*= unconstrained*/, int l /*= 0*/, unsigned u /*= std :: numeric_limits < unsigned > :: max ( )*/ ) :
		ConstrainedObject ( c, l, u ), canonicalBits ( cb ), characterSet ( cs ),
		charSetAlignedBits ( ab ) {
		operator= ( v );
	}
	ConstrainedString & operator= ( const string & v ) {
		if ( v.size ( ) > upperLimit )
			val.assign ( v, 0, upperLimit );
		else {
			val = v;
			if ( int ( val.size ( ) ) < lowerLimit )
				val.resize ( lowerLimit, * characterSet );
		}
		return * this;
	}
	static string toLiteral ( const string & s ) {
		string t ( 1, '\"' );
		for ( string :: size_type i = 0, sz = s.size ( ); i < sz; i ++ ) {
			switch ( unsigned char c = s [ i ] ) {
				case '"':
					t += "\\\"";
					break;
				case '\a':
					t += "\\a";
					break;
				case '\b':
					t += "\\b";
					break;
				case '\f':
					t += "\\f";
					break;
				case '\n':
					t += "\\n";
					break;
				case '\r':
					t += "\\r";
					break;
				case '\t':
					t += "\\t";
					break;
				case '\v':
					t += "\\v";
					break;
				default:
					if ( std :: isprint ( c ) )
						t += c;
					else {
						t += '\\';
						t += char ( c / 64 + '0' );
						t += char ( ( c % 64 ) / 8 + '0' );
						t += char ( c % 8 + '0' );
						t += 'o';
					}
			}
		}
		t += '"';
		return t;
	}
public:
	operator const string & ( ) const {
		return val;
	}
	void encode ( ostream & os ) const {
		string :: size_type len = val.size ( );
		constrainedLengthEncode ( os, unsigned ( len ) );
		if ( len == 0 ) // 10.9.3.3
			return;
		if ( constraint == unconstrained || upperLimit > 16 ||
			upperLimit * charSetAlignedBits > 15 + ( lowerLimit == int ( upperLimit ) ) ) {
			// 26.5.7
			os.byteAlign ( );
			if ( charSetAlignedBits == 8 ) {
				os.alignedStringEncode ( val );
				return;
			}
		}
		if ( charSetAlignedBits >= canonicalBits && canonicalBits > 4 ) {
			for ( string :: size_type i = 0; i < len; i ++ )
				os.multiBitEncode ( val [ i ], charSetAlignedBits );
		} else {
			for ( string :: size_type i = 0; i < len; i ++ ) {
				const void * ptr = std :: memchr ( characterSet, val [ i ],
					1 << charSetAlignedBits );
				std :: ptrdiff_t pos = 0;
				if ( ptr != 0 )
					pos = ( static_cast < const char * > ( ptr ) - characterSet );
				os.multiBitEncode ( unsigned ( pos ), charSetAlignedBits );
			}
		}
	}
	void printOn ( std :: ostream & os ) const {
		os << toLiteral ( val );
	}
};

#define DECLARE_STRING_CLASS( name, cset ) \
static const char canonicalSet##name [ sizeof ( cset ) < 17 ? 16 : \
	( sizeof ( cset ) == 17 ? 17 : ( sizeof ( cset ) < 257 ? 256 : sizeof ( cset ) ) ) ] = cset; \
class name##String : public ConstrainedString { \
	public: \
	name##String * clone ( ) const { \
		return new name##String ( * this ); \
	} \
	name##String ( istream & is, ConstraintType c, int l, unsigned u ) : \
		ConstrainedString ( is, sizeof ( cset ) > 17 ? 8 : 4, canonicalSet##name, \
		sizeof ( cset ) > 17 ? 8 : 4, c, l, u ) { } \
	name##String ( istream & is, ConstraintType c, int l, unsigned u, const char * set, unsigned ab ) : \
		ConstrainedString ( is, sizeof ( cset ) > 17 ? 8 : 4, set, ab, c, l, u ) { } \
	name##String ( const string & v = string ( ), ConstraintType c = unconstrained, int l = 0, \
		unsigned u = std :: numeric_limits < unsigned > :: max ( ) ) \
		: ConstrainedString ( v, sizeof ( cset ) > 17 ? 8 : 4, canonicalSet##name, \
		sizeof ( cset ) > 17 ? 8 : 4, c, l, u ) { } \
	name##String ( const string & v, ConstraintType c, int l, unsigned u, const char * set, unsigned ab ) : \
		ConstrainedString ( v, sizeof ( cset ) > 17 ? 8 : 4, set, ab, c, l, u ) { } \
	name##String & operator= ( const string & v ) { \
		ConstrainedString :: operator= ( v ); \
		return * this; \
	} \
};

DECLARE_STRING_CLASS(Numeric,	" 0123456789")
DECLARE_STRING_CLASS(Printable, " '()+,-./0123456789:=?"
				"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
				"abcdefghijklmnopqrstuvwxyz")
DECLARE_STRING_CLASS(Visible,	" !\"#$%&'()*+,-./0123456789:;<=>?"
				"@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_"
				"`abcdefghijklmnopqrstuvwxyz{|}~")
DECLARE_STRING_CLASS(IA5,	"\000\001\002\003\004\005\006\007"
				"\010\011\012\013\014\015\016\017"
				"\020\021\022\023\024\025\026\027"
				"\030\031\032\033\034\035\036\037"
				" !\"#$%&'()*+,-./0123456789:;<=>?"
				"@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_"
				"`abcdefghijklmnopqrstuvwxyz{|}~\177")
DECLARE_STRING_CLASS(General,	"\000\001\002\003\004\005\006\007"
				"\010\011\012\013\014\015\016\017"
				"\020\021\022\023\024\025\026\027"
				"\030\031\032\033\034\035\036\037"
				" !\"#$%&'()*+,-./0123456789:;<=>?"
				"@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_"
				"`abcdefghijklmnopqrstuvwxyz{|}~\177"
				"\200\201\202\203\204\205\206\207"
				"\210\211\212\213\214\215\216\217"
				"\220\221\222\223\224\225\226\227"
				"\230\231\232\233\234\235\236\237"
				"\240\241\242\243\244\245\246\247"
				"\250\251\252\253\254\255\256\257"
				"\260\261\262\263\264\265\266\267"
				"\270\271\272\273\274\275\276\277"
				"\300\301\302\303\304\305\306\307"
				"\310\311\312\313\314\315\316\317"
				"\320\321\322\323\324\325\326\327"
				"\330\331\332\333\334\335\336\337"
				"\340\341\342\343\344\345\346\347"
				"\350\351\352\353\354\355\356\357"
				"\360\361\362\363\364\365\366\367"
				"\370\371\372\373\374\375\376\377")

#undef DECLARE_STRING_CLASS

/* not used in our asns and requires ptlib
class GeneralisedTime : public VisibleString {
public:
	GeneralisedTime * clone ( ) const {
		return new GeneralisedTime ( * this );
	}
	GeneralisedTime ( istream & is, ConstraintType c, int l, unsigned u ) : VisibleString ( is, c, l, u ) { }
	GeneralisedTime ( const PTime & time = PTime ( ) ) {
		operator= ( time );
	}
	GeneralisedTime & operator= ( const PTime & time ) {
		PString t = time.AsString ( "yyyyMMddhhmmss.uz" );
		t.Replace ( "GMT", "Z" );
		VisibleString :: operator= ( static_cast < const char * > ( t ) );
		return * this;
	}
	operator PTime ( ) const {
		PString value = val.c_str ( );
		int year = value ( 0, 3 ).AsInteger ( );
		int month = value ( 4, 5 ).AsInteger ( );
		int day = value ( 6, 7 ).AsInteger ( );
		int hour = value ( 8, 9 ).AsInteger ( );
		int minute = value ( 10, 11 ).AsInteger ( );
		int seconds = 0;
		int zonePos = 12;

		if ( isdigit ( value [ 12 ] ) ) {
			seconds = value ( 12, 13 ).AsInteger ( );
			if ( value [ 14 ] != '.' )
				zonePos = 14;
			else {
				zonePos = 15;
				while ( isdigit ( value [ zonePos ] ) )
					zonePos ++;
			}
		}

		int zone = PTime :: Local;
		switch ( value [ zonePos ] ) {
			case 'Z':
				zone = PTime :: UTC;
				break;
			case '+':
			case '-':
				zone = value ( zonePos + 1, zonePos + 2 ).AsInteger ( ) * 60 +
					value ( zonePos + 3, zonePos + 4 ).AsInteger ( );
		}
		return PTime ( seconds, minute, hour, day, month, year, zone );
	}
};
*/

class BMPString : public ConstrainedObject {
protected:
	ushort_string val;
public:
	BMPString * clone ( ) const {
		return new BMPString ( * this );
	}
	BMPString ( istream & is, ConstraintType c /*= unconstrained*/, int l /*= 0*/, unsigned u /*= std :: numeric_limits < unsigned > :: max ( )*/ ) :
		ConstrainedObject ( c, l, u ) {
		unsigned len = constrainedLengthDecode ( is );
		if ( constraint != unconstrained && upperLimit <= 1 ) {
			val.reserve ( len );
			for ( unsigned i = 0; i < len; i ++ )
				val.push_back ( char ( is.multiBitDecode ( 16 ) ) );
			return;
		}
		is.byteAlign ( );
		val = is.alignedShortStringDecode ( len );
	}
	BMPString ( const ushort_string & v = ushort_string ( ),
		ConstraintType c = unconstrained, int l = 0, unsigned u = std :: numeric_limits < unsigned > :: max ( ) ) :
		ConstrainedObject ( c, l, u ) {
		operator= ( v );
	}
	BMPString ( const string & v, ConstraintType c /*= unconstrained*/,
		int l /*= 0*/, unsigned u /*= std :: numeric_limits < unsigned > :: max ( )*/ ) :
		ConstrainedObject ( c, l, u ) {
		operator= ( v );
	}
	BMPString & operator= ( const ushort_string & v ) {
		if ( v.size ( ) > upperLimit )
			val.assign ( v, 0, upperLimit );
		else {
			val = v;
			if ( int ( val.size ( ) ) < lowerLimit )
				val.resize ( lowerLimit, 0 );
		}
		return * this;
	}
	BMPString & operator= ( const string & v ) {
		ushort_string ucs2;
		ucs2.reserve ( v.size ( ) );
		unsigned i = 0;
		while ( i < v.size ( ) ) {
			if ( ( v [ i ] & 0x80 ) == 0 )
				ucs2.push_back ( ( unsigned char ) v [ i ++ ] );
			else if ( ( v [ i ] & 0xe0 ) == 0xc0 ) {
				ucs2.push_back ( short ( ( ( v [ i ] & 0x1f ) << 6 ) | ( v.at ( i + 1 ) & 0x3f ) ) );
				i += 2;
			} else if ( ( v [ i ] & 0xf0 ) == 0xe0 ) {
				ucs2.push_back ( short ( ( ( v [ i ] & 0x0f ) << 12 ) |
					( ( v.at ( i + 1 ) & 0x3f ) << 6 ) | ( v.at ( i + 2 ) & 0x3f ) ) );
				i += 3;
			} else {
				ucs2.push_back ( 0xffff );
				i ++;
			}
		}
		return operator= ( ucs2 );
	}
	operator const ushort_string & ( ) const {
		return val;
	}
	string str ( ) const {
		string s;
		ushort_string :: size_type sz = val.size ( );
		s.reserve ( sz * 3 );
		for ( ushort_string :: size_type i = 0; i < sz; i ++ ) {
			unsigned v = val [ i ];
			if ( v < 0x80 )
				s.push_back ( char ( v ) );
			else if ( v < 0x800 ) {
				s.push_back ( char ( 0xc0 + ( v >> 6 ) ) );
				s.push_back ( char ( 0x80 + ( v & 0x3f ) ) );
			} else {
				s.push_back ( char ( 0xd0 + ( v >> 12 ) ) );
				s.push_back ( char ( 0x80 + ( ( v >> 6 ) & 0x3f ) ) );
				s.push_back ( char ( 0x80 + ( v & 0x3f ) ) );
			}
		}
		return s;
	}
	void encode ( ostream & os ) const {
		ushort_string :: size_type len = val.size ( );
		constrainedLengthEncode ( os, unsigned ( len ) );
		if ( constraint != unconstrained && upperLimit <= 1 ) {
			for ( ushort_string :: size_type i = 0; i < len; i ++ )
				os.multiBitEncode ( val [ i ], 16 );
			return;
		}
		os.byteAlign ( );
		os.alignedShortStringEncode ( val );
	}
	void printOn ( std :: ostream & os ) const {
		std :: streamsize indent = os.precision ( ) + 2;
		ushort_string :: size_type sz = val.size ( );
		os << ' ' << sz << " characters {\n";
		ushort_string :: size_type i = 0;
		while ( i < sz ) {
			os << std :: setw ( int ( indent ) ) << ' ' << std :: hex << std :: setfill ( '0' );
			for ( unsigned j = 0; j < 8; j ++ )
				if ( i + j < sz )
					os << std :: setw ( 4 ) << val [ i + j ] << ' ';
				else
					os << "     ";
			os << "  ";
			for ( unsigned j = 0; j < 8; j ++ ) {
				if ( i + j < sz ) {
					unsigned short c = val [ i + j ];
					if ( std :: isprint ( c ) )
						os << char ( c );
					else
						os << ' ';
				}
			}
			os << std :: dec << std :: setfill ( ' ' ) << '\n';
			i += 8;
		}
		os << std :: setw ( int ( indent - 1 ) ) << '}';
	}
};

class Choice : public Object {
protected:
	Object * choice;
	unsigned tag;
	unsigned numChoices;
	unsigned short nextPosition;
	bool extendable;
	Choice ( const Choice & c ) : Object ( c ), choice ( c.choice ? c.choice -> clone ( ) : 0 ), tag ( c.tag ),
		numChoices ( c.numChoices ), nextPosition ( c.nextPosition ), extendable ( c.extendable ) { }
	Choice ( istream & is, unsigned nc, bool e /*= false*/ ) : choice ( 0 ),
		numChoices ( nc ), nextPosition ( 0 ), extendable ( e ) {
		if ( extendable && is.bitDecode ( ) ) {
			tag = is.smallUnsignedDecode ( );
			tag += numChoices;
			nextPosition = short ( is.lengthDecode ( 0, std :: numeric_limits < int > :: max ( ) ) );
			nextPosition = short ( nextPosition + is.getPosition ( ) );
			return;
		}
		if ( numChoices < 2 ) {
			tag = 0;
			return;
		}
		tag = is.unsignedDecode ( 0, numChoices - 1 );
		return;
	}
	Choice ( unsigned nc, bool e /*= false*/ ) : choice ( 0 ),
		numChoices ( nc ), nextPosition ( 0 ), extendable ( e ) { }
	virtual const NamesMapType & getNames ( ) const = 0;
	Choice & operator= ( const Choice & c ) {
		assignCloned ( choice, c.choice );
		tag = c.tag;
		return * this;
	}
	void setTag ( unsigned t, Object * o ) {
		delete choice;
		choice = o;
		tag = t;
	}
public:
	void encode ( ostream & os ) const {
		if ( extendable ) {
			bool extended = tag >= numChoices;
			os.bitEncode ( extended );
			if ( extended ) {
				os.smallUnsignedEncode ( tag - numChoices );
				choice -> anyEncode ( os );
				return;
			}
		}
		if ( numChoices > 1 )
			os.unsignedEncode ( tag, 0, numChoices - 1 );
		choice -> encode ( os );
	}
	unsigned getTag ( ) const {
		return tag;
	}
	~Choice ( ) {
		delete choice;
	}
	void printTagName ( std :: ostream & os ) const {
		const NamesMapType & names = getNames ( );
		NamesMapType :: const_iterator i = names.find ( tag );
		if ( __builtin_expect ( i != names.end ( ), true ) )
			os << i -> second;
		else
			os << '<' << tag << '>';
	}
	void printOn ( std :: ostream & os ) const {
		printTagName ( os );
		os << ' ' << * choice;
	}
};

class Sequence : public Object {
protected:
	class OptionMap {
	protected:
		typedef unsigned val_type;
		val_type val;
		static const val_type one = 1;
		unsigned totalBits;
	public:
		OptionMap ( istream & is, unsigned n ) : val ( 0 ), totalBits ( n ) {
			if ( totalBits > sizeof ( val ) * 8 )
				throw std :: domain_error ( "too long optionmap" );
			if ( totalBits <= 16 ) {
				val = is.multiBitDecode ( totalBits ) << ( sizeof ( val ) * 8 - totalBits );
				return;
			}
			is.byteAlign ( );
			switch ( ( totalBits - 1 ) >> 3 ) {
				case 7:
					val = is.alignedByteDecode ( );
					val <<= 8;
				case 6:
					val |= is.alignedByteDecode ( );
					val <<= 8;
				case 5:
					val |= is.alignedByteDecode ( );
					val <<= 8;
				case 4:
					val |= is.alignedByteDecode ( );
					val <<= 8;
				case 3:
					val |= is.alignedByteDecode ( );
					val <<= 8;
			}
			val |= is.alignedByteDecode ( );
			val <<= 8;
			val |= is.alignedByteDecode ( );
			val <<= 8;
			val |= is.alignedByteDecode ( );
			val &= std :: numeric_limits < unsigned > :: max ( ) << ( sizeof ( val ) * 8 - totalBits );
		}
		explicit OptionMap ( unsigned n ) : val ( 0 ), totalBits ( n ) {
			if ( totalBits > sizeof ( val ) * 8 )
				throw std :: domain_error ( "too long optionmap" );
		}
		bool operator[] ( unsigned i ) const {
			return ( val >> ( sizeof ( val ) * 8 - 1 - i ) ) & 1;
		}
		void set ( unsigned i ) {
			if ( i >= totalBits )
				throw std :: out_of_range ( "too big index in optionmap" );
			val |= one << ( sizeof ( val ) * 8 - 1 - i );
		}
		void clear ( unsigned i ) {
			if ( i >= totalBits )
				throw std :: out_of_range ( "too big index in optionmap" );
			val &= ~ ( one << ( sizeof ( val ) * 8 - 1 - i ) );
		}
		bool empty ( ) const {
			return val == 0;
		}
		unsigned size ( ) const {
			return totalBits;
		}
		void encode ( ostream & os ) const {
			if ( totalBits <= 16 ) {
				os.multiBitEncode ( val >> ( sizeof ( val ) * 8 - totalBits ), totalBits );
				return;
			}
			os.byteAlign ( );
			unsigned shiftBits = sizeof ( val ) * 8 - 8;
			os.alignedByteEncode ( char ( val >> shiftBits ) );
			shiftBits -= 8;
			os.alignedByteEncode ( char ( val >> shiftBits ) );
			shiftBits -= 8;
			os.alignedByteEncode ( char ( val >> shiftBits ) );
			switch ( ( totalBits - 1 ) >> 3 ) {
				case 7:
					shiftBits -= 8;
					os.alignedByteEncode ( char ( val >> shiftBits ) );
				case 6:
					shiftBits -= 8;
					os.alignedByteEncode ( char ( val >> shiftBits ) );
				case 5:
					shiftBits -= 8;
					os.alignedByteEncode ( char ( val >> shiftBits ) );
				case 4:
					shiftBits -= 8;
					os.alignedByteEncode ( char ( val >> shiftBits ) );
				case 3:
					shiftBits -= 8;
					os.alignedByteEncode ( char ( val >> shiftBits ) );
			}
		}
	};

	class ExtensionMap {
	protected:
		typedef unsigned val_type;
		val_type val;
		static const val_type one = 1;
		unsigned totalBits;
		static unsigned trailingZeroes ( unsigned v ) {
			return __builtin_ctz ( v );
		}
		static unsigned trailingZeroes ( unsigned long v ) {
			return __builtin_ctzl ( v );
		}
		static unsigned trailingZeroes ( unsigned long long v ) {
			return __builtin_ctzll ( v );
		}
	public:
		explicit ExtensionMap ( istream & is ) {
			totalBits = is.smallUnsignedDecode ( ) + 1;
			if ( totalBits > sizeof ( val ) * 8 )
				throw std :: runtime_error ( "too big extension map" );
			val = is.multiBitDecode ( totalBits ) << ( sizeof ( val ) * 8 - totalBits );
		}
		ExtensionMap ( ) : val ( 0 ), totalBits ( 0 ) { }
		bool operator[] ( unsigned i ) const {
			return ( val >> ( sizeof ( val ) * 8 - 1 - i ) ) & 1;
		}
		void set ( unsigned i ) {
			if ( i >= sizeof ( val ) * 8 )
				throw std :: out_of_range ( "too big index in extensionmap" );
			val |= ( one << ( sizeof ( val ) * 8 - 1 - i ) );
			if ( totalBits <= i )
				totalBits = i + 1;
		}
		void clear ( unsigned i ) {
			if ( i >= sizeof ( val ) * 8 )
				return;
			val &= ~ ( one << ( sizeof ( val ) * 8 - 1 - i ) );
		}
		bool empty ( ) const {
			return val == 0;
		}
		unsigned size ( ) const {
			return totalBits;
		}
		void encode ( ostream & os ) const {
			val_type bitsLeft = val_type ( sizeof ( val ) * 8 - trailingZeroes ( val ) );
			os.smallUnsignedEncode ( bitsLeft - 1 );
			os.multiBitEncode ( val >> ( sizeof ( val ) * 8 - bitsLeft ), bitsLeft );
		}
	};

	bool extendable;
	mutable unsigned short nextExtensionPosition;
	unsigned knownExtensions;
	OptionMap optionMap;
	ExtensionMap extensionMap;
	typedef std :: vector < const Object *, __ASN_ALLOCATOR < const Object * > > FieldVector;
	FieldVector fields;
	Sequence & operator= ( const Sequence & s ) {
		extendable = s.extendable;
		nextExtensionPosition = s.nextExtensionPosition;
		optionMap = s.optionMap;
		extensionMap = s.extensionMap;
		FieldVector :: size_type fs = fields.size ( );
		FieldVector :: size_type sfs = s.fields.size ( );
		for ( FieldVector :: size_type i = sfs; i < fs; i ++ )
			delete fields [ i ];
		fields.resize ( sfs );
		for ( FieldVector :: size_type i = 0; i < sfs; i ++ )
			assignCloned ( fields [ i ], s.fields [ i ] );
		return * this;
	}
	Sequence ( const Sequence & s ) : Object ( s ), extendable ( s.extendable ),
		nextExtensionPosition ( s.nextExtensionPosition ), knownExtensions ( s.knownExtensions ),
		optionMap ( s.optionMap ), extensionMap ( s.extensionMap ), fields ( s.fields.size ( ) ) {
		FieldVector :: size_type sz = fields.size ( );
		try {
			for ( FieldVector :: size_type i = 0; i < sz; i ++ ) {
				if ( s.fields [ i ] )
					fields [ i ] = s.fields [ i ] -> clone ( );
			}
		} catch ( ... ) {
			for ( FieldVector :: size_type i = 0; i < sz; i ++ )
				delete fields [ i ];
			throw;
		}
	}
	Sequence ( istream & is, unsigned nOpts, bool extend, unsigned nExtend ) :
		extendable ( extend ), nextExtensionPosition ( short ( extend ? is.bitDecode ( ) : 0 ) ),
		knownExtensions ( nExtend ), optionMap ( is, nOpts ) { }
	Sequence ( unsigned nOpts, bool extend, unsigned nExtend ) :
		extendable ( extend ), nextExtensionPosition ( 0 ),
		knownExtensions ( nExtend ), optionMap ( nOpts ) { }
	void includeOptionalField ( unsigned f ) {
		if ( f < optionMap.size ( ) )
			optionMap.set ( f );
		else
			extensionMap.set ( f - optionMap.size ( ) );
	}
	void removeOptionalField ( unsigned f ) {
		if ( f < optionMap.size ( ) )
			optionMap.clear ( f );
		else
			extensionMap.clear ( f - optionMap.size ( ) );
	}
	bool extensionMapDecode ( istream & is ) {
		if ( nextExtensionPosition == 1 ) {
			extensionMap = ExtensionMap ( is );
			is.byteAlign ( );
			nextExtensionPosition = short ( is.getPosition ( ) );
			return true;
		}
		return false;
	}
	bool hasKnownExtensionToDecode ( istream & is, unsigned f ) {
		if ( ! extensionMap [ f - optionMap.size ( ) ] )
			return false;
		is.setPosition ( nextExtensionPosition );
		nextExtensionPosition = short ( is.lengthDecode ( 0, std :: numeric_limits < int > :: max ( ) ) );
		nextExtensionPosition = short ( nextExtensionPosition + is.getPosition ( ) );
		return true;
	}
	void unknownExtensionsDecode ( istream & is ) {
		if ( ! fields.empty ( ) )
			throw std :: logic_error ( "not empty fields in unknownExtensionsDecode" );
		is.setPosition ( nextExtensionPosition );
		if ( extensionMap.size ( ) <= knownExtensions )
			return;
		fields.reserve ( extensionMap.size ( ) - knownExtensions );
		for ( unsigned i = knownExtensions, sz = extensionMap.size ( ); i < sz; i ++ ) {
			if ( extensionMap [ i ] )
				fields.push_back ( new OctetString ( is, unconstrained, 0,
					std :: numeric_limits < int > :: max ( ) ) );
			else
				fields.push_back ( 0 );
		}
	}
	~Sequence ( ) {
		for ( FieldVector :: size_type i = 0, sz = fields.size ( ); i < sz; i ++ )
			delete fields [ i ];
	}
	void unknownExtensionsEncode ( ostream & os ) const {
		for ( FieldVector :: size_type i = 0, sz = fields.size ( ); i < sz; i ++ )
			if ( extensionMap [ unsigned ( i + knownExtensions ) ] )
				fields [ i ] -> encode ( os );
	}
	void preambleEncode ( ostream & os ) const {
		if ( extendable )
			os.bitEncode ( ! extensionMap.empty ( ) );
		optionMap.encode ( os );
	}
	bool extensionMapEncode ( ostream & os ) const {
		if ( ! extensionMap.empty ( ) ) {
			extensionMap.encode ( os );
			return true;
		}
		return false;
	}
	void knownExtensionEncode ( ostream & os, unsigned f, const Object * o ) const {
		if ( extensionMap [ f - optionMap.size ( ) ] )
			o -> anyEncode ( os );
	}
	void unknownExtensionsPrint ( std :: ostream & os ) const {
		for ( FieldVector :: size_type i = 0, sz = fields.size ( ); i < sz; i ++ ) {
			if ( extensionMap [ unsigned ( i + knownExtensions ) ] )
				os << "field[" << i << "] = " << * fields [ i ] << std :: endl;
		}
	}
	template < class T > T * pushTemporaryMember ( T * o ) {
		try {
			fields.push_back ( o );
			return o;
		} catch ( ... ) {
			delete o;
			throw;
		}
	}
	void clearTemporaryMembers ( ) {
		FieldVector ( ).swap ( fields );
	}
public:
	bool hasOptionalField ( unsigned t ) const {
		if ( t < optionMap.size ( ) )
			return optionMap [ t ];
		return extensionMap [ t - optionMap.size ( ) ];
	}
};

class Array : public ConstrainedObject {
protected:
	typedef std :: vector < Object *, __ASN_ALLOCATOR < Object * > > VectorType;
	VectorType val;
	Array ( const Array & a ) : ConstrainedObject ( a ), val ( a.val.size ( ) ) {
		VectorType :: size_type vs = a.val.size ( );
		try {
			for ( VectorType :: size_type i = 0; i < vs; i ++ )
//				if ( a.val [ i ] )
				val [ i ] = a.val [ i ] -> clone ( );
		} catch ( ... ) {
			for ( VectorType :: size_type i = 0; i < vs; i ++ )
				delete val [ i ];
			throw;
		}
	}
	~Array ( ) {
		for ( VectorType :: size_type i = 0, vs = val.size ( ); i < vs; i ++ )
			delete val [ i ];
	}
	typedef Object * ( Creator ) ( istream & is );
	typedef Object * ( CreatorDefault ) ( );
	Array ( istream & is, Creator cr, ConstraintType c, int l, unsigned u ) :
		ConstrainedObject ( c, l, u ), val ( constrainedLengthDecode ( is ) ) {
		VectorType :: size_type vs = val.size ( );
		try {
			for ( VectorType :: size_type i = 0; i < vs; i ++ )
				val [ i ] = cr ( is );
		} catch ( ... ) {
			for ( VectorType :: size_type i = 0; i < vs; i ++ )
				delete val [ i ];
			throw;
		}
	}
	Array ( unsigned s, CreatorDefault cr, ConstraintType c, int l, unsigned u ) :
		ConstrainedObject ( c, l, u ), val ( s ) {
		try {
			for ( unsigned i = 0; i < s; i ++ )
				val [ i ] = cr ( );
		} catch ( ... ) {
			for ( unsigned i = 0; i < s; i ++ )
				delete val [ i ];
			throw;
		}
	}
	Array ( CreatorDefault, ConstraintType c, int l, unsigned u ) :
		ConstrainedObject ( c, l, u ) {	}
	Array & operator= ( const Array & a ) {
		VectorType :: size_type avs = a.val.size ( );
		VectorType :: size_type vs = val.size ( );
		for ( VectorType :: size_type i = avs; i < vs; i ++ )
			delete val [ i ];
		val.resize ( avs );
		for ( VectorType :: size_type i = 0; i < avs; i ++ )
			assignCloned ( val [ i ], a.val [ i ] );
		return * this;
	}
	virtual Object * createObjectVirt ( ) const = 0;
	const Object & operator[] ( std :: size_t i ) const {
		return * val.at ( i );
	}
	Object & operator[] ( std :: size_t i ) {
		return * val.at ( i );
	}
	void push_back ( Object * o ) {
		try {
			val.push_back ( o );
		} catch ( ... ) {
			delete o;
			throw;
		}
	}
public:
	void encode ( ostream & os ) const {
		constrainedLengthEncode ( os, unsigned ( val.size ( ) ) );
		for ( VectorType :: size_type i = 0, vs = val.size ( ); i < vs; i ++ )
			val [ i ] -> encode ( os );
	}
	void clear ( ) {
		for ( VectorType :: size_type i = 0, vs = val.size ( ); i < vs; i ++ )
			delete val [ i ];
		val.clear ( );
	}
	void erase ( std :: size_t i ) {
		delete val [ i ];
		val.erase ( val.begin ( ) + i );
	}
	void setSize ( std :: size_t n ) {
		VectorType :: size_type oldSize = val.size ( );
		for ( VectorType :: size_type i = n; i < oldSize; i ++ )
			delete val [ i ];
		val.resize ( n );
		for ( VectorType :: size_type i = oldSize; i < n; i ++ )
			val [ i ] = createObjectVirt ( );
	}
	std :: size_t size ( ) const {
		return val.size ( );
	}
	bool empty ( ) const {
		return val.empty ( );
	}
	void reserve ( std :: size_t n ) {
		val.reserve ( n );
	}
	void printOn ( std :: ostream & os ) const {
		std :: streamsize indent = os.precision ( ) + 2;
		os << val.size ( ) << " entries {\n";
		for ( VectorType :: size_type i = 0, vs = val.size ( ); i < vs; i ++ )
			os << std :: setw ( int ( indent + 1 ) ) << '[' << i << "]=" << std :: setprecision ( int ( indent ) ) << * val [ i ] << '\n';
		os << std :: setw ( int ( indent - 1 ) ) << '}';
	}
};

struct NamesPair {
	unsigned first;
	const char * second;
	operator std :: pair < const unsigned, string > ( ) const {
		return std :: pair < const unsigned, string > ( first, second );
	}
};

}

#endif
