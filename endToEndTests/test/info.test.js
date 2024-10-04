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
      .expect({ nBitmapsSize: 3931, sequenceCount: 100, totalSize: 24198036, numberOfPartitions: 10 });
  });

  it('should return detailed info about the current state of the database', { timeout: 5000 }, async () => {
    await server
      .get('/info?details=true')
      .expect(200)
      .expect('Content-Type', 'application/json')
      .expect(function (response) {
        const returnedInfo = response.body;

        expect(returnedInfo).to.have.property('bitmapContainerSizePerGenomeSection');

        expect(returnedInfo.bitmapContainerSizePerGenomeSection).to.have.property(
          'bitmapContainerSizeStatistic'
        );
        expect(returnedInfo.bitmapContainerSizePerGenomeSection.bitmapContainerSizeStatistic).to.deep.equal({
          numberOfArrayContainers: 48579,
          numberOfBitsetContainers: 0,
          numberOfRunContainers: 284,
          numberOfValuesStoredInArrayContainers: 66892,
          numberOfValuesStoredInBitsetContainers: 0,
          numberOfValuesStoredInRunContainers: 2875,
          totalBitmapSizeArrayContainers: 133784,
          totalBitmapSizeBitsetContainers: 0,
          totalBitmapSizeRunContainers: 4824,
        });

        expect(returnedInfo.bitmapContainerSizePerGenomeSection).to.have.property(
          'sizePerGenomeSymbolAndSection'
        );
        expect(
          returnedInfo.bitmapContainerSizePerGenomeSection.sizePerGenomeSymbolAndSection
        ).to.have.property('-');
        expect(returnedInfo.bitmapContainerSizePerGenomeSection.sizePerGenomeSymbolAndSection['-']).to.be.an(
          'array'
        );
        expect(
          returnedInfo.bitmapContainerSizePerGenomeSection.sizePerGenomeSymbolAndSection
        ).to.have.property('N');
        expect(returnedInfo.bitmapContainerSizePerGenomeSection.sizePerGenomeSymbolAndSection.N).to.be.an(
          'array'
        );
        expect(
          returnedInfo.bitmapContainerSizePerGenomeSection.sizePerGenomeSymbolAndSection
        ).to.have.property('NOT_N_NOT_GAP');
        expect(
          returnedInfo.bitmapContainerSizePerGenomeSection.sizePerGenomeSymbolAndSection.NOT_N_NOT_GAP
        ).to.be.an('array');

        expect(returnedInfo).to.have.property('bitmapSizePerSymbol');
        expect(returnedInfo.bitmapSizePerSymbol).to.deep.equal({
          '-': 2422869,
          'A': 2536840,
          'B': 2392240,
          'C': 2486656,
          'D': 2392240,
          'G': 2489072,
          'H': 2392240,
          'K': 2392370,
          'M': 2392330,
          'N': 2392240,
          'R': 2392290,
          'S': 2392240,
          'T': 2552937,
          'V': 2392240,
          'W': 2392290,
          'Y': 2392270,
        });
      })
      .expect(expectHeaderToHaveDataVersion);
  });
});
