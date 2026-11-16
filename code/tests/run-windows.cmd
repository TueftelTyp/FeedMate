@echo off
setlocal
if not exist tests\build mkdir tests\build
rem Requires Zig 0.13.0 on PATH. Run from the project root.
zig c++ -std=c++17 -Dlocaltime_r=feedmate_localtime_r -include tests/windows_time.h tests/controller_test.cpp -o tests/build/controller_test.exe
if errorlevel 1 exit /b 1
tests\build\controller_test.exe
