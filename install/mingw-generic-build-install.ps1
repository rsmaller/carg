$mingwpath = gcc -print-search-dirs | findstr install:.
$mingwpath = $mingwpath.Substring(9) + "include/"
gcc -I../include -c -fpic ../src/carg_impl.c -o carg_impl.o
gcc -shared -o carg.dll carg_impl.o "-Wl,--out-implib,carg.dll.a"
Remove-Item carg_impl.o
mkdir -Force "C:\Program Files\libcarg\"
Copy-Item carg.dll "C:\Program Files\libcarg\"
$prevPath = [System.Environment]::GetEnvironmentVariable("Path", "Machine")
$newPath = "$prevPath;C:\Program Files\libcarg;"
[System.Environment]::SetEnvironmentVariable("Path", $newPath, "Machine")
Copy-Item ..\include\carg.h $mingwpath