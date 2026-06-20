//// ONLINE UK

const Config = createPerfectNightPeerConfig({
    host: "0.peerjs.com",
    path: "/",
    iceServers: [
        {
            urls: [
                // "turn:perfect-night.aventu.re:3478",
                "stun:perfect-night.aventu.re:3478",
            ],
            username: "perfectnight",
            credential: "RohM0sWu2cEo8JjLSRwgahlkq6O48Oe0anoNsr1FAqUhK9",
        },
    ],
});
