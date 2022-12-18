const listGamepad = new Map([])

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
    edit = false;
    // Optional add life and amm. in the mobile view 
    // life = 0; 
    // ammunition = 0;

    constructor(conn, id, index, name, shortName, type = "mobile") {
        this.conn = conn;
        this.id = id;
        this.index = index + 1;
        this.name = name || "Player " + this.index;
        this.shortName = shortName || "P" + this.index;
        this.type = type;
        this.connect();
    }

    connect() {
        this.conn.on('data', (data) => {
            if (data != null && (typeof data === 'object' || typeof data === 'function')) {
                if (data.j === "l") {
                    this.axes[0] = data.x;
                    this.axes[1] = data.y;
                }
                else {
                    this.axes[2] = data.x;
                    this.axes[3] = data.y;
                }
                const dataSend = {t: data.t};
                if (this.edit) {
                    dataSend.index = this.index;
                    dataSend.color = this.color;
                    this.edit = false;
                }
                this.conn.send(dataSend);
            }
            else {
                addMessage("<span class=\"peerMsg\">Peer: </span>" + data)
            }
        });
        this.conn.on('close', () => {
            // status.innerHTML = "Connection reset<br>Awaiting connection...";
            this.conn = null;
            this.status = "Disconnect"
            console.log("Disconnect " + this.id);
            // listGamepad.delete(this.id);
        });
    }

    // I can't send data (socket, WebRTC)
    // initColorGamepad() {
    //     console.log('InitColorGamepadPhone', this.conn, this.conn.open, this.color);
    //     if (this.conn) {
    //         console.log('conn');
    //         this.conn.send({
    //             type: "color",
    //             value: this.color
    //         })
    //     }
    //     if (this.conn && this.conn.open) {
    //         this.conn.send("HELLO !");
    //     }
    // }

    disconnect() {
        this.conn = null;
        this.status = "Disconnect"
        console.log("Disconnect " + this.id);
        listGamepad.delete(this.id);
    }
}