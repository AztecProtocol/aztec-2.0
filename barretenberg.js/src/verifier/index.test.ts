import { BarretenbergWasm } from '../../src/wasm';
import { CreateProof, Note } from '../client_proofs/create';
import { Crs } from '../crs';
import { Schnorr } from '../crypto/schnorr/index';
import { Verifier } from '../../src/verifier';
import { dummyProofString, dummyProofBuffer } from '../utils/dummyProof';

describe('Verifier', () => {
  let barretenberg!: BarretenbergWasm;
  let createProof!: CreateProof;
  let schnorr!: Schnorr;
  let verifier!: Verifier;
  const proofData: string = dummyProofString

  beforeAll(async () => {
    // barretenberg = new BarretenbergWasm();
    // await barretenberg.init();

    // const crs = new Crs(32768);
    // await crs.download();

    // schnorr = new Schnorr(barretenberg);
    // createProof = new CreateProof(barretenberg);
    // createProof.init(crs);

    verifier = new Verifier(proofData);
  });

  // won't work, current Barretenberg build is for Turbo Plonk - proof was generated with PLONK
//   it.skip('should verify proof using Barretenberg', async () => {
//     const isValid = createProof.verifyProof(dummyProofBuffer);
//     console.log()
//     expect(isValid).toBe(true);
//   }, 60000);

  describe('Success states', () => {
    it.only('should extract proof components', async () => {
        const { data, G1Points, fieldElements } = verifier.decodeProof(proofData);
        console.log({ data });
        // expect(isValid).toBe(true);
      });
    
      it.skip('should compute challenges', async () => {
    
      });
  });

  describe('Failure states', () => {
    it('should fail if point not on curve', () => {});
  });


});
