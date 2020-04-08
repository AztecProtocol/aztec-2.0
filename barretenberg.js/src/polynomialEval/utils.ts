import BN from 'bn.js';
import { toBN } from 'web3-utils';
import * as bn128 from '@aztec/bn128';

export function toRedBN(input: string): any {
    return toBN(input).toRed(bn128.groupReduction);
  }
  
export function createRedBN(input: any): any {
    return new BN(input).toRed(bn128.groupReduction);
}
  
export function redDivision(numerator: any, denominator: any): BN {
    return (numerator.redMul(denominator.redInvm())).fromRed();
}

/**
 * Used to calculate expressions of the form:
 * (x + z + ay)
 * 
 * Takes inputs in non reduction form, returns the result in reduction form
 */
export function computeBracket(x: any, a: any, y: any, z: any): any {
    return toRedBN(x).redAdd(toRedBN(z)).redAdd(toRedBN(a).toRedMul(toRedBN(y)));
}