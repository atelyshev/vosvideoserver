echo on
IF NOT EXIST %1Release mkdir %1Release
IF NOT EXIST %1Release\PocoFoundation.dll copy %THIRDPARTY_ROOT%\poco-1.4.7p1\lib\PocoFoundation.dll %1Release
IF NOT EXIST %1Release\opencv_core2410.dll copy %THIRDPARTY_ROOT%\opencv\x86\vc12\bin\opencv_core2410.dll %1Release
IF NOT EXIST %1Release\opencv_highgui2410.dll copy %THIRDPARTY_ROOT%\opencv\x86\vc12\bin\opencv_highgui2410.dll %1Release
