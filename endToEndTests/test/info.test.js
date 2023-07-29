const { server } = require('./common');
const { expect } = require('chai');

describe('The /info endpoint', () => {
  it('should return info about the current state of the database', done => {
    server
      .get('/info')
      .expect(200)
      .expect('Content-Type', 'application/json')
      .expect({ nBitmapsSize: 3898, sequenceCount: 100, totalSize: 60074145 })
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
          numberOfArrayContainers: 47970,
          numberOfBitsetContainers: 209,
          numberOfRunContainers: 209,
          numberOfValuesStoredInArrayContainers: 64283,
          numberOfValuesStoredInBitsetContainers: 0,
          numberOfValuesStoredInRunContainers: 2410,
          totalBitmapSizeArrayContainers: 128566,
          totalBitmapSizeBitsetContainers: 0,
          totalBitmapSizeRunContainers: 3538,
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
          '-': 6003470,
          'A': 6127203,
          'B': 5980600,
          'C': 6073069,
          'D': 5980600,
          'G': 6075909,
          'H': 5980600,
          'K': 5980630,
          'M': 5980620,
          'N': 5980600,
          'R': 5980620,
          'S': 5980600,
          'T': 6139332,
          'V': 5980600,
          'W': 5980600,
          'Y': 5980620,
        });
      })
      .end(done);
  });
});
