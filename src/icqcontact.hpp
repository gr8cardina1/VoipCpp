#ifndef __ICQCONTACT_HPP
#define __ICQCONTACT_HPP
#pragma interface

struct IcqContact {
	ss :: string number;
	ss :: string name;
	int id;
	IcqContact ( int i, const ss :: string & nu, const ss :: string & na ) : number ( nu ), name ( na ), id ( i ) { }
};
typedef std :: vector < IcqContact, __SS_ALLOCATOR < IcqContact > > IcqContactsVector;

#endif
