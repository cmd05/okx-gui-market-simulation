import numpy as np
import pandas as pd
from sklearn.linear_model import LinearRegression
import json
import os
import pickle

from utils import extract_features, FEATURE_COLS

def generate_dataset(json_snapshots):
    features = []
    for snap in json_snapshots:
        f = extract_features(snap)
        features.append(f)

    df = pd.DataFrame(features)
    df.fillna(0, inplace=True)
    return df

def train_slippage_model(df):
    x = df[FEATURE_COLS]
    y = df["slippage_pct"]
    model = LinearRegression().fit(x, y)
    return model

def load_json_snapshots(prefix):
    directory="./data"
    prefix = prefix + "response_"
    snapshots = []
    for fname in os.listdir(directory):
        if fname.startswith(prefix) and fname.endswith(".json"):
            with open(os.path.join(directory, fname), "r") as f:
                snapshots.append(json.load(f))

    return snapshots

def train_model(pre):
    historical_jsons = load_json_snapshots(pre)
    df_train = generate_dataset(historical_jsons)
    model = train_slippage_model(df_train)

    with open(pre + "slippage_model.bin", "wb") as f:
        pickle.dump(model, f)

train_model("eth_")
train_model("btc_")
