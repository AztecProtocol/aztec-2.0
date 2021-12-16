import { TxType } from 'barretenberg/blockchain';
import { TxHash } from 'barretenberg/tx_hash';
import { toBufferBE } from 'bigint-buffer';
import { Connection, In, IsNull, MoreThanOrEqual, Not, Repository } from 'typeorm';
import { AccountDao } from '../entity/account';
import { RollupDao } from '../entity/rollup';
import { RollupProofDao } from '../entity/rollup_proof';
import { TxDao } from '../entity/tx';
import { txDaoToAccountDao } from './tx_dao_to_account_dao';

export type RollupDb = {
  [P in keyof TypeOrmRollupDb]: TypeOrmRollupDb[P];
};

export class TypeOrmRollupDb implements RollupDb {
  private txRep: Repository<TxDao>;
  private rollupProofRep: Repository<RollupProofDao>;
  private rollupRep: Repository<RollupDao>;
  private accountRep: Repository<AccountDao>;

  constructor(private connection: Connection) {
    this.txRep = this.connection.getRepository(TxDao);
    this.rollupProofRep = this.connection.getRepository(RollupProofDao);
    this.rollupRep = this.connection.getRepository(RollupDao);
    this.accountRep = this.connection.getRepository(AccountDao);
  }

  public async addTx(txDao: TxDao) {
    await this.connection.transaction(async transactionalEntityManager => {
      if (txDao.txType === TxType.ACCOUNT) {
        await transactionalEntityManager.save(txDaoToAccountDao(txDao));
      }
      await transactionalEntityManager.save(txDao);
    });
  }

  public async addTxs(txs: TxDao[]) {
    await this.connection.transaction(async transactionalEntityManager => {
      const accountDaos = txs.filter(tx => tx.txType === TxType.ACCOUNT).map(txDaoToAccountDao);
      await transactionalEntityManager.save(accountDaos);
      await transactionalEntityManager.save(txs);
    });
  }

  public async getTx(txId: Buffer) {
    return this.txRep.findOne({ id: txId }, { relations: ['rollupProof', 'rollupProof.rollup'] });
  }

  public async getPendingTxCount() {
    return this.txRep.count({
      where: { rollupProof: null },
    });
  }

  public async deletePendingTxs() {
    await this.txRep.delete({ rollupProof: null });
  }

  public async getTotalTxCount() {
    return this.txRep.count();
  }

  public async getJoinSplitTxCount() {
    return this.txRep.count({ where: { txType: Not(TxType.ACCOUNT) } });
  }

  public async getAccountTx(aliasHash: Buffer) {
    return this.accountRep.findOne({ aliasHash });
  }

  public async getLatestAccountTx(accountPubKey: Buffer) {
    return this.accountRep.findOne(
      { accountPubKey },
      {
        order: { nonce: 'DESC' },
      },
    );
  }

  public async getAccountTxCount() {
    return this.txRep.count({ where: { txType: TxType.ACCOUNT } });
  }

  public async getAccountCount() {
    return this.accountRep.count();
  }

  public async getTotalRollupsOfSize(rollupSize: number) {
    return await this.rollupProofRep
      .createQueryBuilder('rp')
      .leftJoin('rp.rollup', 'r')
      .where('rp.rollupSize = :rollupSize AND r.mined IS NOT NULL', { rollupSize })
      .getCount();
  }

  public async getUnsettledTxCount() {
    return await this.txRep.count({ where: { mined: null } });
  }

  private async getUnsettledTxs() {
    return await this.txRep.find({ where: { mined: null } });
  }

  public async getUnsettledJoinSplitTxs() {
    return await this.txRep.find({
      where: { txType: Not(TxType.ACCOUNT), mined: null },
    });
  }

  public async getUnsettledAccountTxs() {
    return await this.txRep.find({
      where: { txType: TxType.ACCOUNT, mined: null },
    });
  }

  public async getPendingTxs(take?: number) {
    return this.txRep.find({
      where: { rollupProof: null },
      order: { created: 'ASC' },
      take,
    });
  }

  public async getUnsettledNullifiers() {
    const unsettledTxs = await this.getUnsettledTxs();
    return unsettledTxs.map(tx => [tx.nullifier1, tx.nullifier2]).flat();
  }

  public async nullifiersExist(n1: Buffer, n2: Buffer) {
    const count = await this.txRep
      .createQueryBuilder('tx')
      .where('tx.nullifier1 IS :n1 OR tx.nullifier1 IS :n2 OR tx.nullifier2 IS :n1 OR tx.nullifier2 IS :n2', { n1, n2 })
      .getCount();
    return count > 0;
  }

  public async addRollupProof(rollupDao: RollupProofDao) {
    await this.rollupProofRep.save(rollupDao);
  }

  public async getRollupProof(id: Buffer, includeTxs = false) {
    return this.rollupProofRep.findOne({ id }, { relations: includeTxs ? ['txs'] : undefined });
  }

  public async deleteRollupProof(id: Buffer) {
    await this.rollupProofRep.delete({ id });
  }

