#include <string>
#include <sstream>
#include <cstring>
#include <cstdarg>
#include <cstdlib>
#include <popt.h>
#include "pointer.hpp"
#include "mysql.hpp"

bool MySQL::set_throw(bool f)
{
	bool t = alwaysThrow;
	alwaysThrow = f;
	return t;
}

ss :: string MySQL :: escape ( const ss :: string & s ) {
	int l = s.length ( );
	char * t = new char [ ( l << 1 ) + 1 ];
	mysql_escape_string ( t, s.c_str ( ), l );
	ss :: string r = t;
	delete t;
	return r;
}

ss :: string MySQL :: escape ( const char * s ) {
	int l = strlen ( s );
	char * t = new char [ ( l << 1 ) + 1 ];
	mysql_escape_string ( t, s, l );
	ss :: string r = t;
	delete t;
	return r;
}

MyField :: MyField ( MYSQL_FIELD * ff ) : f ( ff ) { }

char * MyField :: name ( ) {
	return f -> name;
}

char * MyField :: table ( ) {
	return f -> table;
}

int MyField :: len ( ) {
	return f -> length;
}

int MyField :: maxLen ( ) {
	return f -> max_length;
}

MyResult :: MyResult ( MYSQL_RES * mr, bool u ) : r ( mr ), used ( u ) { }

MyResult :: ~MyResult ( ) {
	if ( r ) {
		if ( used )
			while ( fetchRow ( ) )
				;
		mysql_free_result ( r );
	}
}

int MyResult :: eof ( ) const {
	return mysql_eof ( r );
}

const char * * MyResult :: fetchRow ( ) {
	return const_cast < const char * * > ( mysql_fetch_row ( r ) );
}

MyField * MyResult :: fetchField ( ) {
	if ( MYSQL_FIELD * f = mysql_fetch_field ( r ) )
		return new MyField ( f );
	return 0;
}

int MyResult :: fieldCount ( ) const {
	return r -> field_count;
}

int MyResult :: rowCount ( ) const {
	return r -> data -> rows;
}

void MyResult :: seek ( unsigned n ) {
	mysql_data_seek ( r, n );
}

MyResult * MySQL :: storeRes ( bool use ) {
	MYSQL_RES * tmp;
	if ( use )
		tmp = mysql_use_result ( & mysql );
	else
		tmp = mysql_store_result ( & mysql );
	if ( tmp == 0 )
		return 0;
	return new MyResult ( tmp, use );
}

MySQL :: MySQL ( ) {
	mysql_init ( & mysql );
	connect ( );
}

MySQL :: MySQL ( const char * host, const char * user, const char * pass,
	const char * db, unsigned port, const char * sock, int opts ) {
	mysql_init ( & mysql );
	connect ( host, user, pass, db, port, sock, opts );
}

MySQL :: ~MySQL ( ) {
	close ( );
}

MySQL :: MySQL ( const MySQL & m ) {
	mysql_init ( & mysql );
	connect ( m.connectHost, m.connectUser, m.connectPass, m.connectDb, m.connectPort, m.connectSock,
		m.connectOpts );
}

int MySQL :: connect ( const char * host, const char * user, const char * pass,
	const char * db, unsigned port, const char * sock, int opts ) {
	connectHost = host;
	connectUser = user;
	connectPass = pass;
	connectDb = db;
	connectPort = port;
	connectSock = sock;
	connectOpts = opts;
	alwaysThrow = getenv ( "MYSQL_ALWAYS_THROW" );
	int t = 5;
	mysql_options ( & mysql, MYSQL_OPT_CONNECT_TIMEOUT, reinterpret_cast < const char * > ( & t ) );
	ss :: ostringstream os;
	os << "mysql_real_connect ( " << host << ", " << user << ", " << pass <<
		", " << db << ", " << port << ", " << sock << " )";
	lastQuery = os.str ( );
	if ( mysql_real_connect ( & mysql, host, user, pass, db, port, sock, opts ) )
		return 0;
	return - 1;
}

