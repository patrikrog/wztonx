#!/usr/bin/env sh
set -x

make clean && make && lldb -- ./nlwtn Character.wz
