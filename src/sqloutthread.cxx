#pragma implementation
#include <ptlib.h>
#include "ss.hpp"
#include "allocatable.hpp"
#include "sqloutthread.hpp"
#include "pointer.hpp"
#include <stdexcept>
#include "mysql.hpp"
#include <deque>
#include "automutex.hpp"
#include <ptlib/svcproc.h>
#include <ptlib/sockets.h>
#include "aftertask.hpp"
#include "ipport.hpp"
#include "icqcontact.hpp"
#include "radgwinfo.hpp"
#include "Conf.hpp"

class SQLOutThread :: Impl : public Allocatable < __SS_ALLOCATOR >, boost :: noncopyable {
	PMutex mut;
	PSyncPoint sp;
	MySQL m;
	typedef std :: deque < ss :: string, __SS_ALLOCATOR < ss :: string > > QueriesType;
	QueriesType queries;
public:
	explicit Impl ( Impl * & o );
	void Main ( );
	void add ( const ss :: string & q );
	void Close ( );
};

SQLOutThread :: Impl :: Impl ( Impl * & o ) : m ( "", "psbc", "", "radius", 0, "/opt/psbc/var/db_3336.sock" ) {
	o = this;
	conf -> checkCrashedActiveCalls ( );
	add ( "drop table if exists ActiveCalls" );
	add ( "create table ActiveCalls ( id int unsigned not null primary "
		"key, ref smallint unsigned, inIP char ( 15 ) not null, name char "
		"( 20 ), rname char ( 20 ), startTime datetime not null, setupTime datetime, dialedDigits"
		" char ( 32 ), connectTime datetime, convertedDigits char ( 32 ), "
		"outIP char ( 15 ), inCallId char ( 50 ), outCallId char ( 50 ), outName char ( 40 ), outRname char ( 20 ), "
		"callingDigits char ( 32 ), priceCache text ) ENGINE = MyISAM" );
	add ( "drop table if exists RadiusActiveCalls" );
	add ( "create table RadiusActiveCalls ( id char ( 35 ) not null primary key, acctn varchar ( 10 )"
		" not null, CalledStationId varchar ( 32 ) not null, h323ConfId char ( 32 ) not null, "
		"price float ( 10, 5 ) unsigned not null, code varchar ( 16 ) not null, startTime "
		"datetime not null, creditTime smallint unsigned not null ) ENGINE = MyISAM" );
	add ( "drop table if exists Commands" );
	add ( "create table Commands ( id smallint unsigned not null primary key, command varchar "
		"( 255 ) not null ) ENGINE = MyISAM" );
	add ( "drop table if exists SmsCallBackActiveCalls" );
	add ( "create table SmsCallBackActiveCalls ( id int unsigned not null primary key, acctn char ( 16 ) not null, "
		"ref smallint unsigned not null, callId char ( 50 ) not null, digits char ( 32 ) not null, startTime "
		"datetime not null, name char ( 20 ), connectTime datetime, convertedDigits char ( 32 ), "
		"ip char ( 15 ) ) ENGINE = MyISAM" );
	add ( "drop table if exists CardsState" );
	add ( "CREATE TABLE `CardsState` ( `id` int(11) NOT NULL auto_increment, `acctn` mediumint(8) unsigned NOT NULL default '0', "
	 	"`state` varchar(20) NOT NULL default '', `expires` time NOT NULL default '00:00:00', "
	 	"`ip` varchar(15) NOT NULL default '', `port` int(10) unsigned NOT NULL default '0', "
		"`eventTime` datetime NOT NULL default '0000-00-00 00:00:00', "
		"`proto` smallint NOT NULL default 0, `locks` smallint NOT NULL default 0, "
		"PRIMARY KEY (`id`) ) ENGINE = MyISAM" );
	add ( "drop table if exists GateKeepersState" );
	add ( "CREATE TABLE `GateKeepersState` ( `id` int(11) NOT NULL auto_increment, "
		"`gkName` varchar(32) NOT NULL default '0', `state` varchar(20) NOT NULL default '', "
		"`expires` time NOT NULL default '00:00:00', `ip` varchar(15) NOT NULL default '', "
		"`port` int(10) unsigned NOT NULL default '0', `eventTime` datetime NOT NULL default '0000-00-00 00:00:00', "
		"PRIMARY KEY (`id`) ) ENGINE = MyISAM" );
	add ( "CREATE TABLE IF NOT EXISTS `FakeCalls` ( `id` tinyint(3) unsigned NOT NULL auto_increment, "
		"`callDateTime` datetime NOT NULL default '0000-00-00 00:00:00', `outPeerId` int(10) unsigned NOT NULL default '0', "
		"`fakeMotive` enum('NoMotive','BrokenLen','BrokenSequence','TooFastConnect','TooFastProceeding',"
		"'TooFastAlerting','RTPtimeout') NOT NULL default 'NoMotive', "
		"`callProceedingTime` bigint(20) unsigned NOT NULL default '0', "
		"`alertingTime` bigint(20) unsigned NOT NULL default '0', "
		"`connectTime` bigint(20) unsigned NOT NULL default '0', "
		"`callDuration` bigint(20) unsigned NOT NULL default '0', "
		"PRIMARY KEY (`id`) ) ENGINE = MyISAM" );
	add ( "CREATE TABLE IF NOT EXISTS `CardMigrations` ( `id` int(11) NOT NULL auto_increment, "
		"`eventTime` datetime NOT NULL default '0000-00-00 00:00:00', `acctn` mediumint(9) NOT NULL default '0', "
		"`ip` varchar(15) NOT NULL default '', PRIMARY KEY (`id`) ) ENGINE = MyISAM" );
}

