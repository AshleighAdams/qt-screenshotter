#!/bin/bash

#make sure libqt4-devel or libqt4-dev libqt4-dev-bin are in stalled

./Clean.sh
./ThirdParty/Premake4.elf --qt-shared --file=ScreenShotter.lua gmake
