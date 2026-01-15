'use client'

import { useEffect, useState } from 'react'
import { LineChart, Line, XAxis, YAxis, CartesianGrid, Tooltip, ResponsiveContainer, Legend } from 'recharts'
import { BbaData } from '@/lib/grpc-client'

interface BbaDisplayProps {
  data: BbaData | null
}

interface BbaPoint {
  time: string
  bid: number
  ask: number
}

export function BbaDisplay({ data }: BbaDisplayProps) {
  const [priceHistory, setPriceHistory] = useState<BbaPoint[]>([])

  useEffect(() => {
    if (data) {
      const now = new Date()
      const timeStr = `${now.getHours()}:${now.getMinutes()}:${now.getSeconds()}`
      setPriceHistory((prev) => {
        const updated = [...prev, { 
          time: timeStr, 
          bid: data.bestBidPrice,
          ask: data.bestAskPrice
        }]
        // Keep only last 60 data points (1 minute at 1 second intervals)
        return updated.slice(-60)
      })
    }
  }, [data])

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
      <div className="grid grid-cols-2 gap-4 mb-4">
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
      <div className="mb-4 pt-4 border-t border-gray-700">
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
      {priceHistory.length > 0 && (
        <div className="h-48">
          <ResponsiveContainer width="100%" height="100%">
            <LineChart data={priceHistory}>
              <CartesianGrid strokeDasharray="3 3" stroke="#374151" />
              <XAxis 
                dataKey="time" 
                stroke="#9ca3af"
                tick={{ fill: '#9ca3af', fontSize: 10 }}
                interval="preserveStartEnd"
              />
              <YAxis 
                stroke="#9ca3af"
                tick={{ fill: '#9ca3af', fontSize: 10 }}
                domain={['dataMin - 10', 'dataMax + 10']}
              />
              <Tooltip
                contentStyle={{ backgroundColor: '#1f2937', border: '1px solid #374151', borderRadius: '4px' }}
                labelStyle={{ color: '#9ca3af' }}
              />
              <Legend 
                wrapperStyle={{ color: '#9ca3af', fontSize: '12px' }}
              />
              <Line 
                type="monotone" 
                dataKey="bid" 
                stroke="#4ade80" 
                strokeWidth={2}
                dot={false}
                name="Bid"
              />
              <Line 
                type="monotone" 
                dataKey="ask" 
                stroke="#f87171" 
                strokeWidth={2}
                dot={false}
                name="Ask"
              />
            </LineChart>
          </ResponsiveContainer>
        </div>
      )}
    </div>
  )
}
