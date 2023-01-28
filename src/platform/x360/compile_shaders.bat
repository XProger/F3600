@echo off
rd /S /Q shaders\out
mkdir shaders\out

call :compile blob
call :compile gui
call :compile object
call :compile shield
call :compile ship
call :compile track

EXIT /B %ERRORLEVEL%

:compile
    SETLOCAL
    echo compile shaders/%~1.hlsl
    echo #include "%~1_vs.h" >> shaders/out/shaders.h
    echo #include "%~1_ps.h" >> shaders/out/shaders.h
    fxc_x360 /nologo /T vs_2_0 /Zpr /Fh shaders/out/%~1_vs.h /Vn %~1_vs shaders/%~1.hlsl /DVERTEX
    fxc_x360 /nologo /T ps_2_0 /Zpr /Fh shaders/out/%~1_ps.h /Vn %~1_ps shaders/%~1.hlsl /DPIXEL
    ENDLOCAL
EXIT /B 0
