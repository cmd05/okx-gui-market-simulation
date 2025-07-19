#include <iostream>
#include <fstream>
#include <string>
#include <memory>

#include <chrono>
using namespace std::chrono_literals;

#include <websocket/websocket.h>
#include <client_trader/client_trader.h>
#include <gui/GUIState.h>
#include <gui/GUIMain.h>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include <chrono>
#include <lib/utilities.h>
#include <lib/benchmark.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

constexpr int MAX_SOCKET_BUFFER = 16384;

float g_curr_time = 0;
float g_last_time = 0;
float g_tick_latency = 0;
float g_fps = 0;

InputWindowState g_input_window_state;

std::chrono::time_point<std::chrono::high_resolution_clock> g_timer_start;
benchmark g_benchmark {"g_benchmark"};

/// ------------ Socket Initialization ------------
int socket_client_init() {
    const char* server_ip = "127.0.0.1";
    const int server_port = 9000;

    // create socket
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Socket creation failed");
        return 1;
    }

    // setup address and parameters
    sockaddr_in serv_addr{};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(server_port);

    if (inet_pton(AF_INET, server_ip, &serv_addr.sin_addr) <= 0) {
        perror("Invalid address");
        return 1;
    }

    // connect to the server
    if (connect(sock, (sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cerr << "Connection failed: " << std::strerror(errno) << std::endl;
        return 1;
    } else {
        std::cout << "Connected to Python server." << std::endl;
    }

    return sock;
}

/// ------------ Market Impact Calculations ------------
float ac_market_temporary_impact(float volume) {
    return g_input_window_state.eta * pow(volume, g_input_window_state.alpha);
}

float ac_market_permanent_impact(float volume) {
    return g_input_window_state.gamma * pow(volume, g_input_window_state.beta);
}

float estimate_market_impact(float volume) {
    float temp_impact = ac_market_temporary_impact(volume);
    float perm_impact = ac_market_permanent_impact(volume);

    return temp_impact + perm_impact;
}

/// ------------ Slippage Calculations ------------
json find_expected_slippage(int sock, InputData& input_data) {
    json request;
    request["method"] = "expected_slippage";

    request["params"] = json::object();
    request["params"]["instrument"] = input_data.instrument;
    request["params"]["order_sz"] = input_data.order_sz;
    request["params"]["fee_pct"] = input_data.fee_pct;
    request["params"]["volatility_pct"] = input_data.volatility_pct;
    request["params"]["asks"] = input_data.live_data["asks"];
    request["params"]["bids"] = input_data.live_data["bids"];

    // send request
    std::string req_str = request.dump();
    send(sock, req_str.c_str(), req_str.size(), 0);

    // receive response
    char buffer[MAX_SOCKET_BUFFER] = {0};
    int bytes = recv(sock, buffer, sizeof(buffer) - 1, 0);

    if (bytes > 0) {
        std::string resp_str(buffer, bytes);
        json response = json::parse(resp_str);
        return response;
    }

    return {};
}

int main() {
    // start global timer
    g_timer_start = std::chrono::high_resolution_clock::now();

    int client_socket = socket_client_init();

    // GUI Initialization
    GUIMain gui_main;

    // Initialize input and output data state
    InputData input_data;
    gui_main.fill_input_data_gui(input_data);
    OutputData output_data;

    // Initialize trader class
    ClientTrader trader;
    int ws_connection = trader.connect(input_data.instrument);

    // Show connection status
    trader.print_messages(ws_connection);
    benchmark calc_benchmark {"calc_benchmark"};

    while (!gui_main.window_should_close()) {
        gui_main.process_input();
        gui_main.clear_buffers();

        // handle gui logic
        if(g_input_window_state.update_btn_clicked) {
            bool valid_update = true;

            if(g_input_window_state.instrument != input_data.instrument) {
                // new instrument
                if(std::find(g_input_window_state.allowed_instruments.begin(),
                        g_input_window_state.allowed_instruments.end(), 
                        g_input_window_state.instrument) == g_input_window_state.allowed_instruments.end()) {
                    // invalid instrument specified
                    g_input_window_state.error_txt = "Invalid instrument specified";
                    valid_update = false;
                }

                if(valid_update) {
                    // update input data and setup the new connection
                    g_input_window_state.error_txt = "";
                    input_data.instrument = g_input_window_state.instrument;
                    ws_connection = trader.connect(input_data.instrument);
                }
            } else {
                g_input_window_state.error_txt = "";
            }

            if(valid_update) {
                gui_main.fill_input_data_gui(input_data);
            }

            g_input_window_state.update_btn_clicked = false;
        }

        std::cout << g_input_window_state.selected_tier << '\n';

        // add the live data
        auto latest_msg = trader.get_latest_message(ws_connection);

        if(latest_msg != nullptr && !latest_msg->empty()) {
            json parsed_msg = json::parse(latest_msg->substr(WS_MSG_TYPE_LEN));
    
            input_data.live_data["asks"] = std::move(parsed_msg["asks"]);
            input_data.live_data["bids"] = std::move(parsed_msg["bids"]);

            // std::cout << input_data << "\n\n";
        }

        // calc_benchmark.start();
        // run the calculations only if input data is valid
        if(input_data.live_data["asks"].size() > 0 && input_data.live_data["bids"].size() > 0) {
            json j_slippage = find_expected_slippage(client_socket, input_data);
            float mid_price = j_slippage["result"]["mid_price"].get<float>();
            float volume = ((float) input_data.order_sz) / mid_price;
            float market_impact_pct = estimate_market_impact(volume);

            output_data.slippage =  (j_slippage["result"]["predicted_slippage_pct"].get<float>() * 0.01) * input_data.order_sz;
            output_data.market_impact = market_impact_pct * input_data.order_sz;
            output_data.fees = (input_data.fee_pct * 0.01) * input_data.order_sz;
            output_data.net_cost = output_data.slippage + output_data.market_impact * output_data.fees;

            output_data.mid_price = mid_price;

            // std::cout << output_data << '\n';
        }

        // calc_benchmark.end();

        gui_main.imgui_new_frame();
        gui_main.imgui_left_window();
        gui_main.imgui_right_window(input_data, output_data);
        gui_main.imgui_render();

        gui_main.window_swap_buffers();
        gui_main.poll_events();

        gui_main.calc_frame_times();
    }

    close(client_socket);

    return 0;
}