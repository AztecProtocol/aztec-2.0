import BN from 'bn.js';
import * as bn128 from '@aztec/bn128';

export function extractVariables(names: string[], data: string, elementSize: number) {
    const extractData: object = {};

    names.forEach((name, index) => {
        const elementStart = index * elementSize;
        const hexDataElement = data.slice(elementStart, elementStart + elementSize);

        let dataElement: any;
        if (elementSize === 128) { // G1 element
            dataElement = hexToGroupPoint(hexDataElement.slice(0, 64), hexDataElement.slice(64, 128));
        } else if (elementSize === 64) {  // field element
            dataElement = hexToGroupScalar(hexDataElement);
        }

        extractData[name] = dataElement;
      });

    return extractData;
}

/**
 * Converts a hexadecimal input to a group point
 * @param {string} xHex hexadecimal representation of x coordinate
 * @param {string} yHex hexadecimal representation of y coordinate
 * @returns {BN} bn.js formatted version of a point on the bn128 curve
 */
 function hexToGroupPoint(xHex, yHex) {
    let x = new BN(xHex.slice(2), 16);
    let y = new BN(yHex.slice(2), 16);
    // if (!x.lt(bn128.curve.p)) {
    //     this.errors.push(errors.codes.X_TOO_BIG);
    // }
    // if (!y.lt(bn128.curve.p)) {
    //     this.errors.push(errors.codes.Y_TOO_BIG);
    // }
    x = x.toRed(bn128.curve.red);
    y = y.toRed(bn128.curve.red);
    const lhs = y.redSqr();
    const rhs = x
        .redSqr()
        .redMul(x)
        .redAdd(bn128.curve.b);
    // if (!lhs.fromRed().eq(rhs.fromRed())) {
    //     this.errors.push(errors.codes.NOT_ON_CURVE);
    // }
    return bn128.curve.point(x, y);
}

/**
 * Convert a hexadecimal input into a scalar bn.js
 *
 * @param {string} hex hex input
 * @param {boolean} canbeZero control to determine hex input can be zero
 * @returns {BN} bn.js formatted version of the scalar
 */
function hexToGroupScalar(hex, canBeZero = false) {
    const hexBN = new BN(hex.slice(2), 16);
    // if (!hexBN.lt(bn128.curve.n)) {
    //     this.errors.push(errors.codes.SCALAR_TOO_BIG);
    // }
    // if (!canBeZero && hexBN.eq(ZERO_BN)) {
    //     this.errors.push(errors.codes.SCALAR_IS_ZERO);
    // }
    return hexBN.toRed(bn128.groupReduction);
}