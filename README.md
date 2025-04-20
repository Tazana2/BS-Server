# BS-Server — Battleship Server 🛡️⚓

A multiclient server written in C that manages turn-based Battleship games over a custom network protocol.

This server uses `poll()` to handle concurrent client connections and a custom text-based protocol (BSProtocol) for communication.

See more documentation in [our wiki page](https://github.com/Tazana2/BS-Server/wiki)!

---

## Key Features

- Support for multiple simultaneous clients.
- User login and logout handling.
- Game invitation and match management.
- Turn-based gameplay between players.
- Attack validation and result processing (HIT/MISS/SINK).
- Custom text-based protocol (BSProtocol).
- Modular and maintainable architecture.
- Logging system for debugging and game tracking.

---

## Requirements

- Unix-based system (Linux/macOS/WSL).
- GCC (C compiler).
- Make (to build the project).

---

## 🛠️ Build Instructions

From the `src/` directory, run:

```bash
make
```

This will produce the `server` executable.

---

## 🚀 Running the Server

To start the server:

```bash
./server <ip> <port> <path_to_log_file>
```

### Example

```bash
./server 127.0.0.1 8080 server_log.txt
```

> The server will start listening for client connections at the specified IP and port. All activity will be logged to the file provided.

---

## Technologies Used

- C
- TCP sockets
- Non-blocking I/O with `poll()`
- Modular design in C