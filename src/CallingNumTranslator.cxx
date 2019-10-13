#pragma implementation
#include "ss.hpp"
#include "CallingNumTranslator.hpp"
#include "allocatable.hpp"
#include "pointer.hpp"
#include "ipport.hpp"
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/sequenced_index.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include "codecinfo.hpp"
#include "signallingoptions.hpp"
#include "latencylimits.hpp"
#include "profitguard.hpp"
#include "tarifround.hpp"
#include "tarifinfo.hpp"
#include "pricedata.hpp"
#include "outcarddetails.hpp"
#include "outchoice.hpp"
#include "priceelement.hpp"
#include "icqcontact.hpp"
#include "radgwinfo.hpp"
#include "sourcedata.hpp"
#include <boost/regex.hpp>
#include "mysql.hpp"
#include <ptlib.h>
#include "aftertask.hpp"
#include "moneyspool.hpp"
#include "outpriceelement.hpp"
#include "customercalls.hpp"
#include "registeredcard.hpp"
#include "RegisteredOutPeersMap.hpp"
#include <boost/multi_index/member.hpp>
#include "callbuffer.hpp"
#include "mgcpconf.hpp"
#include "confdata.hpp"
#include <ptlib.h>
#include <ptlib/svcproc.h>

CallingNumTranslator :: CallingNumTranslator ( ) { }

CallingNumTranslator :: ~CallingNumTranslator ( ) { }

void CallingNumTranslator :: translate ( const InPeerInfo * ip, const OutPeerInfo * op, ss :: string & anum ) {
	_translateForIn ( ip, op, anum );
	_translateForOut ( op, anum );
}

void CallingNumTranslator :: translate ( const CardInfo * ci, const OutPeerInfo * op, ss :: string & anum ) {
	_translateForIn ( ci, op, anum );
	_translateForOut ( op, anum );
}

void CallingNumTranslator :: _translateForIn ( const InPeerInfo * ip, const OutPeerInfo * op, ss :: string & anum ) {
	PSYSTEMLOG ( Info, "(storm): CallingNumTranslator : translate for InPeer. Anum=" << anum );
	if ( ! ip ) { // no InPeer
		PSYSTEMLOG ( Info, "(storm): CallingNumTranslator : InPeer not found. Skipped" );
		return;
	}
	PSYSTEMLOG ( Info, "(storm): CallingNumTranslator : InPeer " << ip -> name << " found" );
	int outGroup = _getOutGroup ( op );
	PSYSTEMLOG ( Info, "(storm): CallingNumTranslator : OutGroup = " << outGroup );
	IntStringStringMap :: const_iterator i = ip -> aNumReplaceMap.find ( outGroup );
	if ( i == ip -> aNumReplaceMap.end ( ) ) { // no replace for this Group
		i = ip -> aNumReplaceMap.find ( 0 ); // trying default Group
		if ( i == ip -> aNumReplaceMap.end ( ) ) { // no replace for default Group
			PSYSTEMLOG ( Info, "(storm): CallingNumTranslator : No replace for OutGroup " << outGroup );
			return;
		}
	}
	PSYSTEMLOG ( Info, "(storm): CallingNumTranslator : InPeer normal replace" );
	_replaceAnumber ( i -> second, anum );
	PSYSTEMLOG ( Info, "(storm): CallingNumTranslator : InPeer replaced A-num=" << anum );
}

void CallingNumTranslator :: _translateForIn ( const CardInfo * ci, const OutPeerInfo * op, ss :: string & anum ) {
	PSYSTEMLOG ( Info, "(storm): CallingNumTranslator : translate for InCard. Anum=" << anum );
	if ( ! ci ) { // no Card
		PSYSTEMLOG ( Info, "(storm): CallingNumTranslator : Card not found. Skipped" );
		return;
	}
	PSYSTEMLOG ( Info, "(storm): CallingNumTranslator : Card " << ci -> acctn << " found" );
	int outGroup = _getOutGroup ( op );
	PSYSTEMLOG ( Info, "(storm): CallingNumTranslator : outGroup = " << outGroup );
	_getCardAnumberForOutGroup ( ci -> numbers, ci -> outGroupToANum, outGroup, anum );
	PSYSTEMLOG ( Info, "(storm): CallingNumTranslator : using card A-number = " << anum );
	IntStringStringMap :: const_iterator i = ci -> aNumReplaceMap.find ( outGroup );
	if ( i == ci -> aNumReplaceMap.end ( ) ) { // no replace for this Group
		i = ci -> aNumReplaceMap.find ( 0 ); // trying default Group
		if ( i == ci -> aNumReplaceMap.end ( ) ) { // no replace for default Group
			PSYSTEMLOG ( Info, "(storm): Conf :: getAnumber. No replace for OutGroup " << outGroup );
			return;
		}
	}
	PSYSTEMLOG ( Info, "(storm): CallingNumTranslator : Card normal replace" );
	_replaceAnumber ( i -> second, anum );
	PSYSTEMLOG ( Info, "(storm): CallingNumTranslator : Card replaced A-num=" << anum );
}

