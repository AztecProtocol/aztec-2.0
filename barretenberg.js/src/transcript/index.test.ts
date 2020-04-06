import * as bn128 from '@aztec/bn128';
import BN from 'bn.js';
import { keccak256, padLeft } from 'web3-utils';

import Transcript from './index';
import { expectedChallenges, expectedVChallenges, proofVariables } from '../utils/dummyProof';

const {
  circuitSize,
  numPublicInputs,
  publicInputs,
  aCommit,
  bCommit,
  cCommit,
  zCommit,
  tlowCommit,
  tmidCommit,
  thighCommit,
  w_1,
  w_2,
  w_3,
  w_3_omega,
  z_omega,
  sigma_1,
  sigma_2,
  r,
  t,
} = proofVariables;
const { expectedInit, expectedBeta, expectedGamma, expectedAlpha, expectedZeta } = expectedChallenges;

describe('Transcript', () => {
  let transcript: Transcript;

  beforeEach(() => {
    transcript = new Transcript();
  });

  describe('Base operations', () => {
    it('should hash a set of field elements', () => {
      const fieldElement1 = `0x${padLeft(new BN('12345', 16).toString(16), 64)}`;
      const fieldElement2 = `0x${padLeft(new BN('8a4b8e', 16).toString(16), 64)}`;
      console.log({ fieldElement1 });

      transcript.appendFieldElement(fieldElement1);
      transcript.appendFieldElement(fieldElement2);

      const expectedHash: string = keccak256(fieldElement1.concat(fieldElement2.slice(2)));
      const hashedData = transcript.keccak();
      expect(hashedData).toBe(expectedHash);
    });

    it('should hash a set of points', () => {
      const point1: string = `0x${padLeft('0x763de812dec810215bc17987e5d2a0ba8dfb00d9b7df29ab857ee446d98e4cf2201082870c343ebcabce44eea82167c55d321a3f816c7e4e56b729f9dd6a6a9', 128)}`;
      const point2: string = `0x${padLeft('0x15f28289950c3d809a96598945f198061c072ec936a5666a5ee01af6632ae86044a4a7aa3d630f8128673a386a8580910cb002d7c1e785185a2c27e5cd91ec4', 128)}`;

      transcript.appendPoint(point1);
      transcript.appendPoint(point2);
      const hashedData: string = transcript.keccak();

      console.log(point1.concat(point2.slice(2)));
      const expectedHash: string = keccak256(point1.concat(point2.slice(2)));
      expect(hashedData).toBe(expectedHash);
    });
  });

  describe('Challenge generation', () => {
    it('should compute preamble challenge', () => {
      const initChallenge: string = transcript.preamble(circuitSize, numPublicInputs);
      expect(initChallenge).toBe(expectedInit);
    });

    it('should compute second round challenges', () => {
      transcript.preamble(circuitSize, numPublicInputs);
      const { beta, gamma } = transcript.secondRound(publicInputs, aCommit, bCommit, cCommit);
      expect(beta).toBe(expectedBeta);
      expect(gamma).toBe(expectedGamma);
    });

    it('should compute third round challenge', () => {
      transcript.preamble(circuitSize, numPublicInputs);
      transcript.secondRound(publicInputs, aCommit, bCommit, cCommit);

      const alpha: string = transcript.thirdRound(zCommit);
      expect(alpha).toBe(expectedAlpha);
    });

    it('should compute fourth round challenges', () => {
      transcript.preamble(circuitSize, numPublicInputs);
      transcript.secondRound(publicInputs, aCommit, bCommit, cCommit);
      transcript.thirdRound(zCommit);

      const zeta: string = transcript.fourthRound(tlowCommit, tmidCommit, thighCommit);
      expect(zeta).toBe(expectedZeta);
    });

    it('should compute fifth round challenges', () => {
      transcript.preamble(circuitSize, numPublicInputs);
      transcript.secondRound(publicInputs, aCommit, bCommit, cCommit);
      transcript.thirdRound(zCommit);
      transcript.fourthRound(tlowCommit, tmidCommit, thighCommit);

      const vChallenges: string[] = transcript.fifthRound(w_1, w_2, w_3, w_3_omega, z_omega, sigma_1, sigma_2, r, t);

      vChallenges.forEach((challenge, index) => {
        expect(challenge).toBe(expectedVChallenges[index]);
      });
    });
  });
});
