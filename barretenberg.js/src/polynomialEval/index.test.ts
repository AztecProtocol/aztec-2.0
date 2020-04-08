import { computeZeroPolyEval, computeLagrangeEval, computePublicInputEval, computeQuotient, computePartialOpening } from './index';

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
  beforeEach(() => {});

  it('should compute zero polynomial evaluation', () => {
    const zeroEval: any = computeZeroPolyEval(expectedZeta, circuitSize).toString(16);
    console.log({ zeroEval });
    const expectedZeroEval: string = '';
    // expect(zeroEval).toBe(expectedZeroEval);
  });

  it('should compute Lagrange polynomial evaluation', () => {
    const zeroEval: any = computeZeroPolyEval(expectedZeta, circuitSize);
    const lagrangeEval = computeLagrangeEval(zeroEval, expectedZeta, circuitSize).toString(16);
    console.log({ lagrangeEval });
    const expectedLagrangeEval: string = '';
    // expect(lagrangeEval.toBe(expectedLagrangeEval);
  });

  it.only('should compute public input polynomial evaluation', () => {
    // TODO: how to calculate all L_i lagrange evals?
    const publicInputEval: any = computePublicInputEval(publicInputs, lagrangeEvals).toString(16);
    const expectedPublicInputEval: string = '';
    // expect(publicInputEval).toBe(expectedPublicInputEval);
  });

  it('should compute quotient polynomial evaluation', () => {
    const zeroEval: any = computeZeroPolyEval(expectedZeta, circuitSize).toString(16);
    const publicInputEval: any = computePublicInputEval(publicInputs, lagrangeEvals).toString(16);
    const quotientEval = computeQuotient(publicInputEval, zeroEval, lagrangeEval, rBar, expectedBeta, expectedGamma, expectedAlpha, aBar, bBar, cBar, sigma1Bar, sigma2Bar, zwBar);
    const expectedQuotientEval: string = '';
    // expect(quotientEval).toBe(expectedQuotientEval);
  });

//   it('should compute partial opening commitment', () => {
//     const partialOpeningCommit = computePartialOpening();
//     const expectedPartialOpeningCommit: string = '';
//     expect(partialOpeningCommit).toBe(expectedPartialOpeningCommit);
//   });
});
