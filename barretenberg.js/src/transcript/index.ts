
import { keccak256, padLeft, sha3, soliditySha3 } from 'web3-utils';
import BN from 'BN.js';

export default class Transcript {
    data: string;

    constructor() {
        this.data = '';
    }

    /**
     * Compute challenge associated with prover initialisation round
     */
    preamble(circuitSize: string, publicInputSize: string) {
        this.appendScalar(circuitSize);
        this.appendScalar(publicInputSize);
        console.log('this.data: ', this.data);
        return this.keccak();
    }

    /**
     * Compute beta and gamma challenges associated with prover second round
     */
    secondRound() {
        
        
    }

    /**
     * Compute alpha challenge associated with prover third round
     */
    thirdRound() {

    };

    /**
     * Append an elliptic.js point. Pushes the y coord first, followed by the x coord
     */
    appendPoint(point: any) {
        this.data += point.y.fromRed().toString(16);
        this.data += point.x.fromRed().toString(16);
    }

    /**
     * Append a bn.js field element instance 
     */
    appendFieldElement(fieldElement: any) {
        this.data += fieldElement.toString(16);
    }

    /**
     * Append a bn.js scalar and format to 4 bytes - this method is specifically for appending the circuitSize 
     * and publicInput size, which are both represented using 4 bytes
     * 
     * 4 bytes = 8 characters
     */
    appendScalar(scalar: any) {
        // need to convert the input to a 4 byte representation
        let byte_0 = scalar & 0xff;
        console.log({ byte_0 });
        let byte_1 = (scalar >> 8) & 0xff;
        console.log({ byte_1 });
        let byte_2 = (scalar >> 16) & 0xff;
        console.log({ byte_2 });
        let byte_3 = (scalar >> 24) & 0xff;
        console.log({ byte_3 });
        this.data += padLeft(byte_0.toString(16), 2);
        this.data += padLeft(byte_1.toString(16), 2);
        this.data += padLeft(byte_2.toString(16), 2);
        this.data += padLeft(byte_3.toString(16), 2);
    }

    /**
     * Apply the Fiat Shamir protocol to generate a challenge
     */
    keccak() {
        this.data = keccak256(`0x${this.data}`);
        return this.data;
    }

    /**
     * Append 1 byte to the transcript, used to create a subsequent challenge based off current
     */
    appendByte() {
        this.data += '1';
    }
}