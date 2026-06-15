const { randomUUID } = require('crypto');

const SHORT_ID_LENGTH = 4;

const generateShortPeerId = () => randomUUID().replaceAll('-', '').substring(0, SHORT_ID_LENGTH);

const createPeerServerOptions = (overrides = {}) => {
  return {
    path: '/',
    generateClientId: generateShortPeerId,
    ...overrides,
  };
};

module.exports = {
  createPeerServerOptions,
  generateShortPeerId,
};
