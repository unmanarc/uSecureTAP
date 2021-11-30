#include <thread>

#include <cx2_prg_service/application.h>
#include <cx2_net_sockets/socket_acceptor_multithreaded.h>
#include <cx2_mem_vars/a_uint16.h>
#include <cx2_mem_vars/a_bool.h>
#include <cx2_mem_vars/a_ipv4.h>

#include <sys/time.h>
#include <fstream>

#include "pkt_dissector.h"
#include "app_options.h"
#include "tls_callbacks.h"
#include "virtiface_reader.h"

using namespace CX2;
using namespace CX2::Memory;
using namespace CX2::Application;

#define VERSION_MAJOR 1
#define VERSION_MINOR 0
#define VERSION_PATCH 0

class SecureTAPApp : public CX2::Application::Application
{
public:
    SecureTAPApp() {
    }

    void _shutdown()
    {
        log->log0(__func__,Logs::LEVEL_INFO, "SIGTERM received.");
    }

    void _initvars(int argc, char *argv[], Arguments::GlobalArguments * globalArguments)
    {
        globalArguments->setInifiniteWaitAtEnd(true);

        /////////////////////////
        struct timeval time;
        gettimeofday(&time,nullptr);
        srand(((time.tv_sec * 1000) + (time.tv_usec / 1000))*getpid());

        globalArguments->setVersion( VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH, "alpha" );

        globalArguments->setLicense("GPLv3");
        globalArguments->setAuthor("Aarón Mizrachi");
        globalArguments->setEmail("aaron@unmanarc.com");
        globalArguments->setDescription("Unmanarc's Secure TAP Bridge");

        globalArguments->addCommandLineOption("TAP Interface", 'i', "interface" , "Interface Name"  , "utap0",                      Abstract::TYPE_STRING);

        globalArguments->addCommandLineOption("TLS Options", '9', "notls" , "Disable the use of TLS"  , "false",                    Abstract::TYPE_BOOL);
        globalArguments->addCommandLineOption("TLS Options", '4', "ipv4" , "Use only IPv4"  , "true",                               Abstract::TYPE_BOOL);
        globalArguments->addCommandLineOption("TLS Options", 'y', "cafile" , "X.509 Certificate Authority Path", "",                Abstract::TYPE_STRING);
        globalArguments->addCommandLineOption("TLS Options", 'k', "keyfile" , "X.509 Private Key Path (For listening mode)"  , "",  Abstract::TYPE_STRING);
        globalArguments->addCommandLineOption("TLS Options", 'c', "certfile" , "X.509 Certificate Path (For listening mode)"  , "", Abstract::TYPE_STRING);
        globalArguments->addCommandLineOption("TLS Options", 'l', "listen" , "Use in listen mode"  , "false",                       Abstract::TYPE_BOOL);
        globalArguments->addCommandLineOption("TLS Options", 'p', "port" , "Port"  , "443",                                         Abstract::TYPE_UINT16);
        globalArguments->addCommandLineOption("TLS Options", 'a', "addr" , "Address"  , "*",                                        Abstract::TYPE_STRING);
        globalArguments->addCommandLineOption("TLS Options", 't', "threads" , "Max Concurrent Connections (Threads)"  , "1024",     Abstract::TYPE_UINT16);

        globalArguments->addCommandLineOption("Authentication", 'f', "peersfile" , "Formatted multi line file (IP:PSK:MAC, first line is for myself)"  , "", Abstract::TYPE_STRING);
        globalArguments->addCommandLineOption("Other Options", 's', "sys" , "Journalctl Log Mode (don't print colors or dates)"  , "false",                  Abstract::TYPE_BOOL);

#ifndef _WIN32
        globalArguments->addCommandLineOption("Other Options", 'x', "uid" , "Drop Privileged to UID"  , "0",                        Abstract::TYPE_UINT16);
        globalArguments->addCommandLineOption("Other Options", 'g', "gid" , "Drop Privileged to GID"  , "0",                        Abstract::TYPE_UINT16);
#endif

    }

