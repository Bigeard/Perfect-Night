//// LOCAL

const Config = {
    reliable: false, // Low Latency (not sure...)
    secure: false,
    // host: "192.168.12.1",
    host: "192.168.0.205",
    port: 9000,
    path: "/peer",
    config: {
        iceServers: [
            {
                urls: [
                    // "turn:192.168.12.1:3478",
                    "turn:192.168.0.205:3478"
                ],
                username: "perfect",
                credential: "perfect"
            },
        ],
    },
    iceServers: [
        {
            urls: [
                // "turn:192.168.12.1:3478",
                "turn:192.168.0.205:3478"
            ],
            username: "perfect",
            credential: "perfect"
        },
    ],
    serialization: 'none',
    debug: 0,
};