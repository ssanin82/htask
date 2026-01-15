'use client'

import { useEffect, useState } from 'react'
import { LineChart, Line, XAxis, YAxis, CartesianGrid, Tooltip, ResponsiveContainer } from 'recharts'

interface MidPriceDisplayProps {
  midPrice: number | null
}

interface PricePoint {
  time: string
  price: number
}

export function MidPriceDisplay({ midPrice }: MidPriceDisplayProps) {
  const [priceHistory, setPriceHistory] = useState<PricePoint[]>([])

  useEffect(() => {
    if (midPrice !== null) {
      const now = new Date()
      const timeStr = `${now.getHours()}:${now.getMinutes()}:${now.getSeconds()}`
      setPriceHistory((prev) => {
        const updated = [...prev, { time: timeStr, price: midPrice }]
        // Keep only last 60 data points (1 minute at 1 second intervals)
        return updated.slice(-60)
      })
    }
  }, [midPrice])

  return (
    <div className="bg-gray-900 rounded-lg p-4 border border-gray-700">
      <h2 className="text-xl font-semibold mb-4">Mid Price</h2>
      {midPrice !== null ? (
        <>
          <div className="text-3xl font-bold text-blue-400 text-center mb-4">
            ${midPrice.toFixed(2)}
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
                  <Line 
                    type="monotone" 
                    dataKey="price" 
                    stroke="#60a5fa" 
                    strokeWidth={2}
                    dot={false}
                  />
                </LineChart>
              </ResponsiveContainer>
            </div>
          )}
        </>
      ) : (
        <div className="text-gray-500 text-center">Loading...</div>
      )}
    </div>
  )
}
