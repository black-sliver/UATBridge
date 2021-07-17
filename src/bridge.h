#ifndef _BRIDGE_H_INCLUDED
#define _BRIDGE_H_INCLUDED

#include <uatserverpp.hpp>
#include "wsgameserver.h"

namespace UATBridge {

class Bridge
{
public:
    static const char* NAME;
    static const char* VERSION;

protected:
    typedef nlohmann::json json;

    asio::io_service _service;
    UAT::Server* _uatserver;
    WSGameServer* _wsgameserver;
    
    bool enable_uat(json& name, json& version, json& slots);
    bool enable_uat();

public:
    Bridge();
    virtual ~Bridge();

    bool run();
};

} // namespace UATBridge


#endif // _BRIDGE_H_INCLUDED
