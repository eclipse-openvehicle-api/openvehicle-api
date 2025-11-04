#include "hardware_ident.h"

#ifdef _WIN32
#pragma push_macro("interface")
#undef interface
#pragma push_macro("GetObject")
#undef GetObject

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <WinSock2.h>
#include <Windows.h>
#include <array>

// Resolve conflict
#pragma pop_macro("GetObject")
#pragma pop_macro("interface")
#ifdef GetClassInfo
#undef GetClassInfo
#endif
#ifndef __GNUC__
#pragma comment(lib, "Rpcrt4.lib")
#endif
#elif defined __linux__
#include <iostream>
//#include <unistd.h>
//#include <ifaddrs.h>
//#include <netinet/in.h>
//#include <arpa/inet.h>
//#include <cstdint>
//#include <linux/if_packet.h>
#else
#error OS is not supported!
#endif

uint64_t CHardwareIdent::GetHardwareID()
{
#ifdef _WIN32
    uint64_t MACAddr = 0;

    UUID uuid;
    if (UuidCreateSequential(&uuid) == RPC_S_UUID_NO_ADDRESS)
    {
        SDV_LOG_ERROR("Error getting UUID!");
        return 0;
    }

    // Converting MAC address to uint64
    // INFO: Last 6 bytes of the uuid.Data4 contain MAC address and first two bytes contains variant and version,
    // that's why only last 6 bytes are taken here
    for (uint32_t ui_it = 2; ui_it < 8; ui_it++)
    {
        MACAddr = (MACAddr << 8) | uuid.Data4[ui_it];
    }

    if (!MACAddr)
    {
        SDV_LOG_ERROR("Error getting MAC Address!");
        return 0;
    }

    return MACAddr;
#elif defined __linux__
    //struct ifaddrs *ifap, *ifa;
    //uint64_t MACAddr = 0;
    //if (getifaddrs(&ifap) == -1)
    //{
    //    SDV_LOG_ERROR("Error getting 'if address'!");
    //    return 0;
    //}
    //for (ifa = ifap; ifa != nullptr; ifa = ifa->ifa_next)
    //{
    //    if (ifa->ifa_addr != nullptr && ifa->ifa_addr->sa_family == AF_PACKET)
    //    {
    //        struct sockaddr_ll* scktAddr = (struct sockaddr_ll*)ifa->ifa_addr;
    //        if (scktAddr)
    //        {
    //            // Converting MAC address to uint64
    //            for (uint32_t ui_it = 0; ui_it < 6; ui_it++)
    //            {
    //                MACAddr = (MACAddr << 8) | scktAddr->sll_addr[ui_it];
    //            }
    //            if (MACAddr != 0)
    //            {
    //                freeifaddrs(ifap);
    //                return MACAddr;
    //            }
    //        }
    //    }
    //}

    //freeifaddrs(ifap);
    //SDV_LOG_ERROR("Error getting MAC address of the interface!");
    //return 0;
    return 0x0102030405060708ll;
#else
#error OS is not supported!
#endif
}
