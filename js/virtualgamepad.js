let menuAction = 0;
const listGamepad = new Map([]);
const disconnectGamepad = [];

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
                this.axes[0] = data.lx || 0;
                this.axes[1] = data.ly || 0;
                this.axes[2] = data.rx || 0;
                this.axes[3] = data.ry || 0;
                const dataSend = { t: data.t };

                // See if there have been any edit.
                // Or if the customer needs to have the latest edit.
                if (this.edit || data.e) {
                    dataSend.index = this.index;
                    dataSend.color = this.color;
                    dataSend.life = this.life;
                    dataSend.ammunition = this.ammunition;
                    this.edit = false;
                }
                if (data.ma) { // Menu Action
                    menuAction = data.ma;
                }
                this.conn.send(dataSend);
            }
            else {
                console.info("Peer:", data);
            }
        });
        this.conn.on('disconnect', () => {
            this.status = "Disconnect";
            console.info("Disconnect", this.id, this.name);
        });
        this.conn.on('close', () => {
            this.status = "Close";
            console.info("Close", this.id, this.name);
            this.conn = null;
            // disconnectGamepad.push(this.id)
            // listGamepad.delete(this.id);
        });
    }
}