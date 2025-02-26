#!/bin/sh

test "${abs_top_builddir+set}" = set || {
	echo "set abs_top_builddir" 1>&2
	exit 1
}

. "$abs_top_builddir/common/test-init.sh"

: ${P11_MODULE_PATH="$abs_top_builddir"/.libs}

setup() {
	testdir=$PWD/test-objects-$$
	test -d "$testdir" || mkdir "$testdir"
	cd "$testdir"
	mkdir tokens
	cat > softhsm2.conf <<EOF
directories.tokendir = $PWD/tokens/
EOF
	export SOFTHSM2_CONF=$PWD/softhsm2.conf

	: ${SOFTHSM2_UTIL=softhsm2-util}
	if ! "$SOFTHSM2_UTIL" --version >/dev/null; then
		skip "softhsm2-util not found"
		return
	fi
	softhsm2-util --init-token --free --label test-import-key --so-pin 12345 --pin 12345

	: ${PKG_CONFIG=pkg-config}
	if ! "$PKG_CONFIG" p11-kit-1 --exists; then
		skip "pkgconfig(p11-kit-1) not found"
		return
	fi

	module_path=$("$PKG_CONFIG" p11-kit-1 --variable=p11_module_path)
	if ! test -e "$module_path/libsofthsm2.so"; then
		skip "unable to resolve libsofthsm2.so"
		return
	fi

	ln -sf "$module_path"/libsofthsm2.so "$P11_MODULE_PATH"
}

teardown() {
	unset SOFTHSM2_CONF
	rm -rf "$testdir"
}

test_import_cert() {
	# Taken from: trust/fixtures/thawte.pem
	cat > export.exp <<EOF
-----BEGIN CERTIFICATE-----
MIIEKjCCAxKgAwIBAgIQYAGXt0an6rS0mtZLL/eQ+zANBgkqhkiG9w0BAQsFADCB
rjELMAkGA1UEBhMCVVMxFTATBgNVBAoTDHRoYXd0ZSwgSW5jLjEoMCYGA1UECxMf
Q2VydGlmaWNhdGlvbiBTZXJ2aWNlcyBEaXZpc2lvbjE4MDYGA1UECxMvKGMpIDIw
MDggdGhhd3RlLCBJbmMuIC0gRm9yIGF1dGhvcml6ZWQgdXNlIG9ubHkxJDAiBgNV
BAMTG3RoYXd0ZSBQcmltYXJ5IFJvb3QgQ0EgLSBHMzAeFw0wODA0MDIwMDAwMDBa
Fw0zNzEyMDEyMzU5NTlaMIGuMQswCQYDVQQGEwJVUzEVMBMGA1UEChMMdGhhd3Rl
LCBJbmMuMSgwJgYDVQQLEx9DZXJ0aWZpY2F0aW9uIFNlcnZpY2VzIERpdmlzaW9u
MTgwNgYDVQQLEy8oYykgMjAwOCB0aGF3dGUsIEluYy4gLSBGb3IgYXV0aG9yaXpl
ZCB1c2Ugb25seTEkMCIGA1UEAxMbdGhhd3RlIFByaW1hcnkgUm9vdCBDQSAtIEcz
MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAsr8nLPvb2FvdeHsbnndm
gcs+vHyu86YnmjSjaDFxODNi5PNxZnmxqWWjpYvVj2AtP0LMqmsywCPLLEHd5N/8
YZzic7IilRFDGF/Eth9XbAoFWCLINkw6fKXRz4aviKdEAhN0cXMKQlkC+BsUa0Lf
b1+6a4KinVvnSr0eAXLbS3ToO39/fR8EtCab4LRarEc9VbjXsCZSKAExQGbY2SS9
9irY7CFJXJv2eul/VTV+lmuNk5Mny5K76qxAwJ/C+IDPXfRa3M50hqY+bAtTyr2S
zhkGcuYMXDhpxwTWvGzOW/b3aJzcJRVIiKHpqfiYnODz1TEoYRFsZ5aNOZnLwkUk
OQIDAQABo0IwQDAPBgNVHRMBAf8EBTADAQH/MA4GA1UdDwEB/wQEAwIBBjAdBgNV
HQ4EFgQUrWyqlGCc7eT/+j4KdCtjA/e2Wb8wDQYJKoZIhvcNAQELBQADggEBABpA
2JVlrAmSicY59BDlqQ5mU1143vokkbvnRFHfxhY0Cu9qRFHqKweKA3rD6z8KLFIW
oCtDuSWQP3CpMyVtRRooOyfPqsMpQhvfO0zAMzRbQYi/aytlryjvsvXDqmbOe1bu
t8jLZ8HJnBoYuMTDSQPxYA5QzUbF83d597YV4Djbxy8ooAw/dyZ02SUS2jHaGh7c
KUGRIjxpp7sC8rZcJwOJ9Abqm+RyguOhCcHpABnTPtRwa7pxpqpYrvS76Wy274fM
m7v/OeZWYdMKp8RcTGB7BXcmer/YB1IsYvdwY9k5vG8cwnncdimvzsUsZAReiDZu
MdRAGmI0Nj81Aa6sY6A=
-----END CERTIFICATE-----
EOF

	if ! "$abs_top_builddir"/p11-kit/p11-kit-testable import-object -q --login --file="export.exp" --label=cert "pkcs11:token=test-import-key?pin-value=12345"; then
		assert_fail "unable to run: p11-kit import-object"
	fi

	if ! "$abs_top_builddir"/p11-kit/p11-kit-testable export-object -q --login "pkcs11:token=test-import-key;object=cert?pin-value=12345" > export.out; then
		assert_fail "unable to run: p11-kit export-object"
	fi

	: ${DIFF=diff}
	if ! ${DIFF} export.exp export.out > export.diff; then
		sed 's/^/# /' export.diff
		assert_fail "output contains incorrect result"
	fi
}

