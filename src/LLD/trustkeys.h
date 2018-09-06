#ifndef LOWER_LEVEL_LIBRARY_LLD_TRUSTKEYS
#define LOWER_LEVEL_LIBRARY_LLD_TRUSTKEYS

#include "sector.h"
#include "../core/core.h"
#include "../wallet/db.h"

/** Lower Level Database Name Space. **/
namespace LLD
{
    class CTrustDB : public SectorDatabase
    {
    public:
        /** The Database Constructor. To determine file location and the Bytes per Record. **/
        CTrustDB(const char* pszMode="r+") : SectorDatabase("trust", "trust", pszMode) {}

        bool WriteMyKey(Core::CTrustKey cTrustKey);
        bool ReadMyKey(Core::CTrustKey& cTrustKey);
        bool EraseMyKey();
    };
}

#endif
