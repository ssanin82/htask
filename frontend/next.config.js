/** @type {import('next').NextConfig} */
const nextConfig = {
  reactStrictMode: true,
  // Note: gRPC client is only used server-side in API routes,
  // so no webpack fallbacks needed for client bundle
};

module.exports = nextConfig;
