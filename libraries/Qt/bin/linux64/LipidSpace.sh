#!/usr/bin/env bash

LD_LIBRARY_PATH=$PWD:$LD_LIBRARY_PATH
export LD_LIBRARY_PATH
export XDG_SESSION_TYPE=xce
/bin/bash -c '"$PWD/LipidSpace"'
