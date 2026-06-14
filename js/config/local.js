//// LOCAL

const Config = {
    reliable: false, // Low Latency (not sure...)
    secure: true,
    // host: "192.168.12.1",
    // host: "127.0.0.1",
    port: 443,
    path: "/peer",
    config: {
        iceServers: [
            {
                urls: [
                    "stun:stun.l.google.com:19302",
                ]
            },
            // {
            //     urls: [
            //         "turn:192.168.12.1:3478",
            //         "turn:192.168.0.205:3478"
            //     ],
            //     username: "perfect",
            //     credential: "perfect"
            // },
        ],
    },
    iceServers: [
        {
            urls: [
                "stun:stun.l.google.com:19302",
            ]
        },
        // {
        //     urls: [
        //         "turn:192.168.12.1:3478",
        //         "turn:192.168.0.205:3478"
        //     ],
        //     username: "perfect",
        //     credential: "perfect"
        // },
    ],
    serialization: 'none',
    debug: 0,
};