#pragma implementation
#include "ss.hpp"
#include "allocatable.hpp"
#include <iomanip>
#include <stdexcept>
#include <limits>
#include <cstring>
#include "asn.hpp"
#include "h225.hpp"
#include <ptlib.h>
#include "h323.hpp"
#include <ptlib/sockets.h>
#include "automutex.hpp"
#include "random.hpp"

namespace H323 {
static bool isE164 ( const ss :: string & str ) {
	static ss :: string testChars ( "1234567890*#" );
	return ! str.empty ( ) && str.find_first_not_of ( testChars ) == ss :: string :: npos;
}

void setAliasAddress ( const ss :: string & name, H225 :: AliasAddress & alias ) {
	setAliasAddress ( name, alias, isE164 ( name ) ? H225 :: AliasAddress :: e_dialedDigits : H225 :: AliasAddress :: e_h323_ID );
}

void setAliasAddress ( const ss :: string & name, H225 :: AliasAddress & alias, H225 :: AliasAddress :: Choices tag ) {
	// See if explicitly specified the alias type, otherwise guess it from
	// the string, if all digits then assume an e164 address.
	alias.setTag ( tag );
	switch ( alias.getTag ( ) ) {
		case H225 :: AliasAddress :: e_dialedDigits:
			static_cast < H225 :: AliasAddress_dialedDigits & > ( alias ) = name;
			break;
		case H225 :: AliasAddress :: e_url_ID:
			static_cast < H225 :: AliasAddress_url_ID & > ( alias ) = name;
			break;
		case H225 :: AliasAddress :: e_email_ID:
			static_cast < H225 :: AliasAddress_email_ID & > ( alias ) = name;
			break;
		case H225 :: AliasAddress :: e_h323_ID:
			static_cast < H225 :: AliasAddress_h323_ID & > ( alias ) = name;
			break;
/*		case H225 :: AliasAddress :: e_transportID: {
			H323TransportAddress addr = name;
			addr.SetPDU(alias);
			break;
		}
		case H225 :: AliasAddress :: e_partyNumber: {
			H225 :: PartyNumber & party = alias;
			if (strncmp(name, E164NumberPrefix, sizeof(E164NumberPrefix)-1) == 0) {
        party.SetTag(H225_PartyNumber::e_e164Number);
        H225_PublicPartyNumber & number = party;
        number.m_publicNumberDigits = name.Mid(sizeof(E164NumberPrefix)-1);
      }
      else if (strncmp(name, PrivatePartyPrefix, sizeof(PrivatePartyPrefix)-1) == 0) {
        party.SetTag(H225_PartyNumber::e_privateNumber);
        H225_PrivatePartyNumber & number = party;
        number.m_privateNumberDigits = name.Mid(sizeof(PrivatePartyPrefix)-1);
      }
      else if (strncmp(name, DataPartyPrefix, sizeof(DataPartyPrefix)-1) == 0) {
        party.SetTag(H225_PartyNumber::e_dataPartyNumber);
        (H225_NumberDigits &)party = name.Mid(sizeof(DataPartyPrefix)-1);
      }
      else if (strncmp(name, TelexPartyPrefix, sizeof(TelexPartyPrefix)-1) == 0) {
        party.SetTag(H225_PartyNumber::e_telexPartyNumber);
        (H225_NumberDigits &)party = name.Mid(sizeof(TelexPartyPrefix)-1);
      }
      else if (strncmp(name, NSPNumberPrefix, sizeof(NSPNumberPrefix)-1) == 0) {
        party.SetTag(H225_PartyNumber::e_nationalStandardPartyNumber);
        (H225_NumberDigits &)party = name.Mid(sizeof(NSPNumberPrefix)-1);
      }
    }*/

		default:
			throw std :: domain_error ( "unsupported alias address tag" );
	}
}

ss :: string getAliasAddressString ( const H225 :: AliasAddress & alias ) {
	switch ( alias.getTag ( ) ) {
		case H225 :: AliasAddress :: e_dialedDigits:
			return static_cast < const H225 :: AliasAddress_dialedDigits & > ( alias );
		case H225 :: AliasAddress :: e_url_ID:
			return static_cast < const H225 :: AliasAddress_url_ID & > ( alias );
		case H225 :: AliasAddress :: e_email_ID:
			return static_cast < const H225 :: AliasAddress_email_ID & > ( alias );
		case H225 :: AliasAddress :: e_h323_ID:
			return static_cast < const H225 :: AliasAddress_h323_ID & > ( alias ).str ( );
/*		case H225 :: AliasAddress :: e_transportID:
			return H323TransportAddress(alias);
		case H225 :: AliasAddress :: e_partyNumber: {
			const H225 :: PartyNumber & party = alias;
			switch ( party.getTag ( ) ) {
				case H225 :: PartyNumber :: e_e164Number: {
					const H225 :: PublicPartyNumber & number = party;
					return E164NumberPrefix + (PString)number.m_publicNumberDigits;
				} case H225 :: PartyNumber :: e_privateNumber: {
					const H225 :: PrivatePartyNumber & number = party;
					return PrivatePartyPrefix + (PString)number.m_privateNumberDigits;
				} case H225 :: PartyNumber :: e_dataPartyNumber:
					return DataPartyPrefix + (PString)(const H225_NumberDigits &)party;
				case H225 :: PartyNumber :: e_telexPartyNumber:
					return TelexPartyPrefix + (PString)(const H225_NumberDigits &)party;
				case H225 :: PartyNumber :: e_nationalStandardPartyNumber:
					return NSPNumberPrefix + (PString)(const H225_NumberDigits &)party;
			}
			break;
		}*/
		default:
			break;
	}
	return "";
}

ss :: string globallyUniqueId ( ) {
	unsigned long long timestamp;
	static unsigned long long deltaTime = ( unsigned long long ) 10000000 * 24 * 60 * 60 *
		( 16 // Days from 15th October
		+ 31 // Days in December 1583
		+ 30 // Days in November 1583
		+ ( 1970 - 1583 ) * 365 // Days in years
		+ ( 1970 - 1583 ) / 4 // Leap days
		- 3 ); // Allow for 1700, 1800, 1900 not leap years
	struct timeval tv;
	gettimeofday ( & tv, 0 );
	timestamp = ( tv.tv_sec * ( unsigned long long ) 1000000 + tv.tv_usec ) * 10;
	timestamp += deltaTime;

	static short clockSequence = short ( Random :: number ( ) );
	static unsigned long long lastTimestamp = 0;
	static PEthSocket :: Address macAddress;
	static bool needMacAddress = true;
	static PMutex mut;
	{
		AutoMutex am ( mut );
		if ( lastTimestamp < timestamp )
			lastTimestamp = timestamp;
		else
			clockSequence ++;
		if ( needMacAddress ) {
			PIPSocket :: InterfaceTable interfaces;
			if ( PIPSocket :: GetInterfaceTable ( interfaces ) ) {
				for ( PINDEX i = 0; i < interfaces.GetSize ( ); i ++ ) {
					PString macAddrStr = interfaces [ i ].GetMACAddress ( );
					if ( ! macAddrStr && macAddrStr != "44-45-53-54-00-00") { /* not Win32 PPP device */
						macAddress = macAddrStr;
						if ( macAddress != NULL ) {
							needMacAddress = false;
							break;
						}
					}
				}
			}
			if ( needMacAddress ) {
				macAddress.ls.l = Random :: number ( );
				macAddress.ls.s = (WORD) Random :: number ( );
				macAddress.b [ 0 ] = char ( macAddress.b [ 0 ] | '\x80' );
				needMacAddress = false;
			}
		}
	}
	ss :: string s;
	s.push_back ( char ( timestamp ) );
	s.push_back ( char ( timestamp >> 8 ) );
	s.push_back ( char ( timestamp >> 16 ) );
	s.push_back ( char ( timestamp >> 24 ) );
	s.push_back ( char ( timestamp >> 32 ) );
	s.push_back ( char ( timestamp >> 40 ) );
	s.push_back ( char ( timestamp >> 48 ) );
	s.push_back ( char ( ( ( timestamp >> 56 ) & 0x0f ) + 0x10 ) ); // Version number is 1

	s.push_back ( char ( ( ( clockSequence >> 8 ) & 0x1f ) | 0x80 ) ); // DCE compatible GUID
	s.push_back ( char ( clockSequence ) );

	s.push_back ( macAddress.b [ 0 ] );
	s.push_back ( macAddress.b [ 1 ] );
	s.push_back ( macAddress.b [ 2 ] );
	s.push_back ( macAddress.b [ 3 ] );
	s.push_back ( macAddress.b [ 4 ] );
	s.push_back ( macAddress.b [ 5 ] );
	return s;
}

ss :: string printableCallId ( const ss :: string & confId ) {
	if ( confId.size ( ) != 16 )
		return confId;
	ss :: ostringstream os;
	typedef unsigned char uchar;
	int a = uchar ( confId [ 0 ] ) * 0x1000000 + uchar ( confId [ 1 ] ) * 0x10000 + uchar ( confId [ 2 ] ) * 0x100 + uchar ( confId [ 3 ] );
	os << std :: hex << a << '-';
	a = uchar ( confId [ 4 ] ) * 0x1000000 + uchar ( confId [ 5 ] ) * 0x10000 + uchar ( confId [ 6 ] ) * 0x100 + uchar ( confId [ 7 ] );
	os << std :: hex << a << '-';
	a = uchar ( confId [ 8 ] ) * 0x1000000 + uchar ( confId [ 9 ] ) * 0x10000 + uchar ( confId [ 10 ] ) * 0x100 + uchar ( confId [ 11 ] );
	os << std :: hex << a << '-';
	a = uchar ( confId [ 12 ] ) * 0x1000000 + uchar ( confId [ 13 ] ) * 0x10000 + uchar ( confId [ 14 ] ) * 0x100 + uchar ( confId [ 15 ] );
	os << std :: hex << a;
	return os.str ( );
}

}
