#!/bin/bash

clang-format --verbose -i include/*.hpp
clang-format --verbose -i include/inline/*.inl
clang-format --verbose -i src/*.cpp
