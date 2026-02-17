import { expectHeaderToHaveDataVersion, server } from './common.js';
import { expect } from 'chai';
import { describe, it } from 'node:test';
import fs from 'fs';
import path from 'path';
import { dirname } from 'node:path';
import { fileURLToPath } from 'node:url';
import { tableFromIPC } from 'apache-arrow';

const __dirname = dirname(fileURLToPath(import.meta.url));

const ARROW_MIME = 'application/vnd.apache.arrow.stream';

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

function arrowTableToObjects(table) {
  const decoder = new TextDecoder();
  const rows = [];
  for (let i = 0; i < table.numRows; i++) {
    const obj = {};
    for (const field of table.schema.fields) {
      const column = table.getChild(field.name);
      const value = column.get(i);
      if (value === null) {
        obj[field.name] = null;
      } else if (typeof value === 'bigint') {
        obj[field.name] = Number(value);
      } else if (value instanceof Uint8Array) {
        obj[field.name] = decoder.decode(value);
      } else {
        obj[field.name] = value;
      }
    }
    rows.push(obj);
  }
  return rows;
}

function parseNdjsonResponse(response) {
  return response.text
    .split(/\n/)
    .filter(it => it !== '')
    .map(it => JSON.parse(it));
}

const formats = [
  {
    name: 'NDJSON',
    request: query => server.post('/query').send(query),
    expectedContentType: 'application/x-ndjson',
    parseResponse: response => parseNdjsonResponse(response),
  },
  {
    name: 'Arrow IPC',
    request: query => server.post('/query').set('Accept', ARROW_MIME).send(query).responseType('blob'),
    expectedContentType: ARROW_MIME,
    parseResponse: response => arrowTableToObjects(tableFromIPC(response.body)),
  },
];

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

  for (const format of formats) {
    describe(`[${format.name}]`, () => {
      testCases.forEach(testCase =>
        it('should return data for the test case ' + testCase.testCaseName, async () => {
          const response = await format.request(testCase.query);

          const errorMessage =
            `[${format.name}] Actual result is:\n` +
            (format.name === 'NDJSON' ? response.text : '<binary Arrow IPC>') +
            '\n';
          expect(response.status, errorMessage).to.equal(200);
          expect(response.header['content-type'], errorMessage).to.equal(format.expectedContentType);
          expectHeaderToHaveDataVersion(response);
          const rows = format.parseResponse(response);
          expect(rows, errorMessage).to.deep.equal(testCase.expectedQueryResult);
        })
      );
    });
  }

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
      const response = await server.post('/query').send(testCase.query);

      const errorMessage = 'Actual result is:\n' + response.text + '\n';
      expect(response.status, errorMessage).to.equal(400);
      expect(response.header['content-type'], errorMessage).to.equal('application/json');
      return expect(response.body, errorMessage).to.deep.equal(testCase.expectedError);
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
