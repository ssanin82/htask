# Simple Price Aggregator

Aggregates spot BTCUSDT prices from 3 urces:
- Binance
- OKX
- Gate.io
## Implementation: Assumptions and Reasoning
These sources are chosen due to the simplicity of protocol and good liquidity. Calculating entry price for 50 million dollars notional as given in the formulation is rarely feasible even considering these are one of the most liquid exchanges (Binance beats everything else by margin).
### Price/Volume Calculations
In my implementation I decided to avoid working with floating point arithmetic due to very bad experience I encounter when protocol is designed to have price/size as floats. Potential problems: floating point numbers are not easy to precisely convert to a floating point string with a given precision, such conversion may have a significant PnL impact dut to potential imprecision, especially for strategies when 1 price tick off means money loss.

It may be an overkill for the task, but I decided to go for it to avoid unnecessary questions about rounding errors. As an example of a good approach (a bit different from chosen in this implementation) - in Tower Research they represent price/size as pairs of integers.

Prices/sizes are converted to floats at consumer component side for display. Price precision chosen is 2, size precision chosen is 8. This is derived from current tick sizes and lot sizes in selected exchanges for BTCUSDT:
- Binance Reference Data
  - URL: https://api.binance.com/api/v3/exchangeInfo
  - BTCUSDT lot size as of today: 0.00001  (size precision: 5)
  - BTCUSDT tick size as of today: 0.01    (price precision: 2)
- OKX Reference Data
  - URL: https://www.okx.com/api/v5/public/instruments?instType=SPOT
  - BTC-USDT lot size as of today: 0.00000001 (size precision: 8)
  - BTC-USDT tick size as of today: 0.1       (price precision: 1)
- Gate.io Reference Data
  - URL: https://api.gateio.ws/api/v4/spot/currency_pairs/BTC_USDT
  - BTC-USDT lot size as of today: 0.000001 (size precision == 6)
  - BTC-USDT tick size as of today: 0.1     (price precision == 1)

The chosen price/size precision is maximum of corresponding precisions on all 3 exchanges. It would have been impossible to use this technique with more exotic exchanges, like Synfutures, which do not have a fixed price tick.

### Other Assumptions and Simplifications:
- BTCUSDT is hardcoded in gateways to simplify the task
- Some market data streams may be faster than the chosen ones, but I went for maximum data in attempt to price tens of millions dollars of notional as formulated in the task. Simplified partial market data streams would not be sufficient.
- Market data levels are stored in map instead of unordered_map to preserve sorting by price.
- For simplicity pricer threads run forever, websockets never stop
- There is some simplistic reconnection logic in websocket threads (basically, if exception happened, start everything all over again; warning: based on "while (true" antipattern)). Experience shows that these market data connections are dropped by the exchanges. Not often, but usually in very inconvenient and important moments, which may lead to deterioration of PnL.
- For Gate.IO 400 level update was chosen to accommodate more liquidity, even though 50-level update is a faster stream (20 ms for 50 levels, 100 ms for 400 levels) 
- For OKX faster order book streams are not available for non-VIP subscribers.
- For Binance I didn't go for partial 5, 10, 20 level depth updates, even though they are much simpler. I went for full order book snapshot, even though it requires some synchronization (subscribe and buffer, request snapshot, filter and process the bufferred updates, keep receiving the stream)
- nlohmann/json was chosen for simplicity and speed of implementation. Since the chosen market data streams are generally per 100 ms, it is not ultra low latency system. In my previous company we used simdjson.
- Since the pricer is not ultra low latency, the consolidated order book is synchronized just with a mutex. If performance requirements are stricter, there are many ways to improve on it. It is just for the given task further improvements don't result in a gain of anything valuable.
- Floating point arithmetic is replaced with integer arithmetic. Depending on type of calculations, we may arrive to fractional number results, however they should be truncated. No orders can be sent to exchange if price and size are not multiples of tick size and lot size correspondingly - it would result in the order rejection (and trading block for the account if the sending strategy keeps retrying to send constantly failing orders too often)
### Design Simplifications
- OrderBook::print/printExtended functions have (hardcoded) knowledge about the exchanges, which makes the code a less extensible, but these are utility functions, they don't have not impact on the main functionality. It could be possible to somehow "register" running market data feeds and implement these print functions in generic way, I didn't go that way to avoid overcomplication the design for this small task.
- To accommodate Binance order snapshot synchronization, Binance starts first with some headstart of few seconds. It needs it's own market data clean on start/restart - handling the restart in generic way would overcomplicate the implementation, having to be able to c;ear the consolidated order book for one exchange only while other exchanges keep updating it. The synchronization for this start is based on sleep, which is not ideal, but simplified for the task only. In production things like this should be event-based (spin-wait or conditional variable).







## ToDo - external configuration
- gRPC server port, address
- In production there should be a service discovery mode, or some conteiner orchestration like Kubernetes used. For the task purpose and simplicuty, the publishing topics are hardcoded in the components.
- The pricer publishes at fixed time intervals. Ideally, it should be real-time, but Binance/OKX/Gate.io do not give real-time market data, except ticker (best bid/ask). Some tun-time approximation can be made based on real-time ticker combined with order book updates, but whether it is acceptable to use - that is the path to be taken in consideration with the quant/the person creating the model for the strategy.


# TODO building/starting/stopping
