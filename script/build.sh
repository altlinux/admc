#!/bin/bash 
set -euxo pipefail

cp -r /app/ $HOME/app 
cd $HOME/app 
mkdir build 
cd build && cmake ..
make -j `nproc`
