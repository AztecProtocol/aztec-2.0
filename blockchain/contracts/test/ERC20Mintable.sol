// SPDX-License-Identifier: GPL-2.0-only
// Copyright 2020 Spilsbury Holdings Ltd
pragma solidity >=0.6.10 <0.8.0;

import {ERC20} from '@openzeppelin/contracts/token/ERC20/ERC20.sol';

/**
 * @dev Warning: do not deploy in real environments, for testing only
 * ERC20 contract where anybody is able to mint
 */
contract ERC20Mintable is ERC20 {
    constructor() public ERC20('Test', 'TEST') {}

    function mint(address _to, uint256 _value) public returns (bool) {
        _mint(_to, _value);
        return true;
    }
}
