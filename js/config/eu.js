//// ONLINE EU

const Config = createPerfectNightPeerConfig({
    host: "perfect-night-374900.oa.r.appspot.com",
    iceServers: [
        {
            urls: [
                "stun:stun.l.google.com:19302",
            ],
        },
        // {
        //     urls: [
        //         "turn:eu-0.turn.peerjs.com:3478",
        //     ],
        //     username: "peerjs",
        //     credential: "peerjsp"
        // },
    ],
});
