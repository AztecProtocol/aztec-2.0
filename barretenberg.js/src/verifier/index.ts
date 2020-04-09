import {
  computeZeroPolyEval,
  computeLagrangeEval,
  computePublicInputEval,
  computeQuotient,
  computePartialOpening,
  computeBatchOpening,
  computeBatchEvaluation,
} from '../polynomialEval';
import BaseVerifier from './BaseVerifier';
import Transcript from '../transcript';
import BN from 'bn.js';

export class Verifier extends BaseVerifier {
  public data!: any;
  public G1Points!: any[];
  public fieldElements!: any[];
  public publicInputs!: any[];
  public zeroEval!: BN;
  public lagrangeEval!: BN;
  public publicInputEval!: any;
  public quotientEval!: any;
  public partialOpening!: any;
  public batchOpening!: any;
  public batchEvaluation!: any;

  constructor(proofData: string, verificationKey: string) {
    super(verificationKey);

    const { data, G1Points, fieldElements, publicInputs } = this.decodeProof(proofData);
    this.data = data;
    this.G1Points = G1Points;
    this.fieldElements = fieldElements;
    this.publicInputs = publicInputs;

    this.decodeVerificationKey();
    this.extractPreProcessedInput();
    this.validateInputs(this.G1Points, this.fieldElements, this.publicInputs);
    this.computeChallenges();
    this.computePolynomialEvaluations();
  }

  /**
   * Compute beta, gamma, alpha, zeta, v challenges
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

    const vChallenges: string[] = transcript.fifthRound(
      this.data.aBar.hex,
      this.data.bBar.hex,
      this.data.cBar.hex,
      this.data.W_w.hex,
      this.data.zwBar.hex, // TODO: check
      this.data.sigma1Bar.hex,
      this.data.sigma2Bar.hex,
      this.data.rBar.hex,
      this.data.W.hex, // TODO: check
    );

    super.convertChallengesToBN(initChallenge, beta, gamma, alpha, zeta, vChallenges);
  }

  /**
   * Compute the evaluations of the necessary polynomials, required for the verification
   * equation
   */
  public computePolynomialEvaluations() {
    this.zeroEval = computeZeroPolyEval(this.zeta, this.circuitSize);
    this.lagrangeEval = computeLagrangeEval(this.zeroEval, this.zeta, this.circuitSize);

    // TODO: calculate the lagrange evaluations
    this.publicInputEval = computePublicInputEval(this.publicInputs, this.lagrangeEvals);
    this.quotientEval = computeQuotient(
      this.publicInputEval,
      this.zeroEval,
      this.lagrangeEval,
      this.data.rBar.BN,
      this.beta,
      this.gamma,
      this.alpha,
      this.data.aBar.BN,
      this.data.bBar.BN,
      this.data.cBar.BN,
      this.data.sigma1Bar,
      this.data.sigma2Bar,
      this.data.zwBar
    );

    // TODO: update following methods once preprocessed input is sourced
    this.partialOpening = computePartialOpening(this.beta, this.gamma, this.alpha, this.data.aBar.BN, this.data.bBar.BN, this.data.cBar.BN, this.data.sigma1Bar.BN, this.data.sigma2Bar.BN, this.data.zwBar.BN, this.vChallenges[0], qM, qL, qR, qO, qC, this.zeta, k1, k2, this.lagrangeEval, u, z, sigma3Bar);
    this.batchOpening = computeBatchOpening();
    this.batchEvaluation = computeBatchEvaluation();
  }

  /**
   * Verify the proof by evaluating a pairing
   * Note: requires use of mcl-wasm library
   */
  public verifyProof() {
      /**
       * Perform pairing check according to round 12 in paper
       */
  }
}
