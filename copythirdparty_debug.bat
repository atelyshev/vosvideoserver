echo on
IF NOT EXIST %1Debug mkdir %1Debug
IF NOT EXIST %1Debug\PocoFoundationd.dll copy %THIRDPARTY_ROOT%\poco-1.4.7p1\lib\PocoFoundationd.dll %1Debug
IF NOT EXIST %1Debug\opencv_core2410d.dll copy %OPENCV_DIR%\bin\opencv_core2410d.dll %1Debug
IF NOT EXIST %1Debug\opencv_highgui2410d.dll copy %OPENCV_DIR%\bin\opencv_highgui2410d.dll %1Debug

