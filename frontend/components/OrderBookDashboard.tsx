'use client'

import { useOrderBook, useBbaData, useVolBandsData, usePriceBandsData } from '@/lib/hooks'
import { OrderBook } from './OrderBook'
import { BbaDisplay } from './BbaDisplay'
import { VolBandsDisplay } from './VolBandsDisplay'
import { MidPriceDisplay } from './MidPriceDisplay'

export function OrderBookDashboard() {
  const binance = useOrderBook('binance')
  const okx = useOrderBook('okx')
  const gateio = useOrderBook('gateio')
  const synthetic = useOrderBook('synthetic')
  const bba = useBbaData()
  const vbd = useVolBandsData()
  const pbd = usePriceBandsData()

  return (
    <div className="space-y-6">
      {/* Top row: BBA, Mid Price, Vol Bands */}
      <div className="grid grid-cols-1 md:grid-cols-3 gap-4">
        <BbaDisplay data={bba.data || null} />
        <MidPriceDisplay midPrice={pbd.data?.mid || null} />
        <VolBandsDisplay data={vbd.data || null} />
      </div>

      {/* Order Books */}
      <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-4 gap-4">
        <OrderBook
          title="Binance"
          bids={binance.data?.bids || []}
          asks={binance.data?.asks || []}
        />
        <OrderBook
          title="OKX"
          bids={okx.data?.bids || []}
          asks={okx.data?.asks || []}
        />
        <OrderBook
          title="Gate.io"
          bids={gateio.data?.bids || []}
          asks={gateio.data?.asks || []}
        />
        <OrderBook
          title="Synthetic"
          bids={synthetic.data?.bids || []}
          asks={synthetic.data?.asks || []}
        />
      </div>

      {/* Error display */}
      {(binance.error ||
        okx.error ||
        gateio.error ||
        synthetic.error ||
        bba.error ||
        vbd.error ||
        pbd.error) && (
        <div className="bg-red-900/20 border border-red-700 rounded-lg p-4">
          <div className="text-red-400 font-semibold">Connection Error</div>
          <div className="text-sm text-gray-400 mt-2">
            Please ensure the gRPC server is running on localhost:50051
          </div>
        </div>
      )}
    </div>
  )
}
