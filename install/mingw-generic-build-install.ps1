$mingwpath = gcc -print-search-dirs | findstr install:.
$mingwpath = $mingwpath.Substring(9) + "include/"
gcc -I../include -c -fpic ../src/carg_impl.c
gcc -shared -o carg.dll carg_impl.o
gendef carg.dll
dlltool -d carg.def -l carg.lib -D carg.dll
Remove-Item carg_impl.o
Remove-Item carg.def
mkdir -Force "C:\Program Files\libcarg\"
Copy-Item carg.dll "C:\Program Files\libcarg\"
$prevPath = [System.Environment]::GetEnvironmentVariable("Path", "Machine")
$newPath = "$prevPath;C:\Program Files\libcarg;"
[System.Environment]::SetEnvironmentVariable("Path", $newPath, "Machine")
Copy-Item ..\include\carg.h $mingwpath