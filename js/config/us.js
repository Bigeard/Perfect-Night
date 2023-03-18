//// ONLINE US

const Config = {
    reliable: false, // Low Latency (not sure...)
    secure: true,
    host: "0.peerjs.com",
    port: 443,
    path: "/",
    config: {
        iceServers: [
            {
                urls: [
                    "stun:stun.l.google.com:19302",
                ]
            },
            {
                urls: [
                    "turn:us-0.turn.peerjs.com:3478",
                ],
                username: "peerjs",
                credential: "peerjsp"
            }
        ],
    },
    iceServers: [
        {
            urls: [
                "stun:stun.l.google.com:19302",
            ]
        },
        {
            urls: [
                "turn:us-0.turn.peerjs.com:3478",
            ],
            username: "peerjs",
            credential: "peerjsp"
        },
    ],
    serialization: 'none',
    debug: 0,
};