void SQLOutThread :: Impl :: add ( const ss :: string & q ) {
	QueriesType :: size_type s;
	{
		AutoMutex am ( & mut );
		s = queries.size ( );
		queries.push_back ( q );

	}
	sp.Signal ( );
	if ( s > 1000 )
		PSYSTEMLOG ( Error, "MySQL queries count " << s );
}

void SQLOutThread :: Impl :: Close ( ) {
	PSYSTEMLOG ( Info, "SQLOutThread :: Close ( )" );
	add ( "" );
}

void SQLOutThread :: Impl :: Main ( ) {
	while ( true ) {
		sp.Wait ( );
		try {
			ss :: string q;
			{
				AutoMutex am ( & mut );
				if ( queries.empty ( ) ) {
					PSYSTEMLOG ( Error, "MySQL query queue empty" );
					continue;
				}
				q = queries.front ( );
				queries.pop_front ( );
			}
			if ( q.empty ( ) )
				break;
			if ( m.query ( q ) )
				PSYSTEMLOG ( Error, "MySQL error: " << m.error ( )
					<< ", lastQuery: " << m.getLastQuery ( ) );
			else
				PSYSTEMLOG ( Info, "MySQL query ok: " << m.getLastQuery ( ) );
		} catch ( std :: exception & e ) {
			PSYSTEMLOG ( Error, "exception: " << e.what ( ) );
		}
	}
	PSYSTEMLOG ( Info, "SQLOutThread ended" );
}

SQLOutThread :: SQLOutThread ( SQLOutThread * & o ) : PThread ( 1000, NoAutoDeleteThread, HighestPriority,
	"SQLOutThread" ) {
	o = this;
	impl = new Impl ( impl );
	Resume ( );
}

SQLOutThread :: ~SQLOutThread ( ) {
	delete impl;
}

void SQLOutThread :: add ( const ss :: string & q ) {
	impl -> add ( q );
}

void SQLOutThread :: Close ( ) {
	impl -> Close ( );
}

void SQLOutThread :: Main ( ) {
	impl -> Main ( );
}
