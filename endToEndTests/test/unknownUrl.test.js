const { server } = require('./common');

describe('SILO', () => {
  it('should return a proper 404 message when GETting an unknown url', done => {
    server
      .get('/unknown-url')
      .expect(404)
      .expect('Content-Type', 'application/json')
      .expect({ error: 'not found' })
      .end(done);
  });

  it('should return a proper 404 message when POSTing to an unknown url', done => {
    server
      .post('/unknown-url')
      .expect(404)
      .expect('Content-Type', 'application/json')
      .expect({ error: 'not found' })
      .end(done);
  });
});
