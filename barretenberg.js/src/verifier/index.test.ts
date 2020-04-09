import { BarretenbergWasm } from '../wasm';
import { Crs } from '../crs';
import { Schnorr } from '../crypto/schnorr/index';
import { fetchCode } from '../wasm';
import { destroyWorker, createWorker, } from '../wasm/worker_factory';
import { Verifier } from './index';
import { dummyProof, dummyProofBuffer } from '../utils/dummyProof';

// TODO: update when have complete verifier algo
describe('Verifier', () => {
  let barretenberg!: BarretenbergWasm;
  let schnorr!: Schnorr;
  let verifier!: Verifier;
  const proofData: string = dummyProof

  beforeAll(async () => {
    // barretenberg = new BarretenbergWasm();

    // const code = await fetchCode();

    // barretenberg = await createWorker();
    // await barretenberg.init(code);

    // const crs = new Crs(32768);
    // await crs.download();

    // schnorr = new Schnorr(barretenberg);
    // createProof = new CreateProof(barretenberg);
    // createProof.init(crs);

    const verificationKey: any = null;
    verifier = new Verifier(proofData, verificationKey);
  });

//   it.skip('should verify proof using Barretenberg', async () => {
//     const isValid = createProof.verifyProof(dummyProofBuffer);
//     console.log()
//     expect(isValid).toBe(true);
//   }, 60000);

  describe('Success states', () => {
    it.only('should extract proof components', async () => {
        const { data, G1Points, fieldElements } = verifier.decodeProof(proofData);
        // expect(isValid).toBe(true);
      });
    
      it.skip('should compute challenges', async () => {
    
      });
  });

  describe('Failure states', () => {
    it('should fail if point not on curve', () => {});
  });


});
