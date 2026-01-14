import { GrpcClient, OrderBookData, BbaData, VolBandsData, PriceBandsData } from './grpc-client'

const GRPC_SERVER = process.env.GRPC_SERVER || 'localhost:50051'

class GrpcCache {
  private client: GrpcClient | null = null
  private cache: Map<string, any> = new Map()
  private subscribers: Map<string, Set<(data: any) => void>> = new Map()

  constructor() {
    this.initialize()
  }

  private initialize() {
    try {
      this.client = new GrpcClient(GRPC_SERVER)
      this.subscribeToAll()
    } catch (error) {
      console.error('Failed to initialize gRPC client:', error)
      // Retry after 5 seconds
      setTimeout(() => {
        if (!this.client) {
          this.initialize()
        }
      }, 5000)
    }
  }

  private subscribeToAll() {
    if (!this.client) return

    // Subscribe to order books
    const exchanges = ['binance', 'okx', 'gateio', 'synthetic']
    exchanges.forEach((exchange) => {
      const topic = `orderbook_${exchange}`
      this.client!.subscribe(topic, (message) => {
        try {
          const data = this.client!.parseOrderBookData(message)
          this.updateCache(topic, data)
        } catch (err) {
          console.error(`Error parsing order book for ${exchange}:`, err)
        }
      })
    })

    // Subscribe to BBA
    this.client.subscribe('bba', (message) => {
      try {
        const data = this.client!.parseBbaData(message)
        this.updateCache('bba', data)
      } catch (err) {
        console.error('Error parsing BBA:', err)
      }
    })

    // Subscribe to VBD
    this.client.subscribe('vbd', (message) => {
      try {
        const data = this.client!.parseVolBandsData(message)
        this.updateCache('vbd', data)
      } catch (err) {
        console.error('Error parsing VBD:', err)
      }
    })

    // Subscribe to PBD
    this.client.subscribe('pbd', (message) => {
      try {
        const data = this.client!.parsePriceBandsData(message)
        this.updateCache('pbd', data)
      } catch (err) {
        console.error('Error parsing PBD:', err)
      }
    })
  }

  private updateCache(key: string, data: any) {
    this.cache.set(key, data)
    const callbacks = this.subscribers.get(key)
    if (callbacks) {
      callbacks.forEach((callback) => callback(data))
    }
  }

  get(key: string): any {
    return this.cache.get(key)
  }

  subscribe(key: string, callback: (data: any) => void) {
    if (!this.subscribers.has(key)) {
      this.subscribers.set(key, new Set())
    }
    this.subscribers.get(key)!.add(callback)

    // Return current value if available
    const current = this.cache.get(key)
    if (current) {
      callback(current)
    }

    // Return unsubscribe function
    return () => {
      this.subscribers.get(key)?.delete(callback)
    }
  }
}

// Singleton instance
let cacheInstance: GrpcCache | null = null

export function getGrpcCache(): GrpcCache {
  if (!cacheInstance) {
    cacheInstance = new GrpcCache()
  }
  return cacheInstance
}
