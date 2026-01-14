import * as grpc from '@grpc/grpc-js'
import * as protoLoader from '@grpc/proto-loader'
import path from 'path'

const PROTO_PATH = path.join(process.cwd(), 'proto', 'pubsub.proto')

const packageDefinition = protoLoader.loadSync(PROTO_PATH, {
  keepCase: true,
  longs: String,
  enums: String,
  defaults: true,
  oneofs: true,
})

const protoDescriptor = grpc.loadPackageDefinition(packageDefinition) as any
const pubsub = protoDescriptor.pubsub

export interface OrderBookLevel {
  price: number
  size: number
}

export interface OrderBookData {
  exchange: string
  bids: OrderBookLevel[]
  asks: OrderBookLevel[]
}

export interface BbaData {
  bestBidPrice: number
  bestBidSize: number
  bestAskPrice: number
  bestAskSize: number
}

export interface VolBandsData {
  mlnBuyPrice: Record<number, number>
  mlnSellPrice: Record<number, number>
}

export interface PriceBandsData {
  mid: number
  bboUpBps: Record<number, number>
  bboDownBps: Record<number, number>
}

export class GrpcClient {
  private client: any
  private serverUrl: string

  constructor(serverUrl: string = 'localhost:50051') {
    this.serverUrl = serverUrl
    this.client = new pubsub.PubSubService(
      serverUrl,
      grpc.credentials.createInsecure()
    )
  }

  subscribe(topic: string, onMessage: (data: any) => void): () => void {
    const request = { topic }
    const call = this.client.Subscribe(request)

    call.on('data', (message: any) => {
      onMessage(message)
    })

    call.on('error', (err: Error) => {
      console.error('gRPC error:', err)
    })

    call.on('end', () => {
      console.log('Stream ended')
    })

    return () => {
      call.cancel()
    }
  }

  scaleDown(value: number, scale: number): number {
    return value / Math.pow(10, scale)
  }

  parseOrderBookData(message: any): OrderBookData {
    const obd = message.obd
    return {
      exchange: obd.exchange,
      bids: (obd.bids || []).map((level: any) => ({
        price: this.scaleDown(level.price, 2),
        size: this.scaleDown(level.size, 8),
      })),
      asks: (obd.asks || []).map((level: any) => ({
        price: this.scaleDown(level.price, 2),
        size: this.scaleDown(level.size, 8),
      })),
    }
  }

  parseBbaData(message: any): BbaData {
    const bba = message.bba
    return {
      bestBidPrice: this.scaleDown(bba.best_bid_price, 2),
      bestBidSize: this.scaleDown(bba.best_bid_size, 8),
      bestAskPrice: this.scaleDown(bba.best_ask_price, 2),
      bestAskSize: this.scaleDown(bba.best_ask_size, 8),
    }
  }

  parseVolBandsData(message: any): VolBandsData {
    const vbd = message.vbd
    const mlnBuyPrice: Record<number, number> = {}
    const mlnSellPrice: Record<number, number> = {}

    Object.entries(vbd.mln_buy_price || {}).forEach(([key, value]: [string, any]) => {
      mlnBuyPrice[parseInt(key)] = this.scaleDown(value, 2)
    })

    Object.entries(vbd.mln_sell_price || {}).forEach(([key, value]: [string, any]) => {
      mlnSellPrice[parseInt(key)] = this.scaleDown(value, 2)
    })

    return { mlnBuyPrice, mlnSellPrice }
  }

  parsePriceBandsData(message: any): PriceBandsData {
    const pbd = message.pbd
    const bboUpBps: Record<number, number> = {}
    const bboDownBps: Record<number, number> = {}

    Object.entries(pbd.bbo_up_bps || {}).forEach(([key, value]: [string, any]) => {
      bboUpBps[parseInt(key)] = this.scaleDown(value, 2)
    })

    Object.entries(pbd.bbo_down_bps || {}).forEach(([key, value]: [string, any]) => {
      bboDownBps[parseInt(key)] = this.scaleDown(value, 2)
    })

    return {
      mid: this.scaleDown(pbd.mid, 2),
      bboUpBps,
      bboDownBps,
    }
  }
}
