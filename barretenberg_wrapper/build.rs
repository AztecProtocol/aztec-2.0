extern crate cmake;

fn main() {
    // Builds the project in ../barretenberg into dst
    // compile with debug mode because clang13 does not compile in release mode
    // with google benchmarks
    let mut config = cmake::Config::new("../barretenberg");
    // config.profile("Debug");

    // barretenberg currently requires manually specifying whether we're
    // compiling for an apple m1 or not, so we attempt to detect it here.
    if cfg!(all(target_os = "macos", target_arch = "aarch64")) {
        config.define("APPLE_M1", "1");
    }

    let dst = config.build();
    //println!("cargo:warning={}", dst.display());

    println!(
        "cargo:rustc-link-search={}/build/src/aztec/plonk/composer",
        dst.display()
    );
    println!(
        "cargo:rustc-link-search={}/build/src/aztec/crypto/blake2s",
        dst.display()
    );
    println!(
        "cargo:rustc-link-search={}/build/src/aztec/crypto/schnorr",
        dst.display()
    );
    println!(
        "cargo:rustc-link-search={}/build/src/aztec/ecc",
        dst.display()
    );
    println!(
        "cargo:rustc-link-search={}/build/src/aztec/env",
        dst.display()
    );
    println!(
        "cargo:rustc-link-search={}/build/src/aztec/srs",
        dst.display()
    );
    println!(
        "cargo:rustc-link-search={}/build/src/aztec/numeric",
        dst.display()
    );
    println!(
        "cargo:rustc-link-search={}/build/src/aztec/dsl",
        dst.display()
    );
    if cfg!(target_os = "macos") {
        println!("cargo:rustc-link-search=/usr/local/opt/llvm/lib");
    } else {
        println!("cargo:rustc-link-search=/usr/lib/llvm-10/lib");
    }
    println!(
        "cargo:rustc-link-search={}/build/src/aztec/plonk/transcript",
        dst.display()
    );
    println!(
        "cargo:rustc-link-search={}/build/src/aztec/stdlib/primitives",
        dst.display()
    );
    println!(
        "cargo:rustc-link-search={}/build/src/aztec/stdlib/hash/sha256",
        dst.display()
    );
    println!(
        "cargo:rustc-link-search={}/build/src/aztec/stdlib/hash/blake2s",
        dst.display()
    );
    println!(
        "cargo:rustc-link-search={}/build/src/aztec/stdlib/encryption/schnorr",
        dst.display()
    );
    println!(
        "cargo:rustc-link-search={}/build/src/aztec/stdlib/hash/pedersen",
        dst.display()
    );
    println!(
        "cargo:rustc-link-search={}/build/src/aztec/plonk/proof_system",
        dst.display()
    );
    println!(
        "cargo:rustc-link-search={}/build/src/aztec/plonk/reference_string",
        dst.display()
    );
    println!(
        "cargo:rustc-link-search={}/build/src/aztec/polynomials",
        dst.display()
    );
    println!(
        "cargo:rustc-link-search={}/build/src/aztec/crypto/keccak",
        dst.display()
    );
    println!(
        "cargo:rustc-link-search={}/build/src/aztec/crypto/pedersen",
        dst.display()
    );

    println!("cargo:rustc-link-lib=static=composer");
    println!("cargo:rustc-link-lib=static=transcript");
    println!("cargo:rustc-link-lib=static=stdlib_primitives");
    println!("cargo:rustc-link-lib=static=stdlib_sha256");
    println!("cargo:rustc-link-lib=static=stdlib_blake2s");
    println!("cargo:rustc-link-lib=static=stdlib_schnorr");
    println!("cargo:rustc-link-lib=static=stdlib_pedersen");
    println!("cargo:rustc-link-lib=static=proof_system");
    println!("cargo:rustc-link-lib=static=reference_string");
    println!("cargo:rustc-link-lib=static=polynomials");

    println!("cargo:rustc-link-lib=static=blake2s");
    println!("cargo:rustc-link-lib=static=schnorr");
    println!("cargo:rustc-link-lib=static=numeric");
    println!("cargo:rustc-link-lib=static=ecc");
    println!("cargo:rustc-link-lib=static=dsl");
    println!("cargo:rustc-link-lib=static=srs");

    println!("cargo:rustc-link-lib=static=pedersen");
    println!("cargo:rustc-link-lib=static=keccak");
    println!("cargo:rustc-link-lib=static=env");
    if cfg!(target_os = "macos") {
        println!("cargo:rustc-link-lib=omp");
        println!("cargo:rustc-link-lib=c++");
    } else {
        println!("cargo:rustc-link-lib=libomp");
        println!("cargo:rustc-link-lib=stdc++");
    }
}
