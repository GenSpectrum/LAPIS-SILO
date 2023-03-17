const { server } = require('./common');

describe('The /info endpoint', () => {
  it('should return info about the current state of the database', done => {
    server
      .get('/info')
      .expect(200)
      .expect('Content-Type', 'application/json')
      .expect({ nBitmapsSize: 3552, sequenceCount: 100, totalSize: 66458430 })
      .end(done);
  });
});
