#ifndef _WSGAMESERVER_H_INCLUDED
#define _WSGAMESERVER_H_INCLUDED

#if !defined(ASIO_STANDALONE) && !defined(ASIO_BOOST)
#define ASIO_STANDALONE
#endif
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include "gameserver.h"


namespace UATBridge {


class WSGameServer : public GameServer
{
protected:
    typedef websocketpp::server<websocketpp::config::asio> WSServer;

public:
    static constexpr int DEFAULT_PORT = 65397;
    static constexpr int FALLBACK_PORT = 44442;

protected:
    WSServer _wss;
    std::list<WSServer::connection_ptr> _connections;

    void log(const std::string& msg)
    {
        _wss.get_alog().write(websocketpp::log::alevel::app, "Game: " + msg);
    }

    void on_message(websocketpp::connection_hdl hdl, WSServer::message_ptr msg)
    {
        log("message");
        try {
            json packet = json::parse(msg->get_payload());
            valijson::Validator validator;
            json reply = {};
            std::list<json> vars; // collect and handle multiple Vars at once

            // FIXME: it is undefined what to do if we encounter a bad command
            //        inside an othervise valid packet. For now we validate
            //        individual commands and accept the rest
            JsonSchemaAdapter packetAdapter(packet);
            if (!validator.validate(_packetSchema, packetAdapter, NULL)) {
                throw std::runtime_error("Packet validation failed");
            }

            for (auto& command: packet) {
                std::string cmd = command["cmd"];
                JsonSchemaAdapter commandAdapter(command);
                valijson::ValidationResults res;
                valijson::ValidationResults::Error err;
                try {
                    if (cmd != "Var" && !vars.empty()) {
                        // push vars if next command is not a var to retain order
                        if (_onVars) _onVars(this, vars);
                        vars.clear();
                    }
                    if (cmd == "GameInfo") {
                        if (!validator.validate(_gameInfoSchema, commandAdapter, &res)) {
                            if (res.popError(err)) {
                                throw CommandError(cmd, err);
                            }
                            throw CommandError(cmd, "", "unknown");
                        }
                        if (_onGameInfo) _onGameInfo(this, command);
                    } else if (cmd == "Var") {
                        if (!validator.validate(_varSchema, commandAdapter, &res)) {
                            if (res.popError(err)) {
                                throw CommandError(cmd, err);
                            }
                            throw CommandError(cmd, "", "unknown");
                        }
                        vars.push_back(command);
                    } else {
                        throw CommandError(cmd, "", "unnknown cmd");
                    }
                } catch (CommandError& ex) {
                    reply.push_back(ex.to_json());
                } catch (std::exception& ex) {
                    throw std::runtime_error("Exception while handling " + cmd +
                                             ": " + ex.what());
                }
            }
            if (!reply.empty()) _wss.send(hdl, reply.dump(), websocketpp::frame::opcode::text);
            if (_onVars && !vars.empty()) _onVars(this, vars);
        } catch (std::exception& ex) {
            _wss.get_con_from_hdl(hdl)->close(websocketpp::close::status::invalid_subprotocol_data, ex.what());
        }
    }

    void on_open(websocketpp::connection_hdl hdl)
    {
        log("open");

        _connections.push_back(_wss.get_con_from_hdl(hdl));

        json info = { {
            {"cmd", "Info"},
            {"name", _name},
            {"version", _version},
            {"protocol", PROTOCOL_VERSION},
        }, };
        _wss.send(hdl, info.dump(), websocketpp::frame::opcode::text);
    }

    void on_close(websocketpp::connection_hdl hdl)
    {
        log("close");
        _connections.remove(_wss.get_con_from_hdl(hdl));
    }

public:
    WSGameServer(const std::string& name, const std::string& version, asio::io_service* service=nullptr)
        : GameServer(name, version)
    {
        _wss.clear_access_channels(websocketpp::log::alevel::frame_header);
        _wss.clear_access_channels(websocketpp::log::alevel::frame_payload);
        _wss.clear_access_channels(websocketpp::log::alevel::control);
        _wss.init_asio(service);
        _wss.set_message_handler([this] (websocketpp::connection_hdl hdl, WSServer::message_ptr msg) {
            this->on_message(hdl,msg);
        });
        _wss.set_open_handler([this] (websocketpp::connection_hdl hdl) {
            this->on_open(hdl);
        });
        _wss.set_close_handler([this] (websocketpp::connection_hdl hdl) {
            this->on_close(hdl);
        });
        _wss.set_reuse_addr(true);
        auto ports = { DEFAULT_PORT, FALLBACK_PORT };
        for (auto& port : ports) {
            try {
                _wss.listen(port);
                _wss.start_accept();
                log("listening on " + std::to_string(port));
                break;
            } catch (websocketpp::exception& ex) {
                log("could not listen on " + std::to_string(port));
                if (&port == ports.end()-1) throw ex; //rethrow
            }
        }
    }

    virtual ~WSGameServer()
    {
    }

    void stop()
    {
        // NOTE: run this on the same service (io_service::post) or add locking
        log("stop listen");
        _wss.stop_listening();
        log("closing connections");
        for (auto& conn: _connections) {
            conn->close(websocketpp::close::status::going_away, "");
        }
    }
};


} // namespace UATBridge


#endif // _WSGAMESERVER_H_INCLUDED
