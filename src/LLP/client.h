#ifndef NEXUS_LLP_CLIENT_H
#define NEXUS_LLP_CLIENT_H

#include "types.h"

namespace LLP
{
    class Outbound : public Connection
    {
        Service_t IO_SERVICE;

    public:
        std::string IP, PORT;

        /** Outgoing Client Connection Constructor **/
        Outbound(std::string ip, std::string port) : IP(ip), PORT(port) { }

        bool Connect()
        {
            try
            {
                using boost::asio::ip::tcp;

                tcp::resolver               RESOLVER(IO_SERVICE);
                tcp::resolver::query      QUERY   (tcp::v4(), IP.c_str(), PORT.c_str());
                tcp::resolver::iterator   ADDRESS = RESOLVER.resolve(QUERY);

                SOCKET = Socket_t(new tcp::socket(IO_SERVICE));
                SOCKET -> connect(*ADDRESS, ERROR_HANDLE);

                if(ERROR_HANDLE)
                    return false;

                CONNECTED = true;
                TIMER.Start();

                printf("***** Connected to %s:%s...\n", IP.c_str(), PORT.c_str());

                return true;
            }
            catch(...){ Disconnect(); }

            return false;
        }

    };

    class CoreOutbound : public Outbound
    {
    public:
        CoreOutbound(std::string ip, std::string port) : Outbound(ip, port){}

        enum
        {
            /** DATA PACKETS **/
            TIME_DATA     = 0,
            ADDRESS_DATA  = 1,
            TIME_OFFSET   = 2,

            /** DATA REQUESTS **/
            GET_OFFSET    = 64,


            /** REQUEST PACKETS **/
            GET_TIME      = 129,
            GET_ADDRESS   = 130,


            /** GENERIC **/
            PING          = 253,
            CLOSE         = 254
        };

        inline Packet NewPacket() { return this->INCOMING; }

        inline Packet GetPacket(unsigned char HEADER)
        {
            Packet PACKET;
            PACKET.HEADER = HEADER;
            return PACKET;
        }

        inline void GetOffset(unsigned int nTimestamp)
        {
            Packet REQUEST = GetPacket(GET_OFFSET);
            REQUEST.LENGTH = 4;
            REQUEST.DATA   = uint2bytes(nTimestamp);

            this->WritePacket(REQUEST);
        }

        inline void GetTime()
        {
            Packet REQUEST = GetPacket(GET_TIME);
            this->WritePacket(REQUEST);
        }

        void Close()
        {
            Packet RESPONSE = GetPacket(CLOSE);
            this->WritePacket(RESPONSE);
            this->Disconnect();
        }

        inline void GetAddress()
        {
            Packet REQUEST = GetPacket(GET_ADDRESS);
            this->WritePacket(REQUEST);
        }
    };

}



#endif