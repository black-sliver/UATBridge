#include "bridge.h"
#include <iostream>


namespace UATBridge {

const char* Bridge::NAME = "UATBridge";
const char* Bridge::VERSION = "0.1.0";


Bridge::Bridge()
{
    _uatserver = nullptr; // don't listen until game is connected
    _wsgameserver = new WSGameServer(NAME, VERSION, &_service);

    _wsgameserver->set_game_info_handler([this](GameServer*, json& info) {
        std::cout << "GameInfo: " << info["name"].dump() << " "
                  << info["version"].dump() << std::endl;
        enable_uat(info["name"], info["version"], info["slots"]);
    });

    _wsgameserver->set_vars_handler([this](GameServer*, std::list<json>& jvars) {
        for (auto& var: jvars) {
            std::cout << var["slot"].dump() << ":" << var["name"].dump()
                      << " = " << var["value"].dump() << std::endl;
        }

        std::list<UAT::Var> vars;
        for (auto& jvar: jvars) {
            UAT::Var var;
            var.name = jvar["name"];
            var.slot = jvar["slot"].is_string() ? jvar["slot"] : "";
            var.value = jvar["value"];
            vars.push_back(var);
        }
        if (_uatserver) {
            _uatserver->set_vars(vars);
            enable_uat();
        } else {
            enable_uat();
            _uatserver->set_vars(vars);
        }
    });
}

Bridge::~Bridge()
{
    delete _uatserver;
    delete _wsgameserver;
}

bool Bridge::run()
{
    asio::signal_set signals(_service, SIGINT, SIGTERM);
    signals.async_wait([this,&signals](...) {
        if (_wsgameserver) _wsgameserver->stop();
        if (_uatserver) _uatserver->stop();
        signals.clear(); // second signal will terminate
    });
    _service.run();
    std::cout << "Bridge closed" << std::endl;
    return true;
}

bool Bridge::enable_uat(json& name, json& version, json& jslots)
{
    if (!_uatserver) _uatserver = new UAT::Server(&_service);
    _uatserver->set_name(name.is_string() ? name : "");
    _uatserver->set_version(version.is_string() ? version : "");
    std::set<std::string> slots;
    if (jslots.is_array()) {
        for (auto& slot : jslots) {
            slots.insert(slot.get<std::string>());
        }
    }
    _uatserver->set_slots(slots);
    return _uatserver->start();
}

bool Bridge::enable_uat()
{
    if (!_uatserver) _uatserver = new UAT::Server(&_service);
    return _uatserver->start();
}

} // namespace UATBridge
