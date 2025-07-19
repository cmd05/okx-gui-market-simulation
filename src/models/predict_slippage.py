import pandas as pd

import pickle
from utils import extract_features, FEATURE_COLS

def predict_slippage_runtime(model, current_json):
    f = extract_features(current_json, usd_quantity=current_json["order_sz"], \
            volatility=current_json["volatility_pct"], fee_percent=current_json["fee_pct"])
    X_live = pd.DataFrame([[f["spread_pct"], f["imbalance"]]], columns=FEATURE_COLS)
    slippage = abs(model.predict(X_live)[0])
    return {
        "predicted_slippage_pct": slippage,
        "spread_pct": f["spread_pct"],
        "mid_price": f["mid_price"],
    }

# live_json = {
#     "asks": [["105.2", "1.0"], ["105.3", "1.5"], ["105.4", "1.5"]],
#     "bids": [["105.0", "1.0"], ["104.9", "1.5"], ["104.8", "1.0"]]
# }

# pre = "btc_"
# file = open(pre + "slippage_model.bin", "rb")
# model = pickle.load(file)

# runtime_result = predict_slippage_runtime(
#     model=model,
#     current_json=live_json,
#     volatility=0.015,
#     fee_pct=0.05,
#     usd_quantity=100
# )

# print("Runtime Slippage Prediction:")
# print(runtime_result)