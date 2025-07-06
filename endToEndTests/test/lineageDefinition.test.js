import { expectHeaderToHaveDataVersion, server } from './common.js';
import { describe, it } from 'node:test';
import { expect } from 'chai';

describe('The /lineageDefinition endpoint', () => {
  it('should return the lineage index', async () => {
    let searchString = `A: \{\}
A.1:
  parents:
  - A
A.11:
  parents:
  - A`;
    await server
      .get('/lineageDefinition/pango_lineage')
      .expect(200)
      .expect('Content-Type', 'application/yaml')
      .expect(expectHeaderToHaveDataVersion)
      .then(response => {
        expect(response.text).to.match(new RegExp(`^${searchString}`));
      });
  });

  it('should return an error for column name that is not present', async () => {
    await server
      .get('/lineageDefinition/columnThatIsNotPresent')
      .expect(400)
      .expect('Content-Type', 'application/json')
      .expect(expectHeaderToHaveDataVersion)
      .then(response => {
        expect(response.body.message).to.equal(
          'The column columnThatIsNotPresent does not exist in this instance.'
        );
      });
  });

  it('should return an error for column name that does not have a lineage index', async () => {
    await server
      .get('/lineageDefinition/region')
      .expect(400)
      .expect('Content-Type', 'application/json')
      .expect(expectHeaderToHaveDataVersion)
      .then(response => {
        expect(response.body.message).to.equal('The column region does not have a lineageIndex defined.');
      });
  });
});
