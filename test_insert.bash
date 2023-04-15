#!/bin/bash
./server "CREATE TABLE insert_table(id int, t_name char, col1 int, col2 int);"
./server "INSERT INTO insert_table VALUES (1,'N1',1,1);"
./server "INSERT INTO insert_table VALUES (2,'N2',2,2);"
./server "INSERT INTO insert_table VALUES (3,'N3',3,3);"
./server "INSERT INTO insert_table VALUES (4,'N4',4,4);"
