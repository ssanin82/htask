# Simple Price Aggregator



Aggregates spot BTCUSDT prices from 3 sources:

- Binance

- OKX

- Gate.io

## Implementation: Assumptions and Reasoning

These sources are chosen due to the simplicity of the protocol and good liquidity. Calculating the entry price for a $ 50 million notional amount, as given in the formulation, is rarely feasible, even considering that these are one of the most liquid exchanges (Binance beats everything else by a margin).

### Price/Volume Calculations

In my implementation, I decided to avoid working with floating point arithmetic due to a very bad experience I encountered when the protocol is designed to have price/size as floats. Potential problems: Floating-point numbers are not easy to precisely convert to a floating-point string with a given precision. Such conversions may have a significant PnL impact due to potential imprecision, especially for strategies where a single price tick off means a loss of money (e.g., when you should keep remaining below the best price by 1 tick).



It may be overkill for the task, but I decided to go for it to avoid unnecessary questions about rounding errors. As an example of a good approach (a bit different from chosen in this implementation), in Tower Research (my past experience), they represent price/size as pairs of integers.



Prices/sizes are converted to floats at the consumer component side for display. Price precision chosen is 2, size precision chosen is 8. This is derived from current tick sizes and lot sizes in selected exchanges for BTCUSDT:

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



The chosen price/size precision is the maximum of the corresponding precisions on all 3 exchanges. It would have been impossible to use this technique with more exotic exchanges, like Synfutures, which do not have a fixed price tick.

*NOTE*: In a consolidated order book often bid and ask cross, which never happens in an order book of an individual exchange.

### Other Assumptions and Simplifications:

- BTCUSDT is hardcoded in gateways to simplify the task

- Some market data streams may be faster than the chosen ones, but I went for maximum data in an attempt to price tens of millions of dollars of notional as formulated in the task. Simplified partial market data streams would not be sufficient.

- Market data levels are stored in a map instead of unordered_map to preserve sorting by price.

- For simplicity, pricer threads run forever, websockets never stop

- There is some simplistic reconnection logic in websocket threads (basically, if an exception happened, start everything all over again; warning: based on "while (true" antipattern)). Experience shows that these market data connections are dropped by the exchanges. Not often, but usually in very inconvenient and important moments, which may lead to deterioration of P&L.

- For Gate.IO a 400-level update was chosen to accommodate more liquidity, even though a 50-level update is a faster stream (20 ms for 50 levels, 100 ms for 400 levels) 

- For OKX, faster order book streams are not available for non-VIP subscribers.

- For Binance, I didn't go for partial 5, 10, 20 level depth updates, even though they are much simpler. I went for a full order book snapshot, even though it requires some synchronization (subscribe and buffer, request snapshot, filter and process the buffered updates, keep receiving the stream)

- nlohmann/json was chosen for simplicity and speed of implementation. Since the chosen market data streams are generally per 100 ms, it is not ultra low latency system. In my previous company, we used simdjson.

- Since the pricer is not ultra-low-latency, the consolidated order book is synchronized just with a mutex. If performance requirements are stricter, there are many ways to improve it. It is just for the given task; further improvements don't result in a gain of anything valuable.

- Floating-point arithmetic is replaced with integer arithmetic. Depending on the type of calculations, we may arrive to fractional number results; however, they should be truncated. No orders can be sent to the exchange if the price and size are not multiples of the tick size and lot size, respectively. It would result in the order rejection (and trading block for the account if the sending strategy keeps retrying to send constantly failing orders too often)

### Design Simplifications

- OrderBook::print/printExtended functions have (hardcoded) knowledge about the exchanges, which makes the code less extensible, but these are utility functions; they don't have not impact on the main functionality. It could be possible to somehow "register" running market data feeds and implement these print functions in a generic way, I didn't go that way to avoid overcomplication the design for this small task.

- To accommodate Binance order snapshot synchronization, Binance starts with a head start of a few seconds. It needs it's own market data clean on start/restart - handling the restart in a generic way would overcomplicate the implementation, having to be able to c;ear the consolidated order book for one exchange only while other exchanges keep updating it. The synchronization for this start is based on sleep, which is not ideal, but simplified for the task only. In production, things like this should be event-based (spin-wait or conditional variable).

- In production, there should be some service discovery mechanism, or some container orchestration like Kubernetes used. For the task purpose and simplicity, the publishing topics are hardcoded in the components, number of components is fixed and considered to be known.

- The pricer publishes at fixed time intervals. Ideally, it should be real-time. Even though we don't have real-time market data, it is possible to implement some "simulation" of a real-time order book combining ticker data with depth updates. It may be good enough for some strategy to reason about the liquidity and prices, however probably not in the scope of this task.

- There are some hardcodings in the code to save time. Ideally, it should be stored as external configurations to avoid rebuilding each time after a change - I had a good experience with MongoDB (also, I saw some people trying to store these configs directly as a part of Kubernetes manifests, but it usually resulted in release confusion). This is not in the scope of this task, I believe.

- ### Other Considerations

- For the gRPC server, I went for asynchronous event processing, as simple as it can be. It would publish the data and cleanup the subscriber sessions on disconnect properly. Since it is async, it should be able to process a reasonable message load, though it is not on the task scope.

- Maybe, a separate gRPC server component is not absolutely necessary for this task (the task definition diagram does not show a separate server component), but it would be a bit trickier to do; maybe something like ZeroMQ would be more suitable. I went for it as the requirement was "scalability", you never know how many publishers eventually the system will have if it is used in production long enough.



## Building/Starting/Stopping

All to be done from the root folder of the project.

- Build: `docker-compose build`

- Start: `docker-compose up -d`

- Stopping: `docker-compose down`

- Tracing best bid/opper prices: `docker-compose logs -f get_bba`

- Tracing notional volume bands: `docker-compose logs -f get_vbd`

- Tracing mid price with "deviations": `docker-compose logs -f get_pbd`

- NOTE I did try uploading the image to DockerHub to save time on verifying the task, but considering the size of that image (several gigabytes), pulling it from DockerHub is not much faster, if at all, compared to building it from Dockerfile