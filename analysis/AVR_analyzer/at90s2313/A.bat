rem cd at90sxxxx
rem ..\avrasm -g test_232.asm lst.lst rom.rom
rem  ..\avrasm -g test_ti.asm lst.lst rom.rom
rem ..\avrasm -g peak_det.asm lst.lst rom.rom
rem ..\avrasm -g ana_100.asm lst.lst rom.rom
rem ..\avrasm -g ana_101.asm lst.lst rom.rom
..\avrasm -g ana_102.asm lst.lst rom.rom
move rom.rom ..\rom.rom
