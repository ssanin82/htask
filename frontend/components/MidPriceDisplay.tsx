'use client'

interface MidPriceDisplayProps {
  midPrice: number | null
}

export function MidPriceDisplay({ midPrice }: MidPriceDisplayProps) {
  return (
    <div className="bg-gray-900 rounded-lg p-4 border border-gray-700">
      <h2 className="text-xl font-semibold mb-4">Mid Price</h2>
      {midPrice !== null ? (
        <div className="text-4xl font-bold text-blue-400 text-center">
          ${midPrice.toFixed(2)}
        </div>
      ) : (
        <div className="text-gray-500 text-center">Loading...</div>
      )}
    </div>
  )
}
