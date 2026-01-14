'use client'

import { useQuery } from '@tanstack/react-query'
import { OrderBookData, BbaData, VolBandsData, PriceBandsData } from './grpc-client'

async function fetchOrderBook(exchange: string): Promise<OrderBookData> {
  const res = await fetch(`/api/orderbook/${exchange}`)
  if (!res.ok) throw new Error('Failed to fetch order book')
  return res.json()
}

async function fetchBba(): Promise<BbaData> {
  const res = await fetch('/api/bba')
  if (!res.ok) throw new Error('Failed to fetch BBA data')
  return res.json()
}

async function fetchVolBands(): Promise<VolBandsData> {
  const res = await fetch('/api/vbd')
  if (!res.ok) throw new Error('Failed to fetch volume bands')
  return res.json()
}

async function fetchPriceBands(): Promise<PriceBandsData> {
  const res = await fetch('/api/pbd')
  if (!res.ok) throw new Error('Failed to fetch price bands')
  return res.json()
}

export function useOrderBook(exchange: string) {
  return useQuery({
    queryKey: ['orderbook', exchange],
    queryFn: () => fetchOrderBook(exchange),
    refetchInterval: 1000, // 1 second
    staleTime: 500,
  })
}

export function useBbaData() {
  return useQuery({
    queryKey: ['bba'],
    queryFn: fetchBba,
    refetchInterval: 1000,
    staleTime: 500,
  })
}

export function useVolBandsData() {
  return useQuery({
    queryKey: ['vbd'],
    queryFn: fetchVolBands,
    refetchInterval: 1000,
    staleTime: 500,
  })
}

export function usePriceBandsData() {
  return useQuery({
    queryKey: ['pbd'],
    queryFn: fetchPriceBands,
    refetchInterval: 1000,
    staleTime: 500,
  })
}
