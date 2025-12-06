#!/bin/bash

COPT_EXE=$1

./${COPT_EXE} 0 3000 200;      echo ""
./${COPT_EXE} 1 300000 20000;  echo ""
./${COPT_EXE} 2 20 200000000;  echo ""
./${COPT_EXE} 3 1600 1;        echo ""
