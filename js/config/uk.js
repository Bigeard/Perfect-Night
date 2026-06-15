//// ONLINE UK

const Config = createPerfectNightPeerConfig({
    host: "perfect-night.aventu.re",
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
