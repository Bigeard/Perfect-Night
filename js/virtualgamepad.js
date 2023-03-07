// Management of the mobile gamepad
let perf = false;
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
            if (parseInt(data[0])) {
                data = data.split(",");
                // data 0 = time (int)
                // data 1 = lx (float)
                // data 2 = ly (float)
                // data 3 = rx (float)
                // data 4 = ry (float)
                const t = parseInt(data[0]);
                if (this.lastUpdate > t) return; // Out of date
                this.lastUpdate = t;

                this.axes[0] = parseFloat(data[1]);
                this.axes[1] = parseFloat(data[2]);
                this.axes[2] = parseFloat(data[3]);
                this.axes[3] = parseFloat(data[4]);

                if (!perf) this.conn.send(t); // -> Pong
                // Edit
                if (this.edit) this.conn.send(JSON.stringify(this.checkEdit({ t: data.t })));
            }
            else {
                data = JSON.parse(data);
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
                if (this.conn) this.conn.send(JSON.stringify(dataSend));
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