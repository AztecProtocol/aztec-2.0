import { AssetId } from 'barretenberg/asset';
import { Blockchain, PriceFeed } from 'barretenberg/blockchain';
import { EthPriceFeed } from 'blockchain';
import { PriceTracker } from './price_tracker';

type Mockify<T> = {
  [P in keyof T]: jest.Mock;
};

describe('price tracker', () => {
  const assetIds = [AssetId.ETH, AssetId.DAI];
  const refreshInterval = 500;
  const recordDuration = 1000;
  const initialTime = 1618222000000;
  let dateSpy: jest.SpyInstance<number>;
  let gasPriceFeed: Mockify<PriceFeed>;
  let tokenPriceFeed: Mockify<PriceFeed>;
  let blockchain: Mockify<Blockchain>;
  let priceTracker!: PriceTracker;

  beforeEach(() => {
    dateSpy = jest.spyOn(Date, 'now').mockImplementation(() => initialTime);

    gasPriceFeed = {
      price: jest.fn().mockResolvedValue(123n),
    } as any;

    tokenPriceFeed = {
      price: jest.fn().mockResolvedValue(456n),
    } as any;

    blockchain = {
      getGasPriceFeed: jest.fn().mockReturnValue(gasPriceFeed),
      getPriceFeed: jest.fn().mockImplementation((assetId: AssetId) => {
        if (assetId === AssetId.ETH) {
          return new EthPriceFeed();
        }
        return tokenPriceFeed;
      }),
    } as any;

    priceTracker = new PriceTracker(blockchain, assetIds, refreshInterval, recordDuration);
  });

  afterEach(async () => {
    dateSpy.mockRestore();
    await priceTracker.stop();
  });

  it('record prices', async () => {
    await priceTracker.start();
    expect(priceTracker.getGasPrice()).toBe(123n);
    expect(priceTracker.getAssetPrice(AssetId.ETH)).toBe(10n ** 18n);
    expect(priceTracker.getAssetPrice(AssetId.DAI)).toBe(456n);
  });

  it('record and get historical prices', async () => {
    await priceTracker.start();
    gasPriceFeed.price.mockResolvedValue(789n);
    tokenPriceFeed.price.mockResolvedValue(876n);
    dateSpy.mockImplementation(() => initialTime + refreshInterval);
    await new Promise(resolve => setTimeout(resolve, refreshInterval));

    expect(priceTracker.getGasPrice()).toBe(789n);
    expect(priceTracker.getAssetPrice(AssetId.ETH)).toBe(10n ** 18n);
    expect(priceTracker.getAssetPrice(AssetId.DAI)).toBe(876n);

    const msAgo = refreshInterval;
    expect(priceTracker.getGasPrice(msAgo)).toBe(123n);
    expect(priceTracker.getAssetPrice(AssetId.ETH, msAgo)).toBe(10n ** 18n);
    expect(priceTracker.getAssetPrice(AssetId.DAI, msAgo)).toBe(456n);
  });

  it('return 0 if historical price is not set', async () => {
    await priceTracker.start();
    gasPriceFeed.price.mockResolvedValue(789n);
    tokenPriceFeed.price.mockResolvedValue(876n);
    dateSpy.mockImplementation(() => initialTime + refreshInterval);
    await new Promise(resolve => setTimeout(resolve, refreshInterval));

    const msAgo = refreshInterval + 1;
    expect(priceTracker.getGasPrice(msAgo)).toBe(0n);
    expect(priceTracker.getAssetPrice(AssetId.ETH, msAgo)).toBe(0n);
    expect(priceTracker.getAssetPrice(AssetId.DAI, msAgo)).toBe(0n);
  });

  it('return 0 if not started yet', async () => {
    expect(priceTracker.getGasPrice()).toBe(0n);
    expect(priceTracker.getAssetPrice(AssetId.ETH)).toBe(0n);
    expect(priceTracker.getAssetPrice(AssetId.DAI)).toBe(0n);
  });

  it('return min historical price within record duration', async () => {
    expect(priceTracker.getMinGasPrice()).toBe(0n);
    expect(priceTracker.getMinAssetPrice(AssetId.DAI)).toBe(0n);

    gasPriceFeed.price.mockResolvedValue(10n);
    tokenPriceFeed.price.mockResolvedValue(3n);
    await priceTracker.start();

    expect(priceTracker.getMinGasPrice()).toBe(10n);
    expect(priceTracker.getMinAssetPrice(AssetId.DAI)).toBe(3n);

    gasPriceFeed.price.mockResolvedValue(30n);
    tokenPriceFeed.price.mockResolvedValue(2n);
    await new Promise(resolve => setTimeout(resolve, refreshInterval));

    expect(priceTracker.getMinGasPrice()).toBe(10n);
    expect(priceTracker.getMinAssetPrice(AssetId.DAI)).toBe(2n);

    gasPriceFeed.price.mockResolvedValue(20n);
    tokenPriceFeed.price.mockResolvedValue(4n);
    await new Promise(resolve => setTimeout(resolve, refreshInterval));

    expect(priceTracker.getMinGasPrice()).toBe(10n);
    expect(priceTracker.getMinAssetPrice(AssetId.DAI)).toBe(2n);

    gasPriceFeed.price.mockResolvedValue(40n);
    tokenPriceFeed.price.mockResolvedValue(3n);
    await new Promise(resolve => setTimeout(resolve, refreshInterval));

    expect(priceTracker.getMinGasPrice()).toBe(20n);
    expect(priceTracker.getMinAssetPrice(AssetId.DAI)).toBe(2n);
  });
});
