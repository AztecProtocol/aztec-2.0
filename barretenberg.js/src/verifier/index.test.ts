import { BarretenbergWasm } from '../wasm';
import { Schnorr } from '../crypto/schnorr/index';
import { Verifier } from './index';
import { dummyProof, proofVariables } from '../utils/dummyProof';

// TODO: update when have complete verifier algo
describe('Verifier', () => {
  let barretenberg!: BarretenbergWasm;
  let schnorr!: Schnorr;
  let verifier!: Verifier;
  const proofData: string = dummyProof
  const g1Size = 128;
  const fieldElementSize = 64;

  beforeAll(async () => {
    const verificationKey: any = null;
    verifier = new Verifier(proofData, verificationKey);
  });


  describe('Success states', () => {
    it.only('should extract proof components', async () => {
        console.log({ proofData });

        const { data, G1Points, fieldElements } = verifier.decodeProof(proofData);

        const proofDataLength = proofData.length;
9
      });
  });
});
