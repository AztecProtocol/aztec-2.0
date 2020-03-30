import { BarretenbergWasm } from '../../src/wasm';
import PolynomialEvalUtils from './PolynomialEvalUtils';

export class Verifier extends PolynomialEvalUtils {
    constructor(proofData: string) {
        super();

        this.extractProof(proofData);
        this.validateInputs(proofData);
        // const { beta, gamma, alpha, epsilon, v, u } = this.computeChallenges();
    }

    public extractProof(proofData: string) {
    };

    public validateInputs(proofData: string) {}

    public computePolynomialEvaluations() {}

    public verifyProof() {}
}