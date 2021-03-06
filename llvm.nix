{stdenv, cmake, ninja, python, git}:
stdenv.mkDerivation rec {
    name = "llvm-${version}";
    version = "10";
    src = ./llvm-project;
    buildInputs = [ cmake ninja python git ];

    cmakeFlags = [ ''-GNinja -S../llvm -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_BUILD_TYPE:STRING='MinSizeRel' -DCMAKE_CXX_STANDARD:STRING='17' -DLLVM_TARGETS_TO_BUILD:STRING='X86;WebAssembly' -DLLVM_BUILD_LLVM_DYLIB:BOOL=OFF -DLLVM_ENABLE_TERMINFO:BOOL=OFF -DLLVM_INCLUDE_DOCS:BOOL=OFF -DLLVM_OPTIMIZED_TABLEGEN:BOOL=ON -DLLVM_INCLUDE_EXAMPLES:BOOL=OFF -DLLVM_INCLUDE_TESTS:BOOL=OFF -DLLVM_INCLUDE_BENCHMARKS:BOOL=OFF -DLLVM_ENABLE_ZLIB:BOOL=OFF -DLLVM_APPEND_VC_REV:BOOL=OFF -DLLVM_ENABLE_PROJECTS='lld' '' ];
  }