void CallingNumTranslator :: _translateForOut ( const OutPeerInfo * op, ss :: string & anum ) {
	PSYSTEMLOG ( Info, "(storm): CallingNumTranslator : translating for OutPeer. Anum" << anum );
	if ( ! op ) {
		PSYSTEMLOG ( Info, "(storm): CallingNumTranslator : OutPeer not found. Skipped" );
		return;
	}
	_replaceAnumber ( op -> aNumReplaceMap, anum );
	PSYSTEMLOG ( Info, "(storm): CallingNumTranslator : outTranslation replaced A-num=" << anum );
}

int CallingNumTranslator :: _getOutGroup ( const OutPeerInfo * op ) {
	if ( ! op )
		return 0;
	return op -> outGroupId;
}

void CallingNumTranslator :: _getCardAnumberForOutGroup ( const StringIntMap & numbers,
	const IntStringMap & outGroupToANum, int outGroup, ss :: string & anum ) {
	StringIntMap :: const_iterator i = numbers.find ( anum );
	if ( i != numbers.end ( ) && i -> second == outGroup )
		return;
	IntStringMap :: const_iterator j = outGroupToANum.find ( outGroup );
	if ( j != outGroupToANum.end ( ) ) {
		anum = j-> second;
		return;
	}
	if ( ! outGroup )
		return;
	if ( i != numbers.end ( ) && ! i -> second )
		return;
	j = outGroupToANum.find ( 0 );
	if ( j != outGroupToANum.end ( ) )
		anum = j-> second;
}

void CallingNumTranslator :: _replaceAnumber ( const StringStringMap & replaceMap, ss :: string & anum ) {
	int maxPrefixLen = -1;
	ss :: string prefix;
	// replace for ANY number
	StringStringMap :: const_iterator i2 = replaceMap.find ( "ANY" );
	if ( i2 != replaceMap.end ( ) ) {
		anum = i2 -> second;
		int startpos = anum.find ( '-' );
		if ( startpos <= 0 ) {
			PSYSTEMLOG ( Info, "(storm): CallingNumTranslator : replace for ANY " << anum );
			return;
		}
		long loLim = std :: atol ( anum.substr ( 0, startpos ).c_str ( ) );
		long hiLim = std :: atol ( anum.substr ( startpos + 1, anum.size ( ) ).c_str ( ) );
		if ( loLim > hiLim ) {
			long temp = hiLim;
			hiLim = loLim;
			loLim = temp;
		}
		long genNum = loLim;
		if ( loLim != hiLim )
			genNum += rand ( ) % ( hiLim - loLim );
		ss :: ostringstream os;
		os << genNum;
		anum = os.str ( );
		PSYSTEMLOG ( Info, "(storm): CallingNumTranslator : generated ANum " << anum <<
			" from range " << loLim << " .. " << hiLim );
		return;
	}
	// general replace for longest prefix
	for ( StringStringMap :: const_iterator i = replaceMap.begin ( ); i != replaceMap.end ( ); i++ ) {
		if ( anum.compare ( 0, i -> first.size ( ), i -> first ) == 0 ) {
			if ( ( int ) i -> first.size ( ) > maxPrefixLen ) {
				maxPrefixLen = i -> first.size ( );
				prefix = i -> second;
			}
		}
	}
	if ( maxPrefixLen >= 0 ) {
		anum.erase ( 0, maxPrefixLen );
		anum = prefix + anum;
	}
}


