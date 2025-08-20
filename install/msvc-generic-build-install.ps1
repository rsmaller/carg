$msvcver = (ls "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Tools\MSVC" -Name | Sort-Object Name -Descending)[0]
$msvcpath = "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Tools\MSVC\" + $msvcver + "\include"
cp ../include/carg.h $msvcpath
cl /LD ../src/carg_impl.c /Fe:carg.dll
cp carg.dll C:\Windows\SysWOW64\
rm carg.exp
rm carg_impl.obj