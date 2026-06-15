const assert = require('assert');
const { createPeerServerOptions, generateShortPeerId } = require('./peer-id-realm');

const id = generateShortPeerId();
assert.match(id, /^[0-9a-f]{4}$/);

const options = createPeerServerOptions();
assert.strictEqual(options.path, '/');
assert.strictEqual(options.generateClientId, generateShortPeerId);
