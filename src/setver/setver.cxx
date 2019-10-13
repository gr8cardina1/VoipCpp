#include <cstdlib>
#include <fstream>
#include <ptlib.h>
#include <ptlib/pprocess.h>
#include "../Version.h"

static const char * GetCodeStatusStr()
{
    switch( PProcess::GKVER_STATUS )
    {
        case PProcess::AlphaCode    :   return "alpha";
        case PProcess::BetaCode     :   return "beta";
        case PProcess::ReleaseCode  :   return "release";
        default                     :   return "unknown";
    }
}

int main(int argc, char *argv[])
{
    char VersionStr[80];

    if ( argc != 2 )
        return 1;

    std :: sprintf( VersionStr,
             "%d.%d%s%d",
             GKVER_MAJOR,
             GKVER_MINOR,
             GetCodeStatusStr(),
             GKVER_BUILD
           );

    std :: ofstream fout(argv[1]);

    if ( !fout )
        return 2;

    fout << "GKVER=" << VersionStr << std :: endl;
    fout.close();
    return 0;
}
