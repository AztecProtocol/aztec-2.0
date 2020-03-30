import PolynomialEvalUtils from './PolynomialEvalUtils';
import BaseVerifier from './BaseVerifier';


export class Verifier extends BaseVerifier {
  public data !: object;
  public G1Points !: any[];
  public fieldElements !: any[];

  constructor(proofData: string) {
    super();

    const { data, G1Points, fieldElements } = this.decodeProof(proofData);
    this.data = data;
    this.G1Points = G1Points;
    this.fieldElements = fieldElements;

    this.validateInputs(this.G1Points, this.fieldElements);
    // const { beta, gamma, alpha, epsilon, v, u } = this.computeChallenges();
  }


  public computePolynomialEvaluations() {}

  public verifyProof() {}

}
