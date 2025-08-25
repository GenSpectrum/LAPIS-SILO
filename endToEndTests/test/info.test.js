import { expect } from 'chai';
import { expectHeaderToHaveDataVersion, server } from './common.js';
import { describe, it } from 'node:test';

describe('The /info endpoint', () => {
  it('should return info about the current state of the database', async () => {
    await server
      .get('/info')
      .expect(200)
      .expect('Content-Type', 'application/json')
      .expect(expectHeaderToHaveDataVersion)
      .expect(response => {
        const returnedInfo = response.body;

        expect(returnedInfo).to.have.property('version').and.match(/.+/);

        const { version, ...infoWithoutVersion } = returnedInfo;
        expect(infoWithoutVersion).to.deep.equal({
          sequenceCount: 100,
          horizontalBitmapsSize: 5899,
          verticalBitmapsSize: 32016,
          numberOfPartitions: 1,
        });
      });
  });
});
