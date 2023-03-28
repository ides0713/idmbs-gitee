#!/bin/bash
cd src/server/parse/
bash gen_parser.sh
cd ../../../build
cmake .. -D DEBUG=1 ..
make
cd ../
