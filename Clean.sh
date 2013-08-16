#!/bin/bash
./ThirdParty/Premake4.elf --qt-shared --file=ScreenShotter.lua clean
rm -rf ./Projects
rm -rf ./Binaries
rm -rf ./qt_moc
rm -rf ./qt_qrc
rm -rf ./qt_ui
rm -r ./*~
