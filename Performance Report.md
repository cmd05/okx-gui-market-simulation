# Performance Report

## Internal Latency Analysis

The system implements latency tracking through:
- High-resolution clock measurements
- Per-tick processing time tracking
- FPS monitoring for UI responsiveness

```cpp
// From client_main.cpp
float g_tick_latency = 0;
float g_fps = 0;
benchmark g_benchmark {"g_benchmark"};
```

The time taken for different operations is tracked using the `chrono` library and rendering times, using `glfwGetTime`.

- The [Latency Benchmarking](#Latency%20Benchmarking) section gives detailed analysis of the latency of various parts of the program.

## CPU Optimizations

![](./_assets/Pasted%20image%2020250521203027.png)

The functions `glfwInit` and `glfwCreateWindow` are only called once during initialization.

- One of the CPU bottlenecks is `glfwSwapBuffers`, which renders the frame and prepares the buffers for the next frame.

The output calculations involve calculations of the expected slippage. It involves the encoding and decoding of JSON data, between the C++ application and Python server to predict the slippage from the live asks and bids from the pre-trained regression model.
- The possible optimizations section details performance optimizations for this function.

![](./_assets/Pasted%20image%2020250521203915.png)

## Memory Usage Analysis

The application does not do any manual dynamic memory allocations. It uses standard containers such as `std::vector` and `std::array`.

The memory benchmarking show that the heap usage is nearly constant, and the allocations are done by ImGui, OpenSSL and other libraries used.

![](./_assets/Pasted%20image%2020250521192037.png)

Checking for memory leaks using `valgrind`.

```
==3992== All heap blocks were freed -- no leaks are possible
==3992==
==3992== ERROR SUMMARY: 0 errors from 0 contexts (suppressed: 0 from 0)
```
## Latency Benchmarking

The latency benchmarking is done using a special class defined for benchmarking.

```cpp
class benchmark {
public:
    benchmark(std::string lab): label{lab} {}
    
    void reset(std::string lab = "");

    void start();
    void end();
private:
    std::string label;

    bool started = false;
    std::chrono::time_point<std::chrono::high_resolution_clock> start_time;
    std::chrono::time_point<std::chrono::high_resolution_clock> end_time;
};
```

In order to support end to end benchmarking (as well as across threads), we have defined a `benchmark g_benchmark {"g_benchmark"};` in `client_main.cpp` which can be accessed by other source files using `extern`.

The tick latency is given by the variable `g_tick_latency`.
- The end to end loop of the UI runs at around 100 FPS, i.e around 10ms tick latency.
	- **Note**: Due to using GUI applications in WSL, the frame rate and latency is lower compared to running directly on the OS, which would give a significant performance increase

The calculations of the estimation costs occur at around 1.5 milliseconds:

```
[      2460ms] Started benchmark: calc_benchmark
[      2462ms] Benchmark: calc_benchmark, took 1588 us
```

### Possible Optimizations

- Using python embedding inside C++ code using `Python.h`, to call the slippage prediction functions.
	- Alternatively, we can use more efficient formats for exchange data between the sockets, such as using MessagePack or Protocol Buffer.
- We can cache the market impact for frequent or close values of the order volume. We can use an `unordered_map` to map the volumes to the respective market impact

- Instead of using a function map in the Python server, we can support batch functions which calculate various parameters in one call instead of multiple socket requests.