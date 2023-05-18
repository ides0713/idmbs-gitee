#!/bin/bash
cd bin/sys || exit
rm -rf `ls -a|grep -E "*.table"`
rm -rf `ls -a|grep -E "*.data"`
rm -rf `ls -a|grep -E "*.index"`
rm clog
cd ../../
echo '' > debug.log