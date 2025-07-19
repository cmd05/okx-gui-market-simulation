import socket
import threading
import json
import pickle

from predict_slippage import predict_slippage_runtime

MAX_BUFFER = 16384

eth_model = None
btc_model = None
models_dict = None

def calc_expected_slippage(params):
    if params["instrument"] not in models_dict:
        return {"error": f"Unsupported instrument: {params["instrument"]}"}

    selected_model = models_dict[params["instrument"]]
    return {"result": predict_slippage_runtime(selected_model, params)}

FUNCTION_MAP = {
    "expected_slippage": calc_expected_slippage,
}

def handle_request(data):
    try:
        request = json.loads(data)
        func_name = request["method"]
        params = request["params"]

        if func_name in FUNCTION_MAP:
            result = FUNCTION_MAP[func_name](params)
            return json.dumps(result)
        else:
            return json.dumps({"error": f"Unknown function: {func_name}"})
    except Exception as e:
        return json.dumps({"error": str(e)})

def handle_client(conn, addr):
    print(f"Connected by {addr}")
    with conn:
        while True:
            try:
                data = conn.recv(MAX_BUFFER)
                if not data:
                    print(f"Connection closed by {addr}")
                    break
                response = handle_request(data.decode())
                conn.sendall(response.encode())
            except Exception as e:
                print(f"Error with {addr}: {e}")
                break

def start_server(host="127.0.0.1", port=9000):
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.bind((host, port))
    s.listen(5)
    print(f"Python server listening on {host}:{port}")

    while True:
        conn, addr = s.accept()
        client_thread = threading.Thread(target=handle_client, args=(conn, addr), daemon=True)
        client_thread.start()

if __name__ == "__main__":
    eth_model_file = open("eth_slippage_model.bin", "rb")
    eth_model = pickle.load(eth_model_file)
    
    btc_model_file = open("btc_slippage_model.bin", "rb")
    btc_model = pickle.load(btc_model_file)

    models_dict = {
        "BTC": btc_model,
        "ETH": eth_model
    }

    start_server()