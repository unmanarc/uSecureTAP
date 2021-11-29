#ifndef PEER_DEFINITION_H
#define PEER_DEFINITION_H

#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string.hpp>
#include <cx2_mem_vars/a_ipv4.h>
#include <cx2_mem_vars/a_macaddr.h>
#include <cx2_prg_logs/applog.h>

#include <vector>

struct sPeerDefinition
{
    sPeerDefinition()
    {
        aIpAddr.s_addr=0;
        cidrNetmask=32;
    }

    bool processIP(std::string & line, CX2::Application::Logs::AppLog * log, uint32_t lineNo)
    {
        bool r = true;
        auto pos =  line.find_first_of(',');
        std::string ipReg = line.substr(0,pos);
        auto val = CX2::Memory::Abstract::IPV4::_fromStringWithNetmask(ipReg,&r);
        if (r)
        {
            aIpAddr = val.first;
            cidrNetmask = val.second;
        }
        else
            log->log0(__func__,CX2::Application::Logs::LEVEL_CRITICAL, "Bad IP Address Format on line: %d", lineNo);

        line = pos!=std::string::npos?line.substr(pos+1):"";
        return r;
    }
    bool processKey(std::string & line, CX2::Application::Logs::AppLog * log, uint32_t lineNo)
    {
        bool r = true;
        auto pos =  line.find_first_of(',');
        key = line.substr(0,pos);
        line = pos!=std::string::npos?line.substr(pos+1):"";

        if (key.empty())
            log->log0(__func__,CX2::Application::Logs::LEVEL_CRITICAL, "Invalid Empty Key on line: %d", lineNo);

        return r;
    }
    bool processEthernetAddress(std::string & line, CX2::Application::Logs::AppLog * log, uint32_t lineNo)
    {
        bool r = true;
        auto pos =  line.find_first_of(',');
        std::string macReg = line.substr(0,pos);

        if (macReg.empty())
        {
            macAddrHash = 0xFFFFFFFF;
            return true; // No mac address specified...
        }

        auto val = CX2::Memory::Abstract::MACADDR::_fromStringToHASH(macReg,&r);
        if (r)
            macAddrHash = val;
        else
            log->log0(__func__,CX2::Application::Logs::LEVEL_CRITICAL, "Bad MAC Address Format on line: %d", lineNo);

        line = pos!=std::string::npos?line.substr(pos+1):"";
        return r;
        return true;
    }

    bool setPeerDefinition(std::string line, CX2::Application::Logs::AppLog * log, uint32_t lineNo)
    {
        if (!processIP(line,log,lineNo))
            return false;

        if (!processKey(line,log,lineNo))
            return false;

        if (!processEthernetAddress(line,log,lineNo))
            return false;

        if (lineNo)
            log->log1(__func__, CX2::Memory::Abstract::IPV4::_toString(aIpAddr,cidrNetmask) ,CX2::Application::Logs::LEVEL_INFO, "New remote peer configured in line %d (MAC=%s)", lineNo, CX2::Memory::Abstract::MACADDR::_fromHASHToString(macAddrHash).c_str() );
        else
            log->log1(__func__, CX2::Memory::Abstract::IPV4::_toString(aIpAddr,cidrNetmask) ,CX2::Application::Logs::LEVEL_INFO, "Local peer configured in line %d", lineNo );
        return true;
    }

    in_addr aIpAddr;
    uint8_t cidrNetmask;
    uint64_t macAddrHash;
    std::string key;
};


#endif // PEER_DEFINITION_H
