import { computeZeroPolyEval, computeLagrangeEval, computePublicInputEval, computeQuotient } from '../polynomialEval';
import BaseVerifier from './BaseVerifier';
import Transcript from '../transcript';
import BN from 'bn.js';

export class Verifier extends BaseVerifier {
  public data!: any;
  public G1Points!: any[];
  public fieldElements!: any[];
  public publicInputs!: any[];

  constructor(proofData: string, verificationKey: string) {
    super(verificationKey);

    const { data, G1Points, fieldElements, publicInputs } = this.decodeProof(proofData);
    this.data = data;
    this.G1Points = G1Points;
    this.fieldElements = fieldElements;
    this.publicInputs = publicInputs;

    this.decodeVerificationKey();
    this.validateInputs(this.G1Points, this.fieldElements, this.publicInputs);
    this.computeChallenges();
  }

  /**
   * Compute beta, gamma, alpha, zeta, v, u challenges
   * Need the hex representation of vars for this function
   */
  public computeChallenges() {
    const transcript = new Transcript();
    const initChallenge: string = transcript.preamble(this.circuitSize, this.numPublicInputs);
    const { beta, gamma } = transcript.secondRound(
      this.publicInputs,
      this.data.aCommit.hex,
      this.data.bCommit.hex,
      this.data.cCommit.hex,
    );
    const alpha: string = transcript.thirdRound(this.data.zCommit.hex);
    const zeta: string = transcript.fourthRound(
      this.data.tlowCommit.hex,
      this.data.tmidCommit.hex,
      this.data.thighCommit.hex,
    );

    // need to find the data below from somewhere
    const vChallenges: string[] = transcript.fifthRound(
      this.data.aEval.hex,
      this.data.bEval.hex,
      this.data.cEval.hex,
      this.data.W_w.hex,
      this.data.zwEval.hex, // TODO: check
      this.data.s1Eval.hex,
      this.data.s2Eval.hex,
      this.data.rEval.hex,
      this.data.W.hex, // TODO: check
    );

    super.convertChallengesToBN(initChallenge, beta, gamma, alpha, zeta, vChallenges);
  }

  /**
   * Compute the evaluations of the necessary polynomials, required for the verification
   * equation
   */
  public computePolynomialEvaluations() {
    const zeroEval = computeZeroPolyEval(this.zeta, this.circuitSize);
    const lagrangeEval = computeLagrangeEval(zeroEval, this.zeta, this.circuitSize);
    // const publicInputEval = computePublicInputEval();
    // const quotientEval = computeQuotient()


  }

  public verifyProof() {}
}
