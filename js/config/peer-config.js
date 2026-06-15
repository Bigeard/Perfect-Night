(function (global) {
    global.createPerfectNightPeerConfig = ({
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
})(window);
