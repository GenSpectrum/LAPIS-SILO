import { expect } from 'chai';
import { server } from './common.js';
import { describe, it } from 'node:test';

describe('The /health endpoint', () => {
  it('should return UP', async () => {
    await server
      .get('/health')
      .expect(200)
      .expect('Content-Type', 'application/json')
      .expect(response => {
        expect(response.body).to.include.key('status');
        expect(response.body.status).to.equal('UP');
      });
  });
});
