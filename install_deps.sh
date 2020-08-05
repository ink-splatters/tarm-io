#!/bin/bash

if [ ! -e build ]; then
    mkdir build
    #IntelliJ
    mkdir cmake-build-debug
    mkdir cmake-bulid-release
fi

if [ -z "$1" ] ; then
   build_type=release
else
   build_type=`echo $build_type | tr '[:upper:]' '[:lower:]'`
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

pushd .
cd build
echo "BUILD CONFIGURATION: $build_type"
conan install .. --build=missing -pr="${machine}_${build_type}" -s compiler.libcxx=libc++
popd
