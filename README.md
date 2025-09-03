# htask

- BTCUSDT is hardcoded in gateways to simplify the task
- No additional market data speed optimizations made due to the time constraints
- prices and sizes are stored as strings to avoid floating point inaccuracies. In production I'd use either 2 integers to represent rational numbers or delta arithmetic with floating points
- order books keep levels in map instead of unordered_map to preserve sorting by key
- for simplicity pricer threads run forever, websockets never stop, reconnect on exception
- for Gate.IO 400 level update was chosen to accommodate more liquidity, even though 50-level update is more frequent
- for OKX faster order book streams are not available for non-VIP subscribers
- nlohmann/json was chosen over simdjson for simplicity (production code would be different)
- the pricer is not ultra-fast as the market data streams are with 100ms frequency. Thus, synchronizing with a mutex should be sufficient for the task
- floating point arithmetic is replaced with integer arithmetic wherever possible to avoid imprecision, some inaccuracy should is ok since sizes and prices change by fixed increments (lot/tick sizes), so some resulting prices or sizes must be truncated to tick or lot size
- OrderBook::print has knowledge about the exchanges, which makes the code a bit less extensible, but print is a utility function, it does not umpact the main functionality
- to get a bigger order book with Binance some synchronization is performed with snapdhot and buffered updates, thus Binance market data has some headstart. The synchronization is based on sleep, in production it should be event-based (spin-wait or conditional variable)


## Binance Reference Data
- https://api.binance.com/api/v3/exchangeInfo
- BTCUSDT lot size as of today: 0.00001
- BTCUSDT tick size as of today: 0.01

## OKX Reference Data
- https://www.okx.com/api/v5/public/instruments?instType=SPOT
- BTC-USDT lot size as of today: 0.00000001
- BTC-USDT tick size as of today: 0.1

## Gate.io Reference Data
- https://api.gateio.ws/api/v4/spot/currency_pairs/BTC_USDT
- BTC-USDT lot size as of today: 0.000001 (amount precision == 6)
- BTC-USDT tick size as of today: 0.1 (precision == 1)
