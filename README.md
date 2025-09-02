# htask

- BTCUSDT is hardcoded in gateways to simplify the task
- No additional market data speed optimizations made due to the time constraints
- prices and sizes are stored as strings to avoid floating point inaccuracies
- order books keep levels in map instead of unordered_map to preserve sorting by key
- for simplicity pricer threads run forever, websockets never stop, reconnect on exception
- for Gate.IO 400 level update was chosen to accommodate more liquidity, even though 50-level update is more frequent



