#!/bin/bash

if [ -e svn.log ] ; then rm svn.log; fi

RECOMPILE=true
UPLOAD=false

while $RECOMPILE ; do

	echo ----------- SVN update >> "$1"

	svn log -r "BASE:HEAD" > svn1.log
	cat svn1.log >> "$1"
	cat svn1.log >> svn.log

	RECOMPILE=false
	svn up --accept theirs-full > svn2.log 2>&1
	if cat svn2.log | grep "Updated to revision "; then RECOMPILE=true; fi
	
	if $RECOMPILE; then

		echo ----------- Compiling >> "$1"
		UPLOAD=true

		cd "build/msvc 2005"

		rm ../../distrib/win32/OpenLieroX.exe

		# You can compile without Cmake, then substitute openlierox.vcproj with Game.vcproj for vcbuild
		cmake -G "Visual Studio 8 2005" -D DEBUG=0 ../..
		vcbuild openlierox.vcproj "RelWithDebInfo|Win32" /useenv >> "$1" 2>&1

		cd ../..
	
		if [ \! -f distrib/win32/OpenLieroX.exe ] ; then 
			echo ----------- Compiling failed - cleanup and try again >> "$1"
			rm -r obj openlierox.dir CMakeFiles CMakeCache.txt
			cmake -G "Visual Studio 8 2005" -D DEBUG=0 ../..
			vcbuild openlierox.vcproj "RelWithDebInfo|Win32" /useenv >> "$1" 2>&1
			if [ \! -f distrib/win32/OpenLieroX.exe ] ; then
				echo ----------- Compiling failed >> "$1"
				exit
			fi
		fi
	fi
done

if ! $UPLOAD; then
	echo ----------- Nothing to upload >> "$1"
	exit
fi

if grep -i "\[Rebuild\]" svn.log; then
	echo ----------- Committing to SVN >> "$1"
	svn commit -m "Updated EXE (auto nightly build)"
fi

REVNUM=`grep -o 'r[0-9]*' optional-includes/generated/Version_generated.h`

cmd /c save_debug_info.bat $REVNUM
zip -r debuginfo_$REVNUM.zip debuginfo/$REVNUM
# echo Debug symbols attached | email --attach debuginfo_$REVNUM.zip --subject 'Debug symbols automated mail' --smtp-server 172.17.57.25 karel.petranek@tiscali.cz"

# Upload file to lxalliance.net gallery - non-trivial!
# Requires file lxalliance.net.cookie with your auth data 
# (copy it from Firefox cookies, it's named LXA_forum)

cp distrib/win32/OpenLieroX.exe OpenLieroX_$REVNUM.exe
zip OpenLieroX_$REVNUM.zip OpenLieroX_$REVNUM.exe

echo ----------- Uploading to LXA >> "$1"

curl	--cookie "$2" \
	--cookie-jar "$2" \
	"http://lxalliance.net/forum/index.php?action=mgallery;sa=post;album=15" \
	> temp.html

SC=`grep '"sc"' temp.html | sed 's/.*value="\([^"]*\).*/\1/'`

curl	--referer "http://lxalliance.net/forum/index.php?action=mgallery;sa=post;album=15" \
	-F "file=@OpenLieroX_${REVNUM}.zip;type=application/zip" \
	-F "title=OpenLieroX_${REVNUM}_EXE" \
	-F "desc=OpenLieroX $REVNUM EXE nightly build" \
	-F "desc_mode=0" \
	-F "keywords=latest,SVN" \
	-F "embed_url=" \
	-F "thumbnail=/dev/null;filename=" \
	-F "sc=$SC" \
	-F "submit_mgal=Submit" \
	--cookie "$2" \
	--cookie-jar "$2" \
	"http://lxalliance.net/forum/index.php?action=mgallery;sa=post;album=15" \
