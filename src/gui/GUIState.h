#pragma once

#include <nlohmann/json.hpp>
using json = nlohmann::json;

struct InputWindowState {
    // fixed values
    constexpr static std::array allowed_instruments = {"BTC", "ETH"}; // currently supported instruments
    constexpr static float fee_pct[5] = { 0.5, 0.4, 0.3, 0.2, 0.1 };

    const char* exchange[1] = {"OKX"};
    const char* order_type = "Market";
    std::string instrument = "BTC";
    int order_sz = 100; // 100 USD equivalent

    const char* tiers[5] = { "Tier 1", "Tier 2", "Tier 3", "Tier 4", "Tier 5" };
    int selected_tier = 0; // 0 to 4

    float volatility_pct = 0.1;

    std::string error_txt;
    bool update_btn_clicked = false;

    // Almgren–Chriss constants
    constexpr static float alpha = 1;
    constexpr static float eta = 0.05;
    constexpr static float beta = 1;
    constexpr static float gamma = 0.05;
};

struct InputData {
    InputData() {
        live_data["asks"] = json::array();
        live_data["bids"] = json::array();
    }

    std::string instrument;
    int order_sz;
    float fee_pct;
    float volatility_pct;

    json live_data;
};

std::ostream& operator<<(std::ostream& out, const InputData& input_data) {
    out << input_data.instrument << "\n"
        << "Sz: " << input_data.order_sz << "\n"
        << "Fee Pct: " << input_data.fee_pct << "\n"
        << "Volatility Pct: " << input_data.volatility_pct << "\n"
        << "asks: " << input_data.live_data["asks"].size() << "\n"
        << "bids: " << input_data.live_data["bids"].size();

    return out;
}

struct OutputData {
    float slippage = 0;
    float market_impact = 0;
    float fees = 0;
    float net_cost = 0;

    float mid_price = 0;
};

std::ostream& operator<<(std::ostream& out, const OutputData& output_data) {
    out << "Slippage Amount (USD): " << output_data.slippage << "\n"
        << "Market Impact (USD): " << output_data.market_impact << "\n"
        << "Fees (USD): " << output_data.fees << "\n"
        << "Net Cost (USD): " << output_data.net_cost;

    return out;
}
