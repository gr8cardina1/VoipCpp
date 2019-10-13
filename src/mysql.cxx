#pragma implementation
#include "ss.hpp"
#include "allocatable.hpp"
#include "pointer.hpp"
#include <stdexcept>
#include "mysql.hpp"
#include <cstdarg>
#include <popt.h>
#include <mysql/errmsg.h>
#include <tr1/functional>
#include "scopeguard.hpp"
#include "deallocator.hpp"
#include <cstring>

class Alarmer {
	Pointer < Alarm > alp;
public:
	Alarmer ( Pointer < Alarm > a, int sec ) : alp ( a ) {
		if ( alp )
			alp -> set ( sec );
	}
	~Alarmer ( ) {
		if ( alp )
			alp -> clear ( );
	}
};

ss :: string MySQL :: escape ( const ss :: string & s ) {
	ss :: string :: size_type l = s.length ( ), sz = ( l << 1 ) + 1;
	char * t = __SS_ALLOCATOR < char > ( ).allocate ( sz );
	ScopeGuard g ( makeGuard ( Deallocator < char > ( t, sz ) ) );
	mysql_escape_string ( t, s.c_str ( ), l );
	return t;
}

ss :: string MySQL :: escape ( const char * s ) {
	std :: size_t l = std :: strlen ( s ), sz = ( l << 1 ) + 1;
	char * t = __SS_ALLOCATOR < char > ( ).allocate ( sz );
	ScopeGuard g = makeGuard ( Deallocator < char > ( t, sz ) );
	mysql_escape_string ( t, s, l );
	return t;
}

MyField :: MyField ( MYSQL_FIELD * ff ) : f ( ff ) { }

const char * MyField :: name ( ) const {
	return f -> name;
}

const char * MyField :: table ( ) const {
	return f -> table;
}

unsigned long MyField :: len ( ) const {
	return f -> length;
}

unsigned long MyField :: maxLen ( ) const {
	return f -> max_length;
}

MyResult :: MyResult ( MySQL :: ProxyRes & p ) : r ( reinterpret_cast < MYSQL_RES * > ( & p ) ) { }

void MyResult :: reset ( MySQL :: ProxyRes & p ) {
	mysql_free_result ( r );
	r = reinterpret_cast < MYSQL_RES * > ( & p );
}

MyResult :: ~MyResult ( ) {
//	if ( used )
//		while ( fetchRow ( ) )
//			;
	mysql_free_result ( r );
}

int MyResult :: eof ( ) const {
	return mysql_eof ( r );
}

const char * * MyResult :: fetchRow ( ) {
	return const_cast < const char * * > ( mysql_fetch_row ( r ) );
}

const MyField * MyResult :: fetchField ( ) {
	if ( MYSQL_FIELD * f = mysql_fetch_field ( r ) )
		return new MyField ( f );
	return 0;
}

int MyResult :: fieldCount ( ) const {
	return r -> field_count;
}

int MyResult :: rowCount ( ) const {
	return int ( r -> data -> rows );
}

void MyResult :: seek ( unsigned n ) {
	mysql_data_seek ( r, n );
}

