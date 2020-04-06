import * as bn128 from '@aztec/bn128';
import { constants, errors } from '@aztec/dev-utils';
import { toBN } from 'web3-utils';
import BN from 'bn.js';
import ProofUtils from './ProofUtils';

export default class BaseVerifier {
  public verificationKey!: string;
  public circuitSize!: BN;
  public numPublicInputs!: BN;

  // Challenges
  public initChallenge!: any;
  public beta!: BN;
  public gamma!: BN;
  public alpha!: BN;
  public zeta!: BN;
  public vChallenges!: BN[];

  constructor(verificationKey: string) {
    this.verificationKey = verificationKey;
  }

  /**
   * Extract the G1Points, field elements and publicInputs from the proofData. Also, convert them from
   * hex into their relevant form - i.e. scalar, elliptic curve point etc
   *
   * @param proofData - proofData output by a PLONK prover
   */
  decodeProof(proofData: string) {
    /**
     * G1End = FStart = ( L + 0x40*7 )
     * FEnd = ( L + 0x40*7 + 0x20*14 )
     *
     * Proof encoding
     * 0x00 - L: public inputs
     * L - G1End: G1 points, excluding W_zw and W_w
     * FStart - FEnd: Field elements
     * FEnd - (FEnd + 0x40): W_z
     * (FEnd + 0x40) - (FEnd + 0x80): W_zw
     */

    // config
    const G1Size: number = 128;
    const fieldElementSize: number = 64;
    const publicInputSize: number = 64;
    const numFieldElements: number = 8;

    // Indexes to data
    const proofLength: number = proofData.length;
    const fieldEnd: number = proofLength - G1Size * 2; // jump backwards over the last two G1
    const fieldStart: number = fieldEnd - fieldElementSize * numFieldElements; // 8 field elements
    const G1End: number = fieldStart; // G1 points end immediately before field elements start
    const G1Start: number = G1End - G1Size * 7; // 7 G1 points in this section
    const publicEnd: number = G1Start;
    const publicStart: number = 0;

    const G1Points: object = this.decodeG1Points(proofData, G1Size, G1Start, G1End);
    const fieldElements: object = this.decodeFieldElements(proofData, fieldElementSize, fieldStart, fieldEnd);
    const publicInputs: string[] = this.decodePublicInputs(proofData, publicInputSize, publicStart, publicEnd);

    const data: object = {
      ...G1Points,
      ...fieldElements,
      ...publicInputs,
    };

    return {
      data,
      G1Points: Object.values(G1Points),
      fieldElements: Object.values(fieldElements),
      publicInputs,
    };
  }

  /**
   * Decode G1 points
   */
  decodeG1Points(proofData, G1Size, start, end) {
    const proofLength = proofData.length;
    // Last 2 appended G1 elements
    const W_w: string = proofData.slice(proofLength - G1Size, proofLength);
    const W: string = proofData.slice(proofLength - 2 * G1Size, proofLength - G1Size);

    const G1Data: string = proofData.slice(start, end).concat(W, W_w);
    const G1Names: string[] = [
      'aCommit',
      'bCommit',
      'cCommit',
      'zCommit',
      'tlowCommit',
      'tmidCommit',
      'thighCommit',
      'W',
      'W_w',
    ];
    return this.extractVariables(G1Names, G1Data, G1Size);
  }

  /**
   * Decode field elements
   */
  decodeFieldElements(proofData: string, fieldElementSize: number, start: number, end: number): object {
    const FieldElementsData: string = proofData.slice(start, end);
    const fieldElementNames: string[] = ['aEval', 'bEval', 'cEval', 's1Eval', 's2Eval', 'rEval', 'zwEval'];
    return this.extractVariables(fieldElementNames, FieldElementsData, fieldElementSize);
  }

  /**
   * Decode publicInputs
   */
  decodePublicInputs(proofData: string, publicInputSize: number, start: number, end: number): string[] {
    const publicInputsData: string = proofData.slice(start, end);

    const extractData: string[] = [];
    let dataElementStart: number = 0;

    let currentLocation: number;
    for (currentLocation = 0; currentLocation < publicInputsData.length; currentLocation += publicInputSize) {
      extractData.push(publicInputsData.slice(dataElementStart, dataElementStart + currentLocation));
      dataElementStart += currentLocation;
    }

    return extractData;
  }

  /**
   * Extract
   *
   * @param names
   * @param data
   * @param elementSize
   */
  extractVariables(names: string[], data: string, elementSize: number): object {
    const extractData: object = {};

    names.forEach((name, index) => {
      const elementStart = index * elementSize;
      const hexDataElement = data.slice(elementStart, elementStart + elementSize);

      let dataElement: any;
      if (elementSize === 128) {
        // G1 element: y coord = first 32 bytes, x coord = second 32 bytes
        dataElement = ProofUtils.hexToGroupPoint(hexDataElement.slice(64, 128), hexDataElement.slice(0, 64));
      } else if (elementSize === 64) {
        // field element or public input
        dataElement = ProofUtils.hexToGroupScalar(hexDataElement);
      }

      extractData[name] = { bn: dataElement, hex: hexDataElement };
    });

    return extractData;
  }

  /**
   * Extract circuit variables from the verification key
   *
   * TODO: when prover exports verificationKey, update this method to use that
   * rather than hard coded
   */
  public decodeVerificationKey() {
    this.circuitSize = new BN(256); // needs slicing out of verification key
    this.numPublicInputs = new BN(1); // needs slicing out of verification key
  }

  /**
   * Check that G1Points are on the bn128 curve and field elements are less than the modulus
   * of the field
   */
  public validateInputs(G1Points: any[], fieldElements: any[], publicInputs: string[]) {
    G1Points.forEach((point) => ProofUtils.validatePointOnCurve(point.bn));
    fieldElements.forEach((fieldElement) => ProofUtils.validateElement(fieldElement.bn));
    publicInputs.forEach((publicInput) => ProofUtils.validateElement(publicInput));
  }

  /**
   * Convert challenges from hex strings to BN instances
   */
  public convertChallengesToBN(
    initChallenge: string,
    beta: string,
    gamma: string,
    alpha: string,
    zeta: string,
    vChallenges: string[],
  ) {
    this.initChallenge = toBN(initChallenge).toRed(bn128.groupReduction);
    this.beta = toBN(beta).toRed(bn128.groupReduction);
    this.gamma = toBN(gamma).toRed(bn128.groupReduction);
    this.alpha = toBN(alpha).toRed(bn128.groupReduction);
    this.zeta = toBN(zeta).toRed(bn128.groupReduction);
    this.vChallenges = vChallenges.map((vChallenge) => toBN(vChallenge).toRed(bn128.groupReduction));
  }
}
