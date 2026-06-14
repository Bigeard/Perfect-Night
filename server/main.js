const express = require('express');
const { ExpressPeerServer } = require('peer');

const { v4: uuidV4 } = require('uuid');
const realm = require('peer/dist/src/models/realm');

const app = express();
const path = require('path');

app.enable('trust proxy');
app.use('/', express.static(path.join(__dirname, '../')))

const PORT = process.env.PORT || 9000;
const server = app.listen(PORT, () => {
  console.log(`App listening on port: ${PORT}`);
  console.log('Press Ctrl+C to quit.');
});

class NewRealm extends realm.Realm {
  generateClientId() {
    let size = 4;
    let clientId = uuidV4().substring(0, size);
    while (this.getClientById(clientId)) {
      clientId = uuidV4().substring(0, size);
      size++;
    }
    return clientId;
  }
}

realm.Realm = NewRealm;

const peerServer = ExpressPeerServer(server, {
  path: '/'
});

app.use('/peer', peerServer);

module.exports = app;