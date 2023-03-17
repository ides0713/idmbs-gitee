#!/bin/bash
cd src/server/parse/
bash gen_parser.sh
cd ../../../build
cmake ..
make
cd ../
