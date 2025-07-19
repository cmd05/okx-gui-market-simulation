# GUI-Based OKX Market Simulation

## Objective

Create a GUI-based high-performance trade simulator leveraging real-time market data to estimate transaction costs and market impact. This system will connect to provided WebSocket endpoints that stream full L2 orderbook data for cryptocurrency exchanges.

OKX API Reference: https://www.okx.com/docs-v5/en/

## Performance Analysis

A detailed analysis of profiling and benchmarking can be found in the [Performance Report](./Performance%20Report.md) document.

## Code Review

The explanation of the code can be found in the [Code Review](./Code%20Review.md) document.

## Demo

https://github.com/user-attachments/assets/144b97f4-ed59-48b2-b47f-6b5ef71a6148

## Build

The code can be built using the commands.

```
cmake -B build
cmake --build build -j$(nproc)
```

It has been built on WSL2 Ubuntu. The executable will be created on the project root as `client_trader`

## Core Components

![](./_assets/Pasted%20image%2020250521184540.png)

## Libraries used

* [GLFW](https://github.com/glfw/glfw)
* [GLAD](https://github.com/Dav1dde/glad)
* [ImGui](https://github.com/Dav1dde/glad)
* [websocketspp](https://github.com/zaphoyd/websocketpp)
* [nlohmann json](https://github.com/nlohmann/json)