    bool _config(int argc, char *argv[], Arguments::GlobalArguments * globalArguments)
    {
        CX2::Network::TLS::Socket_TLS::prepareTLS();
        bool configUseFancy    = !((Memory::Abstract::BOOL *)globalArguments->getCommandLineOptionValue("sys"))->getValue();

        log = new Logs::AppLog();
        log->setPrintEmptyFields(true);
        log->setUserAlignSize(1);
        log->setUsingAttributeName(false);
        log->setUsingColors(configUseFancy);
        log->setUsingPrintDate(configUseFancy);
        log->setModuleAlignSize(36);
        appOptions.log = log;

        std::string passFile = globalArguments->getCommandLineOptionValue("peersfile")->toString();

        if ( !passFile.empty() )
        {
            std::ifstream file(passFile);
            if (file.is_open()) {

                std::string myUserConfigLine, othersConfigLine;
                if (!std::getline(file, myUserConfigLine))
                {
                    log->log0(__func__,Logs::LEVEL_CRITICAL, "Peers File '%s' requires at least 2 lines (one for me, and the other for other peers)", passFile.c_str());
                    exit(-15);
                }

                if (!appOptions.localTapOptions.setPeerDefinition(myUserConfigLine,log,0))
                    exit(-14);

                for (uint32_t lineNo = 1 ; std::getline(file, othersConfigLine);lineNo++)
                {
                    sPeerDefinition loptions;
                    if (!loptions.setPeerDefinition(othersConfigLine,log,lineNo))
                        exit(-16);
                    if (loptions.cidrNetmask!=32)
                    {
                        log->log0(__func__,Logs::LEVEL_CRITICAL, "Netmask is not /32 in configuration line number: %d", lineNo);
                        exit(-17);
                    }
                    if (appOptions.peersDefinition.find(loptions.aIpAddr.s_addr)!=appOptions.peersDefinition.end())
                    {
                        log->log0(__func__,Logs::LEVEL_CRITICAL, "IP address repeated in configuration line number: %d", lineNo);
                        exit(-13);
                    }

                    appOptions.peersDefinition[loptions.aIpAddr.s_addr] = loptions;
                }
                file.close();
            }
            else
            {
                log->log0(__func__,Logs::LEVEL_CRITICAL, "Failed to open peers file '%s'", passFile.c_str());
                exit(-13);
            }
        }

#ifdef _WIN32
        ULONG adapterIndex = 10000;
#endif
        std::string tapReadInterfaceName;
        ethhdr tapIfaceEthAddress;

        appOptions.ifaceName = globalArguments->getCommandLineOptionValue("interface")->toString();

        CX2::Network::Interfaces::NetIfConfig tapIfaceCfg;
        tapIfaceCfg.setUP(true);
        if ( ! appOptions.tapIface.start(&tapIfaceCfg,appOptions.ifaceName) )
        {
            log->log0(__func__,Logs::LEVEL_CRITICAL, "Failed to open TAP Interface %s - %s",appOptions.tapIface.getInterfaceRealName().c_str(),appOptions.tapIface.getLastError().c_str());
            exit(-5);
        }

        sleep(1);

        tapIfaceEthAddress = tapIfaceCfg.getEthernetAddress();

        if (!memcmp(tapIfaceEthAddress.h_dest,"\0\0\0\0\0\0",6))
        {
            log->log0(__func__,Logs::LEVEL_CRITICAL, "Unable to get Hardware Address from TAP interface %s",
                      appOptions.tapIface.getInterfaceRealName().c_str(),
                      appOptions.tapIface.getLastError().c_str());
            exit(-4);
        }
        tapReadInterfaceName = appOptions.tapIface.getInterfaceRealName();
#ifdef _WIN32
        adapterIndex = appOptions.tapIface.getWinTapAdapterIndex();
        log->log0(__func__,Logs::LEVEL_INFO,  "Using TAP-Windows6 Version: %s", appOptions.tapIface.getWinTapVersion().toString().c_str());
        log->log0(__func__,Logs::LEVEL_INFO,  "TAP Network Interface Info: %s", appOptions.tapIface.getWinTapDeviceInfo().c_str());
        log->log0(__func__,Logs::LEVEL_DEBUG, "TAP Network Interface Log Line: %s", appOptions.tapIface.getWinTapLogLine().c_str());
#endif

#ifdef _WIN32
        log->log0(__func__,Logs::LEVEL_INFO, "TAP Network Interface IDX=%lu NAME=%s HWADDR=%s Ready.", adapterIndex,
          #else
        log->log0(__func__,Logs::LEVEL_INFO, "TAP Network Interface NAME=%s HWADDR=%s Ready.",
          #endif
                  tapReadInterfaceName.c_str(),
                  Abstract::MACADDR::_toString(tapIfaceEthAddress.h_dest).c_str());

        appOptions.tapHwAddrHash = Abstract::MACADDR::_toHash(tapIfaceEthAddress.h_dest);
        appOptions.ipv4 = ((Memory::Abstract::BOOL *)globalArguments->getCommandLineOptionValue("ipv4"))->getValue();
        appOptions.notls = ((Memory::Abstract::BOOL *)globalArguments->getCommandLineOptionValue("notls"))->getValue();

        if (appOptions.notls)
            log->log0(__func__,Logs::LEVEL_WARN, "Proceding in plain-text mode, eavesdropping communications will be easy!!!");


        appOptions.cafile = globalArguments->getCommandLineOptionValue("cafile")->toString();
        appOptions.certfile = globalArguments->getCommandLineOptionValue("certfile")->toString();
        appOptions.keyfile = globalArguments->getCommandLineOptionValue("keyfile")->toString();

        appOptions.addr = globalArguments->getCommandLineOptionValue("addr")->toString();
        appOptions.port = ((Memory::Abstract::UINT16 *)globalArguments->getCommandLineOptionValue("port"))->getValue();
        appOptions.listenMode = ((Memory::Abstract::BOOL *)globalArguments->getCommandLineOptionValue("listen"))->getValue();

        appOptions.threadsLimit = ((Memory::Abstract::UINT16 *)globalArguments->getCommandLineOptionValue("threads"))->getValue();


        sock = (appOptions.notls?new Network::Sockets::Socket_TCP:new Network::TLS::Socket_TLS ) ;
        sock->setUseIPv6( !appOptions.ipv4 );


        appOptions.uid = ((Memory::Abstract::UINT16 *)globalArguments->getCommandLineOptionValue("uid"))->getValue();
        appOptions.gid = ((Memory::Abstract::UINT16 *)globalArguments->getCommandLineOptionValue("gid"))->getValue();

        tapIfaceCfg.setIPv4Address(appOptions.localTapOptions.aIpAddr,
                                   CX2::Memory::Abstract::IPV4::_fromCIDRMask(appOptions.localTapOptions.cidrNetmask)
                                   );

        if (!tapIfaceCfg.apply())
        {
            log->log0(__func__,Logs::LEVEL_CRITICAL, "Failed to configure TAP Interface Parameters...");
            exit(-601);
        }

#ifndef _WIN32
        // TODO: pasar a las CX

        if (getgid() != appOptions.gid || getuid() != appOptions.uid)
        {
            // Drop privileges and act like user process:
            if (getgid() != appOptions.gid)
            {
                if (setgid(appOptions.gid))
                {
                    log->log0(__func__,Logs::LEVEL_CRITICAL, "Failed to drop privileges to group %d...",appOptions.gid);
                    exit(-576);
                }
            }
            if (getuid() != appOptions.uid)
            {
                if (setuid(appOptions.uid))
                {
                    log->log0(__func__,Logs::LEVEL_CRITICAL, "Failed to drop privileges to user %d...",appOptions.uid);
                    exit(-579);
                }
            }

            // Now change EUID/EGID...
            if (setegid(appOptions.gid) != 0)
            {
                log->log0(__func__,Logs::LEVEL_CRITICAL, "Failed to drop extended privileges to group %d...",appOptions.gid);
                exit(-578);
            }

            if (seteuid(appOptions.uid) != 0)
            {
                log->log0(__func__,Logs::LEVEL_CRITICAL, "Failed to drop extended privileges to user %d...",appOptions.uid);
                exit(-577);
            }
            else
            {
                log->log0(__func__,Logs::LEVEL_INFO, "Application Privileges changed to UID=%d.",appOptions.uid );
            }
        }
#endif

        if (appOptions.listenMode)
        {
            if (!appOptions.notls)
            {
                if (!appOptions.cafile.empty())
                {
                    if (access(appOptions.cafile.c_str(),R_OK))
                    {
                        log->log0(__func__,Logs::LEVEL_CRITICAL, "X.509 Certificate Authority File Not Found.");
                        exit(-11);
                        return false;
                    }
                    ((CX2::Network::TLS::Socket_TLS *)sock)->setTLSCertificateAuthorityPath(appOptions.cafile.c_str());
                    log->log0(__func__,Logs::LEVEL_INFO, "Clients will be authenticated with the TLS Certificate Authority");
                }
                else
                    log->log0(__func__,Logs::LEVEL_WARN, "Clients can come without TLS signature, Internal VPN IP will be exposed.");

                // Need server certificates:
                if (access(appOptions.keyfile.c_str(),R_OK))
                {
                    log->log0(__func__,Logs::LEVEL_CRITICAL, "X.509 Private Key not found.");
                    exit(-10);
                    return false;
                }
                ((CX2::Network::TLS::Socket_TLS *)sock)->setTLSPrivateKeyPath(appOptions.keyfile.c_str());

                if (access(appOptions.certfile.c_str(),R_OK))
                {
                    log->log0(__func__,Logs::LEVEL_CRITICAL, "X.509 Certificate not found.");
                    exit(-11);
                    return false;
                }
                ((CX2::Network::TLS::Socket_TLS *)sock)->setTLSPublicKeyPath(appOptions.certfile.c_str());
            }

            if (!sock->listenOn( appOptions.port, appOptions.addr.c_str() ))
            {
                log->log0(__func__,Logs::LEVEL_CRITICAL, "Unable to listen at %s:%d",appOptions.addr.c_str(), appOptions.port);
                exit(-20);
                return false;
            }

            multiThreadedAcceptor.setAcceptorSocket(sock);
            multiThreadedAcceptor.setCallbackOnConnect(TLS_Callbacks::onConnect,&appOptions);
            multiThreadedAcceptor.setCallbackOnInitFail(TLS_Callbacks::onInitFailed,this);
            multiThreadedAcceptor.setCallbackOnTimedOut(TLS_Callbacks::onTimeOut,this);
            multiThreadedAcceptor.setMaxConcurrentClients(appOptions.threadsLimit);
        }
        else
        {
            if (!appOptions.notls)
            {
                // Need validate with ca...
                if (access(appOptions.cafile.c_str(),R_OK))
                {
                    log->log0(__func__,Logs::LEVEL_CRITICAL, "X.509 Certificate Authority not found.");
                    exit(-11);
                    return false;
                }
                ((CX2::Network::TLS::Socket_TLS *)sock)->setTLSCertificateAuthorityPath(appOptions.cafile.c_str());
            }
        }


        return true;
    }

    int _start(int argc, char *argv[], Arguments::GlobalArguments * globalArguments)
    {
        std::thread( virtIfaceReader, &appOptions ).detach();

        if (appOptions.listenMode)
        {
            multiThreadedAcceptor.startThreaded();
            log->log0(__func__,Logs::LEVEL_INFO, "VPN Server Loaded @%s:%d", appOptions.addr.c_str(),appOptions.port);
        }
        else
        {
            for (;;)
            {
                if (sock)
                    delete sock;

                sock = (appOptions.notls?new Network::Sockets::Socket_TCP:new Network::TLS::Socket_TLS ) ;
                sock->setUseIPv6( !appOptions.ipv4 );

                if (!appOptions.notls)
                {
                    if (access(appOptions.cafile.c_str(),R_OK))
                    {
                        log->log0(__func__,Logs::LEVEL_CRITICAL, "X.509 Certificate Authority not found.");
                        exit(-11);
                        return false;
                    }
                    ((CX2::Network::TLS::Socket_TLS *)sock)->setTLSCertificateAuthorityPath(appOptions.cafile.c_str());
                }


                log->log0(__func__,Logs::LEVEL_INFO, "Connecting to %s://%s:%d", appOptions.notls?"tcp":"tls",appOptions.addr.c_str(),appOptions.port);

                if (sock->connectTo(appOptions.addr.c_str(),appOptions.port))
                    TLS_Callbacks::onConnect(&appOptions,sock, appOptions.addr.c_str(),true);
                else
                {
                    log->log0(__func__,Logs::LEVEL_ERR, "Connecting failed to %s://%s:%d - [%s]",  appOptions.notls?"tcp":"tls", appOptions.addr.c_str(),appOptions.port, sock->getLastError().c_str());

                    if (!appOptions.notls)
                    {
                        for (const auto & i: ((CX2::Network::TLS::Socket_TLS *)sock)->getTLSErrorsAndClear())
                            log->log0(__func__,Logs::LEVEL_ERR, "TLS Error - [%s]", i.c_str());
                    }
                }

                sleep(5);
            }
        }

        return 0;
    }

private:
    sAppOptions appOptions;
    Logs::AppLog * log;
    Network::Sockets::Socket_TCP *sock;
    Network::Sockets::Acceptors::Socket_Acceptor_MultiThreaded multiThreadedAcceptor;
};



int main(int argc, char *argv[])
{
    SecureTAPApp * main = new SecureTAPApp;
    return StartApplication(argc,argv,main);
}

