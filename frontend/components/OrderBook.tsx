'use client'

import { OrderBookLevel } from '@/lib/grpc-client'

interface OrderBookProps {
  bids: OrderBookLevel[]
  asks: OrderBookLevel[]
  title: string
}

export function OrderBook({ bids, asks, title }: OrderBookProps) {
  const formatPrice = (price: number) => price.toFixed(2)
  const formatSize = (size: number) => {
    // Format with up to 8 decimal places, then remove trailing zeros
    const formatted = size.toFixed(8)
    return formatted.replace(/\.?0+$/, '')
  }

  return (
    <div className="bg-gray-900 rounded-lg p-4 border border-gray-700">
      <h2 className="text-xl font-semibold mb-4 text-center">{title}</h2>
      <div className="grid grid-cols-2 gap-4">
        {/* Bids */}
        <div>
          <div className="text-sm font-semibold mb-2 text-green-400">BIDS</div>
          <div className="space-y-1">
            {bids.length > 0 ? (
              bids.slice(0, 10).map((bid, idx) => (
                <div
                  key={idx}
                  className="flex justify-between items-center text-sm bg-green-900/20 px-3 py-1 rounded gap-4"
                >
                  <span className="text-green-400 font-mono">{formatPrice(bid.price)}</span>
                  <span className="text-gray-300 font-mono text-right">{formatSize(bid.size)}</span>
                </div>
              ))
            ) : (
              <div className="text-gray-500 text-sm">No bids</div>
            )}
          </div>
        </div>

        {/* Asks */}
        <div>
          <div className="text-sm font-semibold mb-2 text-red-400">ASKS</div>
          <div className="space-y-1">
            {asks.length > 0 ? (
              asks.slice(0, 10).map((ask, idx) => (
                <div
                  key={idx}
                  className="flex justify-between items-center text-sm bg-red-900/20 px-3 py-1 rounded gap-4"
                >
                  <span className="text-red-400 font-mono">{formatPrice(ask.price)}</span>
                  <span className="text-gray-300 font-mono text-right">{formatSize(ask.size)}</span>
                </div>
              ))
            ) : (
              <div className="text-gray-500 text-sm">No asks</div>
            )}
          </div>
        </div>
      </div>
    </div>
  )
}
