import { describe, it } from 'node:test';
import { server } from './common.js';

describe('SILO', () => {
  it('should return a proper 404 message when GETting an unknown url', async () => {
    await server.get('/unknown-url').expect(404);
  });

  it('should return a proper 404 message when POSTing to an unknown url', async () => {
    await server.post('/unknown-url').expect(404);
  });
});
