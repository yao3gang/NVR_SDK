rem  ///////////delete///////////
del /Q/S *.ncb
del /Q/S *.bak
del /Q/S *.opt
del /Q/S *.plg
del /Q/S *.user
del /Q/S *.ncb
del /Q/S *.bak
del /Q/S *.opt
del /Q/S *.plg
del /Q/S *.aps
del /Q/S *.user
del /Q/S/AH *.suo

del /Q/S debug\*.*
rd debug\Library\Basic
rd debug\Library
rd debug
del /Q/S release\*.*
rd release\Library\Basic
rd release\Library
rd release

rem  ///////////CommonFunc///////////
del /Q/S CommonFunc\debug\*.*
rd CommonFunc\debug
del /Q/S CommonFunc\release\*.*
rd CommonFunc\release

rem  ///////////FileManager///////////
del /Q/S FileManager\debug\*.*
rd FileManager\debug
del /Q/S FileManager\release\*.*
rd FileManager\release

rem  ///////////FitValidate///////////
del /Q/S FitValidate\debug\*.*
rd FitValidate\debug
del /Q/S FitValidate\release\*.*
rd FitValidate\release

rem  ///////////GDIObject///////////
del /Q/S GDIObject\debug\*.*
rd GDIObject\debug
del /Q/S GDIObject\release\*.*
rd GDIObject\release

rem  ///////////GDISysDep///////////
del /Q/S GDISysDep\debug\*.*
rd GDISysDep\debug
del /Q/S GDISysDep\release\*.*
rd GDISysDep\release

rem  ///////////GridCtrl///////////
del /Q/S GridCtrl\debug\*.*
rd GridCtrl\debug
del /Q/S GridCtrl\release\*.*
rd GridCtrl\release

rem  ///////////IBISGen///////////
del /Q/S IBISGen\debug\*.*
rd IBISGen\debug
del /Q/S IBISGen\release\*.*
rd IBISGen\release

rem  ///////////LibraryManager///////////
del /Q/S LibraryManager\debug\*.*
rd LibraryManager\debug
del /Q/S LibraryManager\release\*.*
rd LibraryManager\release

rem  ///////////LikaiSkin///////////
del /Q/S LikaiSkin\debug\*.*
rd LikaiSkin\debug
del /Q/S LikaiSkin\release\*.*
rd LikaiSkin\release

rem  ///////////ModelEditor///////////
del /Q/S ModelEditor\debug\*.*
rd ModelEditor\debug
del /Q/S ModelEditor\release\*.*
rd ModelEditor\release

rem  ///////////SIMDE///////////
del /Q/S SIMDE\debug\*.*
rd SIMDE\debug
del /Q/S SIMDE\release\*.*
rd SIMDE\release

rem  ///////////SystemDep///////////
del /Q/S SystemDep\debug\*.*
rd SystemDep\debug
del /Q/S SystemDep\release\*.*
rd SystemDep\release

rem  ///////////XMLRW///////////
del /Q/S XMLRW\debug\*.*
rd XMLRW\debug
del /Q/S XMLRW\release\*.*
rd XMLRW\release

