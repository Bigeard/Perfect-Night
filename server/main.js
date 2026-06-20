const express = require('express');
const { ExpressPeerServer } = require('peer');
const { createPeerServerOptions } = require('./peer-id-realm');

const app = express();
const path = require('path');

app.enable('trust proxy');
app.use('/', express.static(path.join(__dirname, '../')))

const PORT = process.env.PORT || 9000;
const HOST = process.env.HOST || '0.0.0.0';
const server = app.listen(PORT, HOST, () => {
  console.log(`App listening on http://${HOST}:${PORT}`);
  console.log('Press Ctrl+C to quit.');
});

const peerServer = ExpressPeerServer(server, createPeerServerOptions());

app.use('/peer', peerServer);

module.exports = app;
