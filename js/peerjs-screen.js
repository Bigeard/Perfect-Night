;(function() {
    var pushState = history.pushState;
    var replaceState = history.replaceState;

    history.pushState = function() {
        pushState.apply(history, arguments);
        window.dispatchEvent(new Event('pushstate'));
        window.dispatchEvent(new Event('locationchange'));
    };

    history.replaceState = function() {
        replaceState.apply(history, arguments);
        window.dispatchEvent(new Event('replacestate'));
        window.dispatchEvent(new Event('locationchange'));
    };

    window.addEventListener('popstate', function() {
        window.dispatchEvent(new Event('locationchange'))
    });
})();

window.addEventListener('locationchange', () => {
    console.log("Change the code (hash):", window.location.hash.substring(1), "Peer Id", PeerServerId);
    console.log("Peer Id", PeerServerId, " OR ", peer.id);
    if (PeerServerId !== window.location.hash.substring(1) && peer.id !== window.location.hash.substring(1)) {
        window.location.reload();
    }
});

console.info(Config);
let perf = false;
let menuAction = 0;

const safeJsonParseScreen = (value, fallback = null) => {
    try {
        return JSON.parse(value);
    } catch (error) {
        console.warn("Invalid JSON payload ignored", error);
        return fallback;
    }
};

const canSendScreenData = (connection) => {
    const channel = connection?.dataChannel;
    return Boolean(connection?.open && (!channel || channel.bufferedAmount < 65536));
};

// Share Screen
let dataReceive = false;
let listScreenShareIndex = 0;
const listScreenShare = new Array(8);

const inputShareScreen = document.getElementById("inputShareScreen");
const buttonShareScreen = document.getElementById("buttonShareScreen");
const clickShareScreen = () => {
    if (peer) {
        peer.disconnect()
        peer = null;
        // console.log(peer);
        PeerServerId = inputShareScreen.value.split('#')[1];
        if (!PeerServerId) {
            PeerServerId = inputShareScreen.value
        }
        // console.log(PeerServerId);
        // join();
        init();
    }
}
window.clickShareScreen = clickShareScreen;

// Copy link
const divCopyLink = document.getElementById("divCopyLink");
const inputCopyLink = document.getElementById("inputCopyLink");
const buttonCopyLink = document.getElementById("buttonCopyLink");
const clickCopyLink = () => {
    inputCopyLink.select();
    inputCopyLink.setSelectionRange(0, 99999);
    if (window.isSecureContext && navigator.clipboard) {
        navigator.clipboard.writeText(inputCopyLink.value);
    } else {
        document.execCommand('copy');
    }
    buttonCopyLink.innerText = "Successfully copied!"
}

window.clickCopyLink = clickCopyLink;

// PeerJS
let gamepadUrl = 'n';
let lastPeerId = null;
let peer = null;
let conn = null;

