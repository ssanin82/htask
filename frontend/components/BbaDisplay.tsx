'use client'

import { BbaData } from '@/lib/grpc-client'

interface BbaDisplayProps {
  data: BbaData | null
}

export function BbaDisplay({ data }: BbaDisplayProps) {
  if (!data) {
    return (
      <div className="bg-gray-900 rounded-lg p-4 border border-gray-700">
        <h2 className="text-xl font-semibold mb-4">Best Bid/Ask</h2>
        <div className="text-gray-500">Loading...</div>
      </div>
    )
  }

  return (
    <div className="bg-gray-900 rounded-lg p-4 border border-gray-700">
      <h2 className="text-xl font-semibold mb-4">Best Bid/Ask</h2>
      <div className="grid grid-cols-2 gap-4">
        <div className="bg-green-900/20 p-3 rounded">
          <div className="text-sm text-gray-400 mb-1">Best Bid</div>
          <div className="text-2xl font-bold text-green-400">
            ${data.bestBidPrice.toFixed(2)}
          </div>
          <div className="text-sm text-gray-300 mt-1">
            Size: {data.bestBidSize.toFixed(8)}
          </div>
        </div>
        <div className="bg-red-900/20 p-3 rounded">
          <div className="text-sm text-gray-400 mb-1">Best Ask</div>
          <div className="text-2xl font-bold text-red-400">
            ${data.bestAskPrice.toFixed(2)}
          </div>
          <div className="text-sm text-gray-300 mt-1">
            Size: {data.bestAskSize.toFixed(8)}
          </div>
        </div>
      </div>
      <div className="mt-4 pt-4 border-t border-gray-700">
        <div className="text-sm text-gray-400">Spread</div>
        <div className="text-lg font-semibold text-yellow-400">
          ${(data.bestAskPrice - data.bestBidPrice).toFixed(2)} (
          {(
            ((data.bestAskPrice - data.bestBidPrice) / data.bestBidPrice) *
            100
          ).toFixed(4)}
          %)
        </div>
      </div>
    </div>
  )
}
