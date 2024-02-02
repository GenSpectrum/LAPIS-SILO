import supertest from 'supertest';
import { expect } from 'chai';

const siloUrl = process.env.SILO_URL;
if (!siloUrl) {
  throw new Error('Test execution issue: Execute the tests with "SILO_URL=<url>" set');
}

export const server = supertest.agent(siloUrl);

export function headerToHaveDataVersion(response) {
  const headers = response.headers;
  expect(headers).to.have.property('data-version');
  const dataVersion = headers['data-version'];
  expect(dataVersion).to.be.a('string');
  expect(dataVersion).to.match(/\d{10}/);
}
