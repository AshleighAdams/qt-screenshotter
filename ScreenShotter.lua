dofile( "ThirdParty/qtpresets.lua" )
EnableOption( "qt-shared" )

solution "ScreenShotter"
	language "C++"
	location "Projects"
	targetdir "Binaries"
	configurations { "Release", "Debug" }

	configuration "Debug"
		flags { "Symbols" }
	configuration "Release"
		flags { "Optimize" }
	
	project "ScreenShotter"
		files
		{
			"Source/**.hpp", "Source/**.cpp"
		}
		vpaths
		{
			["Source Files"] = "Source/**.cpp",
			["Header Files"] = "Source/**.hpp"
		}
		
		kind "ConsoleApp" -- StaticLib, SharedLib
		
		

		configuration "windows"
			libdirs { "ThirdParty/Libraries" }
			includedirs { "ThirdParty/Include" }
			defines { "WINDOWS" }
		
		configuration "linux"
			buildoptions { "-std=c++11" }
			links { "pthread" } -- for std::thread
			defines { "LINUX" }
			buildoptions { "`pkg-config --cflags QtGui QtCore`" } -- Qt includes
			linkoptions { "`pkg-config --libs QtGui QtCore`" }
			
		configuration "Debug"
			targetsuffix "_d"
			
		links { } -- Such as { "GL", "X11" }

		configuration "linux"
			excludes { } -- "Source/WindowsX.cpp"
		configuration "windows"
			excludes { }
			
--$(Configurations)
local mocFiles                          = { } --{ "$(ProjectName)Frame.hpp" }
local qrcFiles                          = { os.matchfiles( "*.qrc" ) }
local uiFiles                           = { os.matchfiles( "*.ui" ) }
local libsToLink                        = { "Core", "Gui" }
qt.Configure( mocFiles, qrcFiles, uiFiles, libsToLink )
