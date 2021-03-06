#!/bin/sh
SDK_VERSION=4.3
ASPEN_ROOT=/Developer/Platforms/iPhoneOS.platform/Developer
SIMULATOR_ASPEN_ROOT=/Developer/Platforms/iPhoneSimulator.platform/Developer
ASPEN_SDK=$ASPEN_ROOT/SDKs/iPhoneOS${SDK_VERSION}.sdk/
SIMULATOR_ASPEN_SDK=$SIMULATOR_ASPEN_ROOT/SDKs/iPhoneSimulator${SDK_VERSION}.sdk

ORIG_PATH=$PATH
PRFX=$PWD/tmp 


if [ ${UNITY_THISISABUILDMACHINE:+1} ]; then
        echo "Erasing builds folder to make sure we start with a clean slate"
        rm -rf builds
fi

setenv () {
	export PATH=$ASPEN_ROOT/usr/bin:$PATH

	export C_INCLUDE_PATH="$ASPEN_SDK/usr/lib/gcc/arm-apple-darwin9/4.2.1/include:$ASPEN_SDK/usr/include"
	export CPLUS_INCLUDE_PATH="$ASPEN_SDK/usr/lib/gcc/arm-apple-darwin9/4.2.1/include:$ASPEN_SDK/usr/include"
	#export CFLAGS="-DZ_PREFIX -DPLATFORM_IPHONE -DARM_FPU_VFP=1 -miphoneos-version-min=3.0 -mno-thumb -fvisibility=hidden -g -O0"
	export CFLAGS="-DHAVE_ARMV6=1 -DZ_PREFIX -DPLATFORM_IPHONE -DARM_FPU_VFP=1 -miphoneos-version-min=3.0 -mno-thumb -fvisibility=hidden -Os"
	export CXXFLAGS="$CFLAGS"
	export CC="gcc-4.2 -arch $1"
	export CXX="g++-4.2 -arch $1"
	export CPP="cpp -nostdinc -U__powerpc__ -U__i386__ -D__arm__"
	export CXXPP="cpp -nostdinc -U__powerpc__ -U__i386__ -D__arm__"
	export LD=$CC
	export LDFLAGS="-liconv -Wl,-syslibroot,$ASPEN_SDK"
}

unsetenv () {
	export PATH=$ORIG_PATH

	unset C_INCLUDE_PATH
	unset CPLUS_INCLUDE_PATH
	unset CC
	unset CXX
	unset CPP
	unset CXXPP
	unset LD
	unset LDFLAGS
	unset PLATFORM_IPHONE_XCOMP
	unset CFLAGS
	unset CXXFLAGS
}

export mono_cv_uscore=yes
export cv_mono_sizeof_sunpath=104
export ac_cv_func_posix_getpwuid_r=yes
export ac_cv_func_backtrace_symbols=no

build_arm_mono ()
{
	setenv "$1"

	make clean
	rm config.h*

	pushd eglib 
	./autogen.sh --host=arm-apple-darwin9 --prefix=$PRFX
	make clean
	popd

	./autogen.sh --prefix=$PRFX --disable-mcs-build --host=arm-apple-darwin9 --disable-shared-handles --with-tls=pthread --with-sigaltstack=no --with-glib=embedded --enable-minimal=jit,profiler,com --disable-nls || exit 1
	perl -pi -e 's/MONO_SIZEOF_SUNPATH 0/MONO_SIZEOF_SUNPATH 104/' config.h
	perl -pi -e 's/#define HAVE_FINITE 1//' config.h
	#perl -pi -e 's/#define HAVE_MMAP 1//' config.h
	perl -pi -e 's/#define HAVE_CURSES_H 1//' config.h
	perl -pi -e 's/#define HAVE_STRNDUP 1//' eglib/config.h
	make

	make || exit 1

	mkdir -p builds/embedruntimes/iphone
	cp mono/mini/.libs/libmono.a "builds/embedruntimes/iphone/libmono-$1.a" || exit 1
}

build_iphone_runtime () 
{
	echo "Building iPhone runtime"
	build_arm_mono "armv7" || exit 1
	build_arm_mono "armv6" || exit 1

	libtool -static -o builds/embedruntimes/iphone/libmono.a builds/embedruntimes/iphone/libmono-armv6.a builds/embedruntimes/iphone/libmono-armv7.a || exit 1
	rm builds/embedruntimes/iphone/libmono-armv6.a
	rm builds/embedruntimes/iphone/libmono-armv7.a
	unsetenv
	echo "iPhone runtime build done"
}

build_iphone_crosscompiler ()
{
	echo "Building iPhone cross compiler";
	export CFLAGS="-DARM_FPU_VFP=1 -DUSE_MUNMAP -DPLATFORM_IPHONE_XCOMP"	

	export PLATFORM_IPHONE_XCOMP=1	

	pushd eglib 
	./autogen.sh --prefix=$PRFX || exit 1
	make clean
	popd
	
	./autogen.sh --prefix=$PRF --with-macversion=10.5 --disable-mcs-build --disable-shared-handles --with-tls=pthread --with-signalstack=no --with-glib=embedded --target=arm-darwin --disable-nls || exit 1
	make clean || exit 1
	make || exit 1
	mkdir -p builds/crosscompiler/iphone
	cp mono/mini/mono builds/crosscompiler/iphone/mono-xcompiler
	unsetenv
	echo "iPhone cross compiler build done"
}

build_iphone_simulator ()
{
	echo "Building iPhone simulator static lib";
	export MACSYSROOT="-isysroot $SIMULATOR_ASPEN_SDK"
	export MACSDKOPTIONS="-miphoneos-version-min=3.0 $MACSYSROOT"
	export CC="$SIMULATOR_ASPEN_ROOT/usr/bin/gcc-4.2"
	export CXX="$SIMULATOR_ASPEN_ROOT/usr/bin/g++-4.2"
	perl build_runtime_osx.pl -iphone_simulator=1 || exit 1
	echo "Copying iPhone simulator static lib to final destination";
	mkdir -p builds/embedruntimes/iphone
	cp mono/mini/.libs/libmono.a builds/embedruntimes/iphone/libmono-i386.a
	unsetenv
}

usage()
{
	echo "available arguments: [--runtime-only|--xcomp-only|--simulator-only]";
}

if [ $# -gt 1 ]; then
 	usage
	exit 1
fi
if [ $# -eq 1 ]; then
	if [ "x$1" == "x--runtime-only" ]; then
		build_iphone_runtime || exit 1
	elif [ "x$1" == "x--xcomp-only" ]; then
		build_iphone_crosscompiler || exit 1	
	elif [ "x$1" == "x--simulator-only" ]; then
		build_iphone_simulator|| exit 1	
	else
		usage
	fi
fi
if [ $# -eq 0 ]; then
	build_iphone_runtime || exit 1
	build_iphone_crosscompiler || exit 1
	build_iphone_simulator || exit 1
fi
