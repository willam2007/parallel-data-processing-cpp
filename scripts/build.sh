#!/usr/bin/env bash
set -euo pipefail
configuration="${1:-debug}"
target="${2:-all}"
case "$configuration" in
  debug) cmake_configuration=Debug ;;
  release) cmake_configuration=Release ;;
  *) echo 'Usage: bash scripts/build.sh debug|release [all|practice01|practice02|practice03|course_checks]' >&2; exit 2 ;;
esac
case "$target" in all|practice01|practice02|practice03|course_checks) ;; *) echo 'Invalid build target' >&2; exit 2 ;; esac
project_root="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$project_root"
extra_options=()
if [[ "$(uname -s)" == Darwin ]]; then
  compiler="${CXX:-$(brew --prefix llvm)/bin/clang++}"
  libomp_prefix="$(brew --prefix libomp)"
  extra_options+=("-DCMAKE_PREFIX_PATH=$libomp_prefix"
                  "-DOpenMP_CXX_INCLUDE_DIR=$libomp_prefix/include"
                  "-DOpenMP_omp_LIBRARY=$libomp_prefix/lib/libomp.dylib")
else
  compiler="${CXX:-g++}"
fi
cmake -S . -B "build/$configuration" -G Ninja \
  "-DCMAKE_BUILD_TYPE=$cmake_configuration" "-DCMAKE_CXX_COMPILER=$compiler" "${extra_options[@]}"
if [[ "$target" == all ]]; then
  cmake --build "build/$configuration" --parallel 2
else
  cmake --build "build/$configuration" --parallel 2 --target "$target"
fi
