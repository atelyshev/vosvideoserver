To properly build the server you have to do next steps:
[1] Create folder thirdparty (or with another name)
[2] Set environment varible THIRDPARTY_ROOT point to that folder
[3] Check in ThirdParty repository from bitbucket
[4] Create folder vosvideocomponents
[5] Set environment varible VOSVIDEO_ROOT point to that folder
[6] Check in VosVideoVP8, RtspxSchemeHandler, MjpegSource and compile them. use regsvr32 to register all prodiced dlls. 
It is COM objects and will be loaded automatically later
[7] Check in rtbcserver into vosvideocomponents folder.
[8] Build it.
