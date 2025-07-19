FEATURE_COLS = ["spread_pct", "imbalance"]

def extract_features(orderbook_json, usd_quantity=100, volatility=None, fee_percent=None):
    # extract best bid and ask prices
    asks = sorted([(float(p), float(s)) for p, s in orderbook_json["asks"]], key=lambda x: x[0])
    bids = sorted([(float(p), float(s)) for p, s in orderbook_json["bids"]], key=lambda x: -x[0])
    best_ask = asks[0][0]
    best_bid = bids[0][0]

    mid_price = (best_ask + best_bid) / 2
    spread = best_ask - best_bid
    spread_pct = spread / mid_price

    base_qty = usd_quantity / mid_price
    total_cost = 0
    filled = 0

    # Simulate the cost
    for price, size in asks:
        if filled + size >= base_qty:
            # partial fill for the price
            total_cost += price * (base_qty - filled)
            break
        else:
            # full price
            total_cost += price * size
            filled += size

    avg_exec_price = total_cost / base_qty
    slippage_pct = (avg_exec_price - mid_price) / mid_price * 100

    depth_ask = sum(size for _, size in asks[:5])
    depth_bid = sum(size for _, size in bids[:5])
    imbalance = (depth_bid - depth_ask) / (depth_bid + depth_ask + 1e-6)

    return {
        "spread_pct": spread_pct,
        "imbalance": imbalance,
        "mid_price": mid_price,
        "slippage_pct": slippage_pct,
        "volatility": volatility,
        "fee_pct": fee_percent,
        "order_qty": base_qty
    }