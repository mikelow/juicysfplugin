@echo off
set BUILD_DIR="../../build"
set TARGET_TYPE=Release

cd /d %BUILD_DIR%

cmake -A x64 ../ -D CMAKE_BUILD_TYPE=%TARGET_TYPE% && cmake --build . --config %TARGET_TYPE%