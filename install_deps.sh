#!/bin/bash

set -x

if [ ! -e build ]; then
    mkdir build
    #IntelliJ
    mkdir cmake-build-debug
    mkdir cmake-bulid-release
fi

unameOut="$(uname -s)"
case "${unameOut}" in
    Linux*)     machine=linux;;
    Darwin*)    machine=macos;;
    *)          machine="UNKNOWN:${unameOut}"
esac

if [[ "${machine}" != "macos" && "${machine}" != "linux" ]]; then
  echo "Error: arch is unsupported: ${unameOut}"
  exit 1
fi

script_dir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
mkdir -p ~/.conan/profiles
conan_profile_path=${script_dir}/conan_profiles
cp  ${conan_profile_path}/${machine}_* ~/.conan/profiles

function build() {

pushd .
cd build
echo "BUILD CONFIGURATION: $1; profile: ${machine}_$1"
conan install .. --build=missing -pr="${machine}_$1" -s compiler.libcxx=libc++
popd

}

build debug
build release
