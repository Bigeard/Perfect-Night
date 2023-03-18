//// ONLINE EU

const Config = {
    reliable: false, // Low Latency (not sure...)
    secure: true,
    host: "perfect-night-374900.oa.r.appspot.com",
    port: 443,
    path: "/peer",
    config: {
        iceServers: [
            {
                urls: [
                    "stun:stun.l.google.com:19302",
                ]
            },
            {
                urls: [
                    "turn:eu-0.turn.peerjs.com:3478",
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
                "turn:eu-0.turn.peerjs.com:3478",
            ],
            username: "peerjs",
            credential: "peerjsp"
        },
    ],
    serialization: 'none',
    debug: 0,
};