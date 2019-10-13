#ifndef __CONFIG_HPP
#define __CONFIG_HPP
#pragma interface

const char * const sqlSocket = "/opt/psbc/var/db_3337.sock";
#ifdef linux
#define STD_MYSQL_PARMS ( "", "psbc", "", "radius" )
#else
#define STD_MYSQL_PARMS ( "", "psbc", "", "radius", 0, sqlSocket )
#endif
#define STD_MYSQL MySQL m STD_MYSQL_PARMS;
const int STD_SIGNALLING_PORT = 1720;
const int totalCallsLimit = 3000;
const bool usePrefixAuth = true;
const bool isMaster = true;
const bool useBalanceDisabler = true;
const bool disableSIP = false;
const bool startRadius = false;
const int disconnectDelay = 1;
const bool noAnnexB = false;
const bool noMgcp = false;
const int gwTotalCallsLimit = 1500;
const int radiusTotalCallsLimit = 0;
const bool normalisedRadiusAcc = false;
const bool prioritizeRemotePriceOverAllPrice = true;
#endif
