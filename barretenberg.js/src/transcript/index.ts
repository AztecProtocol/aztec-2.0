import { keccak256, padLeft, sha3, soliditySha3 } from 'web3-utils';
import BN from 'BN.js';

export default class Transcript {
  data: string;

  constructor() {
    this.data = '';
  }

  /**
   * Compute challenge associated with prover round.
   *
   * Involves adding the circuit size and publicInputSize to the
   * transcript
   */
  preamble(circuitSize: any, publicInputSize: any) {
    this.appendScalar(circuitSize);
    this.appendScalar(publicInputSize);
    return this.keccak();
  }

  /**
   * Compute beta and gamma challenges associated with prover second round.
   *
   * Involves adding the publicInputs, aCommit, bCommit and cCommit to the transcript
   */
  secondRound(publicInputs: string[], aCommit, bCommit, cCommit) {
    publicInputs.forEach((publicInput) => this.appendFieldElement(publicInput));
    this.appendPoint(aCommit);
    this.appendPoint(bCommit);
    this.appendPoint(cCommit);
    const beta: string = this.keccak();

    this.appendByte();
    const gamma: string = this.keccak();

    return { beta, gamma };
  }

  /**
   * Compute alpha challenge associated with prover third round
   *
   * Involves adding Z to the transcript
   */
  thirdRound(zCommit) {
    this.appendPoint(zCommit);
    return this.keccak();
  }

  /**
   * Compute zeta challenge associated with prover fourth round
   *
   * Involves adding t_low, t_mid, t_high to the transcript
   */
  fourthRound(tlowCommit, tmidCommit, thighCommit) {
    this.appendPoint(tlowCommit);
    this.appendPoint(tmidCommit);
    this.appendPoint(thighCommit);
    return this.keccak();
  }

  /**
   * Compute v, u challenges associated with prover fifth round
   *
   * Involves adding
   */
  fifthRound(
    aBar: string,
    bBar: string,
    cBar: string,
    WzBar: string,
    zwBar: string,
    sigma1Bar: string,
    sigma2Bar: string,
    rBar: string,
    t: string,
  ) {
      const numChallenges: number = 10;
      this.appendFieldElement(aBar);
      this.appendFieldElement(bBar);
      this.appendFieldElement(cBar);
      this.appendFieldElement(WzBar);
      this.appendFieldElement(zwBar);
      this.appendFieldElement(sigma1Bar);
      this.appendFieldElement(sigma2Bar);
      this.appendFieldElement(rBar);
      this.appendFieldElement(t);

      const baseHash = this.keccak();

      /**
       * Aim here is to generate a number of challenges, derived from a baseHash/challenge.
       * 
       * To do this, we take the baseHash and then append a byte to the end, which increases in
       * value on each round of the for loop i.e.:
       * 
       * challenge1 = keccak256(${baseHash}'01')
       * challenge2 = keccak256(${baseHash}'02')
       * 
       */
      const vChallenges: string[] = [];
      vChallenges.push(baseHash);
      for (let i = 1; i < numChallenges; i += 1) {
        this.data = baseHash.slice(2);
        this.appendByte('0' + i.toString());

        const challenge = this.keccak();
        vChallenges.push(challenge);
      }

      return vChallenges;
  }

  /**
   * Append a 64 byte hex point. y coord should be first, followed by x coord
   */
  appendPoint(point: string) {
    this.data += padLeft(point.slice(2), 128);
  }

  /**
   * Append a 32 byte hex field element instance
   */
  appendFieldElement(fieldElement: string) {
    this.data += padLeft(fieldElement.slice(2), 64);
  }

  /**
   * Append a bn.js scalar and format to 4 bytes - this method is specifically for appending the circuitSize
   * and publicInput size, which are both represented using 4 bytes
   *
   */
  appendScalar(scalar: any) {
    let byte_0 = scalar & 0xff;
    let byte_1 = (scalar >> 8) & 0xff;
    let byte_2 = (scalar >> 16) & 0xff;
    let byte_3 = (scalar >> 24) & 0xff;
    this.data += padLeft(byte_0.toString(16), 2);
    this.data += padLeft(byte_1.toString(16), 2);
    this.data += padLeft(byte_2.toString(16), 2);
    this.data += padLeft(byte_3.toString(16), 2);
  }

  /**
   * Apply the Fiat Shamir protocol to generate a challenge
   */
  keccak() {
    this.data = keccak256(`0x${this.data}`).slice(2);
    return `0x${this.data}`;
  }

  /**
   * Append a byte of value to the transcript. Appends '1' by default, but an arbitrary value can be passed. 
   * Used to create subsequent challenges based off a base challenge
   */
  appendByte(value: string = '1') {
    this.data += `${value}`;
  }
}