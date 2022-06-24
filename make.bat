@set existent=0
@for %%a in (gcc tdm-gcc tcc) do @(%%a -v > nul 2>&1 && set "existent=1" && set "compiler=%%a")
@if "%existent%"=="0" (@echo Could not find any compiler in path) else @(
	%compiler% src/newtrodit.c -o newtrodit.exe -O2 -luser32 -lkernel32 -m32 -Wno-implicit-function-declaration -Wno-pointer-to-int-cast || @(echo compilation error&&pause>nul&&exit /b 1)
	echo compilation successful
	pause>nul
)
