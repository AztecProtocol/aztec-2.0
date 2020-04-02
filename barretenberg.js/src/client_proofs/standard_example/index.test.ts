import { CreateStandardProof } from './index';
import { BarretenbergWorker } from '../../wasm/worker';
import { fetchCode } from '../../wasm';
import { destroyWorker, createWorker, } from '../../wasm/worker_factory';
import { Prover } from '../prover';
import { Crs } from '../../crs';
import { SinglePippenger } from '../../pippenger';
import { PooledPippenger } from '../../pippenger/pooled_pippenger';
import createDebug from 'debug';

const debug = createDebug('create_proof');

describe('create_standard_proof', () => {
  let barretenberg!: BarretenbergWorker;
  let createStandardProof!: CreateStandardProof;
  let pippenger!: PooledPippenger;

  beforeAll(async () => {
    createDebug.enable('barretenberg*,create_proof')
    
    const code = await fetchCode();
    barretenberg = await createWorker();
    await barretenberg.init(code);

    const crs = new Crs(32*1024);
    await crs.download();

    const keyGenPippenger = new SinglePippenger(barretenberg);
    await keyGenPippenger.init(crs.getData());

    debug("creating workers...");
    let start = new Date().getTime();
    pippenger = new PooledPippenger(barretenberg);
    await pippenger.init(code, crs.getData(), 2);
    debug(`created workers: ${new Date().getTime() - start}ms`);


    const prover = new Prover(barretenberg, crs, pippenger);
    createStandardProof = new CreateStandardProof(barretenberg, prover, keyGenPippenger);

    debug("creating keys...");
    start = new Date().getTime();
    await createStandardProof.init();
    debug(`created circuit keys: ${new Date().getTime() - start}ms`);
  }, 60000);

  afterAll(async () => {
    await pippenger.destroy();
    await destroyWorker(barretenberg);
  });

  it.only('should construct "standard example" proof', async () => {
    debug("creating proof...");
    const start = new Date().getTime();
    const proof = await createStandardProof.createExampleProof();
    console.log({ proof });
    debug(`created proof: ${new Date().getTime() - start}ms`);
    debug(`proof size: ${proof.length}`);

    const verified = await createStandardProof.verifyProof(proof);
    console.log({ verified });
    expect(verified).toBe(true);
  }, 60000);
});
