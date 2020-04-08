import * as bn128 from '@aztec/bn128';
import BN from 'bn.js';
import { toBN } from 'web3-utils';

import { toRedBN, createRedBN, redDivision, computeBracket } from './utils';

export function computeZeroPolyEval(zeta: any, circuitSize: any): BN {
  const zetaRed = toBN(zeta).toRed(bn128.groupReduction);
  circuitSize = toBN(circuitSize);

  return zetaRed.redPow(circuitSize).redSub(createRedBN(1));
}

export function computeLagrangeEval(zeroPolyEval: any, zeta: any, circuitSize: any): BN {
  const numerator = toRedBN(zeroPolyEval);
  const zetaRed = toRedBN(zeta);
  const circuitSizeRed = toRedBN(circuitSize);
  const denominator = circuitSizeRed.redMul(zetaRed.redSub(createRedBN(1)));
  return redDivision(numerator, denominator);
}

export function computePublicInputEval(publicInputs: string[], lagrangeEvals: string[]): BN {
  const publicInputsBN: any = publicInputs.map((publicInput) => toRedBN(publicInput));
  const lagrangeEvalsBN: any = lagrangeEvals.map((lagrangeEval) => toRedBN(lagrangeEval));

  return publicInputsBN.reduce((accumulator, publicInput, index) => {
    const publicLagrangeProduct = publicInput.redMul(lagrangeEvalsBN[index]);
    return accumulator.redAdd(publicLagrangeProduct).fromRed();
  });
}

export function computeQuotient(
  publicInputEval: string,
  zeroEval: string,
  lagrangeEval: string,
  rBar: string,
  beta: string,
  gamma: string,
  alpha: string,
  aBar: string,
  bBar: string,
  cBar: string,
  sigma1Bar: string,
  sigma2Bar: string,
  zwBar: string,
): BN {
  const firstAndSecondElement: BN = toRedBN(rBar).redAdd(toRedBN(publicInputEval));
  const thirdElement: BN = computeBracket(aBar, beta, sigma1Bar, gamma)
    .redMul(computeBracket(bBar, beta, sigma2Bar, gamma))
    .redMul((toRedBN(cBar).redAdd(toRedBN(gamma))).redMul(toRedBN(zwBar)))
    .redMul(toRedBN(alpha));

  const fourthElement: BN = toRedBN(lagrangeEval).redMul(toRedBN(alpha).redPow(new BN(2)));
  const numerator: BN = firstAndSecondElement.redSub(thirdElement).redSub(fourthElement);
  const denominator: BN = toRedBN(zeroEval);
  return redDivision(numerator, denominator);
}

export function computePartialOpening() {}

export function computeBatchOpening() {}

export function computeBatchEvaluation() {}
