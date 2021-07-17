# UAT Protocol

The tracker-facing protocol is descibed [here](https://github.com/black-sliver/PopTracker/blob/master/doc/UAT-PROTO.md).



# Websocket Game Interface

The game-facing websocket protocol is described below. It follows the same style
as UAT, but defines different commands and runs on a different port.

The bridge implements a websocket server, the game implements a websocket client.


## Default Host and Port

`ws://localhost:65397`\
`ws://localhost:44442` fallback


## Framing

See [UAT#Framing](https://github.com/black-sliver/UAT/blob/master/PROTOCOL.md#Framing).


## Naming

See [UAT#Naming](https://github.com/black-sliver/UAT/blob/master/PROTOCOL.md#Naming).


## Commands Client -> Server

### GameInfo
Sets game/mod info and slots (players/seats, if any).\
Always send this after connecting (after receiving `Info`) to notify the bridge
that a game is connected.

Argument  | Required | Type             | Description
--------- | -------- | ---------------- | ----------- 
`name`    | optional | string           | representing the mod or game name
`version` | optional | string           | representing the mod or game version
`slots`   | optional | array of strings | identifiers if the game has multiple players/seats/slots
#### Example
```
> [{"cmd": "GameInfo", "name": "SomeGame Randomizer",
    "version": "1.0.0-mod1.0.0", "slots": []}]
```

### Var
Create or set a variable in the server. This will be forwarded to the tracker as
needed. When connecting to a server, send GameInfo + all variables in one packet
to make sure there are no left over values in the cache.

Argument  | Required | Type             | Description
--------- | -------- | ---------------- | ----------- 
`name`    | required | string           | name of the variable
`value`   | required | any              | value of the variable
`slot`    | optional | string           | identifier for player/seat/slot
#### Example
```
> [{"cmd": "Var", "name": "Sword", "value": 2}, ...]
```


## Commands Server -> Client

### Info
This is automatically sent when connecting to the server to detect protocol
version, features and software name/version.

Argument  | Required | Type             | Description
--------- | -------- | ---------------- | ----------- 
`protocol`| required | integer          | the protocol version/revision
`name`    | optional | string           | representing the server name
`version` | optional | string           | representing the server version
`features`| optional | array of strings | optional features supported
#### Example
```
> [{"cmd": "Info", "name": "UATBridge", "version": "1.0.0",
    "protocol": 0, "features": []}]
```

### ErrorReply
See [UAT#ErrorReply](https://github.com/black-sliver/UAT/blob/master/PROTOCOL.md#ErrorReply).



# TCP Game Interface

For games that can not easily create a websocket, but can create a TCP socket.
This is planned but not implemented yet.
