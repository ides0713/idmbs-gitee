#!/bin/bash
./server "CREATE TABLE insert_table(id int, t_name char, col1 int, col2 int);"
./server "INSERT INTO insert_table VALUES (1,'N1',1,1);"
