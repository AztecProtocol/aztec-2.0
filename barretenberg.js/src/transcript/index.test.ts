import * as bn128 from '@aztec/bn128';
import BN from 'bn.js';
import { keccak256, randomHex } from 'web3-utils';

import Transcript from './index';
import { expectedChallenges, proofVariables } from '../utils/dummyProof';

// const { circuitSize, publicInput } = proofVariables;
const { expectedInit } = expectedChallenges;

describe('Transcript', () => {
    let transcript: Transcript;

    beforeEach(() => {
        transcript = new Transcript();
    });

    it('should hash a set of field elements', () => {
        const fieldElement1 = new BN('12345', 16)
        const fieldElement2 = new BN('8a4b8e', 16)

        transcript.appendFieldElement(fieldElement1);
        transcript.appendFieldElement(fieldElement2);

        const hashedData: string = transcript.keccak();
        const expectedHash: string = keccak256(fieldElement1.toString(16).concat(fieldElement2.toString(16)))
        expect(transcript.data).toBe(expectedHash);
    });

    it('should hash a set of points', () => {
        const point1: any = bn128.curve.point(new BN('763de812dec810215bc17987e5d2a0ba8dfb00d9b7df29ab857ee446d98e4cf', 16), new BN('2201082870c343ebcabce44eea82167c55d321a3f816c7e4e56b729f9dd6a6a9', 16));
        const point2: any = bn128.curve.point(new BN('15f28289950c3d809a96598945f198061c072ec936a5666a5ee01af6632ae860', 16), new BN('44a4a7aa3d630f8128673a386a8580910cb002d7c1e785185a2c27e5cd91ec4', 16));

        transcript.appendPoint(point1);
        transcript.appendPoint(point2);
        const hashedData: string = transcript.keccak()

        const dataToHash: string = point1.y.fromRed().toString(16).concat(
            point1.x.fromRed().toString(16),
            point2.y.fromRed().toString(16),
            point2.x.fromRed().toString(16)
        );
 
        const expectedHash: string = keccak256(dataToHash);
        expect(hashedData).toBe(expectedHash);
    });

    it.only('should compute preamble challenge', () => {
        // console.log({ circuitSize });
        // console.log({ publicInput });
        const circuitSize: any = new BN(256);
        const publicInput: any = new BN(1);
        const initChallenge: string = transcript.preamble(circuitSize, publicInput);
        console.log({ initChallenge });

        const expectedInit: string = '0x2a7713a2494b9aed2d62f46c3aea18dc6fc9b7b61ef0be5632640cfeadb3f159';
        expect(initChallenge).toBe(expectedInit); 
    });

    // it('should compute second round challenge', () => {
    //     const { beta: string, gamma: string } = transcript.secondRound();
    //     expect(beta).toBe(expectedBeta);
    //     expect(gamma).toBe(expectedGamma);
    // });

    // it.skip('should compute third round challenge', () => {
    //     const { beta: string, gamma: string } = transcript.secondRound();
    //     expect(beta).toBe(expectedBeta);
    //     expect(gamma).toBe(expectedGamma);
    // });
});