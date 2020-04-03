const inputString = '13 09 e5 2b 1c 23 ef 58 9b 93 c1 65 df 57 5d ec 87 2a 51 7b ee 6e 1c ce 9a df 6f 09 cf 82 47 93 15 ad 6c a0 00 c0 eb 30 a6 67 fb 65 3a 20 4d 7b b1 09 4a 71 34 9d 82 7c ae 8a 5d 68 44 96 3c e2 01 aa aa ba 5f b9 2b 05 c5 87 da d6 bb 36 58 9c c3 93 70 8d f5 30 93 b3 b6 1a cc 73 62 50 5b 27 20 74 ef 96 3b d0 37 16 ee 70 ee 2c 22 91 7a 16 f0 dd bd e4 a3 35 57 1f dd 56 51 87 12 cd 33 19 1c fc 0c 57 0f 43 52 e5 81 ae 72 da 96 d3 c8 bf 73 80 a2 e6 c7 40 0e 87 9d 40 36 77 97 4d 71 28 14 6e f1 a2 20 67 10 9d 01 b4 8c 23 a6 4c 0b a4 3a 44 16 70 01 36 b2 ff 56 11 f4 56 b3 7b e5 2c 2b f6 51 ee 8d 47 c7 cf 85 a8 2f 43 42 3c 82 a9 95 c5 b0 dc 83 93 32 f5 8e 10 cb 85 12 cf a7 8d 28 57 c8 49 dd 81 ed fb 45 cf 7c b5 86 dc a8 37 64 31 54 37 36 fe 49 17 4d 89 25 1f a8 0f 75 22 2e 5e c5 8a d5 70 73 ab d1 dc 78 a7 1c 66 1d da b0 ab 04 b2 a2 e0 07 d3 4a 82 17 cd 49 66 47 dc 1e 3e 1a 69 d2 84 5b 0a 57 f5 9e c3 d8 7f b2 97 40 76 c4 98 22 cd 1b 1f 49 76 10 3d d1 f3 52 c8 1a 61 1b cb 05 24 8b 35 25 5c 72 5b 93 52 2d f9 b8 ff 29 bb 2d 6c b9 95 b2 5c cb d8 20 10 93 de 16 65 3b 1f a8 29 07 dd 3e 5a b3 10 58 c7 32 b5 ee be 72 9d 12 29 a1 37 f0 b0 6a b4 70 56 d7 ec 10 12 56 89 b3 3b 22 14 58 40 11 42 0f 84 66 e6 a0 2e ba d9 a0 cb 71 ea b4 06 fc de b1 44 6e 65 21 4a 30 6f 62 42 fa fd bf d1 ee 94 fa 93 2d 5e a1 22 13 4a f1 f8 b1 3e 86 c4 e6 62 63 55 cb bf 0b d0 1e 9a 4f ce 52 ee 54 83 2e c5 96 2c 28 0c 5e ad 6a 27 6b 0f 77 ae 67 b3 32 ad 51 4e db ab 06 fa aa 42 c8 2a 47 2e f1 ff b2 d6 ab 70 5a 23 fc ac 0e 89 54 5e bd 74 cc 67 ff 3e 91 1f 15 72 2f d9 45 f1 da 9a 0b 49 74 c5 37 97 18 2e a2 eb fb 87 f2 80 de 64 bc ce 0a d0 59 c4 d1 ae d8 3a 27 d6 0b 92 ea e9 cc 8d f6 88 e2 84 48 c5 5d b9 c7 eb 4a 59 0f a4 c4 c0 76 37 3c 09 31 15 73 fa 18 40 e5 2b f6 6b cd 4a 63 2a 5d 14 4a f7 af 7d 58 09 85 ed 49 f6 29 5b 1b 5c 72 e9 f6 83 f0 84 24 38 4d f4 be 5b 72 86 12 39 a6 2f f0 7d 7c b6 61 21 03 e0 f6 13 0f ef 40 b8 08 65 68 f3 17 1f 0d 13 b4 fc bc 18 aa 1e bd ea 16 bc b4 c1 2b 62 b0 d4 a1 13 0b ce e6 32 4f 9c a2 b9 bc cc 9c 9f 04 06 e8 59 2f a1 52 0e 14 cb c9 e0 75 6a f1 c1 0f c6 64 ef f1 9e de 46 ef f2 44 7c 7a fb 88 09 21 9e b7 cd 0e df 4a 43 b4 c8 d4 4d cb c8 b0 fd 23 24 94 86 a9 b2 d8 b1 d6 07 f9 8c 0b 00 e5 3e 24 f0 21 8b 48 02 9c d1 61 f6 0d 68 b8 f6 ba 40 08 c8 6d 5a 1d d4 76 7a 6b a5 b0 26 7f cb 86 34 1f 66 c2 f8 62 0c 47 69 65 bc 88 7b 37 26 2c 0d 69 ef 0c 19 05 50 8b 16 92 43 59 11 aa 45 f3 f0 00 d2 a8 fe f3 cb 8d fa 16 55 10 63 1c 66 07 3c 60 21 94 93 56 8d aa a6 ed 3d 1b 08 68 87 4e 79';

export const dummyProof = inputString.replace(/\s/g,'');

export const proofVariables = {
    verificationKey: '',
    circuitSize: '256', // 256
    publicInput: '1',
}

export const expectedChallenges = {
    expectedInit: '0x2a7713a2494b9aed2d62f46c3aea18dc6fc9b7b61ef0be5632640cfeadb3f159',
    expectedBeta: '0x2851a6d936165567eb584f748eacb4c4294dc54e031b301f1a192b970adef5ea',
    expectedGamma: '',
    expectedAlpha: '0x051004ed71bab47acf579367a915f64375a139f28fac94d66e416526f2f29e28',
    expectedZ: '0x2ddb3951724f506252bbb1fedaf6db153eac44c4ec7d64ac5b5c519ca59983cd',
    expectedV0: '0x098565e3172a7262bee92c71ddd51e249158056334b08e578b48ebd118609a85i',
    expectedV1: '0x19505f3e85bf4db7231120f1d7fe2868a08dca9b3e70276ec9aae265bd5cbdd5i',
    expectedV2: '0x22baafadede1fe2919b20f8f59ac70618916e1ca2a827f4da303a98a4fc03c67i',
    expectedV3: '0x00580c2dcff1bbfac4129b80d6e10b80818729953f389a9ac4676226588bc2f3i',
    expectedV4: '0x159639eedf8e0a111322d91bbc0cacbd207a75bf8f23c7b9929df76070eac60di',
    expectedV5: '0x21634985f425cb696b3105b2da8b0f8ffc3158faf90fabc969783f792ac0894bi',
    expectedV6: '0x01d6a9a8dfe5d2514a33da9bb1996eff8cc6abe9fbe3cb6c3001301fadcfc8d3i',
    expectedV7: '0x0aecb8daed84acb42fd0d23a60390fc7e7fb000323d691deee8be6ab3d6a718bi',
    expectedV8: '0x085f05d7562e0d1efae83bd7f58888d48c192604554cdd10c40d9fda655fe378i',
    expectedV9: '0x290536700f738ff5f6de721f075b2041ed60e17ce5ff74c9527065c41b0b4f61i',
    expectedU: '',
}


export const dummyProofBuffer: Buffer = Buffer.from(dummyProof);
