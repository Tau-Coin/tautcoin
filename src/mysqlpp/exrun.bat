@echo off
set PROG=%1
if not exist %PROG% set PROG=%1.exe
if not exist %PROG% set PROG=Debug\%1
if not exist %PROG% set PROG=vc2008\x64\Debug\%1.exe
if not exist %PROG% set PROG=vc2008\Debug\%1.exe
if not exist %PROG% set PROG=vc2005\Debug\%1.exe
if not exist %PROG% set PROG=vc2003\Debug\%1.exe
if not exist %PROG% exit
shift

set PATH=Debug;vc2008\x64\Debug;vc2008\Debug;vc2005\Debug;vc2003\Debug;%PATH%
echo Running %PROG%...
%PROG% %1 %2 %3 %4 %5 %6 %7 %8 %9
