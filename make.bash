#!/bin/bash
cd server/parse/
bash gen_parser.sh
cd ../../build
cmake ..
make
cd ../