int MySQL :: connect ( ) {
	return connect ( defaultHost, defaultUser, defaultPassword );
}

void MySQL :: close ( ) {
	lastQuery = "mysql_close";
	mysql_close ( & mysql );
}

int MySQL :: createDB ( const char * db ) {
	ss :: string t = "create database ";
	t += db;
	return query ( t );
}

int MySQL :: selectDB ( const char * db ) {
	lastQuery = ss :: string ( "mysql_select_db ( " ) + db + " )";
	return mysql_select_db ( & mysql, db );
}

int MySQL :: dropDB ( const char * db ) {
	ss :: string t = "drop database ";
	t += db;
	return query ( t );
}

int MySQL :: query ( const char * q ) {
	lastQuery = q;
	Alarmer a ( alp, alsec );
	int r = mysql_query ( & mysql, q );
	if ( r && alwaysThrow )
		throw MySQLError ( * this );
	return r;
}

int MySQL :: query ( const ss :: string & q ) {
	return query ( q.c_str ( ) );
}

const char * MySQL :: error ( ) const {
	return mysql_error ( const_cast < MYSQL * > ( & mysql ) );
}

int MySQL :: err_no ( ) const {
	return mysql_errno ( const_cast < MYSQL * > ( & mysql ) );
}

ss :: string MySQL :: dataBase ( ) {
	return getString ( "select database( )" );
}

ss :: string MySQL :: curDate ( ) {
	return getString ( "select curdate( ) + 0" );
}

ss :: string MySQL :: version ( ) {
	return getString ( "select version( )" );
}

const char * MySQL :: info ( ) {
	return mysql_info ( & mysql );
}

MyResult * MySQL :: listDB ( const char * wild ) {
	lastQuery = ss :: string ( "mysql_list_dbs ( " ) + wild + " )";
	MYSQL_RES * tmp = mysql_list_dbs ( & mysql, wild );
	if ( tmp == 0 )
		return 0;
	return new MyResult ( tmp );
}

MyResult * MySQL :: listTable ( const char * wild ) {
	lastQuery = ss :: string ( "mysql_list_tables ( " ) + wild + " )";
	MYSQL_RES * tmp = mysql_list_tables ( & mysql, wild );
	if ( tmp == 0 )
		return 0;
	return new MyResult ( tmp );
}

MyResult * MySQL :: listField ( const char * table, const char * wild ) {
	lastQuery = ss :: string ( "mysql_list_fields ( " ) + table + ", " + wild + " )";
	MYSQL_RES * tmp = mysql_list_fields ( & mysql, table, wild );
	if ( tmp == 0 )
		return 0;
	return new MyResult ( tmp );
}

int MySQL :: affectedRows ( ) {
	return mysql_affected_rows ( & mysql );
}

MyResult * MySQL :: getQuery ( const char * f, ... ) {
	va_list va;
	va_start ( va, f );
	char * s;
	vasprintf ( & s, f, va );
	va_end ( va );
	PSYSTEMLOG ( Info, "roman 0q: " << s );
	MyResult * r = getQuery ( ss :: string ( s ) );
	free ( s );
	return r;
}

MyResult * MySQL :: getQuery ( const ss :: string & s ) {
PSYSTEMLOG ( Info, "roman 1q: " << s );	
	if ( query ( s ) ) 
		return 0;
	return storeRes ( );
}

long long MySQL :: getLongLong ( const char * f, ... ) {
	va_list va;
	va_start ( va, f );
	char * s;
	vasprintf ( & s, f, va );
	va_end ( va );
	ss :: string st = s;
	free ( s );
	return getLongLong ( st );
}

long long MySQL :: getLongLong ( const ss :: string & f ) {
	Pointer < MyResult > r = getQuery ( f );
	if ( ! r || ! r -> rowCount ( ) )
		return - 1;
	const char * s = * r -> fetchRow ( );
	if ( ! s )
		return - 1;
	return strtoll ( s, 0, 10 );
}

