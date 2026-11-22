#!/bin/zsh
# Build SMCDellDDV.kext (plugin VirtualSMC read-only: RPM kipas + suhu via Dell DDV) tanpa Xcode.
#   DEPS=/folder/deps zsh build.sh   -> ./build/SMCDellDDV.kext
# DEPS berisi: MacKernelSDK/, Lilu/ (source acidanthera/Lilu), VirtualSMC/ (source acidanthera/VirtualSMC)
set -e
ROOT=${0:A:h}; DEPS=${DEPS:?set DEPS=/folder/berisi/MacKernelSDK+Lilu+VirtualSMC}
SRC=$ROOT; OUT=$ROOT/build/SMCDellDDV.kext
NAME=SMCDellDDV; BID=local.dell.SMCDellDDV; VER=1.0.8
SDK=$DEPS/MacKernelSDK
TMP=$(mktemp -d)
mkdir -p $TMP/lilu; ln -s $DEPS/Lilu/Lilu/Headers $TMP/lilu/Headers; ln -s $DEPS/Lilu/Lilu/Library $TMP/lilu/Library
rm -rf $OUT; mkdir -p $OUT/Contents/MacOS
sed -e "s/\$(PRODUCT_BUNDLE_IDENTIFIER)/$BID/g" -e "s/\$(PRODUCT_NAME:rfc1034identifier)/$NAME/g" \
    -e "s/\$(PRODUCT_NAME)/$NAME/g" -e "s/\$(EXECUTABLE_NAME)/$NAME/g" -e "s/\$(DEVELOPMENT_LANGUAGE)/en/g" \
    -e "s/\$(MODULE_VERSION)/$VER/g" -e "s/\$(CURRENT_PROJECT_VERSION)/$VER/g" -e "s/\$(MARKETING_VERSION)/$VER/g" \
    $SRC/Info.plist > $OUT/Contents/Info.plist
grep -q '\$(' $OUT/Contents/Info.plist && { echo "unresolved variable in Info.plist"; grep '\$(' $OUT/Contents/Info.plist; exit 1; }
FLAGS=(-arch x86_64 -mkernel -nostdinc -fno-builtin -fno-common -fno-stack-protector -mmacosx-version-min=10.7
       -mmmx -msse -msse2 -msse3 -mssse3 -mfpmath=sse -fno-asynchronous-unwind-tables
       -DKERNEL -DKERNEL_PRIVATE -DDRIVER_PRIVATE -DAPPLE -DNeXT -DMODULE_VERSION=$VER -DPRODUCT_NAME=$NAME -DLILU_CUSTOM_IOKIT_INIT=1
       -isystem $SDK/Headers -I$TMP/lilu -I$DEPS/VirtualSMC -O2)
cat > $TMP/info.c <<EOC
#include <mach/mach_types.h>
extern kern_return_t _start(kmod_info_t *ki, void *data);
extern kern_return_t _stop(kmod_info_t *ki, void *data);
extern kern_return_t ${NAME}_kern_start(kmod_info_t *ki, void *data);
extern kern_return_t ${NAME}_kern_stop(kmod_info_t *ki, void *data);
__attribute__((visibility("default"))) KMOD_EXPLICIT_DECL($BID, "$VER", _start, _stop)
__private_extern__ kmod_start_func_t *_realmain = ${NAME}_kern_start;
__private_extern__ kmod_stop_func_t *_antimain = ${NAME}_kern_stop;
__private_extern__ int _kext_apple_cc = __APPLE_CC__;
EOC
clang $FLAGS -c $TMP/info.c -o $TMP/info.o
for f in $SRC/*.cpp; do
  clang++ $FLAGS -std=gnu++17 -nostdinc++ -fno-exceptions -fno-rtti -fapple-kext -I$SRC \
    -Wno-deprecated-declarations -Wno-inconsistent-missing-override -Wno-ossharedptr-misuse -Wno-vla \
    -Wno-unknown-warning-option -Wno-stdlibcxx-not-found -c $f -o $TMP/$(basename ${f%.cpp}).o
done
clang++ -arch x86_64 -mkernel -nostdlib -static -Xlinker -kext -Xlinker -export_dynamic -mmacosx-version-min=10.7 \
  $TMP/*.o $SDK/Library/x86_64/libkmod.a -lcc_kext -o $OUT/Contents/MacOS/$NAME
codesign -f -s - $OUT >/dev/null 2>&1 || true
rm -rf $TMP; echo "built $OUT"
