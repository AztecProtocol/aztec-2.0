import BN from 'bn.js';

// TODO: update buffer string to match test proof
const inputString =
  '13 09 e5 2b 1c 23 ef 58 9b 93 c1 65 df 57 5d ec 87 2a 51 7b ee 6e 1c ce 9a df 6f 09 cf 82 47 93 15 ad 6c a0 00 c0 eb 30 a6 67 fb 65 3a 20 4d 7b b1 09 4a 71 34 9d 82 7c ae 8a 5d 68 44 96 3c e2 01 aa aa ba 5f b9 2b 05 c5 87 da d6 bb 36 58 9c c3 93 70 8d f5 30 93 b3 b6 1a cc 73 62 50 5b 27 20 74 ef 96 3b d0 37 16 ee 70 ee 2c 22 91 7a 16 f0 dd bd e4 a3 35 57 1f dd 56 51 87 12 cd 33 19 1c fc 0c 57 0f 43 52 e5 81 ae 72 da 96 d3 c8 bf 73 80 a2 e6 c7 40 0e 87 9d 40 36 77 97 4d 71 28 14 6e f1 a2 20 67 10 9d 01 b4 8c 23 a6 4c 0b a4 3a 44 16 70 01 36 b2 ff 56 11 f4 56 b3 7b e5 2c 2b f6 51 ee 8d 47 c7 cf 85 a8 2f 43 42 3c 82 a9 95 c5 b0 dc 83 93 32 f5 8e 10 cb 85 12 cf a7 8d 28 57 c8 49 dd 81 ed fb 45 cf 7c b5 86 dc a8 37 64 31 54 37 36 fe 49 17 4d 89 25 1f a8 0f 75 22 2e 5e c5 8a d5 70 73 ab d1 dc 78 a7 1c 66 1d da b0 ab 04 b2 a2 e0 07 d3 4a 82 17 cd 49 66 47 dc 1e 3e 1a 69 d2 84 5b 0a 57 f5 9e c3 d8 7f b2 97 40 76 c4 98 22 cd 1b 1f 49 76 10 3d d1 f3 52 c8 1a 61 1b cb 05 24 8b 35 25 5c 72 5b 93 52 2d f9 b8 ff 29 bb 2d 6c b9 95 b2 5c cb d8 20 10 93 de 16 65 3b 1f a8 29 07 dd 3e 5a b3 10 58 c7 32 b5 ee be 72 9d 12 29 a1 37 f0 b0 6a b4 70 56 d7 ec 10 12 56 89 b3 3b 22 14 58 40 11 42 0f 84 66 e6 a0 2e ba d9 a0 cb 71 ea b4 06 fc de b1 44 6e 65 21 4a 30 6f 62 42 fa fd bf d1 ee 94 fa 93 2d 5e a1 22 13 4a f1 f8 b1 3e 86 c4 e6 62 63 55 cb bf 0b d0 1e 9a 4f ce 52 ee 54 83 2e c5 96 2c 28 0c 5e ad 6a 27 6b 0f 77 ae 67 b3 32 ad 51 4e db ab 06 fa aa 42 c8 2a 47 2e f1 ff b2 d6 ab 70 5a 23 fc ac 0e 89 54 5e bd 74 cc 67 ff 3e 91 1f 15 72 2f d9 45 f1 da 9a 0b 49 74 c5 37 97 18 2e a2 eb fb 87 f2 80 de 64 bc ce 0a d0 59 c4 d1 ae d8 3a 27 d6 0b 92 ea e9 cc 8d f6 88 e2 84 48 c5 5d b9 c7 eb 4a 59 0f a4 c4 c0 76 37 3c 09 31 15 73 fa 18 40 e5 2b f6 6b cd 4a 63 2a 5d 14 4a f7 af 7d 58 09 85 ed 49 f6 29 5b 1b 5c 72 e9 f6 83 f0 84 24 38 4d f4 be 5b 72 86 12 39 a6 2f f0 7d 7c b6 61 21 03 e0 f6 13 0f ef 40 b8 08 65 68 f3 17 1f 0d 13 b4 fc bc 18 aa 1e bd ea 16 bc b4 c1 2b 62 b0 d4 a1 13 0b ce e6 32 4f 9c a2 b9 bc cc 9c 9f 04 06 e8 59 2f a1 52 0e 14 cb c9 e0 75 6a f1 c1 0f c6 64 ef f1 9e de 46 ef f2 44 7c 7a fb 88 09 21 9e b7 cd 0e df 4a 43 b4 c8 d4 4d cb c8 b0 fd 23 24 94 86 a9 b2 d8 b1 d6 07 f9 8c 0b 00 e5 3e 24 f0 21 8b 48 02 9c d1 61 f6 0d 68 b8 f6 ba 40 08 c8 6d 5a 1d d4 76 7a 6b a5 b0 26 7f cb 86 34 1f 66 c2 f8 62 0c 47 69 65 bc 88 7b 37 26 2c 0d 69 ef 0c 19 05 50 8b 16 92 43 59 11 aa 45 f3 f0 00 d2 a8 fe f3 cb 8d fa 16 55 10 63 1c 66 07 3c 60 21 94 93 56 8d aa a6 ed 3d 1b 08 68 87 4e 79';

