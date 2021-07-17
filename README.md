# UATBridge

Tool to let applications use UAT protocol that can not easily implement a
websocket server.

It will only start listening on the UAT port once a game is connected to avoid
blocking the port.

If you want to write a program/mod compatible to UATBridge check out
[PROTOCOL.md](PROCOTOL.md). JSON Schema can be found in [schema](schema) or in
the `schema` branch.
