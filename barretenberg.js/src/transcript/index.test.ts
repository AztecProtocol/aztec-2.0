import * as bn128 from '@aztec/bn128';
import BN from 'bn.js';
import { keccak256, randomHex } from 'web3-utils';

import Transcript from './index';
import { expectedChallenges, proofVariables } from '../utils/dummyProof';

// const { circuitSize, numPublicInputs } = proofVariables;
const { expectedInit } = expectedChallenges;

describe('Transcript', () => {
    let transcript: Transcript;

    beforeEach(() => {
        transcript = new Transcript();
    });

    describe('Base operations', () => {
        it('should hash a set of field elements', () => {
            const fieldElement1 = new BN('12345', 16)
            const fieldElement2 = new BN('8a4b8e', 16)
    
            transcript.appendFieldElement(fieldElement1);
            transcript.appendFieldElement(fieldElement2);
    
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
    })

    describe.only('Challenge generation', () => {
        const circuitSize: any = new BN(256);
        const numPublicInputs: any = new BN(1);
        const publicInputs: any = [new BN(456)];
        const aCommit: string = '0x0e24f35b9f2df4eb7c4ba18236935fbf8bc8c7b6b52b315650d05198017939ac28e990d0d01996766774d37656b71c71dff182df3617d4ca7180264b815d7382';
        const bCommit: string = '0xa093a435555f3fc8ecbb3292a8bd98086fb9348bb4457e90806eb9f1713998702c50ef1de5fd9e6aaa15a9e1b48d003cf254e5063cd0a2f97a72b4f8354cd8a';
        const cCommit: string = '0x193341758d4a176c3cb5d135dfb06746b62f77acbefc43246340f6afdcf9b95e131445dfa496875360d375635ea950909acdc865feab9465537dffffb9a402fd';

        it('should compute preamble challenge', () => {
            const initChallenge: string = transcript.preamble(circuitSize, numPublicInputs);
    
            const expectedInit: string = '0x5adb62152a7d3b16e5b33a22bc6b713997fd9ffe98aa2ee7764602929db3f15a';
            expect(initChallenge).toBe(expectedInit); 
        });
    
        it('should compute second round challenges', () => {
            transcript.preamble(circuitSize, numPublicInputs);
            const { beta, gamma } = transcript.secondRound(publicInputs, aCommit, bCommit, cCommit);
    
            console.log({ beta, gamma });
            const expectedBeta: string = '0x694561f6377c67b5fb98b306c5f5646d2f368ede6cb7b30bd63dde9012803017';
            const expectedGamma: string = '0x9a9c139c975983e86a9a9fe6e6eb24528f94ff9cc187ae442509c0bba773792d';
            expect(beta).toBe(expectedBeta);
            expect(gamma).toBe(expectedGamma);
        });
    
        it.only('should compute third round challenge', () => {
            const { beta: string, gamma: string } = transcript.thirdRound();
            expect(beta).toBe(expectedBeta);
            expect(gamma).toBe(expectedGamma);
        });
    })
});