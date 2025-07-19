# Code Review

## Core Components

![](./_assets/Pasted%20image%2020250521184540.png)

## Code Structure

The code structure is as follows

```
├── client
│   └── client_trader.h
├── client_main.cpp
├── gui
│   ├── GUIMain.h
│   └── GUIState.h
├── lib
│   ├── benchmark.h
│   └── utilities.h
├── models
│   ├── btc_slippage_model.bin
│   ├── data
│   │   ├── btc_response_1.json
│   │   ├── <order book training data>
│   ├── eth_slippage_model.bin
│   ├── predict_slippage.py
│   ├── socket_server.py
│   ├── train_slippage.py
│   └── utils.py
└── websocket
    ├── websocket.cpp
    └── websocket.h
```

The main function is contained in the file `client_main.cpp`.

### UI Layer

The UI is handled using **ImGui** and **GLFW** libraries in the C++ application.  

![](./_assets/Pasted%20image%2020250521160937.png)

The interface of the UI has:
- **Left Panel** - Contains Input Parameters. This includes the instrument name, order quantity (fixed), volatility and fee tier
- **Right Panel** - Contains Output Parameters. Mid price, slippage, market impact, fees and the net cost.

The file `GUIState.h` holds the classes `InputWindowState`, `InputData` and `OutputData`. They define the state of the window as well as the data used for output calculations.

### Websockets

The files `websocket/websocket.{h,cpp}` provide declarations and definitions for the application's websocket endpoint.

- It involves a `connection_metadata` class to store the communication between the client and the server.
* The `websocket_endpoint` class holds all methods relevant to websockets communication, such as `connect()`, `send()`, `on_message()` etc. It is done using the [websocketspp](https://github.com/zaphoyd/websocketpp) library

### Client Trader

The `ClientTrader` class provides the interface through which the websocket endpoint is called and the L2 order book is fetched.
- It can hold multiple connections for different instruments such as `BTC`, `ETH` etc.

### Lib/Utilities

* `lib/utilties.h` provides helpful functions for logging, as well as customizing the log stream by using flags. `lib/benchmark.h` provides a class for benchmarking as discussed in the latency report.

````c
// Logging Format
// [<time>] <log type>: <msg>
enum class log_flags {
    none          = 0,
    ws            = 1,
    client_trader = 2,
	// ...
};

constexpr static log_flags ENABLED_LOG_FLAGS = (log_flags::ws | log_flags::client_trader ... )

// Example usage
APP_LOG(log_flags::ws, "Error sending message: " << ec.message());
````

### Estimating Costs

This code estimates the total trading cost of an order using:

- A Python-trained regression model (predicts slippage),
- A C++ implementation of the Almgren–Chriss market impact model, and
- A rule-based fee estimation.

#### Slippage Calculations

The C++ program calls the Python function `predict_slippage_runtime`, which returns the following parameters.

```json
{
  "predicted_slippage_pct": ...,
  "spread_pct": ...,
  "mid_price": ...
}
```

The call is made using a local TCP socket at port `9000` between the C++ program.

A **regression** model is trained based on previous market data, to predict the live slippage values. It is done using the `sklearn` module.

The code for the model is in `models/train_slippage.py` and runtime prediction in `models/predict_slippage.py`.
#### Market Impact

The market impact is predict using the Almgren-Chriss model with the following constants. We do not require the entire dynamic programming loop as suggested in the original code, since we are executing a **single order**, not over a schedule.

```c
// Almgren–Chriss constants
constexpr static float alpha = 1;
constexpr static float eta = 0.05;
constexpr static float beta = 1;
constexpr static float gamma = 0.05;
```

The function `estimate_market_impact` in `client_main.cpp` estimates the market impact by calculating the temporary and permanent market impact.

### Python Server

The python server handles local TCP socket connections to predict slippage at runtime using the regression model. The code is located in `models/socket_server.py`.
- It loads the models for supported instruments (ex: BTC and ETH) using `pickle`.
- It uses `json` for communication over the socket and a function map for the function to be called for a given request.