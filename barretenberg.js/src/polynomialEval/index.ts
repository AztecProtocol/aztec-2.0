import BN from 'bn.js';

import { toRedBN, createRedBN, redDivision, computeBracket } from './utils';

export function computeZeroPolyEval(zeta: any, circuitSize: any): BN {
  console.log({ circuitSize, zeta });
  return zeta.redPow(circuitSize).redSub(createRedBN(1));
}

export function computeLagrangeEval(zeroPolyEval: any, zeta: any, circuitSize: any): BN {
  const numerator = zeroPolyEval;
  const zetaRed = zeta;
  const denominator = circuitSize.redMul(zetaRed.redSub(createRedBN(1)));

  // TODO: check redDivision() function is doing what I want
  return redDivision(numerator, denominator);
}

export function computePublicInputEval(publicInputs: string[], lagrangeEvals: string[]): BN {
  const publicInputsBN: any = publicInputs.map((publicInput) => toRedBN(publicInput));
  const lagrangeEvalsBN: any = lagrangeEvals.map((lagrangeEval) => toRedBN(lagrangeEval));

  return publicInputsBN.reduce((accumulator, publicInput, index) => {
    const publicLagrangeProduct = publicInput.redMul(lagrangeEvalsBN[index]);
    return accumulator.redAdd(publicLagrangeProduct);
  });
}

export function computeQuotient(
  publicInputEval: any,
  zeroEval: any,
  lagrangeEval: any,
  rBar: any,
  beta: any,
  gamma: any,
  alpha: any,
  aBar: any,
  bBar: any,
  cBar: any,
  sigma1Bar: any,
  sigma2Bar: any,
  zwBar: any,
): BN {
  const firstAndSecondTerm: any = rBar.redAdd(publicInputEval);
  const thirdTerm: BN = computeBracket(aBar, beta, sigma1Bar, gamma)
    .redMul(computeBracket(bBar, beta, sigma2Bar, gamma))
    .redMul(cBar.redAdd(gamma))
    .redMul(zwBar)
    .redMul(alpha);

  const fourthTerm: BN = lagrangeEval.redMul(alpha).redPow(new BN(2));
  const numerator: BN = firstAndSecondTerm.redSub(thirdTerm).redSub(fourthTerm);
  const denominator: BN = zeroEval;
  return redDivision(numerator, denominator);
}

// TODO: need the selector polynomials
export function computePartialOpening(
  beta: any,
  gamma: any,
  alpha: any,
  aBar: any,
  bBar: any,
  cBar: any,
  sigma1Bar: any,
  sigma2Bar: any,
  zwBar: any,
  vChallenge0: any,
  qM: any,
  qL: any,
  qR: any,
  qO: any,
  qC: any,
  zeta: any,
  k1: any,
  k2: any,
  lagrangeEval: any,
  u: any,
  z: any,
  sigma3Bar: any,
): BN {
  const terms: any[] = [];
  const firstTerm: any = aBar.redMul(bBar).redMul(vChallenge0).redMul(qM);
  terms.push(firstTerm);

  const secondTerm: any = aBar.redMul(vChallenge0).redMul(qL);
  terms.push(secondTerm);

  const thirdTerm: any = bBar.redMul(vChallenge0).redMul(qR);
  terms.push(thirdTerm);

  const fourthTerm: any = cBar.redMul(vChallenge0).redMul(qO);
  terms.push(fourthTerm);

  const fifthTerm: any = vChallenge0.redMul(qO);
  terms.push(fifthTerm);

  const sixthTerm: any = computeBracket(aBar, beta, zeta, gamma)
    .redMul(computeBracket(bBar, beta, k1.redMul(zeta), gamma))
    .redMul(computeBracket(cBar, beta, k2.redMul(zeta), gamma))
    .redMul(alpha)
    .redMul(vChallenge0)
    .redMul(z);
  terms.push(sixthTerm);

  const seventhTerm: any = lagrangeEval
    .redMul(vChallenge0)
    .redMul(alpha.redPow(new BN(2)))
    .redMul(z);
  terms.push(seventhTerm);

  const eigthTerm: any = u.redMul(z);
  terms.push(eigthTerm);

  const ninthTerm: any = computeBracket(aBar, beta, sigma1Bar, gamma)
    .redMul(bBar, beta, sigma2Bar, gamma)
    .redMul(alpha)
    .redMul(vChallenge0)
    .redMul(beta)
    .redMul(zwBar)
    .redMul(sigma3Bar)
    .redNeg(); // the 9th term is negative
  terms.push(ninthTerm);

  // Sum all terms
  return terms.reduce((accumulator, currentTerm) => accumulator.redAdd(currentTerm));
}

export function computeBatchOpening(
  tlow: any,
  tmid: any,
  thigh: any,
  zeta: any,
  partialOpening: any,
  circuitSize: any,
  aCommit: any,
  bCommit: any,
  cCommit: any,
  vChallenge1: any,
  vChallenge2: any,
  vChallenge3: any,
  vChallenge4: any,
  vChallenge5: any,
  sigma1Bar: any,
  sigma2Bar: any,
): any {

  const terms: any[] = [];

  const firstTerm = tlow;
  terms.push(firstTerm);

  const secondTerm = (zeta.redPow(circuitSize)).redMul(tmid);
  terms.push(secondTerm);

  const thirdTerm = (zeta.redPow(circuitSize.mul(new BN(2)))).redMul(thigh)
  terms.push(thirdTerm);

  const fourthTerm = partialOpening;
  terms.push(fourthTerm);

  const fifthTerm = vChallenge1.redMul(aCommit);
  terms.push(fifthTerm);

  const sixthTerm = vChallenge2.redMul(bCommit);
  terms.push(sixthTerm);

  const seventhTerm = vChallenge3.redMul(cCommit);
  terms.push(seventhTerm);

  const eigthTerm = vChallenge4.redMul(sigma1Bar);
  terms.push(eigthTerm);

  const ninthTerm = vChallenge5.redMul(sigma2Bar);
  terms.push(ninthTerm);

  // Sum all terms
  return terms.reduce((accumulator, currentTerm) => accumulator.redAdd(currentTerm));
}

export function computeBatchEvaluation(
  tBar: any,
  rBar: any,
  vChallenge1: any,
  vChallenge2: any,
  vChallenge3: any,
  vChallenge4: any,
  aBar: any,
  bBar: any,
  cBar: any,
  sigma1Bar: any,
  sigma2Bar: any,
  u: any,
  z: any,
) {
  const terms: any[] = [];

  const firstTerm = tBar;
  terms.push(firstTerm);

  const secondTerm = vChallenge1.redMul(rBar);
  terms.push(secondTerm);

  const thirdTerm = vChallenge2.redMul(aBar);
  terms.push(thirdTerm);

  const fourthTerm = vChallenge3.redMul(bBar);
  terms.push(fourthTerm);

  const fifthTerm = vChallenge4.redMul(cBar);
  terms.push(fifthTerm);

  const sixthTerm = vChallenge4.redMul(sigma1Bar);
  terms.push(sixthTerm);

  const seventhTerm = vChallenge4.redMul(sigma2Bar);
  terms.push(seventhTerm);

  const eigthTerm = u.redMul(z);
  terms.push(eigthTerm);

    // Sum all terms
  return terms.reduce((accumulator, currentTerm) => accumulator.redAdd(currentTerm));
}
