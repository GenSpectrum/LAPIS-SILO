const { server } = require('./common');
const fs = require('fs');

const queriesPath = __dirname + '/queries';
const queryTestFiles = fs.readdirSync(queriesPath);

describe('The /query endpoint', () => {
  queryTestFiles
    .map(file => JSON.parse(fs.readFileSync(`${queriesPath}/${file}`)))
    .forEach(testCase =>
      it('should return data for the test case ' + testCase.testCaseName, done => {
        server
          .post('/query')
          .send(testCase.query)
          .expect(200)
          .expect('Content-Type', 'application/json')
          .expect(testCase.expectedResponse)
          .end(done);
      })
    );

  it('should return a bad request response when POSTing an invalid JSON', done => {
    server
      .post('/query')
      .send('{ not a valid json')
      .expect(400)
      .expect('Content-Type', 'application/json')
      .expect({
        error: 'Bad request',
        message: 'The query was not a valid JSON: IsObject()',
      })
      .end(done);
  });

  it('should return a bad request response when POSTing a JSON without filter and action', done => {
    server
      .post('/query')
      .send({ someJson: 'but missing expected properties' })
      .expect(400)
      .expect('Content-Type', 'application/json')
      .expect({
        error: 'Bad request',
        message: 'Query json must contain filter and action.',
      })
      .end(done);
  });
});
