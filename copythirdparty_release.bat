echo on
IF NOT EXIST %1Release mkdir %1Release
IF NOT EXIST %1Release\PocoFoundation.dll copy %THIRDPARTY_ROOT%\poco\bin\PocoFoundation.dll %1Release
IF NOT EXIST %1Release\opencv_core310.dll copy %THIRDPARTY_ROOT%\opencv\bin\Release\opencv_core310.dll %1Release
IF NOT EXIST %1Release\opencv_highgui310.dll copy %THIRDPARTY_ROOT%\opencv\bin\Release\opencv_highgui310.dll %1Release
IF NOT EXIST %1Release\opencv_videoio310.dll copy %THIRDPARTY_ROOT%\opencv\bin\Release\opencv_videoio310.dll %1Release
IF NOT EXIST %1Release\opencv_imgcodecs310.dll copy %THIRDPARTY_ROOT%\opencv\bin\Release\opencv_imgcodecs310.dll %1Release
IF NOT EXIST %1Release\opencv_imgproc310.dll copy %THIRDPARTY_ROOT%\opencv\bin\Release\opencv_imgproc310.dll %1Release
IF NOT EXIST %1Release\cpprest140_2_8.dll copy %THIRDPARTY_ROOT%\casablanca\SDK\lib\Release\cpprest140_2_8.dll %1Release
