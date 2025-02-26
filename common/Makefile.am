
inc_HEADERS += \
	common/pkcs11.h \
	common/pkcs11x.h \
	$(NULL)

noinst_LTLIBRARIES += \
	libp11-common.la \
	libp11-library.la \
	libp11-tool.la \
	$(NULL)

check_LTLIBRARIES += \
	libp11-test.la \
	$(NULL)

libp11_common_la_SOURCES = \
	common/argv.c common/argv.h \
	common/attrs.c common/attrs.h \
	common/array.c common/array.h \
	common/base64.c common/base64.h \
	common/buffer.c common/buffer.h \
	common/compat.c common/compat.h \
	common/constants.c common/constants.h \
	common/debug.c common/debug.h \
	common/dict.c common/dict.h \
	common/hash.c common/hash.h \
	common/hex.c common/hex.h \
	common/lexer.c common/lexer.h \
	common/message.c common/message.h \
	common/path.c common/path.h \
	common/pem.c common/pem.h \
	common/pkcs11.h common/pkcs11x.h common/pkcs11i.h \
	common/runtime.c common/runtime.h \
	common/url.c common/url.h \
	common/vsock.c common/vsock.h \
	common/init.h \
	$(NULL)

libp11_library_la_SOURCES = \
	common/library.c common/library.h \
	$(NULL)

libp11_test_la_SOURCES = \
	common/mock.c common/mock.h \
	common/test.c common/test.h \
	$(NULL)

libp11_tool_la_SOURCES = \
	common/print.c common/print.h \
	common/tool.c common/tool.h \
	$(NULL)

if !OS_WIN32
libp11_tool_la_SOURCES += \
	common/unix-peer.c common/unix-peer.h \
	$(NULL)

if NEED_READPASSPHRASE
libp11_tool_la_SOURCES += \
	common/readpassphrase.c \
	$(NULL)
endif
endif

# Tests ----------------------------------------------------------------

common_LIBS = \
	libp11-test.la \
	libp11-common.la \
	$(NULL)

c_tests += \
	test-tests \
	test-compat \
	test-hash \
	test-dict \
	test-array \
	test-constants \
	test-attrs \
	test-buffer \
	test-url \
	test-path \
	test-lexer \
	test-message \
	test-argv \
	test-runtime \
	$(NULL)

test_argv_SOURCES = common/test-argv.c
test_argv_LDADD = $(common_LIBS)

test_array_SOURCES = common/test-array.c
test_array_LDADD = $(common_LIBS)

test_attrs_SOURCES = common/test-attrs.c
test_attrs_LDADD = $(common_LIBS)

test_buffer_SOURCES = common/test-buffer.c
test_buffer_LDADD = $(common_LIBS)

test_compat_SOURCES = common/test-compat.c
test_compat_LDADD = $(common_LIBS)

test_constants_SOURCES = common/test-constants.c
test_constants_LDADD = $(common_LIBS)

test_dict_SOURCES = common/test-dict.c
test_dict_LDADD = $(common_LIBS)

test_hash_SOURCES = common/test-hash.c
test_hash_LDADD = $(common_LIBS)

test_lexer_SOURCES = common/test-lexer.c
test_lexer_LDADD = $(common_LIBS)

test_message_SOURCES = common/test-message.c
test_message_LDADD = $(common_LIBS)

test_path_SOURCES = common/test-path.c
test_path_LDADD = $(common_LIBS)

test_tests_SOURCES = common/test-tests.c
test_tests_LDADD = $(common_LIBS)

test_url_SOURCES = common/test-url.c
test_url_LDADD = $(common_LIBS)

test_runtime_SOURCES = common/test-runtime.c
test_runtime_LDADD = $(common_LIBS)

check_PROGRAMS += \
	common/frob-getauxval \
	common/frob-getenv \
	$(NULL)

common_frob_getauxval_SOURCES = common/frob-getauxval.c
common_frob_getauxval_LDADD = $(common_LIBS)

common_frob_getenv_SOURCES = common/frob-getenv.c
common_frob_getenv_LDADD = $(common_LIBS)

if !OS_WIN32
check_PROGRAMS += common/frob-getprogname

common_frob_getprogname_SOURCES = common/frob-getprogname.c
common_frob_getprogname_LDADD = $(common_LIBS)
endif

if WITH_ASN1
SUFFIXES += .asn .asn.h
.asn.asn.h:
	$(AM_V_GEN)$(ASN1PARSER) -o $@ $<

asn_h = \
	common/basic.asn.h \
	common/openssl.asn.h \
	common/pkix.asn.h \
	$(NULL)

BUILT_SOURCES += $(asn_h)

EXTRA_DIST += \
	common/basic.asn \
	common/openssl.asn \
	common/pkix.asn \
	$(NULL)

noinst_LTLIBRARIES += libp11-asn1.la

libp11_asn1_la_SOURCES = \
	common/asn1.c common/asn1.h \
	common/oid.c common/oid.h \
	common/persist.c common/persist.h \
	common/types.h \
	$(asn_h) \
	$(NULL)

asn1_LIBS = \
	libp11-asn1.la \
	$(LIBTASN1_LIBS) \
	$(common_LIBS) \
	$(NULL)

test_asn1_SOURCES = common/test-asn1.c
test_asn1_LDADD = $(asn1_LIBS)
test_asn1_CFLAGS = $(LIBTASN1_CFLAGS)

test_oid_SOURCES = common/test-oid.c
test_oid_LDADD = $(asn1_LIBS)
test_oid_CFLAGS = $(LIBTASN1_CFLAGS)

c_tests += \
	test-asn1 \
	test-oid \
	$(NULL)
endif

EXTRA_DIST += common/meson.build