int MySQL :: getInt ( const char * f, ... ) {
	va_list va;
	va_start ( va, f );
	char * s;
	vasprintf ( & s, f, va );
	va_end ( va );
	ss :: string st = s;
	free ( s );
	return getInt ( st );
}

int MySQL :: getInt ( const ss :: string & f ) {
	Pointer < MyResult > r = getQuery ( f );
	if ( ! r || ! r -> rowCount ( ) )
		return - 1;
	const char * s = * r -> fetchRow ( );
	if ( ! s )
		return - 1;
	return atoi ( s );
}

bool MySQL :: getBool ( const char * f, ... ) {
	va_list va;
	va_start ( va, f );
	char * s;
	vasprintf ( & s, f, va );
	va_end ( va );
	ss :: string st = s;
	free ( s );
	return getBool ( st );
}

bool MySQL :: getBool ( const ss :: string & f ) {
	Pointer < MyResult > r = getQuery ( f );
	if ( ! r || ! r -> rowCount ( ) )
		return false;
	const char * s = * r -> fetchRow ( );
	return s && atoi ( s );
}

ss :: string MySQL :: getString ( const char * f, ... ) {
	va_list va;
	va_start ( va, f );
	char * s;
	vasprintf ( & s, f, va );
	va_end ( va );
	ss :: string st = s;
	free ( s );
	return getString ( st );
}

ss :: string MySQL :: getString ( const ss :: string & f ) {
	Pointer < MyResult > r = getQuery ( f );
	if ( ! r || ! r -> rowCount ( ) )
		return "";
	return * r -> fetchRow ( );
}

double MySQL :: getDouble ( const char * f, ... ) {
	va_list va;
	va_start ( va, f );
	char * s;
	vasprintf ( & s, f, va );
	va_end ( va );
	ss :: string st = s;
	free ( s );
	return getDouble ( st );
}

double MySQL :: getDouble ( const ss :: string & f ) {
	Pointer < MyResult > r = getQuery ( f );
	if ( ! r || ! r -> rowCount ( ) )
		return - 1;
	return atof ( * r -> fetchRow ( ) );
}

int MySQL :: doQuery ( const char * f, ... ) {
	va_list va;
	va_start ( va, f );
	char * s;
	vasprintf ( & s, f, va );
	va_end ( va );
	int r = query ( s );
	free ( s );
	return r;
}

const char * mpNames [ ] = {
	"all",
	"alter",
	"create",
	"delete",
	"drop",
	"file",
	"index",
	"insert",
	"process",
	"references",
	"reload",
	"select",
	"shutdown",
	"update",
	"usage"
};

int MySQL :: grant ( Privs privs, const char * db, const char * table,
	const char * cols, const char * user, const char * host,
	const char * passwd ) {
	ss :: string q ( "grant " );
	int first = 1;
	for ( int i = 0; i < 15; i ++ )
		if ( privs & ( 1 << i ) ) {
			if ( first )
				first = 0;
			else
				q += ",";
			q += mpNames [ i ];
			if ( cols && * cols ) {
				q += "(";
				q += cols;
				q += ")";
			}
		}
	q += " on ";
	if ( db ) {
		q += db;
		if ( table ) {
			q += ".";
			q += table;
		}
	} else
		q += table;
	q += " to '";
	q += user;
	q += "'";
	if ( host ) {
		q += "@'";
		q += host;
		q += "'";
	}
	if ( passwd ) {
		q += " identified by '";
		q += passwd;
		q += "'";
	}
	if ( privs & mpGrant )
		q += " with grant option";
	return query ( q );
}

