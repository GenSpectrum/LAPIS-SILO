import { expect } from 'chai';
import { headerToHaveDataVersion, server } from './common.js';
import { describe, it } from 'node:test';

describe('The /info endpoint', () => {
  it('should return info about the current state of the database', async () => {
    await server
      .get('/info')
      .expect(200)
      .expect('Content-Type', 'application/json')
      .expect(headerToHaveDataVersion)
      .expect({ nBitmapsSize: 3898, sequenceCount: 100, totalSize: 26589431 });
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
          numberOfArrayContainers: 48613,
          numberOfBitsetContainers: 0,
          numberOfRunContainers: 195,
          numberOfValuesStoredInArrayContainers: 66796,
          numberOfValuesStoredInBitsetContainers: 0,
          numberOfValuesStoredInRunContainers: 2699,
          totalBitmapSizeArrayContainers: 133592,
          totalBitmapSizeBitsetContainers: 0,
          totalBitmapSizeRunContainers: 4294,
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
          '-': 2662131,
          'A': 2775955,
          'B': 2631464,
          'C': 2725758,
          'D': 2631464,
          'G': 2728138,
          'H': 2631464,
          'K': 2631594,
          'M': 2631554,
          'N': 2631464,
          'R': 2631514,
          'S': 2631464,
          'T': 2791973,
          'V': 2631464,
          'W': 2631514,
          'Y': 2631494,
        });
      })
      .expect(headerToHaveDataVersion);
  });
});
