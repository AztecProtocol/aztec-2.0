
import { keccak256, padLeft, sha3, soliditySha3 } from 'web3-utils';
import BN from 'BN.js';

export default class Transcript {
    data: string;

    constructor() {
        this.data = '';
    }

    /**
     * Compute challenge associated with prover initialisation round. 
     * 
     * Involves adding the circuit size and publicInputSize to the 
     * transcript
     */
    preamble(circuitSize: string, publicInputSize: string) {
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
        publicInputs.forEach(publicInput => this.appendFieldElement(publicInput));
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
    thirdRound() {

    };

    /**
     * Append a 64 byte elliptic.js point. Pushes the y coord first, followed by the x coord
     */
    appendPoint(point: any) {
        // this.data += padLeft(point.y.fromRed().toString(16), 64);
        // this.data += padLeft(point.x.fromRed().toString(16), 64);
        this.data += padLeft(point.slice(2), 128);
    }

    /**
     * Append a 32 byte bn.js field element instance 
     */
    appendFieldElement(fieldElement: any) {
        this.data += padLeft(fieldElement.toString(16), 64);
    }

    /**
     * Append a bn.js scalar and format to 4 bytes - this method is specifically for appending the circuitSize 
     * and publicInput size, which are both represented using 4 bytes
     * 
     * 4 bytes = 8 characters
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
     * Append 1 byte to the transcript, used to create a subsequent challenge based off current
     */
    appendByte() {
        this.data += '1';
    }
}