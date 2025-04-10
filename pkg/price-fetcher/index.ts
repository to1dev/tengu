interface Env {
    CRYPTO_PRICES: KVNamespace;
}

interface CryptoPrice {
    USD: number;
    EUR: number;
}

interface Prices {
    BTC: CryptoPrice;
    ETH: CryptoPrice;
    SOL: CryptoPrice;
    SUI: CryptoPrice;
    timestamp: string;
}

export default {
    async scheduled(
        event: ScheduledEvent,
        env: Env,
        ctx: ExecutionContext
    ): Promise<void> {
        const url =
            "https://min-api.cryptocompare.com/data/pricemulti?fsyms=BTC,ETH,SOL,SUI&tsyms=USD,EUR";
        try {
            const response = await fetch(url);
            if (!response.ok) {
                throw new Error("API request failed");
            }
            const data = (await response.json()) as Record<string, CryptoPrice>;

            if (!data.BTC || !data.ETH || !data.SOL || !data.SUI) {
                throw new Error("Missing required crypto price data");
            }

            const prices: Prices = {
                BTC: { USD: data.BTC.USD, EUR: data.BTC.EUR },
                ETH: { USD: data.ETH.USD, EUR: data.ETH.EUR },
                SOL: { USD: data.SOL.USD, EUR: data.SOL.EUR },
                SUI: { USD: data.SUI.USD, EUR: data.SUI.EUR },
                timestamp: new Date().toISOString(),
            };

            await env.CRYPTO_PRICES.put(
                "latest_prices",
                JSON.stringify(prices)
            );
        } catch (error) {
            console.error("Failed to update prices: ", error);
        }
    },

    async fetch(
        request: Request,
        env: Env,
        ctx: ExecutionContext
    ): Promise<Response> {
        const url = new URL(request.url);

        if (url.pathname === "/prices") {
            const prices = await env.CRYPTO_PRICES.get("latest_prices");
            if (!prices) {
                return new Response("No data available", { status: 404 });
            }

            return new Response(prices, {
                headers: { "Content-Type": "application/json" },
            });
        }

        return new Response("Not Found", { status: 404 });
    },
};
