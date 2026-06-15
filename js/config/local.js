//// LOCAL

if (typeof createPerfectNightPeerConfig !== "function") {
    window.createPerfectNightPeerConfig = ({
        host,
        path = "/peer",
        port = 443,
        secure = true,
        iceServers = [],
        debug = 0,
    }) => ({
        reliable: false,
        secure,
        host,
        port,
        path,
        config: { iceServers },
        iceServers,
        serialization: "none",
        debug,
    });
}

const Config = createPerfectNightPeerConfig({
    // host: "192.168.12.1",
    // host: "127.0.0.1",
    host: "incalescent-phytologic-palmer.ngrok-free.dev",
    iceServers: [
        {
            urls: [
                "stun:stun.l.google.com:19302",
            ],
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
});
