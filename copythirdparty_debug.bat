echo on
IF NOT EXIST %1Debug mkdir %1Debug
IF NOT EXIST %1Debug\boost_date_time-vc110-mt-gd-1_??.dll copy %THIRDPARTY_ROOT%\boost_1_53_0\stage\lib\boost_date_time-vc110-mt-gd-1_??.dll %1Debug
IF NOT EXIST %1Debug\boost_filesystem-vc110-mt-gd-1_??.dll copy %THIRDPARTY_ROOT%\boost_1_53_0\stage\lib\boost_filesystem-vc110-mt-gd-1_??.dll %1Debug
IF NOT EXIST %1Debug\boost_log-vc110-mt-gd-1_??.dll copy %THIRDPARTY_ROOT%\boost_1_53_0\stage\lib\boost_log-vc110-mt-gd-1_??.dll %1Debug
IF NOT EXIST %1Debug\boost_signals-vc110-mt-gd-1_??.dll copy %THIRDPARTY_ROOT%\boost_1_53_0\stage\lib\boost_signals-vc110-mt-gd-1_??.dll %1Debug
IF NOT EXIST %1Debug\boost_system-vc110-mt-gd-1_??.dll copy %THIRDPARTY_ROOT%\boost_1_53_0\stage\lib\boost_system-vc110-mt-gd-1_??.dll %1Debug
IF NOT EXIST %1Debug\boost_thread-vc110-mt-gd-1_??.dll copy %THIRDPARTY_ROOT%\boost_1_53_0\stage\lib\boost_thread-vc110-mt-gd-1_??.dll %1Debug
IF NOT EXIST %1Debug\cpprest110d_1_?.dll copy %THIRDPARTY_ROOT%\casablanca\SDK\bin\x86\Debug\cpprest110d_1_?.dll %1Debug
IF NOT EXIST %1Debug\PocoFoundationd.dll copy %THIRDPARTY_ROOT%\poco-1.4.6\lib\Debug\PocoFoundationd.dll %1Debug

:: Our components, dont check for existance
copy %VOSVIDEO_ROOT%\mjpegsource\Debug\mjpegsource.dll %1Debug /Y
regsvr32 /s %1Debug\mjpegsource.dll
copy %VOSVIDEO_ROOT%\rtspxschemehandler\Debug\rtspxschemehandler.dll %1Debug /Y 
regsvr32 /s %1Debug\rtspxschemehandler.dll
copy %VOSVIDEO_ROOT%\vosvideovp8\Debug\VP8CallbackSink.dll %1Debug /Y 
regsvr32 /s %1Debug\VP8CallbackSink.dll
copy %VOSVIDEO_ROOT%\vosvideovp8\Debug\VP8FileSink.dll %1Debug /Y 
regsvr32 /s %1Debug\VP8CallbackSink.dll
copy %VOSVIDEO_ROOT%\vosvideovp8\Debug\VP8Encoder.dll %1Debug /Y 
regsvr32 /s %1Debug\VP8CallbackSink.dll

