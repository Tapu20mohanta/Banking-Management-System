#!/usr/bin/env bash
set -e

echo "==================================================="
echo "    Banking Management System - Build Script"
echo "==================================================="

if ! command -v gcc &> /dev/null; then
    echo "[ERROR] gcc could not be found. Please install GCC."
    exit 1
fi

echo "[1/2] Compiling C source files with strict C11 flags..."
gcc -std=c11 -Wall -Wextra -Wpedantic -o banking main.c banking.c file_handler.c

echo "[2/2] Build successful: ./banking"
echo "==================================================="
echo "Starting Banking Management System..."
echo "==================================================="
echo ""
./banking
