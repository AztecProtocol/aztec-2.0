import { BarretenbergWasm } from '../../src/wasm';
import { CreateProof, Note } from '../client_proofs/create';
import { Crs } from '../crs';
import { Schnorr } from '../crypto/schnorr/index';
import { Verifier } from '../../src/verifier';
import { dummyProofString, dummyProofBuffer } from './dummyProof';

describe('Verifier', () => {
  let barretenberg!: BarretenbergWasm;
  let createProof!: CreateProof;
  let schnorr!: Schnorr;

  beforeAll(async () => {
    barretenberg = new BarretenbergWasm();
    await barretenberg.init();

    const crs = new Crs(32768);
    await crs.download();

    schnorr = new Schnorr(barretenberg);
    createProof = new CreateProof(barretenberg);
    createProof.init(crs);
  });

  // won't work, current Barretenberg build is for Turbo Plonk - proof was generated with PLONK
  it.skip('should verify proof using Barretenberg', async () => {
    const isValid = createProof.verifyProof(dummyProofBuffer);
    console.log()
    expect(isValid).toBe(true);
  }, 60000);

  it('should extract proof components', async () => {
    const verifier: Verifier = new Verifier(dummyProof);
    const isValid = verifier.verifyProof();
    expect(isValid).toBe(true);
  }, 60000);


//   it('should verify proof using TypeScript verifier', async () => {
//     const verifier: Verifier = new Verifier(dummyProof);
//     const isValid = verifier.verifyProof();
//     expect(isValid).toBe(true);
//   }, 60000);

});
