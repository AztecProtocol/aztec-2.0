import PolynomialEvalUtils from './PolynomialEvalUtils';
import decodeProof from '../decoder';


export class Verifier extends PolynomialEvalUtils {
  public data !: object;

  constructor(proofData: string) {
    super();
    this.data = decodeProof(proofData);
    // this.extractProof(proofData);
    // this.validateInputs(proofData);
    // const { beta, gamma, alpha, epsilon, v, u } = this.computeChallenges();
  }

  public validateInputs(data: string) {}

  public computePolynomialEvaluations() {}

  public verifyProof() {}

}
