#ifndef __H323_HPP
#define __H323_HPP
#pragma interface

namespace H323 {

void setAliasAddress ( const ss :: string & name, H225 :: AliasAddress & alias, H225 :: AliasAddress :: Choices tag );
void setAliasAddress ( const ss :: string & name, H225 :: AliasAddress & alias );
ss :: string getAliasAddressString ( const H225 :: AliasAddress & alias );
ss :: string globallyUniqueId ( );
ss :: string printableCallId ( const ss :: string & confId );

}
#endif
