import { NextResponse } from 'next/server'
import { getGrpcCache } from '@/lib/grpc-cache'

export async function GET() {
  try {
    const cache = getGrpcCache()
    const data = cache.get('pbd')
    
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
