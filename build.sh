#!/bin/bash
set -e

SCRIPT_DIR=$(realpath "$(dirname "${BASH_SOURCE[0]}")")
echo $SCRIPT_DIR

BUILD_DIR="$SCRIPT_DIR/build-$CIFUZZ_BUILD_STEP"
mkdir -p $BUILD_DIR
cd $BUILD_DIR

export CFLAGS+=" -fno-sanitize=undefined"

$SCRIPT_DIR/configure CFLAGS="$CFLAGS -DNO_CHECK_MEMWIPE" \
	--enable-imc-test \
	--enable-monolithic \
	--disable-shared \
	--enable-static
make -j


${CC} ${CFLAGS} ${FUZZ_TEST_CFLAGS} ${FUZZ_TEST_LDFLAGS} -o $SCRIPT_DIR/$FUZZ_TEST $SCRIPT_DIR/$FUZZ_TEST.c -Wl,-rpath '-Wl,$$ORIGIN' -L. \
	--include $BUILD_DIR/config.h \
	-I$SCRIPT_DIR/src/libstrongswan \
	-I$SCRIPT_DIR/src/libimcv \
	-I$SCRIPT_DIR/src/libtncif \
	-I$SCRIPT_DIR/src \
	-I$SCRIPT_DIR/src/libcharon/encoding/payloads \
	-I$SCRIPT_DIR/src/libcharon \
	-I$SCRIPT_DIR/src/libtpmtss \
	-I$SCRIPT_DIR/src/libtnccs \
	-I$SCRIPT_DIR/src/libstrongswan/settings \
	-I$SCRIPT_DIR/src/libtnccs/plugins/tnccs_2 \
	-I$SCRIPT_DIR/src/libcharon/plugins/vici \
	-I$SCRIPT_DIR/src/libstrongswan/collections \
	$BUILD_DIR/src/libstrongswan/.libs/libstrongswan.a \
        -Wl,-Bstatic -lcrypto -Wl,-Bdynamic -v

#cifuzz:build-template:begin
## Fuzz test {{ .FileName }}
#cifuzz:build-template:end
# Fuzz test fuzz_settings_parser_parse.cpp
# Fuzz test fuzz_settings_parser_parse_1.c
# Fuzz test fuzz_payload_create.c
# Fuzz test fuzz_identification_create_from_string_1.c
# Fuzz test fuzz_load_creds_cfg.c
# Fuzz test fuzz_rsa_pss_params_parse.c
# Fuzz test fuzz_settings_parser_parse_4.c
