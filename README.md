# cpp-networking

A compact set of C++ networking examples built with **Boost.Asio** and **Boost.Beast**.

## What is in this repo

This repository contains five standalone examples:

1. **`ex1.cpp`** – async UDP echo server (uppercases incoming messages).
2. **`ex2.cpp`** – ICMP subnet scanner (`CIDR` input, prints responsive hosts).
3. **`ex3_1.cpp`** – synchronous HTTP GET client (`example.com`).
4. **`ex3_2.cpp`** – asynchronous HTTP GET client (`example.com`).
5. **`ex4_async2.cc`** – asynchronous static file HTTP server on port `8080`.

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

> Note: `ex2.cpp` includes `../ICMP/icmp_header.hpp` and `../ICMP/ipv4_header.hpp`.
> Those headers are not present in this repository and must be supplied (for example,
> from the Boost.Asio ICMP ping example layout) before `ex2.cpp` can compile.

---

## Build

From repository root:

```bash
g++ -std=c++17 -O2 -Wall -Wextra -pedantic ex1.cpp -o ex1 -lboost_system -lpthread

g++ -std=c++17 -O2 -Wall -Wextra -pedantic ex2.cpp -o ex2 -lboost_system -lpthread

g++ -std=c++17 -O2 -Wall -Wextra -pedantic ex3_1.cpp -o ex3_1 -lboost_system -lpthread

g++ -std=c++17 -O2 -Wall -Wextra -pedantic ex3_2.cpp -o ex3_2 -lboost_system -lpthread

g++ -std=c++17 -O2 -Wall -Wextra -pedantic ex4_async2.cc -o ex4_async2 -lboost_system -lpthread
```

---

## Run

### 1) UDP uppercase echo server (`ex1`)

Start server on port `9000`:

```bash
./ex1 9000
```

Test with netcat from another terminal:

```bash
echo "hello networking" | nc -u -w1 127.0.0.1 9000
```

Expected response: `HELLO NETWORKING`

### 2) ICMP subnet scanner (`ex2`)

```bash
sudo ./ex2 192.168.1.0/24
```

Prints hosts that replied to ICMP echo requests.

### 3) Sync HTTP client (`ex3_1`)

```bash
./ex3_1
```

Performs an HTTP GET request to `http://example.com/` and prints the full response.

### 4) Async HTTP client (`ex3_2`)

```bash
./ex3_2
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
