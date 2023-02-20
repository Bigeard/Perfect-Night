// This server allows you to generate the static page of the game.
// You can also use the extension live server (VScode).

const express = require('express');
const app = express();
const path = require('path');
const PORT = process.env.PORT || 80;

app.use('/', express.static(path.join(__dirname, '../')))

const server = app.listen(PORT, () => {
  console.log(`App listening on port ${PORT}`);
  console.log('Press Ctrl+C to quit.');
});

module.exports = app;
