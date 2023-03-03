const supertest = require('supertest');

const siloUrl = process.env.SILO_URL;
if (!siloUrl) {
  throw new Error('Test execution issue: Execute the tests with "SILO_URL=<url>" set');
}

const server = supertest.agent(siloUrl);

module.exports = {
  server,
};
