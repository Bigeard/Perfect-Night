        ; (function () {
            var pushState = history.pushState;
            var replaceState = history.replaceState;

            history.pushState = function () {
                pushState.apply(history, arguments);
                window.dispatchEvent(new Event('pushstate'));
                window.dispatchEvent(new Event('locationchange'));
            };

            history.replaceState = function () {
                replaceState.apply(history, arguments);
                window.dispatchEvent(new Event('replacestate'));
                window.dispatchEvent(new Event('locationchange'));
            };

            window.addEventListener('popstate', function () {
                window.dispatchEvent(new Event('locationchange'))
            });
        })();

        window.addEventListener('locationchange', () => {
            window.location.reload();
        });

        const isIphone = /iPad|iPhone|iPod/.test(navigator.userAgent)

        const safeJsonParse = (value, fallback = null) => {
            try {
                return JSON.parse(value);
            } catch (error) {
                console.warn("Invalid JSON payload ignored", error);
                return fallback;
            }
        };

        const safeLocalStorageGet = (key, fallback = null) => {
            try {
                const value = localStorage.getItem(key);
                return value == null ? fallback : value;
            } catch (error) {
                console.warn("Unable to read localStorage", key, error);
                return fallback;
            }
        };

        const safeLocalStorageSet = (key, value) => {
            try {
                localStorage.setItem(key, value);
            } catch (error) {
                console.warn("Unable to write localStorage", key, error);
            }
        };

        const CONTROL_BUFFER_LIMIT = 65536;
        const INPUT_BUFFER_LIMIT = 4096;

        const canSend = (connection, maxBufferedAmount = CONTROL_BUFFER_LIMIT) => {
            const channel = connection?.dataChannel;
            return Boolean(connection?.open && (!channel || channel.bufferedAmount < maxBufferedAmount));
        };

        const sendJson = (connection, payload) => {
            if (canSend(connection)) {
                connection.send(JSON.stringify(payload));
                return true;
            }
            return false;
        };

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
            buttonCopyLink.innerText = "Copied!"
        }
        window.clickCopyLink = clickCopyLink;
        inputCopyLink.value = window.location.origin + window.location.hash;

        // Ping / Latency Test
        // let pings = [0];
        // let w = 0; // Loop 500

        // Find Peer Server Id (in URL)
        const PeerServerId = window.location.hash.substring(1); // remove `#` before the code
        console.info("Peer Server (Main) Id:", PeerServerId);

        // Menu of the gamepad
        const menuGame = document.getElementById("menuGame");
        const informationGame = document.getElementById("informationGame");
        const optionGame = document.getElementById("optionGame");
        const gameSettings = document.getElementById("gameSettings");
        const qrCodeGame = document.getElementById("qrCodeGame");
        const listWindow = ["menuGame", "informationGame", "optionGame", "gameSettings", "qrCodeGame"];

        const windowOpen = (event, doc) => {
            listWindow.forEach(d => {
                const node = window[d];
                if (node != doc) node.style.display = "none";
            });
            if (doc.style.display != "flex") doc.style.display = "flex";
            else doc.style.display = "none";
        }

        const windowClose = (event, parent) => {
            parent ? parent.style.display = "none" : event.target.parentNode.style.display = "none";
        }

        const menuAction = (event, action) => {
            sendJson(conn, {
                t: Date.now(), // Time
                ma: action, // Menu Action
            });
        }

        const changeSettings = (form) => {
            // e.preventDefault();
            const data = new FormData(form);
            const formDataObj = {};
            data.forEach((value, key) => (formDataObj[key] = parseInt(value)));
            sendJson(conn, {
                t: Date.now(), // Time
                s: formDataObj, // Settings
            });
        }

        ['click', 'touchstart'].forEach(evt =>
            window.addEventListener(evt, event => {
                listWindow.forEach(doc => {
                    const node = window[doc];
                    if (!node.contains(event.target) &&
                        !event.target.classList.contains("menuButton") &&
                        !event.target.classList.contains("menuInformation") &&
                        !event.target.classList.contains("optionButton") &&
                        !event.target.classList.contains("optionGameSettings") &&
                        !event.target.classList.contains("qrCodeButton")) {
                        node.style.display = "none";
                    }
                })
            })
        );

        const scrollOnTop = (event) => {
            event.preventDefault();
            document.documentElement.scrollTop = 0;
        }
        window.windowOpen = windowOpen;
        window.windowClose = windowClose;
        window.menuAction = menuAction;
        window.changeSettings = changeSettings;
        window.scrollOnTop = scrollOnTop;

        // QrCode 
        const toSvgString = (qr, border, lightColor, darkColor) => {
            if (border < 0)
                throw new RangeError("Border must be non-negative");
            let parts = [];
            for (let y = 0; y < qr.size; y++) {
                for (let x = 0; x < qr.size; x++) {
                    if (qr.getModule(x, y))
                        parts.push(`M${x + border},${y + border}h1v1h-1z`);
                }
            }
            return `<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE svg PUBLIC "-//W3C//DTD SVG 1.1//EN" "http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd">
<svg xmlns="http://www.w3.org/2000/svg" version="1.1" viewBox="0 0 ${qr.size + border * 2} ${qr.size + border * 2}" stroke="none" style="border-radius: 10px;">
	<rect width="100%" height="100%" fill="${lightColor}"/>
	<path d="${parts.join(" ")}" fill="${darkColor}"/>
</svg>
`;
        }
        const qrCodeDisplay = document.getElementById("qrCodeDisplay");
        const QRC = qrcodegen.QrCode;
        // Simple operation
        console.log(window.location.origin + window.location.hash);
        const qr0 = QRC.encodeText(window.location.origin + window.location.hash, QRC.Ecc.MEDIUM);
        const svg = toSvgString(qr0, 2, "#fff", "#000");
        qrCodeDisplay.innerHTML = svg;


        // NippleJS - Joystick
        const joysticks = document.getElementById("joysticks");
        const buttonShoot = document.getElementById("buttonShoot");
        let onShoot = false;

        // IOS disable zoom page
        if (isIphone) {
            joysticks.addEventListener("touchend", (event) => {
                event.preventDefault();
            }, { passive: false });
            joysticks.addEventListener("gesturestart", (event) => {
                event.preventDefault();
            });
            joysticks.addEventListener("gesturechange", (event) => {
                event.preventDefault();
            });
            joysticks.addEventListener("gestureend", (event) => {
                event.preventDefault();
            });
        }

        // On click shoot !
        buttonShoot.addEventListener("pointerdown", () => {
            onShoot = true;
            buttonShoot.style.opacity = 0.5;
        });
        buttonShoot.addEventListener("pointerout", () => {
            onShoot = false;
            buttonShoot.style.opacity = 0.25;
        });

        const joystickLeft = nipplejs.create({
            zone: document.getElementById("joystickLeft"),
            mode: "static",
            position: { left: "50%", top: "50%" },
            color: "grey",
            size: 185
        });

        const joystickRight = nipplejs.create({
            zone: document.getElementById("joystickRight"),
            mode: "static",
            position: { right: "50%", top: "50%" },
            color: "grey",
            size: 185
        });

        const getJoystick = (collection) => {
            if (collection?.[0]) return collection[0];
            if (typeof collection?.getJoystickByUid === "function") return collection.getJoystickByUid();
            return null;
        };

        const leftJoystick = getJoystick(joystickLeft);
        const rightJoystick = getJoystick(joystickRight);
        if (!leftJoystick || !rightJoystick) {
            throw new Error("Unable to initialize virtual joysticks");
        }

        leftJoystick.resetDirection = function () {
            this.direction = {
                x: false,
                y: false,
                angle: false
            };
            this.frontPosition.x = 0;
            this.frontPosition.y = 0;
        };

        rightJoystick.resetDirection = function () {
            this.direction = {
                x: false,
                y: false,
                angle: false
            };
            this.frontPosition.x = 0;
            this.frontPosition.y = 0;
        };

        // Display none scroll up indicator
        document.addEventListener("scroll", () => {
            if (joysticks.offsetHeight + joysticks.scrollTop >= joysticks.scrollHeight) {
                scrollUp.style.display = "none";
            }
        });

        // Display / Disable control information
        const controlInfo = safeLocalStorageGet("controlInfo");
        if (controlInfo && controlInfo != PeerServerId) {
            joysticks.classList.remove("controlInfo");
        } else {
            safeLocalStorageSet("controlInfo", PeerServerId);
        }

        let conn = null;
        let peer = {};

        // PeerJS - Communication with main screen
        (() => {
            let PeerPlayerId = false;
            let lastPeerId = null;
            const status = document.getElementById("status");
            const ping = document.getElementById("ping");
            const infoContainer = document.getElementById("infoContainer");
            const infoBottom = document.getElementById("infoBottom");
            const playerId = document.getElementById("playerId");
            const playerLife = document.getElementById("playerLife");
            const playerAmmunition = document.getElementById("playerAmmunition");
            const goTop = document.getElementById("goTop");

            let lastPing = 0;
            let lastSendTimestamp = 0;
            let lastInputPayload = "";

            const createInputPacket = (t, lx, ly, rx, ry) => {
                const packet = new ArrayBuffer(24);
                const view = new DataView(packet);
                view.setFloat64(0, t, true);
                view.setFloat32(8, lx, true);
                view.setFloat32(12, ly, true);
                view.setFloat32(16, rx, true);
                view.setFloat32(20, ry, true);
                return packet;
            };

            const reconnectPeer = (message) => {
                status.innerText = message;
                if (!peer || !lastPeerId || peer.destroyed) return;
                try {
                    peer.id = lastPeerId;
                    peer._lastServerId = lastPeerId;
                    if (peer.disconnected) {
                        peer.reconnect();
                    }
                } catch (error) {
                    console.warn("Peer reconnect failed", error);
                }
            };

            const init = () => {
                status.innerText = "No connection";
                peer = new Peer(Config);

                peer.on("open", (NewPeerPlayerId) => {
                    console.info("Peer Player (Client) Id:", NewPeerPlayerId);
                    if (peer.id === null) {
                        peer.id = lastPeerId;
                        console.info("Received null id from peer open");
                    } else {
                        lastPeerId = peer.id;
                    }
                    // Manage Peer Player Id to reconnect easily
                    const peerConnection = safeJsonParse(safeLocalStorageGet("peerConnection"), {});
                    if (peerConnection?.PeerServerId === PeerServerId) {
                        PeerPlayerId = peerConnection.PeerPlayerId
                    }
                    // @DEV
                    if (!PeerPlayerId) {
                        // Store connection player
                        safeLocalStorageSet("peerConnection", JSON.stringify({
                            PeerServerId,
                            PeerPlayerId: NewPeerPlayerId
                        }));
                    }

                    // Join the connection
                    if (PeerServerId) {
                        join(PeerServerId);
                    } else {
                        console.error("No ID - PeerServerId:", PeerServerId);
                    }
                });

                peer.on("connection", (c) => {
                    c.on("open", () => {
                        c.send("Sender does not accept incoming connections");
                        setTimeout(() => c.close(), 500);
                    });
                });

                peer.on("disconnected", () => {
                    reconnectPeer("Connection lost. Please reconnect (disconnected)");
                });

                peer.on("disconnect", () => {
                    reconnectPeer("Connection lost. Please reconnect (disconnect)");
                });

                peer.on("close", () => {
                    reconnectPeer("Connection lost. Please reconnect (close)");
                });

                peer.on("error", (err) => {
                    console.error(err);
                });
            };

            const join = (PeerServerId) => {
                if (conn) {
                    conn.close();
                }
                conn = peer.connect(PeerServerId, {
                    metadata: {
                        id: PeerPlayerId,
                    },
                    reliable: false,
                    serialization: 'raw'
                });
                conn.on("open", () => {
                    status.innerText = "Connected";
                    sendJson(conn, { e: 1 }) // Get last edit
                    console.info("Connected to ", conn.peer)
                });
                conn.on("data", (data) => {
                    if (parseInt(data)) {
                        //// Ping / Latency Test
                        // if (data) {
                        //     if (w >= 1000) w = 0;
                        //     pings[w] = Date.now() - data;
                        //     w++
                        // }
                        // var sum = pings.reduce((p, a) => p + a, 0);
                        // ping.innerText = "Ping: " + (Date.now() - data) + "- T: " + (sum / pings.length);
                        ping.innerText = "Ping: " + (Date.now() - data);
                        lastPing = data;
                    }
                    else {
                        data = safeJsonParse(data);
                        // console.info("Peer:", data);
                        if (data != null && (typeof data === "object" || typeof data === "function")) {
                            if (data.color != null) {
                                buttonShoot.style.backgroundColor = data.color;

                                leftJoystick.options.color = data.color;
                                leftJoystick.ui.front.style.background = data.color;
                                leftJoystick.ui.back.style.background = data.color;

                                rightJoystick.options.color = data.color;
                                rightJoystick.ui.front.style.background = data.color;
                                rightJoystick.ui.back.style.background = data.color;

                                infoContainer.style.background = data.color;
                                playerId.style.background = data.color;
                                playerLife.style.background = data.color;
                                playerAmmunition.style.background = data.color;
                                goTop.style.background = data.color;
                                scrollUp.style.background = data.color;
                            }
                            if (data.index != null) {
                                playerId.innerText = "PLAYER " + data.index;
                            }
                            if (data.life != null || data.life === 0) {
                                playerLife.innerText = "Life " + data.life;
                                if (data.life <= 0) {
                                    joysticks.classList.add("deadBackground");
                                }
                                else {
                                    joysticks.classList.remove("deadBackground");
                                }
                            }
                            if (data.ammunition != null || data.ammunition === 0) {
                                playerAmmunition.innerText = "Ammu. " + data.ammunition;
                            }
                        }
                    }
                });
                conn.on("disconnected", () => {
                    reconnectPeer("Connection disconnected (conn.on(disconnected))");
                });
                conn.on("disconnect", () => {
                    reconnectPeer("Connection disconnect (conn.on(disconnect))");
                });
                conn.on("close", () => {
                    reconnectPeer("Connection closed (conn.on(close))");
                });
            };

            const signal = (sigName) => {
                if (sendJson(conn, { sigName })) {
                    console.info(sigName, "signal sent");
                } else {
                    console.info("Connection is closed");
                }
            }

            let rx = 0;
            let ry = 0;

            const loop = (timestamp) => {
                if (lastPing != 0 && Date.now() - lastPing > 3000) {
                    lastPing = 0;
                    if (peer?.open) peer.disconnect();
                    if (conn?.peer) join(conn.peer);
                }
                if (timestamp - lastSendTimestamp >= 1000 / 60 && leftJoystick && rightJoystick && peer.id && canSend(conn, INPUT_BUFFER_LIMIT)) {
                    const lx = Number(leftJoystick.frontPosition.x.toFixed(2));
                    const ly = Number(leftJoystick.frontPosition.y.toFixed(2));
                    if (onShoot) {
                        rx = 2.5;
                        ry = 2.5;
                    } else {
                        rx = Number(rightJoystick.frontPosition.x.toFixed(2));
                        ry = Number(rightJoystick.frontPosition.y.toFixed(2));
                    }
                    // Time, lx, ly, rx, ry
                    const inputPayload = `${lx},${ly},${rx},${ry}`;
                    if (inputPayload !== lastInputPayload || onShoot) {
                        conn.send(createInputPacket(Date.now(), lx, ly, rx, ry));
                        lastInputPayload = inputPayload;
                    }
                    lastSendTimestamp = timestamp;
                }
                window.requestAnimationFrame(loop);
            }
            window.requestAnimationFrame(loop);
            init();
        })();
