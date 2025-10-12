#!/bin/bash

clang-format --verbose -i include/*.hpp
clang-format --verbose -i include/inline/*.inl
clang-format --verbose -i src/*.cpp

clang-format --verbose -i test/*.hpp
clang-format --verbose -i test/*.cpp

clang-format --verbose -i wrapper/*.hpp
clang-format --verbose -i wrapper/*.cpp
