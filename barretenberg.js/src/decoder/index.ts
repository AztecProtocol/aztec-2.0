import { extractVariables } from '../utils/extractVariables';

export default function decodeProof(proofData: string) {
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
    const G1Size: number = 128;
    const fieldElementSize: number = 64;

    // Start and end points
    const proofLength: number = proofData.length;
    const FEnd: number = proofLength - G1Size * 2; // jump backwards over the last two G1
    const FStart: number = FEnd - fieldElementSize * 14; // 14 field elements
    const G1End: number = FStart; // G1 points end immediately before field elements start
    const G1Start: number = G1End - G1Size * 7; // 7 G1 points in this section

    // Last 2 appended G1 elements
    const W_w: string = proofData.slice(proofLength - G1Size, proofLength);
    const W: string = proofData.slice(proofLength - 2 * G1Size, proofLength - G1Size);

    // G1 points
    const G1Points: string = (proofData.slice(G1Start, G1End)).concat(W, W_w);
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
    const G1Data: object = extractVariables(G1Names, G1Points, G1Size);

    // field elements
    const FieldElements: string = proofData.slice(FStart, FEnd);
    const fieldElementNames: string[] = ['aEval', 'bEval', 'cEval', 's1Eval', 's2Eval', 'rEval', 'zwEval'];
    const fieldElementData: object = extractVariables(fieldElementNames, FieldElements, fieldElementSize);

    const data: object = {
        ...G1Data,
        ...fieldElementData
    };
    return data;
}