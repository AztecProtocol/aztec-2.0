import ProofUtils from './ProofUtils';
import { constants, errors } from '@aztec/dev-utils';

export default class BaseVerifier {
    constructor() {};

    /**
     * 
     */
    public validateInputs(G1Points: any[], fieldElements: any[]) {
        G1Points.forEach(point => ProofUtils.validatePointOnCurve(point));
        fieldElements.forEach(fieldElement => ProofUtils.validateElement(fieldElement));
    }

    /**
     * 
     * @param proofData 
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
        const G1Size: number = 128;
        const fieldElementSize: number = 64;
        const numFieldElements: number = 8;

        // Start and end points
        const proofLength: number = proofData.length;
        console.log({ proofData });
        console.log({ proofLength });
        const FEnd: number = proofLength - G1Size * 2; // jump backwards over the last two G1
        console.log({ FEnd });
        const FStart: number = FEnd - fieldElementSize * numFieldElements; // 14 field elements
        const G1End: number = FStart; // G1 points end immediately before field elements start
        const G1Start: number = G1End - G1Size * 7; // 7 G1 points in this section

        // Last 2 appended G1 elements
        const W_w: string = proofData.slice(proofLength - G1Size, proofLength);
        const W: string = proofData.slice(proofLength - 2 * G1Size, proofLength - G1Size);

        // G1 data
        const G1Data: string = (proofData.slice(G1Start, G1End)).concat(W, W_w);
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
        const G1Points: object = this.extractVariables(G1Names, G1Data, G1Size);

        // field elements
        const FieldElementsData: string = proofData.slice(FStart, FEnd);
        const fieldElementNames: string[] = ['aEval', 'bEval', 'cEval', 's1Eval', 's2Eval', 'rEval', 'zwEval'];
        const fieldElements: object = this.extractVariables(fieldElementNames, FieldElementsData, fieldElementSize);

        const data: object = {
            ...G1Points,
            ...fieldElements
        };
        return { 
            data,
            G1Points: Object.values(G1Points),
            fieldElements: Object.values(fieldElements),
        };
    }

    /**
     * 
     * @param names 
     * @param data 
     * @param elementSize 
     */
    extractVariables(names: string[], data: string, elementSize: number) {
        const extractData: object = {};
    
        names.forEach((name, index) => {
            const elementStart = index * elementSize;
            const hexDataElement = data.slice(elementStart, elementStart + elementSize);
    
            let dataElement: any;
            if (elementSize === 128) { // G1 element

                // y coord = first 32 bytes
                // x coord = second 32 bytes
                dataElement = ProofUtils.hexToGroupPoint(hexDataElement.slice(64, 128), hexDataElement.slice(0, 64));
            } else if (elementSize === 64) {  // field element
                dataElement = ProofUtils.hexToGroupScalar(hexDataElement);
            }
    
            extractData[name] = dataElement;
          });
    
        return extractData;
    }
}