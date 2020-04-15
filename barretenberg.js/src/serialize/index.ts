import { padLeft } from 'web3-utils';

/**
 * Encode the proofData into the form required by the verifier contract. 
 * 
 * Returns numPublicInputs, the publicInputs and the serialized data
 * 
 * Serialized data has the following encoding:
 *          aCommit[x],
            aCommit[y],
            bCommit[x],
            bCommit[y],
            cCommit[x],
            cCommit[d],
            zCommit[x],
            zCommit[y],
            tCommit[low],
            tCommit[mid],
            tCommit[high],
            wCommit[x],
            wCommit[y],
            w_wCommit[x],
            w_wCommit[y],
            aBar,
            bBar,
            cBar,
            W_zBar,
            z_wBar,
            sigma1Bar,
            sigma2Bar,
            rBar,
 * 
 *
 * @param proofData - proofData output by a PLONK prover
 */

export default function serializeProof(proofData: string): any {
    /**
     * G1End = FStart = ( L + 0x40*7 )
     * FEnd = ( L + 0x40*7 + 0x20*14 )
     *
     * Input proof structure
     * 0x00 - L: public inputs
     * L - G1End: G1 points, excluding W_zw and W_w
     * FStart - FEnd: Field elements
     * FEnd - (FEnd + 0x40): W_z
     * (FEnd + 0x40) - (FEnd + 0x80): W_zw
     * 
     */

    // config
    const G1Size: number = 128;
    const G1CoordSize: number = G1Size / 2;
    const fieldElementSize: number = 64;
    const publicInputSize: number = 64;
    const numFieldElements: number = 8;
    const numG1Points: number = 9;

    // Indexes to data
    const proofLength: number = proofData.length;
    const fieldEnd: number = proofLength - G1Size * 2; // jump backwards over the last two G1
    const fieldStart: number = fieldEnd - fieldElementSize * numFieldElements; // 8 field elements
    const G1End: number = fieldStart; // G1 points end immediately before field elements start
    const G1Start: number = G1End - G1Size * 7; // 7 G1 points in this section
    const publicEnd: number = G1Start;
    const publicStart: number = 0;

    const publicInputData: string = proofData.slice(publicStart, publicEnd);
    const fieldElementData: string = proofData.slice(fieldStart, fieldEnd);
    const G1Data: string = proofData.slice(G1Start, G1End) + proofData.slice(fieldEnd, proofLength) // append last 2 G1 points

    const numPublicInputs: string = padLeft(publicEnd/publicInputSize, 64);

    const publicInputs: string [] = serializeVariables(publicInputData, numPublicInputs, publicInputSize);
    const G1Points: string [] = serializeVariables(G1Data, numG1Points * 2, G1CoordSize);
    const fieldElements: string [] = serializeVariables(fieldElementData, numFieldElements, fieldElementSize);

    const serializedProofData = [...G1Points, ...fieldElements]
    return { numPublicInputs, publicInputs, serializedProofData };
}

function serializeVariables(data, numElements, elementSize) {
    const serializedData: string [] = [];
    for (let elementIndex = 0; elementIndex < parseInt(numElements); elementIndex += 1) {
        const elementStart = elementIndex * elementSize;
        const hexDataElement = data.slice(elementStart, elementStart + elementSize);
        serializedData.push(hexDataElement)
    }
    return serializedData;
}