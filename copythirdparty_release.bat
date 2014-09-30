echo on
IF NOT EXIST %1Release mkdir %1Release
IF NOT EXIST %1Release\boost_date_time-vc110-mt-1_??.dll copy %THIRDPARTY_ROOT%\boost_1_53_0\stage\lib\boost_date_time-vc110-mt-1_??.dll %1Release
IF NOT EXIST %1Release\boost_filesystem-vc110-mt-1_??.dll copy %THIRDPARTY_ROOT%\boost_1_53_0\stage\lib\boost_filesystem-vc110-mt-1_??.dll %1Release
IF NOT EXIST %1Release\boost_log-vc110-mt-1_??.dll copy %THIRDPARTY_ROOT%\boost_1_53_0\stage\lib\boost_log-vc110-mt-1_??.dll %1Release
IF NOT EXIST %1Release\boost_signals-vc110-mt-1_??.dll copy %THIRDPARTY_ROOT%\boost_1_53_0\stage\lib\boost_signals-vc110-mt-1_??.dll %1Release
IF NOT EXIST %1Release\boost_system-vc110-mt-1_??.dll copy %THIRDPARTY_ROOT%\boost_1_53_0\stage\lib\boost_system-vc110-mt-1_??.dll %1Release
IF NOT EXIST %1Release\boost_thread-vc110-mt-1_??.dll copy %THIRDPARTY_ROOT%\boost_1_53_0\stage\lib\boost_thread-vc110-mt-1_??.dll %1Release
IF NOT EXIST %1Release\cpprest110_1_?.dll copy %THIRDPARTY_ROOT%\casablanca\SDK\bin\x86\Release\cpprest110_1_?.dll %1Release
IF NOT EXIST %1Release\PocoFoundation.dll copy %THIRDPARTY_ROOT%\poco-1.4.6\lib\Release\PocoFoundation.dll %1Release

:: Our components, dont check for existance
copy %VOSVIDEO_ROOT%\mjpegsource\Release\mjpegsource.dll %1Release /Y
regsvr32 /s %1Release\mjpegsource.dll
copy %VOSVIDEO_ROOT%\rtspxschemehandler\Release\rtspxschemehandler.dll %1Release /Y
regsvr32 /s %1Release\rtspxschemehandler.dll
copy %VOSVIDEO_ROOT%\vosvideovp8\Release\VP8CallbackSink.dll %1Release /Y
regsvr32 /s %1Release\VP8CallbackSink.dll
copy %VOSVIDEO_ROOT%\vosvideovp8\Release\VP8FileSink.dll %1Release /Y
regsvr32 /s %1Release\VP8CallbackSink.dll
copy %VOSVIDEO_ROOT%\vosvideovp8\Release\VP8Encoder.dll %1Release /Y
regsvr32 /s %1Release\VP8CallbackSink.dll
