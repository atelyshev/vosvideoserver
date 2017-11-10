// Copyright (c) 2011, Zach Burlingame
// All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//     * Neither the name of zachburlingame.com nor the
//       names of its contributors may be used to endorse or promote products
//       derived from this software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDER BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS

#include "hg_version.h"

#define STRINGIZE2(s) #s
#define STRINGIZE(s) STRINGIZE2(s)

#define VERSION_MAJOR               2
#define VERSION_MINOR               0
#define VERSION_REVISION            0
#define VERSION_BUILD               HG_REVISION

#if HG_LOCAL_MODIFICATIONS
#define VERSION_MODIFIER "M" // if build done with local modifications then M appended, useful for patch mode
#else
#define VERSION_MODIFIER
#endif

#define VER_FILE_DESCRIPTION_STR    HG_CHANGESET
#define VER_FILE_VERSION            VERSION_MAJOR, VERSION_MINOR, VERSION_REVISION, VERSION_BUILD
#define VER_FILE_VERSION_STR        STRINGIZE(VERSION_MAJOR)        \
	"." STRINGIZE(VERSION_MINOR)    \
	"." STRINGIZE(VERSION_REVISION) \
	"." HG_CHANGESET_SHORT    \
	VERSION_MODIFIER

#define VER_PRODUCTNAME_STR         "rtbcserver"
#define VER_PRODUCT_VERSION         VER_FILE_VERSION
#define VER_PRODUCT_VERSION_STR     VER_FILE_VERSION_STR

#if LIBRARY_EXPORTS
#define VER_ORIGINAL_FILENAME_STR VER_PRODUCTNAME_STR ".dll"
#else
#define VER_ORIGINAL_FILENAME_STR VER_PRODUCTNAME_STR ".exe"
#endif
#define VER_INTERNAL_NAME_STR       VER_ORIGINAL_FILENAME_STR

#define VER_COPYRIGHT_STR           "Copyright (C) 2017 Vos Video LLC"

#ifdef _DEBUG
#define VER_VER_DEBUG             VS_FF_DEBUG
#else
#define VER_VER_DEBUG             0
#endif

#define VER_FILEOS                  VOS_NT_WINDOWS32
#define VER_FILEFLAGS               VER_VER_DEBUG

#if LIBRARY_EXPORTS
#define VER_FILETYPE              VFT_DLL
#else
#define VER_FILETYPE              VFT_APP
#endif