test_import_pubkey_rsa() {
	# Generated and extracted with:
	# p11-kit generate-keypair --type=rsa --bits=2048 --label=RSA 'pkcs11:model=SoftHSM%20v2'
	# p11tool --export 'pkcs11:model=SoftHSM%20v2;object=RSA;type=public'
	cat > export.exp <<EOF
-----BEGIN PUBLIC KEY-----
MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAwK4hLgsRxltV0MudYWt2
yZYWgjHh1B2G5EuJDRvjkYIGiFl6nDk9akWxawgJoVmnZg3Y0a4ogk0H8++WXOZj
bVL1vYq9fxQm2KMNZPX0TM/efh0P8YM8lAyxiiCnCwGkffbUFqOa++4T/30FRfeX
D1YaNYH1ZPf2CKSlm6uUIyqFOakxLcaTVj6oXif4NWtg6Edu6I1rES1sBMnsKQQE
lLXoKLJovpzZgIeJfaxXhnQxfWFH4Ye0BAF4YtzoDXXnSfKVioiwNS06i6ZH9Ztd
P20Te0/BDjxPMmTwml/j5NW/nM0N4tldl2HxbwgEkq+m4aVFYGrp0KOihgC5kDGO
fQIDAQAB
-----END PUBLIC KEY-----
EOF

	if ! "$abs_top_builddir"/p11-kit/p11-kit-testable import-object -q --login --file="export.exp" --label=rsa "pkcs11:token=test-import-key?pin-value=12345"; then
		assert_fail "unable to run: p11-kit import-object"
	fi

	if ! "$abs_top_builddir"/p11-kit/p11-kit-testable export-object -q --login "pkcs11:token=test-import-key;object=rsa?pin-value=12345" > export.out; then
		assert_fail "unable to run: p11-kit export-object"
	fi

	: ${DIFF=diff}
	if ! ${DIFF} export.exp export.out > export.diff; then
		sed 's/^/# /' export.diff
		assert_fail "output contains incorrect result"
	fi
}

test_import_pubkey_ec() {
	# Generated and extracted with:
	# p11-kit generate-keypair --type=ecdsa --curve=secp256r1 --label=EC 'pkcs11:model=SoftHSM%20v2'
	# p11tool --export 'pkcs11:model=SoftHSM%20v2;object=EC;type=public'
	cat > export.exp <<EOF
-----BEGIN PUBLIC KEY-----
MFkwEwYHKoZIzj0CAQYIKoZIzj0DAQcDQgAEsaTJt0debXaW7Hpcrpn7X07SsTk9
6OKFal97R2f9wetmK/EVyZIjCsth4tvkVtvyVsawBF0A2iI7N+uZdiPshw==
-----END PUBLIC KEY-----
EOF

	if ! "$abs_top_builddir"/p11-kit/p11-kit-testable import-object -q --login --file="export.exp" --label=ec "pkcs11:token=test-import-key?pin-value=12345"; then
		assert_fail "unable to run: p11-kit import-object"
	fi

	if ! "$abs_top_builddir"/p11-kit/p11-kit-testable export-object -q --login "pkcs11:token=test-import-key;object=ec?pin-value=12345" > export.out; then
		assert_fail "unable to run: p11-kit export-object"
	fi

	: ${DIFF=diff}
	if ! ${DIFF} export.exp export.out > export.diff; then
		sed 's/^/# /' export.diff
		assert_fail "output contains incorrect result"
	fi
}

run test_import_cert test_import_pubkey_rsa test_import_pubkey_ec
