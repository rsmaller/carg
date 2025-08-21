$msvcver = (Get-ChildItem "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Tools\MSVC" -Name | Sort-Object Name -Descending)[0]
$msvcpath = "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Tools\MSVC\" + $msvcver + "\include"
Copy-Item ../include/carg.h $msvcpath
cl /LD ../src/carg_impl.c /Fe:carg.dll
mkdir -Force "C:\Program Files\libcarg\"
Copy-Item carg.dll "C:\Program Files\libcarg\"
$prevPath = [System.Environment]::GetEnvironmentVariable("Path", "Machine")
$newPath = "$prevPath;C:\Program Files\libcarg;"
[System.Environment]::SetEnvironmentVariable("Path", $newPath, "Machine")
Remove-Item carg.exp
Remove-Item carg_impl.obj