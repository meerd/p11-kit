/*
 * Copyright (c) 2023, Red Hat, Inc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *     * Redistributions of source code must retain the above
 *       copyright notice, this list of conditions and the
 *       following disclaimer.
 *     * Redistributions in binary form must reproduce the
 *       above copyright notice, this list of conditions and
 *       the following disclaimer in the documentation and/or
 *       other materials provided with the distribution.
 *     * The names of contributors to this software may not be
 *       used to endorse or promote products derived from this
 *       software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
 * THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 *
 * Author: Daiki Ueno
 */

#include "config.h"

#define P11_DEBUG_FLAG P11_DEBUG_TOOL
#include "debug.h"
#include "iter.h"
#include "message.h"
#include "print.h"
#include "tool.h"

#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef ENABLE_NLS
#include <libintl.h>
#define _(x) dgettext(PACKAGE_NAME, x)
#else
#define _(x) (x)
#endif

void  print_token_info    (p11_list_printer *printer,
                           CK_TOKEN_INFO    *info);

char *format_token_uri    (CK_TOKEN_INFO    *info);

int   p11_kit_list_tokens (int               argc,
                           char             *argv[]);

static int
list_tokens (const char *token_str,
	     bool only_uris)
{
	int ret = 1;
	CK_FUNCTION_LIST **modules = NULL;
	P11KitUri *uri = NULL;
	P11KitIter *iter = NULL;
	p11_list_printer printer;

	p11_list_printer_init (&printer, stdout, 0);

	uri = p11_kit_uri_new ();
	if (uri == NULL) {
		p11_message (_("failed to allocate memory"));
		goto cleanup;
	}

	if (p11_kit_uri_parse (token_str, P11_KIT_URI_FOR_TOKEN, uri) != P11_KIT_URI_OK) {
		p11_message (_("failed to parse URI"));
		goto cleanup;
	}

	modules = p11_kit_modules_load_and_initialize (0);
	if (modules == NULL) {
		p11_message (_("failed to load and initialize modules"));
		goto cleanup;
	}

	iter = p11_kit_iter_new (uri, P11_KIT_ITER_WITH_TOKENS |
				 P11_KIT_ITER_WITHOUT_OBJECTS);
	if (iter == NULL) {
		p11_debug ("failed to initialize iterator");
		goto cleanup;
	}

	p11_kit_iter_begin (iter, modules);
	while (p11_kit_iter_next (iter) == CKR_OK) {
		CK_TOKEN_INFO *info = p11_kit_iter_get_token (iter);
		char *value;

		if (only_uris) {
			value = format_token_uri (info);
			if (value)
				printf ("%s\n", value);
			free (value);
		} else {
			value = p11_kit_space_strdup (info->label, sizeof (info->label));
			p11_list_printer_start_section (&printer, "token", "%s", value);
			free (value);

			print_token_info (&printer, info);
			p11_list_printer_end_section (&printer);
		}
	}

	ret = 0;

cleanup:
	p11_kit_iter_free (iter);
	p11_kit_uri_free (uri);
	if (modules != NULL)
		p11_kit_modules_finalize_and_release (modules);

	return ret;
}

int
p11_kit_list_tokens (int argc,
		     char *argv[])
{
	int opt;
	bool only_uris = false;

	enum {
		opt_verbose = 'v',
		opt_quiet = 'q',
		opt_help = 'h',
		opt_only_urls = CHAR_MAX + 1,
	};

	struct option options[] = {
		{ "verbose", no_argument, NULL, opt_verbose },
		{ "quiet", no_argument, NULL, opt_quiet },
		{ "only-uris", no_argument, NULL, opt_only_urls },
		{ "help", no_argument, NULL, opt_help },
		{ 0 },
	};

	p11_tool_desc usages[] = {
		{ 0, "usage: p11-kit list-tokens" },
		{ opt_verbose, "show verbose debug output", },
		{ opt_quiet, "suppress command output", },
		{ opt_only_urls, "only print token URIs", },
		{ 0 },
	};

	while ((opt = p11_tool_getopt (argc, argv, options)) != -1) {
		switch (opt) {

		case opt_verbose:
			p11_kit_be_loud ();
			break;

		case opt_quiet:
			p11_kit_be_quiet ();
			break;

		case opt_only_urls:
			only_uris = true;
			break;

		case opt_help:
			p11_tool_usage (usages, options);
			return 0;
		case '?':
			return 2;
		default:
			assert_not_reached ();
			break;
		}
	}

	argc -= optind;
	argv += optind;

	if (argc != 1) {
		p11_tool_usage (usages, options);
		return 2;
	}

	return list_tokens (*argv, only_uris);
}
