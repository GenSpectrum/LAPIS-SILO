const { server } = require('./common');
const { expect } = require('chai');

describe('The /info endpoint', () => {
  it('should return info about the current state of the database', done => {
    server
      .get('/info')
      .expect(200)
      .expect('Content-Type', 'application/json')
      .expect({ nBitmapsSize: 3898, sequenceCount: 100, totalSize: 68915226 })
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
          numberOfArrayContainers: 745081,
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
          '-': 6019718,
          'A': 9498982,
          'B': 5980600,
          'C': 8141784,
          'D': 5980600,
          'G': 8291636,
          'H': 5980600,
          'K': 5980730,
          'M': 5980690,
          'N': 5980600,
          'R': 5980650,
          'S': 5980600,
          'T': 9770332,
          'V': 5980600,
          'W': 5980650,
          'Y': 5980630,
        });
      })
      .end(done);
  });
});
