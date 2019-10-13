#ifndef __DBCONF_HPP
#define __DBCONF_HPP
#pragma interface

class DBConf {
	MySQL & m;
	const ss :: string table;
public:
	~DBConf ( );
	DBConf ( MySQL & mm, const char * t ) : m ( mm ),
		table ( t ) { }
	int set ( const char * name, int val, const char * ss = iVal );
	int set ( const ss :: string & name, int val, const char * ss = iVal );
	int set ( const char * name, float val );
	int set ( const char * name, const ss :: string & val, const char * ss );
	int get ( const char * name, int & val, const char * ss = iVal );
	int get ( const ss :: string & name, int & val, const char * ss = iVal );
	int get ( const char * name, float & val );
	int get ( const char * name, ss :: string & val, const char * ss = tVal );
	int setDate ( const char * name, const ss :: string & val ) {
		return set ( name, val, dVal );
	}
	int setDate ( const char * name, int val ) {
		return set ( name, val, dVal );
	}
	int getDate ( const char * name, ss :: string & val ) {
		return get ( name, val, dVal );
	}
	int getDate ( const char * name, int & val ) {
		return get ( name, val, dVal );
	}
	static const char * iVal;
	static const char * tVal;
	static const char * fVal;
	static const char * dVal;
	int createConfTable ( );
	void checkConfTable ( );
	int update ( const char* name, double val, const char * ss = iVal );
};
#endif
