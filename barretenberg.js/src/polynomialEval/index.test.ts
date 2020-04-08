import * as bn128 from '@aztec/bn128';
import {
  computeZeroPolyEval,
  computeLagrangeEval,
  computePublicInputEval,
  computeQuotient,
  computePartialOpening,
  computeBatchOpening,
  computeBatchEvaluation,
} from './index';
import { toRedBN } from './utils';
import { expectedChallenges, expectedVChallenges, proofVariables } from '../utils/dummyProof';

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
const { expectedInit, expectedBeta, expectedGamma, expectedAlpha, expectedZeta } = expectedChallenges;

describe('polynomialEval', () => {
  const qM = '0x1';
  const qL = '0x2';
  const qR = '0x3';
  const qO = '0x4';
  const qC = '0x5';
  const vChallenge0 = expectedVChallenges[0];
  const k1 = '0x7';
  const k2 = '0x8';
  const u = '0x10';
  const z = '0x11';
  const sigma3Bar = '0x12';

  it('should compute zero polynomial evaluation', () => {
    const zeroEval: any = computeZeroPolyEval(toRedBN(expectedZeta), circuitSize).toString(16);
    // const expectedZeroEval: string = '';
    // expect(zeroEval).toBe(expectedZeroEval);
  });

  it('should compute Lagrange polynomial evaluation', () => {
    const zeroEval: any = computeZeroPolyEval(toRedBN(expectedZeta), circuitSize);
    const lagrangeEval = computeLagrangeEval(
      zeroEval,
      toRedBN(expectedZeta),
      circuitSize.toRed(bn128.groupReduction),
    ).toString(16);
    // const expectedLagrangeEval: string = '';
    // expect(lagrangeEval.toBe(expectedLagrangeEval);
  });

  it('should compute public input polynomial evaluation', () => {
    // TODO: Calculate lagrange evals correctly
    const dummyLagrangeEvals: string[] = ['0x5'];
    const publicInputEval: any = computePublicInputEval(publicInputs, dummyLagrangeEvals).toString(16);
    // const expectedPublicInputEval: string = '';
    // expect(publicInputEval).toBe(expectedPublicInputEval);
  });

  it('should compute quotient polynomial evaluation', () => {
    const zeroEval: any = computeZeroPolyEval(toRedBN(expectedZeta), circuitSize);

    const dummyLagrangeEvals: string[] = ['0x5'];
    const publicInputEval: any = computePublicInputEval(publicInputs, dummyLagrangeEvals);

    const lagrangeEval = computeLagrangeEval(zeroEval, toRedBN(expectedZeta), circuitSize.toRed(bn128.groupReduction));

    const quotientEval: any = computeQuotient(
      publicInputEval,
      zeroEval,
      lagrangeEval,
      toRedBN(rBar),
      toRedBN(expectedBeta),
      toRedBN(expectedGamma),
      toRedBN(expectedAlpha),
      toRedBN(aBar),
      toRedBN(bBar),
      toRedBN(cBar),
      toRedBN(sigma1Bar),
      toRedBN(sigma2Bar),
      toRedBN(zwBar),
    ).toString(16);
    // const expectedQuotientEval: string = '';
    // expect(quotientEval).toBe(expectedQuotientEval);
  });

  it('should compute partial opening commitment', () => {
    const zeroEval: any = computeZeroPolyEval(toRedBN(expectedZeta), circuitSize);
    const lagrangeEval: any = computeLagrangeEval(
      zeroEval,
      toRedBN(expectedZeta),
      circuitSize.toRed(bn128.groupReduction),
    );
    const partialOpeningCommit: any = computePartialOpening(
      toRedBN(expectedBeta),
      toRedBN(expectedGamma),
      toRedBN(expectedAlpha),
      toRedBN(aBar),
      toRedBN(bBar),
      toRedBN(cBar),
      toRedBN(sigma1Bar),
      toRedBN(sigma2Bar),
      toRedBN(zwBar),
      toRedBN(vChallenge0),
      toRedBN(qM),
      toRedBN(qL),
      toRedBN(qR),
      toRedBN(qO),
      toRedBN(qC),
      toRedBN(expectedZeta),
      toRedBN(k1),
      toRedBN(k2),
      toRedBN(lagrangeEval),
      toRedBN(u),
      toRedBN(z),
      toRedBN(sigma3Bar),
    ).toString(16);
    console.log({ partialOpeningCommit });
    //   const expectedPartialOpeningCommit: string = '';
    //   expect(partialOpeningCommit).toBe(expectedPartialOpeningCommit);
  });

  it('should compute batch opening commitment', () => {
    const zeroEval: any = computeZeroPolyEval(toRedBN(expectedZeta), circuitSize);
    const lagrangeEval: any = computeLagrangeEval(
      zeroEval,
      toRedBN(expectedZeta),
      circuitSize.toRed(bn128.groupReduction),
    );
    const partialOpeningCommit: any = computePartialOpening(
      toRedBN(expectedBeta),
      toRedBN(expectedGamma),
      toRedBN(expectedAlpha),
      toRedBN(aBar),
      toRedBN(bBar),
      toRedBN(cBar),
      toRedBN(sigma1Bar),
      toRedBN(sigma2Bar),
      toRedBN(zwBar),
      toRedBN(vChallenge0),
      toRedBN(qM),
      toRedBN(qL),
      toRedBN(qR),
      toRedBN(qO),
      toRedBN(qC),
      toRedBN(expectedZeta),
      toRedBN(k1),
      toRedBN(k2),
      toRedBN(lagrangeEval),
      toRedBN(u),
      toRedBN(z),
      toRedBN(sigma3Bar),
    );
    const batchOpeningCommit: any = computeBatchOpening(
      toRedBN(tlowCommit),
      toRedBN(tmidCommit),
      toRedBN(thighCommit),
      toRedBN(expectedZeta),
      partialOpeningCommit,
      circuitSize,
      toRedBN(aCommit),
      toRedBN(bCommit),
      toRedBN(cCommit),
      toRedBN(expectedVChallenges[1]),
      toRedBN(expectedVChallenges[2]),
      toRedBN(expectedVChallenges[3]),
      toRedBN(expectedVChallenges[4]),
      toRedBN(expectedVChallenges[5]),
      toRedBN(sigma1Bar),
      toRedBN(sigma2Bar),
    );
    // const expectedBatchOpening: string = '';
    // expect(batchOpeningCommit).toBe(expectedBatchOpeningCommit);
  });

  it('should compute batch evaluation commitment', () => {
    const batchEvaluationCommit: any = computeBatchEvaluation(
        toRedBN(t), // TODO: check
        toRedBN(rBar),
        toRedBN(expectedVChallenges[0]),
        toRedBN(expectedVChallenges[1]),
        toRedBN(expectedVChallenges[2]),
        toRedBN(expectedVChallenges[3]),
        toRedBN(aBar),
        toRedBN(bBar),
        toRedBN(cBar),
        toRedBN(sigma1Bar),
        toRedBN(sigma2Bar),
        toRedBN(u),
        toRedBN(z),
    );
    // const expectedBatchEvaluationCommit: string = '';
    // expect(batchEvaluationCommit).toBe(expectedBatchEvaluationCommit);
  });
});
