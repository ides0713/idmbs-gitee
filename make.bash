#!/bin/bash
cd src/server/parse/ || exit
bash gen_parser.sh
cd ../../../build || exit
cmake .. -D DEBUG=1 ..
make -j 4
cd ../
