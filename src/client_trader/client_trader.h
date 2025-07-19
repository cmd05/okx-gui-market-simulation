#pragma once

#include <websocket/websocket.h>
#include <gui/GUIState.h>

extern InputWindowState g_input_window_state;

class ClientTrader {
public:
    con_id_type connect(std::string instrument) {
        auto it = m_con_map.find(instrument);
        if(it != m_con_map.end()) {
            APP_LOG(log_flags::client_trader, "Instrument already connected");
            return it->second;
        }

        const static std::string base_url = "wss://ws.gomarket-cpp.goquant.io/ws/l2-orderbook/okx/";

        if(std::find(g_input_window_state.allowed_instruments.begin(), g_input_window_state.allowed_instruments.end(), instrument) == g_input_window_state.allowed_instruments.end()) {
            APP_LOG(log_flags::client_trader, "Incorrect instrument specified");
            return -1;
        }

        std::string url = base_url + instrument + "-USDT-SWAP";
        con_id_type id = m_endpoint.connect(url);

        // add delay to wait for messages to start
        std::this_thread::sleep_for(200ms);

        m_con_map[instrument] = id;
        return id;
    }

    std::string* get_latest_message(con_id_type id) {
        return m_endpoint.get_latest_message(id);
    }

    void print_messages(con_id_type id) {
        connection_metadata::ptr metadata_ptr = m_endpoint.get_metadata(id);

        if(!metadata_ptr)
            APP_LOG(log_flags::client_trader, "Error fetching metadata");
        else
            APP_PRINT(*metadata_ptr);
    }
protected:
    std::unordered_map<std::string, con_id_type> m_con_map;
    websocket_endpoint m_endpoint;
};