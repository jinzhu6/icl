#!/bin/bash

# Check https://github.com/KratosMultiphysics/Kratos/wiki/How-to-use-Clang-Tidy-to-automatically-correct-code

script_dir="$(dirname "$0")"
mkdir -p ${script_dir}/../build
cd ${script_dir}/../build
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ..
cp ${script_dir}/run-clang-tidy.py  ${script_dir}/../build

checks=(modernize-avoid-bind \
        modernize-avoid-c-arrays \
        modernize-concat-nested-namespaces \
        modernize-deprecated-headers \
        modernize-deprecated-ios-base-aliases \
        modernize-loop-convert \
        modernize-make-shared \
        modernize-make-unique \
        modernize-pass-by-value \
        modernize-raw-string-literal \
        modernize-redundant-void-arg \
        modernize-replace-auto-ptr \
        modernize-replace-random-shuffle \
        modernize-return-braced-init-list \
        modernize-shrink-to-fit \
        modernize-unary-static-assert \
        modernize-use-auto \
        modernize-use-bool-literals \
        modernize-use-default-member-init \
        modernize-use-emplace \
        modernize-use-equals-default \
        modernize-use-equals-delete \
        modernize-use-nodiscard \
        modernize-use-noexcept \
        modernize-use-nullptr \
        modernize-use-override \
        modernize-use-trailing-return-type \
        modernize-use-transparent-functors \
        modernize-use-uncaught-exceptions \
        modernize-use-using)

for check in "${checks[@]}"
do
    echo python run-clang-tidy.py -header-filter=".*" -checks="-*,${check}" -fix
    echo git commit -m "${check}"
done
