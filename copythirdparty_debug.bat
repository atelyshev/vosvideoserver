echo on
IF NOT EXIST %1Debug mkdir %1Debug
IF NOT EXIST %1Debug\PocoFoundationd.dll copy %THIRDPARTY_ROOT%\poco\bin\PocoFoundationd.dll %1Debug
IF NOT EXIST %1Debug\opencv_core310d.dll copy %THIRDPARTY_ROOT%\opencv\bin\Debug\opencv_core310d.dll %1Debug
IF NOT EXIST %1Debug\opencv_highgui310d.dll copy %THIRDPARTY_ROOT%\opencv\bin\Debug\opencv_highgui310d.dll %1Debug
IF NOT EXIST %1Debug\opencv_videoio310d.dll copy %THIRDPARTY_ROOT%\opencv\bin\Debug\opencv_videoio310d.dll %1Debug
IF NOT EXIST %1Debug\opencv_imgcodecs310d.dll copy %THIRDPARTY_ROOT%\opencv\bin\Debug\opencv_imgcodecs310d.dll %1Debug
IF NOT EXIST %1Debug\opencv_imgproc310d.dll copy %THIRDPARTY_ROOT%\opencv\bin\Debug\opencv_imgproc310d.dll %1Debug
IF NOT EXIST %1Debug\cpprest140d_2_8.dll copy %THIRDPARTY_ROOT%\casablanca\SDK\lib\Debug\cpprest140d_2_8.dll %1Debug
IF NOT EXIST %1Debug\cpprest140d_2_8.dll copy %THIRDPARTY_ROOT%\casablanca\SDK\lib\Debug\cpprest140d_2_8.dll %1Debug
IF NOT EXIST %1Debug\libgstreamer-1.0-0.dll copy %GSTREAMER_1_0_ROOT_X86%\bin\*.dll %1Debug

