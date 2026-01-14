import { NextRequest, NextResponse } from 'next/server'
import { getGrpcCache } from '@/lib/grpc-cache'

export async function GET(
  request: NextRequest,
  { params }: { params: { exchange: string } }
) {
  try {
    const cache = getGrpcCache()
    const exchange = params.exchange
    const topic = `orderbook_${exchange}`
    
    const data = cache.get(topic)
    if (!data) {
      return NextResponse.json(
        { error: 'No data available yet' },
        { status: 503 }
      )
    }

    return NextResponse.json(data)
  } catch (error) {
    return NextResponse.json(
      { error: error instanceof Error ? error.message : 'Unknown error' },
      { status: 500 }
    )
  }
}
