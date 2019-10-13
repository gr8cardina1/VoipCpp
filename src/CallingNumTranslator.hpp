#ifndef _CALLINGNUMTRANSLATOR_HPP_
#define _CALLINGNUMTRANSLATOR_HPP_
#pragma interface

class InPeerInfo;
class OutPeerInfo;
class CardInfo;

// ne znayu zachem ono videleno iz conf, no esli net dannih, to eto dolgen bit namespace

class CallingNumTranslator {
public:
	CallingNumTranslator ( );
	virtual ~CallingNumTranslator ( );

	void translate ( const InPeerInfo * ip, const OutPeerInfo * op, ss :: string & anum );
	void translate ( const CardInfo * ci, const OutPeerInfo * op, ss :: string & anum );

private:
	void _translateForIn ( const InPeerInfo * ip, const OutPeerInfo * op, ss :: string & anum );
	void _translateForIn ( const CardInfo * ci, const OutPeerInfo * op, ss :: string & anum );
	void _translateForOut ( const OutPeerInfo * op, ss :: string & anum );

	void _replaceAnumber ( const StringStringMap & replaceMap, ss :: string & anum );
	int _getOutGroup ( const OutPeerInfo * op );
	void _getCardAnumberForOutGroup ( const StringIntMap & numbers,
		const IntStringMap & outGroupToANum, int outGroup, ss :: string & anum );
};

#endif //_CALLINGNUMTRANSLATOR_HPP_