int MySQL :: addHost ( Privs privs, const char * host, const char * db ) {
	ss :: ostringstream os;
	os << "insert into mysql.host ";
	ss :: string q;
	ss :: string q2 ( host );
	q2 += ",";
	q2 += db;
	if ( privs == mpAll )
		q2 += ",'Y','Y','Y','Y','Y','Y','Y','Y','Y','Y'";
	else {
		q = "host, db";
		for ( int i = 1; i < 14; i ++ )
			if ( privs & ( 1 << i ) ) {
				q += ",";
				q += mpNames [ i ];
				q += "_priv";
				q2 += ",'Y'";
			}
	}
	if ( ! q.empty ( ) )
		os << "( " << q << " ) ";
	os << "values ( " << q2 << " )";
	return query ( os.str ( ) );
}

int MySQL :: lockTables ( const char * * tbls, const Lock * opts ) {
	ss :: string s ( "lock tables " );
	while ( * tbls ) {
		s += * tbls ++;
		switch ( * opts ++ ) {
			case lockRead:
				s += " read";
				break;
			case lockWriteLP:
				s += " low_priority";
			case lockWrite:
				s += " write";
				break;
			default:
				return -1;
		}
		if ( * tbls )
			s += ",";
	}
	return query ( s );
}

int MySQL :: unLockTables ( ) {
	return query ( "unlock tables" );
}

int MySQL :: modifyColumn ( const char * table, const char * name,
	const char * type ) {
	return doQuery ( "alter table %s modify %s %s", table, name, type );
}

int MySQL :: addColumn ( const char * table, const char * name,
	const char * type, const char * place ) {
	return doQuery ( "alter table %s add %s %s %s", table, name, type, place );
}

int MySQL :: addIndex ( const char * table, const char * cols ) {
	return doQuery ( "alter table %s add index ( %s )", table, cols );
}

int MySQL :: addUnique ( const char * table, const char * cols ) {
	return doQuery ( "alter table %s add unique ( %s )", table, cols );
}

int MySQL :: getLock ( const char * s, int timeout ) {
	MyResult * r = getQuery ( "select get_lock('%s',%i)", s, timeout );
	if ( ! r )
		return - 1;
	const char * p = * r -> fetchRow ( );
	if ( ! p ) {
		delete r;
		return - 1;
	}
	int c = atoi ( p );
	delete r;
	if ( c == 1 )
		return 0;
	return - 2;
}

int MySQL :: releaseLock ( const char * s ) {
	MyResult * r = getQuery ( "select release_lock('%s')", s );
	if ( ! r )
		return - 1;
	const char * p = * r -> fetchRow ( );
	if ( ! p ) {
		delete r;
		return - 1;
	}
	int c = atoi ( p );
	delete r;
	if ( c == 1 )
		return 0;
	return - 2;
}

const char * MySQL :: defaultHost = "";
const char * MySQL :: defaultUser = "";
const char * MySQL :: defaultPassword = "";

poptOption MySQL :: poptOptions [ ] = {
	{
		"mysql-host",
		0,
		POPT_ARG_STRING,
		& defaultHost,
		0,
		"host of MySQL server",
		"HOST"
	}, {
		"mysql-password",
		0,
		POPT_ARG_STRING,
		& defaultPassword,
		0,
		"password on MySQL server",
		"PASSWORD"
	}, {
		"mysql-user",
		0,
		POPT_ARG_STRING,
		& defaultUser,
		0,
		"user on MySQL server",
		"USER"
	}, {
		0,
		0,
		0,
		0,
		0,
		0,
		0
	}
};

int MySQL :: shutDown ( ) {
	return mysql_shutdown ( & mysql, SHUTDOWN_DEFAULT );
}

void MySQL :: setAlarm ( Pointer < Alarm > p, int sec ) {
	alp = p;
	alsec = sec;
	alp -> setThreadId ( getInt ( "select connection_id()" ) );
}
const ss :: string & MySQL :: getLastQuery ( ) const {
	return lastQuery;
}
long long MySQL :: insertId ( ) {
	return mysql_insert_id ( & mysql );
}
