import * as bn128 from '@aztec/bn128';
import { constants, errors } from '@aztec/dev-utils';
import BN from 'bn.js';

const { AztecError } = errors;

export default class ProofUtils {
    /**
     * Converts a hexadecimal input to a group point
     * @param {string} xHex hexadecimal representation of x coordinate
     * @param {string} yHex hexadecimal representation of y coordinate
     * @returns {BN} bn.js formatted version of a point on the bn128 curve, in the 
     * reduction context
     */
    static hexToGroupPoint(xHex: string, yHex: string) {
        let x: any = new BN(xHex, 16);
        let y: any = new BN(yHex, 16);
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
     * @returns {BN} bn.js formatted version of the scalar, in the reduction 
     * context
     */
    static hexToGroupScalar(hex, canBeZero = false) {
        const hexBN = new BN(hex, 16);
        // if (!hexBN.lt(bn128.curve.n)) {
        //     this.errors.push(errors.codes.SCALAR_TOO_BIG);
        // }
        // if (!canBeZero && hexBN.eq(ZERO_BN)) {
        //     this.errors.push(errors.codes.SCALAR_IS_ZERO);
        // }
        return hexBN.toRed(bn128.groupReduction);
    }

    /**
     * Validate point is on curve
     *
     * @param {BN[]} point bn.js format of a point on the curve
     */
    static validatePointOnCurve(point) {
        const lhs = point.y.redSqr();
        const rhs = point.x
            .redSqr()
            .redMul(point.x)
            .redAdd(bn128.curve.b);
        const isOnCurve = lhs.fromRed().eq(rhs.fromRed());

        if (!isOnCurve) {
            throw new AztecError(errors.codes.NOT_ON_CURVE, {
                message: 'A proof G1 point is not on the curve',
                point
            })
        }
    }

    /**
     * Validate field element is part of the field
     * 
     * @param fieldElement 
     */
    static validateElement(fieldElement) {
        const isValid = fieldElement.fromRed().lt(bn128.curve.n);
        if (!isValid) {
            throw new AztecError('BAD_FIELD_ELEMENT', {
                message: 'A field element is not part of the expected field',
                fieldElement
            })
        }
    }
}



