#!/usr/bin/env node
import { ContractFactory, Signer } from 'ethers';
import MockPriceFeed from '../artifacts/contracts/test/MockPriceFeed.sol/MockPriceFeed.json';

export async function deployPriceFeed(signer: Signer, initialPrice = 480000000000000n) {
  console.error('Deploying MockPriceFeed...');
  const priceFeedLibrary = new ContractFactory(MockPriceFeed.abi, MockPriceFeed.bytecode, signer);
  const priceFeed = await priceFeedLibrary.deploy(initialPrice);
  console.error(`MockPriceFeed contract address: ${priceFeed.address}. Initial price: ${initialPrice}.`);

  return priceFeed;
}