export const dummyProof = inputString.replace(/\s/g, '');

export const proofVariables = {
  verificationKey: '',
  circuitSize: new BN(256),
  numPublicInputs: new BN(1),
  publicInputs: [`0x${(new BN(456)).toString(16)}`],
  aCommit: '0x0e24f35b9f2df4eb7c4ba18236935fbf8bc8c7b6b52b315650d05198017939ac28e990d0d01996766774d37656b71c71dff182df3617d4ca7180264b815d7382',
  bCommit: '0xa093a435555f3fc8ecbb3292a8bd98086fb9348bb4457e90806eb9f1713998702c50ef1de5fd9e6aaa15a9e1b48d003cf254e5063cd0a2f97a72b4f8354cd8a',
  cCommit: '0x193341758d4a176c3cb5d135dfb06746b62f77acbefc43246340f6afdcf9b95e131445dfa496875360d375635ea950909acdc865feab9465537dffffb9a402fd',
  zCommit: '0x04cd3e6dfc01427ff62394ea90480542c6f4e9a9ce91e2d971b9ab5eac00c23721bddf0b2500281f9162f6feb8d2065888c693efaa7714a40680466ec4524e71',
  tlowCommit: '0x1e0b4a150c41279691bba05b48110d84d193c525289f34999ff868b55cd4387221ecb393e1824c1a265a4ae467a22038326c099b7e30a09d28e6c989b19191ed',
  tmidCommit: '0x17e379ea5744c246d7c80fc5928dc34aa21b2dc0b888c4ea6a7a24fcd4f366e40cc03b532c0b3fb280dedb62d6fb5694d1aa69047ffc014288861df0a30ef07b',
  thighCommit: '0x11329b071c99101df51ccf559c328a45df6731da19785bca297f0393b119415e05343912d5cc2f9808cd2f58ac9a9a7ba6a89448f6b8fcdeead1035550932ded',
  aBar: '0x0c18380d815de5135f7c11f22e6521bdddbf88f19d439dd94997d067ebd379a2',
  bBar: '0x208e404869aceb372c60bd194bd95e4ea0ea0d92c9875dccb2d024df82f89f2b',
  cBar: '0x0d0f71006c55d6e586de927f9eda4af2218d30825baf003f02f4a92023d50f78',
  WzBar: '0x1c8008461a416cda024be4ef650f94a58bbc124ba9ea04007a08a690c2f9a875',
  zwBar: '0x197e41847bcb33d961393f1d8fce7e3e9e339a54134b19f32c0e09df203f6673',
  sigma1Bar: '0x02b1aa3f32b4f41a8e51861bb4417461c4d8a81f91dbcfc28d6bccb7194b5398',
  sigma2Bar: '0x1a459028f36659547665cea8d79ff22d4bdb3883e15cdf4a74374be7308d2e76',
  rBar: '0x1d10329493bd494f0c229adb8877df2d34b6d55b80e337febf3876e4bd164129',
  t: '0x2d3fe98b9b385133dba228c898f326d324399d221a7dfd002d165d10d343cb87',

};

export const expectedChallenges = {
  expectedInit: '0x5adb62152a7d3b16e5b33a22bc6b713997fd9ffe98aa2ee7764602929db3f15a',
  expectedBeta: '0x694561f6377c67b5fb98b306c5f5646d2f368ede6cb7b30bd63dde9012803017',
  expectedGamma: '0x9a9c139c975983e86a9a9fe6e6eb24528f94ff9cc187ae442509c0bba773792d',
  expectedAlpha: '0xe7e33ca57cb9099226f92e0810fb928c101adbaaeea09e9bc90a6d4dc89547a5',
  expectedZeta: '0x28f30c51adfdfed14abd679c0446ecaf6586fce3035c5c5ba7394745aa5a8bcf',
};

export const expectedVChallenges = [
  '0x55ea84efc512ad8ddf76c4e879efe5621c0e425a4f61ce6ef1d1dd0932b22513',
  '0x834b7700acb7b964b15fd6a6505c6bbc955e89210f86d6456cadd4405a961ce5',
  '0x0be028329679bd5da1770a9e27cec9c7542f16361df04815b2c2b71b1eadbb1c',
  '0x9bd8d0611de8b70bcaacdbd361c3ea1290d6cfec20e7e79405e21d730cf6ba5a',
  '0x39530566709581b1a016c058e35e7cb7f9c1245ab388c07b8933a393190e2a39',
  '0x60f68eb2d92a379284c60b61e484b8e9a80f98a35749811dd431d99e2759010b',
  '0x03d90e0994e4bf23669e6a4360490e19b9682b99472d0fde77acf59220a44b10',
  '0x3f310dfc08084b224a5325b77da125ee099c9a3dd8967b8ac903d05f01406610',
  '0x949be467b9bbf03f2d59342f4207ee56208f3cfbf289d7ba5744da03d9e76683',
  '0xda602081531a6b02134c11638bec07cc908facb4470cb03d0cc322af0680853f',
];

// TODO: complete to match prover
export const expectedZeroEval = '';
export const expectedLagrangeEval = '';
export const expectedPublicInputEval = '';

export const dummyProofBuffer: Buffer = Buffer.from(dummyProof);