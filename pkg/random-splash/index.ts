interface Env {
    MY_R2_BUCKET: R2Bucket;
}

export default {
    async scheduled(
        event: ScheduledEvent,
        env: Env,
        ctx: ExecutionContext
    ): Promise<void> {
        await updateSplashImage(env);
    },

    async fetch(
        req: Request,
        env: Env,
        ctx: ExecutionContext
    ): Promise<Response> {
        return new Response(
            "Don't Panic! The answer to life, the universe, and everything is 42, not your random image request! 🚀😜",
            { status: 403 }
        );
    },
};

async function updateSplashImage(env: Env): Promise<void> {
    const sourcePath = "images/";
    const destPath = "splash.png";

    const list = await env.MY_R2_BUCKET.list({ prefix: sourcePath });
    const imageFiles = list.objects
        .filter((obj) => /\.png$/i.test(obj.key))
        .map((obj) => obj.key);

    if (imageFiles.length === 0) {
        console.log("No images found in the path");
        return;
    }

    const randomIndex = Math.floor(Math.random() * imageFiles.length);
    const selectedImageKey = imageFiles[randomIndex];

    const imageObject = await env.MY_R2_BUCKET.get(selectedImageKey ?? "");
    if (!imageObject) {
        console.log(`Failed to fetch image: ${selectedImageKey}`);
        return;
    }

    const imageBody = await imageObject.arrayBuffer();

    await env.MY_R2_BUCKET.put(destPath, imageBody, {
        httpMetadata: {
            contentType: imageObject.httpMetadata?.contentType ?? "image/png",
        },
    });

    console.log(`Updated splash.png with ${selectedImageKey}`);
}
