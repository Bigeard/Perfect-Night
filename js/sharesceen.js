// Management of the screen share
// const listScreenShare = new Array(8); 

// class VirtualScreenShare {
//     conn;
//     id;
//     status = "Connect";
//     lastUpdate = 0;

//     constructor(conn, id) {
//         this.conn = conn;
//         this.id = id;
//         this.connect();
//     }

//     connect() {
//         this.conn.on('data', (data) => {
//             console.info("Share Screen Data", data);
//         });
//         this.conn.on('disconnect', () => {
//             this.status = "Disconnect";
//             console.info("Share Screen Disconnect", this.id);
//         });
//         this.conn.on('close', () => {
//             this.status = "Close";
//             console.info("Share Screen Close", this.id);
//             this.conn = null;
//         });
//     }
// }