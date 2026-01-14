'use client'

import { VolBandsData } from '@/lib/grpc-client'

interface VolBandsDisplayProps {
  data: VolBandsData | null
}

export function VolBandsDisplay({ data }: VolBandsDisplayProps) {
  if (!data) {
    return (
      <div className="bg-gray-900 rounded-lg p-4 border border-gray-700">
        <h2 className="text-xl font-semibold mb-4">Volume Bands</h2>
        <div className="text-gray-500">Loading...</div>
      </div>
    )
  }

  const amounts = [1, 5, 10, 25, 50]

  return (
    <div className="bg-gray-900 rounded-lg p-4 border border-gray-700">
      <h2 className="text-xl font-semibold mb-4">Volume Bands</h2>
      <div className="space-y-3">
        {amounts.map((amount) => {
          const buyPrice = data.mlnBuyPrice[amount]
          const sellPrice = data.mlnSellPrice[amount]
          return (
            <div
              key={amount}
              className="bg-gray-800 p-3 rounded border border-gray-700"
            >
              <div className="text-sm font-semibold mb-2">
                ${amount}M Notional
              </div>
              <div className="grid grid-cols-2 gap-2">
                <div>
                  <div className="text-xs text-gray-400 mb-1">Buy Price</div>
                  <div className="text-green-400 font-mono">
                    {buyPrice ? `$${buyPrice.toFixed(2)}` : 'Insufficient'}
                  </div>
                </div>
                <div>
                  <div className="text-xs text-gray-400 mb-1">Sell Price</div>
                  <div className="text-red-400 font-mono">
                    {sellPrice ? `$${sellPrice.toFixed(2)}` : 'Insufficient'}
                  </div>
                </div>
              </div>
            </div>
          )
        })}
      </div>
    </div>
  )
}