const init = () => {
    peer = new Peer(Config);
    console.info(peer);

    peer.on("open", (id) => {
        if (peer.id === null) {
            console.info("Received null id from peer open");
            peer.id = lastPeerId;
        } else {
            lastPeerId = peer.id;
        }

        // QrCode
        if (PeerServerId !== "") {
            gamepadUrl = window.location.href;
            inputCopyLink.value = gamepadUrl;
            window.location.hash = PeerServerId;
            console.info(gamepadUrl);
            join();
        } else {
            const url = new URL(window.location.href);
            url.hash = peer.id;
            gamepadUrl = url.href;
            inputCopyLink.value = gamepadUrl;
            window.location.hash = peer.id;
            console.info(gamepadUrl);
        }
    });

    peer.on("connection", (connection) => {
        if (connection.metadata && connection.metadata.type === "screenShare") {
            menuAction = 5;
            if (listScreenShareIndex === 0) {
                inputShareScreen.style.opacity = '0';
                buttonShareScreen.style.opacity = '0';
                inputShareScreen.addEventListener('transitionend', () => inputShareScreen.remove());
                buttonShareScreen.addEventListener('transitionend', () => buttonShareScreen.remove());
            }
            listScreenShare[listScreenShareIndex] = { conn: connection, init: false };
            listScreenShareIndex++;

            console.info("Main Connected to", connection.peer, connection);
            return;
        }

        const gamepadId = connection.metadata?.id || connection.peer;
        let gamepad = listGamepad.get(gamepadId);

        if (gamepad) { // If connection exist
            if (gamepad.conn && gamepad.conn !== connection) gamepad.conn.close();
            gamepad.conn = connection;
            gamepad.connect();
        }
        else if (listGamepad.size >= 8) { // check if the maximum size is reached
            connection.on("open", () => {
                if (canSendScreenData(connection)) connection.send("Error: To many players");
                setTimeout(() => connection.close(), 500);
            });
            return;
        }
        else { // Create Virtual Gamepad and the player
            if (listGamepad.size === 0) {
                inputShareScreen.style.opacity = '0';
                buttonShareScreen.style.opacity = '0';
                inputShareScreen.addEventListener('transitionend', () => inputShareScreen.remove());
                buttonShareScreen.addEventListener('transitionend', () => buttonShareScreen.remove());
            }
            gamepad = new VirtualGamepad(
                connection,
                gamepadId,
                listGamepad.size
            );
            listGamepad.set(gamepad.id, gamepad);
        }

        const listPlayerConnected = [];
        listGamepad.forEach(g => listPlayerConnected.push({ name: g.name, peerId: g.conn?.peer }));
        console.info("Player connected: ", listPlayerConnected);
    });

    const reconnectPeer = () => {
        if (!peer || !lastPeerId || peer.destroyed) return;
        try {
            peer.id = lastPeerId;
            peer._lastServerId = lastPeerId;
            if (peer.disconnected) peer.reconnect();
        } catch (error) {
            console.warn("Peer reconnect failed", error);
        }
    };

    peer.on("disconnected", () => {
        console.info("Connection lost. Please reconnect");
        reconnectPeer();
        // listGamepad.forEach(g => g.disconnect());
    });

    peer.on("close", () => {
        console.info("Connection destroyed. Please refresh");
        reconnectPeer();
        // listGamepad.forEach(g => g.disconnect());
    });

    peer.on("data", (data) => {
        console.log(data);
    })

    peer.on("error", (err) => {
        alert(err);
        location.replace(window.location.origin);
    });
};

const join = () => {
    if (conn) {
        conn.close();
    }
    conn = peer.connect(PeerServerId, {
        metadata: {
            type: "screenShare",
        },
        reliable: false,
        serialization: 'raw'
    });
    conn.on("open", () => {
        console.info("Second Connected to", conn.peer, conn);
        menuAction = 6;
        buttonShareScreen.innerText = "Successfully connected!"

        inputShareScreen.style.opacity = '0';
        buttonShareScreen.style.opacity = '0';
        inputShareScreen.addEventListener('transitionend', () => inputShareScreen.remove());
        buttonShareScreen.addEventListener('transitionend', () => buttonShareScreen.remove());
    });
    conn.on("data", (data) => {
        if (parseInt(data[0])) {
            dataReceive = data;
        }
        else {
            data = safeJsonParseScreen(data);
            if (!data) return;
            if (data.ma) { // Menu Action
                menuAction = data.ma;
                if (menuAction === 4) perf = !perf;
            }
            // Game Settings
            else if (data.s) changeSettings(data.s);
            else if (data.g) {
                let gamepad = false;
                listGamepad.forEach(g => g.conn?.peer === data.g.peer ? gamepad = g : 0);
                if (!gamepad) {
                    gamepad = new VirtualGamepad(
                        null,
                        data.g.peer,
                        data.g.id,
                    );
                    listGamepad.set(gamepad.id, gamepad);
                }
            }
        }
    });
}

(() => {
    init();
})();
