import PolynomialEvalUtils from './PolynomialEvalUtils';
import BaseVerifier from './BaseVerifier';
import Transcript from '../transcript';
import BN from 'bn.js';

export class Verifier extends BaseVerifier {
  public data!: object;
  public G1Points!: any[];
  public fieldElements!: any[];
  public publicInputs!: any[];
  circuitSize!: string;

  constructor(proofData: string, verificationKey: string) {
    super();

    const { data, G1Points, fieldElements, publicInputs } = this.decodeProof(proofData);
    this.data = data;
    this.G1Points = G1Points;
    this.fieldElements = fieldElements;
    this.publicInputs = publicInputs;
    
    // will need changing depending on setup
    this.circuitSize = verificationKey.slice(0, 0x20);

    this.validateInputs(this.G1Points, this.fieldElements, this.publicInputs);
    this.computeChallenges();
  }

  /**
   * Compute beta, gamma, alpha, zeta, v, u challenges
   */
  public computeChallenges() {
      /**
       * In the prover, the following transcript is built up by concatenating elements
       * together:
       * 
       * preamble round
       * 1) circuitSize
       * 2) publicInputSize
       * 3) publicInputs
       * 
       * first proof round
       * 1) publicInputs
       * Get beta out
       * 2) append 1, get gamma out
       * 
       */
    // need: 1) common preprocessed input, 2) publicInput, 3) proof elements written by prover
    const transcript = new Transcript();
    const preambleHash = transcript.preamble(this.circuitSize, this.publicInputs);
    console.log({ preambleHash });
    // const { beta, gamma } = transcript.secondRound();
    // const alpha = transcript.thirdRound();
    // const zeta = transcript.fourthRound();
    // const { v, u } = transcript.fifthRound();

    // return {
    //     beta,
    //     gamma,
    //     alpha,
    //     zeta,
    //     v,
    //     u
    // }

    // rollingHash.appendPoint(this.data['aCommit']);
    // rollingHash.appendPoint(this.data['bCommit']);
    // rollingHash.appendPoint(this.data['cCommit']);
    // this.publicInputs.forEach(publicInput => rollingHash.appendFieldElement(publicInput));
    // const beta = rollingHash.keccak();

    // rollingHash.appendFieldElement(new BN(1));
    // const gamma = rollingHash.keccak();

    // rollingHash.appendPoint(this.data['zCommit']);
    // const alpha = rollingHash.keccak();

    // rollingHash.appendPoint(this.data['tlowCommit']);
    // rollingHash.appendPoint(this.data['tmidCommit']);
    // rollingHash.appendPoint(this.data['thighCommit']);
    // const zeta = rollingHash.keccak();

    // this.fieldElements.forEach(fieldElement => rollingHash.appendFieldElement(fieldElement));
    // const v = rollingHash.keccak();

    // rollingHash.appendPoint(this.data['W']);
    // rollingHash.appendPoint(this.data['W_w']);
    // const u = rollingHash.keccak();

    // return { beta, gamma, alpha, zeta, v, u };
  }

  public computePolynomialEvaluations() {}

  public verifyProof() {}
}
