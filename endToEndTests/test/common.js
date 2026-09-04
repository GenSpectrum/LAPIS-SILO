import supertest from 'supertest';
import { expect } from 'chai';

const rhydbUrl = process.env.RHYDB_URL;
if (!rhydbUrl) {
  throw new Error('Test execution issue: Execute the tests with "RHYDB_URL=<url>" set');
}

export const server = supertest.agent(rhydbUrl);

export function expectHeaderToHaveDataVersion(response) {
  const headers = response.headers;
  expect(headers).to.have.property('data-version');
  const dataVersion = headers['data-version'];
  expect(dataVersion).to.be.a('string');
  expect(dataVersion).to.match(/\d{10}/);
}

// Parses the `result-ordering` response header describing the order of the result rows: a JSON array with
// one `{ field, order, nullPlacement }` object per sort key, or `[]` when there is no explicit
// ordering.
export function getResultOrdering(response) {
  const headers = response.headers;
  expect(headers).to.have.property('result-ordering');
  return JSON.parse(headers['result-ordering']);
}
