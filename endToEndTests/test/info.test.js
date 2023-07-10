const { server } = require('./common');
const { expect } = require('chai');

describe('The /info endpoint', () => {
  it('should return info about the current state of the database', done => {
    server
      .get('/info')
      .expect(200)
      .expect('Content-Type', 'application/json')
      .expect({ nBitmapsSize: 3898, sequenceCount: 100, totalSize: 66467326 })
      .end(done);
  });

  it('should return detailed info about the current state of the database', function (done) {
    this.timeout(5000);
    server
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
          numberOfArrayContainers: 716007,
          numberOfBitsetContainers: 0,
          numberOfRunContainers: 0,
          numberOfValuesStoredInArrayContainers: 2929577,
          numberOfValuesStoredInBitsetContainers: 0,
          numberOfValuesStoredInRunContainers: 0,
          totalBitmapSizeArrayContainers: 5859154,
          totalBitmapSizeBitsetContainers: 0,
          totalBitmapSizeRunContainers: 0,
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
          '-': 5779958,
          'A': 9190510,
          'B': 5741376,
          'C': 7859992,
          'D': 5741376,
          'G': 8006876,
          'H': 5741376,
          'K': 5741498,
          'M': 5741466,
          'N': 5741376,
          'R': 5741426,
          'S': 5741376,
          'T': 9456412,
          'V': 5741376,
          'W': 5741426,
          'Y': 5741406,
        });
      })
      .end(done);
  });
});
