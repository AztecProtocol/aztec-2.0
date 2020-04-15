import { padLeft } from 'web3-utils'
import serializeProof from './index';
import { dummyProof, proofVariables, expectedChallenges, dummyProofBuffer } from '../utils/dummyProof';


const {
    circuitSize,
    numPublicInputs,
    publicInputs,
    aCommit,
    bCommit,
    cCommit,
    zCommit,
    tlowCommit,
    tmidCommit,
    thighCommit,
    aBar,
    bBar,
    cBar,
    WzBar,
    zwBar,
    sigma1Bar,
    sigma2Bar,
    rBar,
    t,
  } = proofVariables;


describe('serialize', () => {
    const numG1Elements: number = 9;
    const numFieldElements: number = 8;

    it('should serialize a proof', () => {
        const { numPublicInputs, publicInputs, serializedProofData } = serializeProof(dummyProof);

        const serializedLength = serializedProofData.length;
        expect(serializedLength).toBe(numG1Elements * 2 + numFieldElements); // multiple of 2 for x and y coords
        expect(parseInt(numPublicInputs)).toBe(1);
        expect(publicInputs.length).toBe(1)

        // public inputs
        expect(publicInputs[0]).toBe(padLeft(circuitSize.toString(16), 64));

        // serialized data
        expect(serializedProofData[serializedLength - 1]).toBe(rBar.slice(2));
        expect(serializedProofData[serializedLength - 2]).toBe(sigma2Bar.slice(2));
        expect(serializedProofData[serializedLength - 3]).toBe(sigma1Bar.slice(2));
        expect(serializedProofData[serializedLength - 4]).toBe(zwBar.slice(2));
        expect(serializedProofData[serializedLength - 5]).toBe(WzBar.slice(2));
        expect(serializedProofData[serializedLength - 6]).toBe(cBar.slice(2));
        expect(serializedProofData[serializedLength - 7]).toBe(bBar.slice(2));
        expect(serializedProofData[serializedLength - 8]).toBe(aBar.slice(2));
        // expect(serializedProofData[serializedLength - 9]).toBe(W_w.slice(2).slice(0, 64));
        // expect(serializedProofData[serializedLength - 10]).toBe(W_w.slice(2).slice(64, 128));
        // expect(serializedProofData[serializedLength - 11]).toBe(W.slice(2).slice(0, 64));
        // expect(serializedProofData[serializedLength - 12]).toBe(W.slice(2).slice(64, 128));
        expect(serializedProofData[serializedLength - 13]).toBe(thighCommit.slice(2).slice(64, 128));
        expect(serializedProofData[serializedLength - 14]).toBe(thighCommit.slice(2).slice(0, 64));
        expect(serializedProofData[serializedLength - 15]).toBe(tmidCommit.slice(2).slice(64, 128));
        expect(serializedProofData[serializedLength - 16]).toBe(tmidCommit.slice(2).slice(0, 64));
        expect(serializedProofData[serializedLength - 17]).toBe(tlowCommit.slice(2).slice(64, 128));
        expect(serializedProofData[serializedLength - 18]).toBe(tlowCommit.slice(2).slice(0, 64));
        expect(serializedProofData[serializedLength - 19]).toBe(zCommit.slice(2).slice(64, 128));
        expect(serializedProofData[serializedLength - 20]).toBe(zCommit.slice(2).slice(0, 64));
        expect(serializedProofData[serializedLength - 21]).toBe(cCommit.slice(2).slice(64, 128));
        expect(serializedProofData[serializedLength - 22]).toBe(cCommit.slice(2).slice(0, 64));
        expect(serializedProofData[serializedLength - 23]).toBe(bCommit.slice(2).slice(64, 128));
        expect(serializedProofData[serializedLength - 24]).toBe(bCommit.slice(2).slice(0, 64));
        expect(serializedProofData[serializedLength - 25]).toBe(aCommit.slice(2).slice(64, 128));
        expect(serializedProofData[serializedLength - 26]).toBe(aCommit.slice(2).slice(0, 64));
    })
});

