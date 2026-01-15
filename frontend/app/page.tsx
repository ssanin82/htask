'use client'

import { OrderBookDashboard } from '@/components/OrderBookDashboard'

export default function Home() {
  return (
    <main className="min-h-screen p-8">
      <div className="mx-auto" style={{ maxWidth: '1920px' }}>
        <h1 className="text-4xl font-bold mb-8 text-center">
          Order Book Dashboard (Bitcoin Price)
        </h1>
        <OrderBookDashboard />
      </div>
    </main>
  )
}