  /**
   * If a rollup proof is replaced by a larger aggregate, it will become "orphaned" from it's transactions.
   * This removes any rollup proofs that are no longer referenced by transactions.
   */
  public async deleteTxlessRollupProofs() {
    const orphaned = await this.rollupProofRep
      .createQueryBuilder('rollup_proof')
      .select('rollup_proof.id')
      .leftJoin('rollup_proof.txs', 'tx')
      .where('tx.rollupProof IS NULL')
      .getMany();
    await this.rollupProofRep.delete({ id: In(orphaned.map(rp => rp.id)) });
  }

  public async deleteOrphanedRollupProofs() {
    await this.rollupProofRep.delete({ rollup: IsNull() });
  }

  public async getRollupProofsBySize(numTxs: number) {
    return await this.rollupProofRep.find({
      where: { rollupSize: numTxs, rollup: null },
      relations: ['txs'],
      order: { dataStartIndex: 'ASC' },
    });
  }

  public async getNumRollupProofsBySize(numTxs: number) {
    return await this.rollupProofRep.count({
      where: { rollupSize: numTxs, rollup: null },
    });
  }

  public async getNumSettledRollups() {
    return await this.rollupRep.count({
      where: { mined: Not(IsNull()) },
    });
  }

  public async getNextRollupId() {
    const latestRollup = await this.rollupRep.findOne({ mined: Not(IsNull()) }, { order: { id: 'DESC' } });
    return latestRollup ? latestRollup.id + 1 : 0;
  }

  public async getRollup(id: number) {
    return this.rollupRep.findOne({ id }, { relations: ['rollupProof', 'rollupProof.txs'] });
  }

  public async getRollups(take?: number, skip?: number, descending = false) {
    const result = await this.rollupRep.find({
      order: { id: descending ? 'DESC' : 'ASC' },
      relations: ['rollupProof'],
      take,
      skip,
    });
    // Loading these as part of relations above leaks GB's of memory.
    // One would think the following would be much slower, but it's not actually that bad.
    for (const rollup of result) {
      rollup.rollupProof.txs = await this.txRep.find({ where: { rollupProof: rollup.rollupProof } });
    }
    return result;
  }

  public async addRollup(rollup: RollupDao) {
    // We need to erase any existing rollup first, to ensure we don't get a unique violation when inserting a
    // different rollup proof which has a one to one mapping with the rollup.
    await this.connection.transaction(async transactionalEntityManager => {
      for (const tx of rollup.rollupProof.txs) {
        await transactionalEntityManager.delete(this.txRep.target, { id: tx.id });
      }
      await transactionalEntityManager.delete(this.rollupRep.target, { id: rollup.id });
      await transactionalEntityManager.save(rollup);
    });
  }

  public async setCallData(id: number, callData: Buffer) {
    await this.rollupRep.update({ id }, { callData });
  }

  public async confirmSent(id: number, txHash: TxHash) {
    await this.rollupRep.update({ id }, { ethTxHash: txHash.toBuffer() });
  }

  public async confirmMined(
    id: number,
    gasUsed: number,
    gasPrice: bigint,
    mined: Date,
    ethTxHash: TxHash,
    txIds: Buffer[],
  ) {
    await this.connection.transaction(async transactionalEntityManager => {
      await transactionalEntityManager.update(this.txRep.target, { id: In(txIds) }, { mined });
      await transactionalEntityManager.update(
        this.rollupRep.target,
        { id },
        { mined, gasUsed, gasPrice: toBufferBE(gasPrice, 32), ethTxHash: ethTxHash.toBuffer() },
      );
    });
    return (await this.getRollup(id))!;
  }

  public getSettledRollups(from = 0) {
    return this.rollupRep.find({
      where: { id: MoreThanOrEqual(from), mined: Not(IsNull()) },
      order: { id: 'ASC' },
      relations: ['rollupProof'],
    });
  }

  public getLastSettledRollup() {
    return this.rollupRep.findOne(
      { mined: Not(IsNull()) },
      {
        order: { id: 'DESC' },
        relations: ['rollupProof'],
      },
    );
  }

  public getUnsettledRollups() {
    return this.rollupRep.find({
      where: { mined: IsNull() },
      order: { id: 'ASC' },
    });
  }

  public async deleteUnsettledRollups() {
    await this.rollupRep.delete({ mined: IsNull() });
  }

  public async getRollupByDataRoot(dataRoot: Buffer) {
    return this.rollupRep.findOne({ dataRoot });
  }

  public async getDataRootsIndex(root: Buffer) {
    // Lookup and save the proofs data root index (for old root support).
    const emptyDataRoot = Buffer.from('2708a627d38d74d478f645ec3b4e91afa325331acf1acebe9077891146b75e39', 'hex');
    if (root.equals(emptyDataRoot)) {
      return 0;
    }

    const rollup = await this.getRollupByDataRoot(root);
    if (!rollup) {
      throw new Error(`Rollup not found for merkle root: ${root.toString('hex')}`);
    }
    return rollup.id + 1;
  }
}
