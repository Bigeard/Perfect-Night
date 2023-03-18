//// ONLINE UK

const Config = {
    reliable: false, // Low Latency (not sure...)
    secure: true,
    host: "perfect-night.aventu.re",
    port: 443,
    path: "/peer",
    config: {
        iceServers: [
            {
                urls: [
                    "turn:perfect-night.aventu.re:3478",
                    "stun:perfect-night.aventu.re:3478",
                ],
                username: "perfectnight",
                credential: "RohM0sWu2cEo8JjLSRwgahlkq6O48Oe0anoNsr1FAqUhK9"
            },
        ],
    },
    iceServers: [
        {
            urls: [
                "turn:perfect-night.aventu.re:3478",
                "stun:perfect-night.aventu.re:3478",
            ],
            username: "perfectnight",
            credential: "RohM0sWu2cEo8JjLSRwgahlkq6O48Oe0anoNsr1FAqUhK9"
        },
    ],
    serialization: 'none',
    debug: 0
};