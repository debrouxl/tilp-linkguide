del *.plg
del *.ncb
del *.opt
del *.ilk
del *.bak
deltree Debug /y
deltree Release /y
rmdir Debug /s /q
rmdir Release /s /q
cls