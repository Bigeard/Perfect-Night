//// ONLINE US

const Config = createPerfectNightPeerConfig({
    host: "0.peerjs.com",
    path: "/",
    iceServers: [
        {
            urls: [
                "stun:stun.l.google.com:19302",
            ],
        },
        // {
        //     urls: [
        //         "turn:us-0.turn.peerjs.com:3478",
        //     ],
        //     username: "peerjs",
        //     credential: "peerjsp"
        // },
    ],
});
