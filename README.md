# vosvideoserver
Live streaming server built on top WebRTC.Supports IP cameras and Web cameras so far.
Little bit of history. This project was born to fill the gap between commercial camera streaming systems and "home made" port forwarding solutions. As result we have hybrid multiuser surveillance system. 

In general it has no limitation in number of attched cameras, it is matter of media servers you are running this moment. Just install server, register under web profile on http://vosvideo.com and assign next camera to that server.

Future plans:
* Show recorded video on Web console (vosvideo.com)
* Implement DASH for recorded video
* General improvements for Web part, use GRPC instead servicestack, use OAuth instead custom authentication, improve DB schema.
* Rebuild under Linux. Server itself has very small number of Windows dependent code. Only for maintaining service mode. It can be easily removed. Other code is C++14 standard STD

How to start.
1. You need to build needed libraries
2. You need to build VosVideo server
3. Create account on vosvideo.com
4. Configure your Web of IP camera (via vosvideo.com)
5. Watch video
6. Start changing the server.

Needed dev tool:
1. Visual Studio Community 2015

Build libraries. To properly build the server you have to do next steps:
1. Create folder thirdparty (or with another name)
2. Set environment varible THIRDPARTY_ROOT point to that folder
3. Install DEPOT TOOLS https://storage.googleapis.com/chrome-infra/depot_tools.zip
4. Take latest BOOST library from http://www.boost.org/ How to build it
  * Unpack it into THIRDPARTY_ROOT into directory named boost (or make NTFS junction to original directory name, it usually has version number which is not good to place into visual studio project)
  * Go to the directory tools\build\
  * Run bootstrap.bat
  * Copy files builddynamic.bat and buildstatic.bat from this repository to boost_x_xx_x and run them. 
5. Take latest Casablanca library https://casablanca.codeplex.com/ and build it as explained.
6. Take lastest GTEST https://github.com/google/googletest/tree/master/googletest Make junstion with name gtest, build it and copy libraries into gtest/lib/Release and gtest/lib/Debug accordingly
7. Take latest libwebm https://github.com/webmproject/libwebm. 
 * Build it >cd libwebm\source\build >cmake >use generated Visual Studio solution and build Release\Debug versions
 * copy lib files into libwebm\lib\Release and libwebm\lib\Debug accordingly
8. Take opencv https://github.com/opencv/opencv Build it and copy lib files opencv\lib\Release opencv\lib\Debug accordingly
9. Take POCO https://github.com/pocoproject/poco Build it, make junction poco to real poco directory and and copy lib files poco\lib (no Release or Debug needed)
10. Take WebRTC. This is very complex task (thanks google but I give you step by spte instructions, it should be easy this time)
 * Make directory webrtc-checkout or anything you like and make junction called libjingle pointing on it. Then
 * open cmd and set environment variables
  
  set GYP_GENERATORS=msvs-ninja,ninja
  
  set GYP_MSVS_VERSION=2015
  
  set GYP_DEFINES=component=shared_library
  
  set DEPOT_TOOLS_WIN_TOOLCHAIN=0
  * Run command >fetch webrtc    (it took hours)
  * Run command >cd src (go to webrtc-checkout\src)
  * Run command >gclient runhooks
  * Fix some google mistakes: open file src\build\config\win\BUILD.gn and change all :static_crt to :dynamic_crt (CRT - current run time)
  * open file src/third_party/jsoncpp/build.gn and replase source_set("jsoncpp") with static_library("jsoncpp") we need this library but google dont want to build it.
  * Now ready to build WebRTC 
  * Run configurator for Release >gn gen out/Release --args="is_debug=false symbol_level=2 is_component_build=false target_cpu=\"x86\" windows_sdk_path=\"C:\Program Files (x86)\Microsoft SDKs\Windows Kits\10\" libyuv_include_tests=false rtc_use_h264=true rtc_build_json=true" 
  * Run configurator for Debug >gn gen out/Debug --args="is_debug=true is_component_build=false target_cpu=\"x86\" windows_sdk_path=\"C:\Program Files (x86)\Microsoft SDKs\Windows Kits\10\" libyuv_include_tests=false rtc_use_h264=true rtc_build_json=true" 
  * Build Debug >ninja -C out/Debug 
  * Build Release >ninja -C out/Release
  * Copy ALL *.lib files from out/Debug to libkingle\lib\Debug
  * Copy ALL *.lib files from out/Release to libkingle\lib\Release
 
 11. Now you are ready to build vosvideoserver. But before, it is final step you need to modify 
  * src\webrtc\base\array_view.h and src\webrtc\base\buffer.h, comment out 


    // Construct a buffer from the contents of an array.  
    //template <typename U,  
    //          size_t N,  
    //          typename std::enable_if<  
    //              internal::BufferCompat<T, U>::value>::type* = nullptr>  
    //BufferT(U (&array)[N]) : BufferT(array, N) {}  
  

    //template <typename U,  
    //          size_t N,  
    //          typename std::enable_if<  
    //              internal::BufferCompat<T, U>::value>::type* = nullptr>  
    //void SetData(const U (&array)[N]) {  
    //  SetData(array, N);  
    //}    
  
    //template <typename U,  
    //          size_t N,  
    //          typename std::enable_if<  
    //              internal::BufferCompat<T, U>::value>::type* = nullptr>  
    //void AppendData(const U (&array)[N]) {  
    //  AppendData(array, N);  
    //}  

    // Construct an ArrayView from an array.  
    //template <typename U, size_t N>  
    //ArrayView(U (&array)[N]) : ArrayView(array, N) {  
    //  static_assert(Size == N || Size == impl::kArrayViewVarSize,  
    //                "Array size must match ArrayView size");  
    //}  
    BTW if anyone can figure out what is wrong with this template please let me know!
    atelyshev@vosvideo.com
11. Finally Take latest gstreamer https://gstreamer.freedesktop.org/data/pkg/windows/
  
Questions ? 
atelyshev@vosvideo.com
    
