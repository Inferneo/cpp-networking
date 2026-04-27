# cpp-networking

A compact set of C++ networking examples built with **Boost.Asio** and **Boost.Beast**.

## What is in this repo

This repository contains five standalone examples:

1. **`udp_uppercase_echo_server.cpp`** – async UDP echo server (uppercases incoming messages).
2. **`icmp_subnet_scanner.cpp`** – ICMP subnet scanner (`CIDR` input, prints responsive hosts).
3. **`http_client_sync.cpp`** – synchronous HTTP GET client (`example.com`).
4. **`http_client_async.cpp`** – asynchronous HTTP GET client (`example.com`).
5. **`ex4_async2.cc`** – asynchronous static file HTTP server on port `8080`.

### Rename map (old → new)

- `ex1.cpp` → `udp_uppercase_echo_server.cpp`
- `ex2.cpp` → `icmp_subnet_scanner.cpp`
- `ex3_1.cpp` → `http_client_sync.cpp`
- `ex3_2.cpp` → `http_client_async.cpp`

---

## Prerequisites

- C++17-compatible compiler (e.g., `g++` 10+ / `clang++` 12+)
- Boost headers and libraries (Asio/Beast system dependencies)
- Linux/macOS environment (examples use POSIX-style behavior)

For Debian/Ubuntu:

```bash
sudo apt-get update
sudo apt-get install -y build-essential libboost-all-dev
```

> Note: `icmp_subnet_scanner.cpp` includes `../ICMP/icmp_header.hpp` and
> `../ICMP/ipv4_header.hpp`. Those headers are not present in this repository and
> must be supplied (for example, from the Boost.Asio ICMP ping example layout)
> before `icmp_subnet_scanner.cpp` can compile.

---

## Build

From repository root:

```bash
g++ -std=c++17 -O2 -Wall -Wextra -pedantic udp_uppercase_echo_server.cpp -o udp_uppercase_echo_server -lboost_system -lpthread

g++ -std=c++17 -O2 -Wall -Wextra -pedantic icmp_subnet_scanner.cpp -o icmp_subnet_scanner -lboost_system -lpthread

g++ -std=c++17 -O2 -Wall -Wextra -pedantic http_client_sync.cpp -o http_client_sync -lboost_system -lpthread

g++ -std=c++17 -O2 -Wall -Wextra -pedantic http_client_async.cpp -o http_client_async -lboost_system -lpthread

g++ -std=c++17 -O2 -Wall -Wextra -pedantic ex4_async2.cc -o ex4_async2 -lboost_system -lpthread
```

---

## Run

### 1) UDP uppercase echo server (`udp_uppercase_echo_server`)

Start server on port `9000`:

```bash
./udp_uppercase_echo_server 9000
```

Test with netcat from another terminal:

```bash
echo "hello networking" | nc -u -w1 127.0.0.1 9000
```

Expected response: `HELLO NETWORKING`

### 2) ICMP subnet scanner (`icmp_subnet_scanner`)

```bash
sudo ./icmp_subnet_scanner 192.168.1.0/24
```

Prints hosts that replied to ICMP echo requests.

### 3) Sync HTTP client (`http_client_sync`)

```bash
./http_client_sync
```

Performs an HTTP GET request to `http://example.com/` and prints the full response.

### 4) Async HTTP client (`http_client_async`)

```bash
./http_client_async
```

Resolves, connects, writes request, and reads response asynchronously.

### 5) Async static HTTP server (`ex4_async2`)

```bash
./ex4_async2
```

Then open:

- `http://127.0.0.1:8080/` (serves `./index.html`)

If a file is missing, returns `404 file not found`.

---

## Notes

- These examples are intentionally minimal and educational.
- Error handling is basic and can be extended for production use.
- `ex4_async2.cc` currently supports only HTTP `GET` requests.
