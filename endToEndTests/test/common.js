const supertest = require('supertest');
const { expect } = require('chai');

const siloUrl = process.env.SILO_URL;
if (!siloUrl) {
  throw new Error('Test execution issue: Execute the tests with "SILO_URL=<url>" set');
}

const server = supertest.agent(siloUrl);

function headerToHaveDataVersion(response) {
  const headers = response.headers;
  expect(headers).to.have.property('data-version');
  const dataVersion = headers['data-version'];
  expect(dataVersion).to.be.a('string');
  expect(dataVersion).to.match(/\d{10}/);
}

module.exports = {
  server,
  headerToHaveDataVersion,
};
