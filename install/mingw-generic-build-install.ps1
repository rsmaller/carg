$mingwpath = gcc -print-search-dirs | findstr ^install:.*$
$mingwpath = $mingwpath.Substring(9) + "include/"
echo $mingwpath
gcc -I../include -c -fpic ../src/carg_impl.c
gcc -shared -o carg.dll carg_impl.o
gendef carg.dll
dlltool -d carg.def -l carg.lib -D carg.dll
rm carg_impl.o
rm carg.def
cp carg.dll C:\Windows\System32\
cp ..\include\carg.h $mingwpath