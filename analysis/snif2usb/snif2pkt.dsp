# Microsoft Developer Studio Project File - Name="snif2pkt" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=snif2pkt - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "snif2pkt.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "snif2pkt.mak" CFG="snif2pkt - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "snif2pkt - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "snif2pkt - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "snif2pkt - Win32 Release"

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
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD BASE RSC /l 0x40c /d "NDEBUG"
# ADD RSC /l 0x40c /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386 /out:"snif2pkt.exe"

!ELSEIF  "$(CFG)" == "snif2pkt - Win32 Debug"

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
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD BASE RSC /l 0x40c /d "_DEBUG"
# ADD RSC /l 0x40c /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /out:"snif2pkt.exe" /pdbtype:sept

!ENDIF 

# Begin Target

# Name "snif2pkt - Win32 Release"
# Name "snif2pkt - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\snif2pkt.c
# End Source File
# End Group
# Begin Group "Doc Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE="..\..\direct-usb-protocol.txt"
# End Source File
# Begin Source File

SOURCE="..\..\usb-protocol.txt"
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\test.log
# End Source File
# Begin Source File

SOURCE=.\test.pkt
# End Source File
# End Group
# Begin Group "Log Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE="..\logs\ti84+_usb\check_updates.pkt"
# End Source File
# Begin Source File

SOURCE="..\logs\ti84+_usb\clock_get.pkt"
# End Source File
# Begin Source File

SOURCE="..\logs\ti84+_usb\clock_set.pkt"
# End Source File
# Begin Source File

SOURCE="..\logs\ti84+_usb\delete_app.pkt"
# End Source File
# Begin Source File

SOURCE="..\logs\ti84+_usb\delete_var_a.pkt"
# End Source File
# Begin Source File

SOURCE="..\logs\ti84+_usb\delete_var_b.pkt"
# End Source File
# Begin Source File

SOURCE="..\logs\ti84+_usb\device_info.pkt"
# End Source File
# Begin Source File

SOURCE="..\logs\ti84+_usb\dirlist.pkt"
# End Source File
# Begin Source File

SOURCE="..\logs\ti84+_usb\recv_flash_app.pkt"
# End Source File
# Begin Source File

SOURCE="..\logs\ti84+_usb\recv_var_a.pkt"
# End Source File
# Begin Source File

SOURCE="..\logs\ti84+_usb\recv_var_b.pkt"
# End Source File
# Begin Source File

SOURCE="..\logs\ti84+_usb\recv_vars.pkt"
# End Source File
# Begin Source File

SOURCE="..\logs\ti84+_usb\screenshot.pkt"
# End Source File
# Begin Source File

SOURCE="..\logs\ti84+_usb\send_cert.pkt"
# End Source File
# Begin Source File

SOURCE="..\logs\ti84+_usb\send_flash_app.pkt"
# End Source File
# Begin Source File

SOURCE="..\logs\ti84+_usb\send_flash_os.pkt"
# End Source File
# Begin Source File

SOURCE="..\logs\ti84+_usb\send_l1.pkt"
# End Source File
# Begin Source File

SOURCE="..\logs\ti84+_usb\send_var_a.pkt"
# End Source File
# Begin Source File

SOURCE="..\logs\ti84+_usb\send_var_b.pkt"
# End Source File
# Begin Source File

SOURCE="..\logs\ti84+_usb\send_vars.pkt"
# End Source File
# End Group
# End Target
# End Project
