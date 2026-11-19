#!/bin/bash
clear
echo "deleting..."
rm -rf export/*
mkdir export/textures
echo "done! compiling..."
if ! g++ -std=c++17 main.cpp vector.cpp lodepng.c ; then
  echo " failed!!!"
  exit 1
fi
echo "done! running..."
echo "********************"
./a.exe
echo "********************"
echo "finished!"