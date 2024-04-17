import { headerToHaveDataVersion, server } from './common.js';
import { expect } from 'chai';
import { describe, it } from 'node:test';
import fs from 'fs';
import path from 'path';
import { dirname } from 'node:path';
import { fileURLToPath } from 'node:url';

const __dirname = dirname(fileURLToPath(import.meta.url));

function readFilesRecursively(directoryPath) {
  let fileList = [];

  const files = fs.readdirSync(directoryPath);

  files.forEach(file => {
    const filePath = path.join(directoryPath, file);

    if (fs.statSync(filePath).isDirectory()) {
      fileList = fileList.concat(readFilesRecursively(filePath));
    } else if (filePath.endsWith('.json')) {
      fileList.push(filePath);
    }
  });

  return fileList;
}

const queryTestFiles = readFilesRecursively(__dirname + '/queries');
const invalidQueriesPath = __dirname + '/invalidQueries';
const invalidQueryTestFiles = fs.readdirSync(invalidQueriesPath);

describe('The /query endpoint', () => {
  const testCases = queryTestFiles.map(file => {
    const fileContent = fs.readFileSync(file, 'utf8');
    const testCase = JSON.parse(fileContent);
    testCase.fileName = path.basename(file);
    return testCase;
  });

  testCases.forEach(testCase =>
    it('should return data for the test case ' + testCase.testCaseName, async () => {
      const response = await server
        .post('/query')
        .send(testCase.query)
        .expect(200)
        .expect('Content-Type', 'application/x-ndjson')
        .expect(headerToHaveDataVersion);

      let responseLines = response.text
        .split(/\n/)
        .filter(it => it !== '')
        .map(it => JSON.parse(it));

      const errorMessage = 'Actual result is:\n' + response.text + '\n';
      expect(responseLines, errorMessage).to.deep.equal(testCase.expectedQueryResult);
    })
  );

  it('test cases should have unique names', () => {
    const testCaseNames = testCases.map(testCase => testCase.testCaseName);
    const uniqueTestCaseNames = [...new Set(testCaseNames)];

    expect(testCaseNames, 'Found non-unique test case names').to.deep.equal(uniqueTestCaseNames);
  });

  const invalidQueryTestCases = invalidQueryTestFiles.map(file =>
    JSON.parse(fs.readFileSync(`${invalidQueriesPath}/${file}`))
  );
  invalidQueryTestCases.forEach(testCase =>
    it('should return the expected error for the test case ' + testCase.testCaseName, async () => {
      const response = await server
        .post('/query')
        .send(testCase.query)
        .expect(400)
        .expect('Content-Type', 'application/json');
      return expect(response.body).to.deep.equal(testCase.expectedError);
    })
  );

  it('invalid query test cases should have unique names', () => {
    const testCaseNames = invalidQueryTestCases.map(testCase => testCase.testCaseName);
    const uniqueTestCaseNames = [...new Set(testCaseNames)];

    expect(testCaseNames, 'Found non-unique invalid query test case names').to.deep.equal(
      uniqueTestCaseNames
    );
  });

  it('should return a method not allowed response when sending a GET request', async () => {
    await server.get('/query').send().expect(405).expect('Content-Type', 'application/json').expect({
      error: 'Method not allowed',
      message: 'GET is not allowed on resource /query',
    });
  });

  it('should return a bad request response when POSTing an invalid JSON', async () => {
    await server
      .post('/query')
      .send('{ not a valid json')
      .expect(400)
      .expect('Content-Type', 'application/json')
      .expect({
        error: 'Bad request',
        message:
          'The query was not a valid JSON: [json.exception.parse_error.101] ' +
          'parse error at line 1, column 4: syntax error while parsing object key - invalid literal; ' +
          "last read: '{ no'; expected string literal",
      });
  });

  it('should return a bad request response when POSTing a JSON without filter and action', async () => {
    await server
      .post('/query')
      .send({ someJson: 'but missing expected properties' })
      .expect(400)
      .expect('Content-Type', 'application/json')
      .expect({
        error: 'Bad request',
        message: 'Query json must contain filterExpression and action.',
      });
  });

  it('should return a bad request response when POSTing an invalid filter type', async () => {
    await server
      .post('/query')
      .send({
        action: {
          type: 'invalid action',
        },
        filterExpression: {
          type: 'invalid filter type',
        },
      })
      .expect(400)
      .expect('Content-Type', 'application/json')
      .expect({
        error: 'Bad request',
        message: "Unknown object filter type 'invalid filter type'",
      });
  });
});
