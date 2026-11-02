#!/usr/bin/env bash

clang client.c -Iinclude -o client
clang server.c -Iinclude -o server

echo "build compiled"
