#include "trustkeys.h"
#include "../core/core.h"

/** Lower Level Database Name Space. **/
namespace LLD
{
    using namespace std;

    bool CTrustDB::WriteMyKey(Core::CTrustKey cTrustKey)
    {
        return Write(string("mytrustkey"), cTrustKey);
    }

    bool CTrustDB::ReadMyKey(Core::CTrustKey& cTrustKey)
    {
        return Read(string("mytrustkey"), cTrustKey);
    }

    bool CTrustDB::EraseMyKey()
    {
        return Erase(string("mytrustkey"));
    }
}
