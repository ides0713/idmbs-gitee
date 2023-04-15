#!/bin/bash
./server "CREATE TABLE Select_tables_1(id int, age int, u_name char);"
./server "INSERT INTO Select_tables_1 VALUES (1,18,'a');"
./server "INSERT INTO Select_tables_1 VALUES (2,15,'b');"
./server "INSERT INTO Select_tables_1 VALUES (1,20,'a');"
./server "INSERT INTO Select_tables_1 VALUES (2,21,'c');"
./server "INSERT INTO Select_tables_1 VALUES (1,35,'a');"
./server "INSERT INTO Select_tables_1 VALUES (2,37,'a');"
./server "SELECT * FROM Select_tables_1 WHERE id=1;"

