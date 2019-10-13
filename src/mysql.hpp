#ifndef __MYSQL_HPP
#define __MYSQL_HPP
#pragma interface
#include <mysql/mysql.h>

class MyField : public Allocatable < __SS_ALLOCATOR > {
	MYSQL_FIELD * f;
public:
	MyField ( MYSQL_FIELD * ff );
	const char * name ( ) const;
	const char * table ( ) const;
	unsigned long len ( ) const;
	unsigned long maxLen ( ) const;
};

class MyResult;

struct poptOption;

class Alarm : public Allocatable < __SS_ALLOCATOR > {
public:
	virtual void set ( int sec ) = 0;
	virtual void clear ( ) = 0;
	virtual void setThreadId ( int id ) = 0;
	virtual ~Alarm ( ) { };
};

class MySQL : public Allocatable < __SS_ALLOCATOR > {
	class ProxyRes;
	friend class MyResult;
	MYSQL mysql;
	Pointer < Alarm > alp;
	ss :: string lastQuery;
	const char * connectHost;
	const char * connectUser;
	const char * connectPass;
	const char * connectDb;
	const char * connectSock;
	int connectOpts;
	int alsec;
	unsigned connectPort;
	MySQL & operator= ( const MySQL & );
	void reconnect ( );
public :
	enum Privs {
		mpAll = 1,
		mpAlter = 2,
		mpCreate = 4,
		mpDelete = 8,
		mpDrop = 16,
		mpFile = 32,
		mpIndex = 64,
		mpInsert = 128,
		mpProcess = 256,
		mpReferences = 512,
		mpReload = 1024,
		mpSelect = 2048,
		mpShutdown = 4096,
		mpUpdate = 8192,
		mpUsage = 16384,
		mpGrant = 32768
	};
	enum Lock {
		lockRead,
		lockWriteLP,
		lockWrite
	};
	MySQL ( );
	MySQL ( const char * host, const char * user, const char * pass,
		const char * db = 0, unsigned port = 0, const char * sock = 0,
		int opts = 0 );
	MySQL ( const MySQL & m );
	~MySQL ( );
	int connect ( const char * host, const char * user, const char * pass,
		const char * db = 0, unsigned port = 0, const char * sock = 0,
		int opts = 0 );
	int connect ( );
	void close ( );
	int createDB ( const char * db );
	int selectDB ( const char * db );
	int dropDB ( const char * db );
	int query ( const char * q );
	int query ( const ss :: string & q );
	const char * error ( ) const;
	int err_no ( ) const;
	ProxyRes & storeRes ( /*bool use = false*/ ) __attribute__ ( ( warn_unused_result ) );
	ss :: string dataBase ( );
	const char * info ( );
	ProxyRes & listDB ( const char * wild ) __attribute__ ( ( warn_unused_result ) );
	ProxyRes & listTable ( const char * wild ) __attribute__ ( ( warn_unused_result ) );
	ProxyRes & listField ( const char * table, const char * wild ) __attribute__ ( ( warn_unused_result ) );
	unsigned long long affectedRows ( );
	ProxyRes & getQuery ( const char *, ... )
		__attribute__ ( ( format ( printf, 2, 3 ), warn_unused_result ) );
	ProxyRes & getQuery ( const ss :: string & ) __attribute__ ( ( warn_unused_result ) );
	long long getLongLong ( const char *, ... )
		__attribute__ ( ( format ( printf, 2, 3 ) ) );
	long long getLongLong ( const ss :: string & );
	int getInt ( const char *, ... )
		__attribute__ (( format ( printf, 2, 3 ) ));
	int getInt ( const ss :: string & );
	bool getBool ( const char *, ... )
		__attribute__ (( format ( printf, 2, 3 ) ));
	bool getBool ( const ss :: string & );
	ss :: string getString ( const char *, ... )
		__attribute__ ( ( format ( printf, 2, 3 ) ) );
	ss :: string getString ( const ss :: string & );
	double getDouble ( const char *, ... )
		__attribute__ ( ( format ( printf, 2, 3 ) ) );
	int doQuery ( const char *, ... )
		__attribute__ ( ( format ( printf, 2, 3 ) ) );
	double getDouble ( const ss :: string & );
	int grant ( Privs privs, const char * db, const char * table,
		const char * cols, const char * user, const char * host,
		const char * passwd );
	int addHost ( Privs privs, const char * host, const char * db );
	ss :: string curDate ( );
	int lockTables ( const char * * tbls, const Lock * opts );
	int unLockTables ( );
	ss :: string version ( );
	int modifyColumn ( const char * table, const char * name,
		const char * type );
	int addIndex ( const char * table, const char * cols );
	int addUnique ( const char * table, const char * cols );
	int addColumn ( const char * table, const char * name,
		const char * type, const char * place = "" );
	int getLock ( const char * s, int timeout );
	int releaseLock ( const char * s );
	int shutDown ( );

	static const char * defaultHost;
	static const char * defaultUser;
	static const char * defaultPassword;
	static poptOption poptOptions [ ];
	static ss :: string escape ( const char * s );
	static ss :: string escape ( const ss :: string & s );
	void setAlarm ( Pointer < Alarm > p, int sec );
	const ss :: string & getLastQuery ( ) const;
	long long insertId ( );
	static void init ( );
	static void end ( );
};
//const int lockRead = 0;
//const int lockWriteLP = 1;
//const int lockWrite = 2;

const int mpAll = 1, mpAlter = 2, mpCreate = 4, mpDelete = 8, mpDrop = 16,
	mpFile = 32, mpIndex = 64, mpInsert = 128, mpProcess = 256,
	mpReferences = 512, mpReload = 1024, mpSelect = 2048, mpShutdown = 4096,
	mpUpdate = 8192, mpUsage = 16384, mpGrant = 32768;

#define POPT_MYSQL { 0, 0, POPT_ARG_INCLUDE_TABLE, MySQL :: poptOptions, \
	0, "MySQL Arguments", 0 },
class MySQLError : public std :: runtime_error {
public:
	explicit MySQLError ( const MySQL * m ) :
		std :: runtime_error ( ( m -> getLastQuery ( ) + '\n' + m -> error ( ) ).c_str ( ) ) { }
	explicit MySQLError ( const MySQL & m ) :
		std :: runtime_error ( ( m.getLastQuery ( ) + '\n' + m.error ( ) ).c_str ( ) ) { }
};

class MyResult : public Allocatable < __SS_ALLOCATOR > {
	friend class MySQL;
	MYSQL_RES * r;
//	bool used;
	MyResult ( const MyResult & );
	MyResult & operator= ( const MyResult & );
public :
	const char * * fetchRow ( );
	int eof ( ) const;
	explicit MyResult ( MySQL :: ProxyRes & p );
	void reset ( MySQL :: ProxyRes & p );
	~MyResult ( );
	int fieldCount ( ) const;
	int rowCount ( ) const;
	const MyField * fetchField ( );
	void seek ( unsigned n );
};

#endif
