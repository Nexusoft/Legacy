/*******************************************************************************************

            Hash(BEGIN(Satoshi[2010]), END(Sunny[2012])) == Videlicet[2014] ++

 [Learn and Create] Viz. http://www.opensource.org/licenses/mit-license.php

*******************************************************************************************/

#ifndef __cplusplus
# error This header can only be compiled as C++.
#endif

#ifndef __INCLUDED_PROTOCOL_H__
#define __INCLUDED_PROTOCOL_H__

#include "../util/serialize.h"
#include "../util/util.h"
#include <string>
#include "../hash/uint1024.h"

#define NEXUS_PORT  9323
#define NEXUS_CORE_LLP_PORT 9324
#define NEXUS_MINING_LLP_PORT 9325

#define RPC_PORT     9336
#define TESTNET_RPC_PORT 8336

#define TESTNET_PORT 8313
#define TESTNET_CORE_LLP_PORT 8329
#define TESTNET_MINING_LLP_PORT 8325

#define LISPNET_PORT 7313
#define LISPNET_CORE_LLP_PORT 7329
#define LISPNET_MINING_LLP_PORT 7325

extern bool fTestNet;
extern bool fLispNet;

namespace Net
{
    void GetMessageStart(unsigned char pchMessageStart[]);

    static inline unsigned short GetDefaultPort()
    {
        return GetArg("-port", fLispNet ? LISPNET_PORT : (fTestNet ? TESTNET_PORT : NEXUS_PORT));
    }


    /** Message header.
     * (4) message start.
     * (12) command.
     * (4) size.
     * (4) checksum.
     */
    class CMessageHeader
    {
        public:
            CMessageHeader();
            CMessageHeader(const char* pszCommand, unsigned int nMessageSizeIn);

            std::string GetCommand() const;
            bool IsValid() const;

            IMPLEMENT_SERIALIZE
                (
                 READWRITE(FLATDATA(pchMessageStart));
                 READWRITE(FLATDATA(pchCommand));
                 READWRITE(nMessageSize);
                 READWRITE(nChecksum);
                )

        // TODO: make private (improves encapsulation)
        public:
            enum { COMMAND_SIZE=12 };
            unsigned char pchMessageStart[4];
            char pchCommand[COMMAND_SIZE];
            unsigned int nMessageSize;
            unsigned int nChecksum;
    };

    /** nServices flags */
    enum
    {
        NODE_NETWORK = (1 << 0),
    };

    /** A CService with information about it as peer */
    class CAddress : public CService
    {
        public:
            CAddress();
            explicit CAddress(CService ipIn, uint64 nServicesIn = NODE_NETWORK);

            void Init();

            IMPLEMENT_SERIALIZE
                (
                 CAddress* pthis = const_cast<CAddress*>(this);
                 CService* pip = (CService*)pthis;
                 if (fRead)
                     pthis->Init();
                 if (nType & SER_DISK)
                     READWRITE(nVersion);
                 if ((nType & SER_DISK) || (!(nType & SER_GETHASH)))
                     READWRITE(nTime);
                 READWRITE(nServices);
                 READWRITE(*pip);
                )

            void print() const;

        // TODO: make private (improves encapsulation)
        public:
            uint64 nServices;

            // disk and network only
            unsigned int nTime;

            // memory only
            int64 nLastTry;
    };

    /** inv message data */
    class CInv
    {
        public:
            CInv();
            CInv(int typeIn, const uint1024& hashIn);

            IMPLEMENT_SERIALIZE
            (
                READWRITE(type);
                READWRITE(hash);
            )

            friend bool operator<(const CInv& a, const CInv& b);

            bool IsKnownType() const;
            const std::string& GetCommand() const;
            std::string ToString() const;
            void print() const;

        // TODO: make private (improves encapsulation)
        public:
            int type;
            uint1024 hash;
    };
}

#endif // __INCLUDED_PROTOCOL_H__
