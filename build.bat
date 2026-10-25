@echo off
setlocal enabledelayedexpansion

echo ===================================================
echo     Banking Management System - Build Script
echo ===================================================

where gcc >nul 2>nul
if %ERRORLEVEL% NEQ 0 (
    if exist "C:\MinGW\bin\gcc.exe" (
        set "PATH=C:\MinGW\bin;%PATH%"
        echo [INFO] Added C:\MinGW\bin to PATH.
    ) else (
        echo [ERROR] GCC compiler not found in PATH or at C:\MinGW\bin.
        echo Please install MinGW GCC and add it to your PATH.
        pause
        exit /b 1
    )
)

echo [1/2] Compiling C source files with strict standard flags...
gcc -std=c11 -Wall -Wextra -Wpedantic -o banking.exe main.c banking.c file_handler.c

if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] Compilation failed.
    pause
    exit /b %ERRORLEVEL%
)

echo [2/2] Build successful: banking.exe
echo ===================================================
echo Starting Banking Management System...
echo ===================================================
echo.
banking.exe
