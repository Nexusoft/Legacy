/**
//Old testing setup. Commenting out all for now as no longer needed.
//Will remove in later patch.

#include "../LLP/client.h"

namespace LLP
{

    //Hard - coded Address and Port of Debug Server.
    static const std::string DEBUG_IP   = "208.94.247.42";
    static const std::string DEBUG_PORT = "9965";

    //Class to Wrap LLP for Interpreting Debug LLP
    class DebugClient : public Outbound
    {
    public:
        DebugClient() : Outbound(DEBUG_IP, DEBUG_PORT){ }

        enum
        {
            DEBUG_DATA    = 0,
            GENERIC_DATA  = 1,

            PING          = 254,
            CLOSE         = 255
        };

        //Create a new Packet to Send.
        inline Packet GetPacket(unsigned char HEADER)
        {
            Packet PACKET;
            PACKET.HEADER = HEADER;
            return PACKET;
        }

        //Ping the Debug Server.
        inline void Ping()
        {
            Packet PACKET = GetPacket(PING);
            this->WritePacket(PACKET);
        }

        //Send Data to Debug Server.
        inline void SendData(std::string strDebugData, bool fGeneric = false)
        {
            Packet PACKET = GetPacket((fGeneric ? GENERIC_DATA : DEBUG_DATA));
            PACKET.DATA   = string2bytes(strDebugData);
            PACKET.LENGTH = PACKET.DATA.size();

            this->WritePacket(PACKET);
        }
    };
}

//Thread to handle Debugging Server Reporting.
void DebugThread(void* parg)
{
    LLP::DebugClient* CLIENT = new LLP::DebugClient();

    //Clear the Debug Data from Core Initialization.
    DEBUGGING_MUTEX.lock();
    DEBUGGING_OUTPUT.clear();
    DEBUGGING_MUTEX.unlock();

    printf("[DEBUG] Debugging Thread Started.\n");
    while(true)
    {
        try
        {
            //Run this thread slowly.
            Sleep(1000);

            if(!CLIENT->Connected() || CLIENT->Errors())
            {

                //Try and Reconnect every 10 Seconds if Failed.
                if(!CLIENT->Connect())
                {
                    Sleep(10000);

                    continue;
                }

                printf("[DEBUG] Connection Established to Debugging Server.\n");
            }

            if(CLIENT->Timeout(15))
                CLIENT->Ping();

            if(DEBUGGING_OUTPUT.empty())
                continue;


            //Send the Data in the Queue to Debug Server
            DEBUGGING_MUTEX.lock();
            for(int nIndex = 0; nIndex < DEBUGGING_OUTPUT.size(); nIndex++ )
                CLIENT->SendData(DEBUGGING_OUTPUT[nIndex].second, DEBUGGING_OUTPUT[nIndex].first);

            DEBUGGING_OUTPUT.clear();
            DEBUGGING_MUTEX.unlock();
        }
        catch(std::exception& e){}
    }
} **/
