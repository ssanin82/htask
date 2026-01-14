# Order Book Dashboard Frontend

A Next.js dashboard for visualizing real-time order book data from multiple exchanges.

## Features

- Real-time order book visualization for 4 sources:
  - Binance
  - OKX
  - Gate.io
  - Synthetic (aggregated)
- Best Bid/Ask (BBA) display
- Volume Bands Data (VBD) for 1M, 5M, 10M, 25M, 50M notional amounts
- Mid price from synthetic order book
- Dark theme UI
- Updates every 1 second

## Setup

1. Install dependencies:
```bash
pnpm install
```

2. Set environment variable (optional, defaults to localhost:50051):
```bash
export GRPC_SERVER=localhost:50051
```

3. Run the development server:
```bash
pnpm dev
```

4. Open [http://localhost:3000](http://localhost:3000) in your browser

## Architecture

- **Next.js 14** with App Router
- **Tanstack Query** for data fetching and caching
- **Tailwind CSS** for styling
- **gRPC** client (server-side) connecting to the order book aggregator
- API routes act as a proxy between the frontend and gRPC server

## Notes

- The gRPC server must be running on `localhost:50051` (or the configured GRPC_SERVER)
- The frontend polls the API routes every 1 second for updates
- Server-side gRPC cache maintains persistent connections to the gRPC server
