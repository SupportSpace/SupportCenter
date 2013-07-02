cd src
PUSHD %1
call bin\setenv.bat F:\WINDDK\6000\ %2 %3
echo Building through DDK
POPD
build -cZ