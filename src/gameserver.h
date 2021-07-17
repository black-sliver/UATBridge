#ifndef _GAMESERVER_H_INCLUDED
#define _GAMESERVER_H_INCLUDED

#include <nlohmann/json.hpp>
#include <list>
#include <functional>
#include <valijson/adapters/nlohmann_json_adapter.hpp>
#include <valijson/schema.hpp>
#include <valijson/schema_parser.hpp>
#include <valijson/validator.hpp>


namespace UATBridge {


class GameServer
{
public:
    static constexpr int PROTOCOL_VERSION = 0;

protected:
    typedef nlohmann::json json;
    typedef valijson::adapters::NlohmannJsonAdapter JsonSchemaAdapter;

public:
    class CommandError : public std::exception
    {
        std::string _cmd;
        std::string _arg;
        std::string _reason;
        std::string _description;
        std::string _msg;
    public:
        CommandError(const std::string& cmd, const std::string& arg, const std::string& reason)
            : _cmd(cmd), _arg(arg), _reason(reason)
        {
            _msg = _arg.empty() ?
                   _reason + ": " + _cmd :
                   _reason + ": " + _cmd + "." + _arg;
        }
        CommandError(const std::string& cmd, const valijson::ValidationResults::Error& err)
            : _cmd(cmd), _description(err.description)
        {            
            if (err.context.size() < 2) { // error in root
                const char* p0 = err.description.c_str();
                const char* p2 = p0 + err.description.length() - 2;
                if (p2>p0 && *p2 == '\'' && *(p2+1) == '.') {
                    const char* p1 = p2-1;
                    while (p1>p0 && *p1 != '\'') p1--;
                    if (*p1 == '\'') _arg = std::string(p1+1, p2-p1-1);
                }
                _reason = _arg.empty() ? "unknown" : "missing agument";
            } else { // error in property = argument
                _arg = err.context[1];
                _arg = _arg.substr(1, _arg.length()-2);
                _reason = "bad value";
            }

            _msg = _arg.empty() ?
                   _reason + ": " + _cmd + ": " + _description:
                   _reason + ": " + _cmd + "." + _arg + ": " + _description;
        }
        virtual const char* what() const noexcept override { return _msg.c_str(); }
        virtual json to_json() const {
            json j = { {"cmd", "ErrorReply"}, {"name", _cmd}, {"reason", _reason} };
            if (!_arg.empty()) j["argument"] = _arg;
            if (!_description.empty()) j["description"] = _description;
            return j;
        }
    };

protected:
    std::string _name;
    std::string _version;
    std::function<void(GameServer*, json&)> _onGameInfo;
    std::function<void(GameServer*, std::list<json>&)> _onVars;
    const json _packetSchemaJson = R"({
        "type": "array",
        "items": {
            "type": "object",
            "properties": {
                "cmd": { "type": "string" }
            },
            "required": [ "cmd" ]
        }
    })"_json;
    const json _gameInfoSchemaJson = R"({
        "type": "object",
        "properties": {
            "name": { "type": "string" },
            "version": { "type": "string" },
            "slots" : {
                "type": "array",
                "items": { "type": "string" }
            }
        }
    })"_json;
    const json _varSchemaJson = R"({
        "type": "object",
        "properties": {
            "name": { "type": "string" },
            "slot": { "type": "string" },
            "value": {}
        },
        "required": [ "name", "value" ]
    })"_json;
    valijson::Schema _packetSchema;
    valijson::Schema _gameInfoSchema;
    valijson::Schema _varSchema;

public:
    GameServer(const std::string& name, const std::string& version)
    {
        _name = name;
        _version = version;

        valijson::SchemaParser parser;
        parser.populateSchema(JsonSchemaAdapter(_packetSchemaJson), _packetSchema);
        parser.populateSchema(JsonSchemaAdapter(_gameInfoSchemaJson), _gameInfoSchema);
        parser.populateSchema(JsonSchemaAdapter(_varSchemaJson), _varSchema);
    }
    
    virtual ~GameServer()
    {
    }
    
    void set_game_info_handler(std::function<void(GameServer*, json&)> fn)
    {
        _onGameInfo = fn;
    }

    void set_vars_handler(std::function<void(GameServer*, std::list<json>&)> fn)
    {
        _onVars = fn;
    }
};


} // namespace UATBridge


#endif // _GAMESERVER_H_INCLUDED