MySQL :: ProxyRes & MySQL :: storeRes ( ) {
	MYSQL_RES * r = mysql_store_result ( & mysql );
	if ( ! r )
		throw MySQLError ( this );
	return * reinterpret_cast < ProxyRes * > ( r );
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

MySQL :: MySQL ( const MySQL & m ) : Allocatable < __SS_ALLOCATOR > ( ) {
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
	int t = 5;
	mysql_options ( & mysql, MYSQL_OPT_CONNECT_TIMEOUT, reinterpret_cast < const char * > ( & t ) );
#if ( MYSQL_VERSION_ID >= 50013 )
	t = 1;
	mysql_options ( & mysql, MYSQL_OPT_RECONNECT, reinterpret_cast < const char * > ( & t ) );
#endif
	ss :: ostringstream os;
	os << "mysql_real_connect ( " << host << ", " << user << ", " << pass <<
		", " << db << ", " << port << ", " << sock << " )";
	lastQuery = os.str ( );
	if ( mysql_real_connect ( & mysql, host, user, pass, db, port, sock, opts ) )
		return 0;
	return - 1;
}

void MySQL :: reconnect ( ) {
	mysql_real_connect ( & mysql, connectHost, connectUser, connectPass, connectDb, connectPort, connectSock, connectOpts );
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
#if ( MYSQL_VERSION_ID < 50013 )
	if ( r && err_no ( ) == CR_SERVER_GONE_ERROR ) {
		reconnect ( );
		r = mysql_query ( & mysql, q );
	}
#endif
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

MySQL :: ProxyRes & MySQL :: listDB ( const char * wild ) {
	lastQuery = ss :: string ( "mysql_list_dbs ( " ) + wild + " )";
	return * reinterpret_cast < ProxyRes * > ( mysql_list_dbs ( & mysql, wild ) );
}

MySQL :: ProxyRes & MySQL :: listTable ( const char * wild ) {
	lastQuery = ss :: string ( "mysql_list_tables ( " ) + wild + " )";
	return * reinterpret_cast < ProxyRes * > ( mysql_list_tables ( & mysql, wild ) );
}

MySQL :: ProxyRes & MySQL :: listField ( const char * table, const char * wild ) {
	lastQuery = ss :: string ( "mysql_list_fields ( " ) + table + ", " + wild + " )";
	return * reinterpret_cast < ProxyRes * > ( mysql_list_fields ( & mysql, table, wild ) );
}

unsigned long long MySQL :: affectedRows ( ) {
	return mysql_affected_rows ( & mysql );
}

MySQL :: ProxyRes & MySQL :: getQuery ( const char * f, ... ) {
	va_list va;
	va_start ( va, f );
	char * s;
	vasprintf ( & s, f, va );
	va_end ( va );
	ScopeGuard g = makeGuard ( std :: tr1 :: bind ( std :: free, s ) );	
	return getQuery ( ss :: string ( s ) );
}

MySQL :: ProxyRes & MySQL :: getQuery ( const ss :: string & s ) {
	query ( s );
	return storeRes ( );
}

long long MySQL :: getLongLong ( const char * f, ... ) {
	va_list va;
	va_start ( va, f );
	char * s;
	vasprintf ( & s, f, va );
	va_end ( va );
	ScopeGuard g = makeGuard ( std :: tr1 :: bind ( std :: free, s ) );
	ss :: string st = s;
	return getLongLong ( st );
}

long long MySQL :: getLongLong ( const ss :: string & f ) {
	MyResult r ( getQuery ( f ) );
	if ( ! r.rowCount ( ) )
		return - 1;
	const char * s = * r.fetchRow ( );
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
	ScopeGuard g = makeGuard ( std :: tr1 :: bind ( std :: free, s ) );
	ss :: string st = s;
	return getInt ( st );
}

int MySQL :: getInt ( const ss :: string & f ) {
	MyResult r ( getQuery ( f ) );
	if ( ! r.rowCount ( ) )
		return - 1;
	const char * s = * r.fetchRow ( );
	if ( ! s )
		return - 1;
	return std :: atoi ( s );
}

bool MySQL :: getBool ( const char * f, ... ) {
	va_list va;
	va_start ( va, f );
	char * s;
	vasprintf ( & s, f, va );
	va_end ( va );
	ScopeGuard g = makeGuard ( std :: tr1 :: bind ( std :: free, s ) );
	ss :: string st = s;
	return getBool ( st );
}

bool MySQL :: getBool ( const ss :: string & f ) {
	MyResult r ( getQuery ( f ) );
	if ( ! r.rowCount ( ) )
		return false;
	const char * s = * r.fetchRow ( );
	return s && std :: atoi ( s );
}

ss :: string MySQL :: getString ( const char * f, ... ) {
	va_list va;
	va_start ( va, f );
	char * s;
	vasprintf ( & s, f, va );
	va_end ( va );
	ScopeGuard g = makeGuard ( std :: tr1 :: bind ( std :: free, s ) );
	ss :: string st = s;
	return getString ( st );
}

ss :: string MySQL :: getString ( const ss :: string & f ) {
	MyResult r ( getQuery ( f ) );
	if ( ! r.rowCount ( ) )
		return ss :: string ( );
	return * r.fetchRow ( );
}

double MySQL :: getDouble ( const char * f, ... ) {
	va_list va;
	va_start ( va, f );
	char * s;
	vasprintf ( & s, f, va );
	va_end ( va );
	ScopeGuard g = makeGuard ( std :: tr1 :: bind ( std :: free, s ) );
	ss :: string st = s;
	return getDouble ( st );
}

double MySQL :: getDouble ( const ss :: string & f ) {
	MyResult r ( getQuery ( f ) );
	if ( ! r.rowCount ( ) )
		return - 1;
	return std :: atof ( * r.fetchRow ( ) );
}

int MySQL :: doQuery ( const char * f, ... ) {
	va_list va;
	va_start ( va, f );
	char * s;
	vasprintf ( & s, f, va );
	va_end ( va );
	ScopeGuard g = makeGuard ( std :: tr1 :: bind ( std :: free, s ) );
	return query ( s );;
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
				q += ',';
			q += mpNames [ i ];
			if ( cols && * cols ) {
				q += '(';
				q += cols;
				q += ')';
			}
		}
	q += " on ";
	if ( db ) {
		q += db;
		if ( table ) {
			q += '.';
			q += table;
		}
	} else
		q += table;
	q += " to '";
	q += user;
	q += '\'';
	if ( host ) {
		q += "@'";
		q += host;
		q += '\'';
	}
	if ( passwd ) {
		q += " identified by '";
		q += passwd;
		q += '\'';
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
	q2 += ',';
	q2 += db;
	if ( privs == mpAll )
		q2 += ",'Y','Y','Y','Y','Y','Y','Y','Y','Y','Y'";
	else {
		q = "host, db";
		for ( int i = 1; i < 14; i ++ )
			if ( privs & ( 1 << i ) ) {
				q += ',';
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
			s += ',';
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
	MyResult r ( getQuery ( "select get_lock('%s',%i)", s, timeout ) );
	const char * p = * r.fetchRow ( );
	if ( ! p )
		return - 1;
	if ( std :: atoi ( p ) == 1 )
		return 0;
	return - 2;
}

int MySQL :: releaseLock ( const char * s ) {
	MyResult r ( getQuery ( "select release_lock('%s')", s ) );
	const char * p = * r.fetchRow ( );
	if ( ! p )
		return - 1;
	if ( std :: atoi ( p ) == 1 )
		return 0;
	return - 2;
}

void MySQL :: init ( ) {
#if ( MYSQL_VERSION_ID >= 50003 )
	mysql_library_init ( 0, 0, 0 );
#else
	mysql_server_init ( 0, 0, 0 );
#endif
}

void MySQL :: end ( ) {
#if ( MYSQL_VERSION_ID >= 50003 )
	mysql_library_end ( );
#else
	mysql_server_end ( );
#endif
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
#if(MYSQL_VERSION_ID >= 40100)
	return mysql_shutdown ( & mysql, SHUTDOWN_DEFAULT );
#else
	return mysql_shutdown ( & mysql );
#endif //MYSQL_VERSION_ID
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
