// Management of the mobile gamepad
const listGamepad = new Map([]);
const disconnectGamepad = [];
const CONTROL_BUFFER_LIMIT = 65536;
const INPUT_BUFFER_LIMIT = 4096;

const safeJsonParse = (value, fallback = null) => {
    try {
        return JSON.parse(value);
    } catch (error) {
        console.warn("Invalid JSON payload ignored", error);
        return fallback;
    }
};

const canSendPeerData = (conn, maxBufferedAmount = CONTROL_BUFFER_LIMIT) => {
    const channel = conn?.dataChannel;
    return Boolean(conn?.open && (!channel || channel.bufferedAmount < maxBufferedAmount));
};

const sendPeerJson = (conn, payload) => {
    if (canSendPeerData(conn)) {
        conn.send(JSON.stringify(payload));
        return true;
    }
    return false;
};

const isBinaryInputPacket = (data) => {
    if (data instanceof ArrayBuffer) return data.byteLength >= 24;
    if (ArrayBuffer.isView(data)) return data.byteLength >= 24;
    return false;
};

const inputPacketDataView = (data) => {
    if (data instanceof ArrayBuffer) return new DataView(data);
    return new DataView(data.buffer, data.byteOffset, data.byteLength);
};

class VirtualGamepad {
    conn;
    id;
    index;
    name;
    shortName;
    type;
    axes = [0, 0, 0, 0];
    status = "Connect";
    color;
    life = 0;
    ammunition = 0;
    edit = false;
    lastUpdate = 0;

    constructor(conn, id, index, name, shortName, type = "mobile") {
        divCopyLink.style.display = "none";
        this.conn = conn;
        this.id = id;
        this.index = index + 1;
        this.name = name || "Player " + this.index;
        this.shortName = shortName || "P" + this.index;
        this.type = type;

        let i = 0;
        while (i < listScreenShareIndex)
        {
            sendPeerJson(listScreenShare[i]?.conn, {
                t: Date.now(), // Time
                g: {
                    peer: id,
                    id: index,
                }, // Create Virtual Gamepad
            });
            i++;
        }
        if (conn) {
            this.connect();
        }
    }

    connect() {
        const conn = this.conn;

        conn.on('data', (data) => {
            if (this.conn !== conn) return;

            if (isBinaryInputPacket(data)) {
                const view = inputPacketDataView(data);
                const t = view.getFloat64(0, true);
                if (this.lastUpdate > t) return; // Out of date
                this.lastUpdate = t;

                this.axes[0] = view.getFloat32(8, true);
                this.axes[1] = view.getFloat32(12, true);
                this.axes[2] = view.getFloat32(16, true);
                this.axes[3] = view.getFloat32(20, true);

                if (!perf && canSendPeerData(conn, INPUT_BUFFER_LIMIT)) conn.send(t); // -> Pong
                // Edit
                if (this.edit) sendPeerJson(conn, this.checkEdit({ t }));
            }
            else if (parseInt(data[0])) {
                data = data.split(",");
                // data 0 = time (int)
                // data 1 = lx (float)
                // data 2 = ly (float)
                // data 3 = rx (float)
                // data 4 = ry (float)
                const t = parseInt(data[0]);
                if (this.lastUpdate > t) return; // Out of date
                this.lastUpdate = t;

                this.axes[0] = Number.isFinite(parseFloat(data[1])) ? parseFloat(data[1]) : 0;
                this.axes[1] = Number.isFinite(parseFloat(data[2])) ? parseFloat(data[2]) : 0;
                this.axes[2] = Number.isFinite(parseFloat(data[3])) ? parseFloat(data[3]) : 0;
                this.axes[3] = Number.isFinite(parseFloat(data[4])) ? parseFloat(data[4]) : 0;

                if (!perf && canSendPeerData(conn, INPUT_BUFFER_LIMIT)) conn.send(t); // -> Pong
                // Edit
                if (this.edit) sendPeerJson(conn, this.checkEdit({ t }));
            }
            else {
                data = safeJsonParse(data);
                if (!data) return;
                let dataSend = { t: data.t };

                // See if there have been any edit.
                // Or if the customer needs to have the latest edit.
                if (data.ma) { // Menu Action
                    menuAction = data.ma;
                    if (menuAction === 4) perf = !perf;
                }
                // Edit
                else if (data.e) dataSend = this.checkEdit(dataSend);
                // Game Settings
                else if (data.s) changeSettings(data.s);
                sendPeerJson(conn, dataSend);
            }
        });
        conn.on('disconnect', () => {
            if (this.conn !== conn) return;
            this.status = "Disconnect";
            console.info("Disconnect", this.id, this.name);
        });
        conn.on('close', () => {
            if (this.conn !== conn) return;
            this.status = "Close";
            console.info("Close", this.id, this.name);
            this.conn = null;
            // disconnectGamepad.push(this.id)
            // listGamepad.delete(this.id);
        });
    }

    checkEdit(dataSend) {
        this.edit = false;
        dataSend.index = this.index;
        dataSend.color = this.color;
        dataSend.life = this.life;
        dataSend.ammunition = this.ammunition;
        // dataSend.settings = settings;
        return dataSend;
    }
}
