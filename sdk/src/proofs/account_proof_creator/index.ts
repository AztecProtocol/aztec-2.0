import { GrumpkinAddress } from 'barretenberg/address';
import { AccountProver, AccountTx, computeSigningData } from 'barretenberg/client_proofs/account_proof';
import { AliasHash } from 'barretenberg/client_proofs/alias_hash';
import { Pedersen } from 'barretenberg/crypto/pedersen';
import { WorldState } from 'barretenberg/world_state';
import { randomBytes } from 'crypto';
import createDebug from 'debug';
import { Signer } from '../../signer';
import { AccountAliasId } from '../../user';

const debug = createDebug('bb:account_proof');

export class AccountProofCreator {
  constructor(private accountProver: AccountProver, private worldState: WorldState, private pedersen: Pedersen) {}

  public async createAccountTx(
    signer: Signer,
    aliasHash: AliasHash,
    nonce: number,
    migrate: boolean,
    accountPublicKey: GrumpkinAddress,
    newAccountPublicKey?: GrumpkinAddress,
    newSigningPubKey1?: GrumpkinAddress,
    newSigningPubKey2?: GrumpkinAddress,
    accountIndex = 0,
  ) {
    const merkleRoot = this.worldState.getRoot();
    const numNewKeys = [newSigningPubKey1, newSigningPubKey2].filter(k => !!k).length;
    const signingPubKey = signer.getPublicKey();
    const accountPath = await this.worldState.getHashPath(accountIndex);
    const accountAliasId = new AccountAliasId(aliasHash, nonce);
    const gibberish = randomBytes(32);
    const sigMsg = computeSigningData(
      accountAliasId,
      accountPublicKey,
      newAccountPublicKey || accountPublicKey,
      newSigningPubKey1 || GrumpkinAddress.ZERO,
      newSigningPubKey2 || GrumpkinAddress.ZERO,
      this.pedersen,
    );
    const signature = await signer.signMessage(sigMsg);

    return new AccountTx(
      merkleRoot,
      accountPublicKey,
      newAccountPublicKey || accountPublicKey,
      numNewKeys,
      newSigningPubKey1 || GrumpkinAddress.ZERO,
      newSigningPubKey2 || GrumpkinAddress.ZERO,
      accountAliasId,
      migrate,
      gibberish,
      accountIndex,
      accountPath,
      signingPubKey,
      signature,
    );
  }

  public async createProof(
    signer: Signer,
    aliasHash: AliasHash,
    nonce: number,
    migrate: boolean,
    accountPublicKey: GrumpkinAddress,
    newAccountPublicKey?: GrumpkinAddress,
    newSigningPubKey1?: GrumpkinAddress,
    newSigningPubKey2?: GrumpkinAddress,
    accountIndex?: number,
  ) {
    const tx = await this.createAccountTx(
      signer,
      aliasHash,
      nonce,
      migrate,
      accountPublicKey,
      newAccountPublicKey,
      newSigningPubKey1,
      newSigningPubKey2,
      accountIndex,
    );

    debug('creating proof...');
    const start = new Date().getTime();
    const proofData = await this.accountProver.createAccountProof(tx);
    debug(`created proof: ${new Date().getTime() - start}ms`);
    debug(`proof size: ${proofData.length}`);

    return proofData;
  }
}
