#!/bin/sh
set -e
cmake --build build
./build/run_sunspider_like kraken-1.0/
./build/run_sunspider_like kraken-1.1/
./build/run_sunspider_like sunspider-1.0/
./build/run_octane
