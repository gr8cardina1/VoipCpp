#include <cstring>
#include <sstream>
#include <string>
#include <cstdio>
#include <cstdlib>
#include "pointer.hpp"
#include "mysql.hpp"
#include "dbconf.hpp"

const char * DBConf :: iVal = "ival";
const char * DBConf :: tVal = "tval";
const char * DBConf :: fVal = "fval";
const char * DBConf :: dVal = "dval";

DBConf :: ~DBConf ( ) { }
int DBConf :: createConfTable ( ) {
	ss :: ostringstream s;
	s << "create table " << table << " ( name varchar(64) primary key, " <<
		iVal << " int, " << fVal << " float, " << tVal <<
		" varchar(255), " << dVal << " date )";
	return m.query ( s.str ( ) );
}
void DBConf :: checkConfTable ( ) {
	Pointer < MyResult > r = m.getQuery ( "show columns from %s", table.c_str ( ) );
	if ( ! r )
		createConfTable ( );
}

int DBConf :: get ( const char * name, int & val, const char * ss ) {
	Pointer < MyResult > r = m.getQuery ( "select %s + 0 from %s where name = '%s'", ss, table.c_str ( ), name );
	if ( ! r || r -> rowCount ( ) != 1 )
		return - 1;
	val = atoi ( * r -> fetchRow ( ) );
	return 0;
}

int DBConf :: get ( const char * name, float & val ) {
	Pointer < MyResult > r = m.getQuery ( "select %s from %s where name='%s'", fVal, table.c_str ( ), name );
	if ( ! r || r -> rowCount ( ) != 1 )
		return - 1;
	val = atof ( * r -> fetchRow ( ) );
	delete r;
	return 0;
}

int DBConf :: get ( const char * name, ss :: string & val, const char * ss ) {
	Pointer < MyResult > r = m.getQuery ( "select %s from %s where name='%s'", ss, table.c_str ( ), name );
	if ( ! r || r -> rowCount ( ) != 1 )
		return - 1;
	val = * r -> fetchRow ( );
	return 0;
}

int DBConf :: set ( const char * name, int val, const char * ss ) {
	return m.doQuery ( "replace %s set name = '%s', %s = %i", table.c_str ( ), name, ss, val );
}

int DBConf :: set ( const char * name, float val ) {
	return m.doQuery ( "replace %s set name = '%s', %s = %f", table.c_str ( ), name, fVal, val );
}

int DBConf :: set ( const char * name, const ss :: string & val, const char * ss ) {
	return m.doQuery ( "replace %s set name = '%s', %s = '%s'", table.c_str ( ), name, ss, val.c_str ( ) );
}

int DBConf :: update ( const char * name, double val, const char * ss ) {
	return m.doQuery ( "update %s set name = '%s', %s = '%f'", table.c_str ( ), name, ss, val );
}
    