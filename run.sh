#!/bin/bash

cd build
cmake ..
make

cp nexis_compiler ../nexis_compiler

./nexis_compiler ../example.nx
