# Microsoft Developer Studio Project File - Name="NetDvr" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=NetDvr - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "NetDvr.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "NetDvr.mak" CFG="NetDvr - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "NetDvr - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "NetDvr - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "NetDvr - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "NETDVR_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "..\common\include" /I "IpHelper" /I "jpeg6b" /D "WIN32" /D "_AFXDLL" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "NETDVR_EXPORTS" /FR /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x804 /d "NDEBUG"
# ADD RSC /l 0x804 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 libjpeg.lib custommp4.lib ctrlprotocol.lib ws2_32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /stack:0x1400000 /dll /machine:I386 /nodefaultlib:"LIBC" /def:"..\common\include\netdvr.def" /out:"..\lib\MyNetDvr2.dll" /implib:"..\lib\MyNetDvr2.lib" /libpath:"..\lib" /libpath:"jpeg6b"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "NetDvr - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "NETDVR_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "..\common\include" /I "IpHelper" /I "jpeg6b" /D "WIN32" /D "_AFXDLL" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "NETDVR_EXPORTS" /FR /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x804 /d "_DEBUG"
# ADD RSC /l 0x804 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 libjpeg.lib custommp4.lib ctrlprotocol.lib ws2_32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /stack:0x1400000 /dll /debug /machine:I386 /nodefaultlib:"MSVCRT" /def:"..\common\include\netdvr.def" /out:"..\lib\MyNetDvr2.dll" /implib:"..\lib\NetDvr2.lib" /pdbtype:sept /libpath:"..\lib" /libpath:"jpeg6b"
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "NetDvr - Win32 Release"
# Name "NetDvr - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\AudioPlay.cpp
# End Source File
# Begin Source File

SOURCE=.\avilib.c
# End Source File
# Begin Source File

SOURCE=.\debug_new.cpp
# End Source File
# Begin Source File

SOURCE=.\NetDvr.cpp
# End Source File
# Begin Source File

SOURCE=..\common\include\netdvr.def
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\NetDvrPrivate.cpp
# End Source File
# Begin Source File

SOURCE=.\TLFileLib.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\AudioPlay.h
# End Source File
# Begin Source File

SOURCE=..\common\include\avilib.h
# End Source File
# Begin Source File

SOURCE=.\debug_new.h
# End Source File
# Begin Source File

SOURCE=..\common\include\netdvr.h
# End Source File
# Begin Source File

SOURCE=.\NetDvrPrivate.h
# End Source File
# Begin Source File

SOURCE=..\common\include\TLFileLib.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\NetDvr.rc
# End Source File
# End Group
# End Target
# End Project
