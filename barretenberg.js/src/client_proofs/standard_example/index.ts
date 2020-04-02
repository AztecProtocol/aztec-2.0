import { Signature } from '../../crypto/schnorr';
import { BarretenbergWorker } from '../../wasm/worker';
import { Prover } from '../prover';
import { SinglePippenger } from '../../pippenger';

export class CreateStandardProof {
  constructor(private wasm: BarretenbergWorker, private prover: Prover, private keyGenPippenger: SinglePippenger) {
  }

  public async init() {
    const pointTablePtr = this.keyGenPippenger.getPointTableAddr();
    const numPoints = this.keyGenPippenger.getNumCrsPoints();
    await this.wasm.transferToHeap(this.prover.getG2Data(), 0);
    await this.wasm.call("standard_example_init_keys", pointTablePtr, numPoints, 0);
  }

  public async createExampleProof() {
    const proverPtr = await this.wasm.call("standard_example_new_prover");
    const proof = await this.prover.createStandardProof(proverPtr);
    await this.wasm.call("standard_example_delete_prover", proverPtr);
    return proof;
  }

  public async verifyProof(proof: Buffer) {
    const proofPtr = await this.wasm.call("bbmalloc", proof.length);
    await this.wasm.transferToHeap(proof, proofPtr);
    const verified = await this.wasm.call("standard_example_verify_proof", proofPtr, proof.length) ? true : false;
    await this.wasm.call("bbfree", proofPtr);
    return verified;
  }
}
