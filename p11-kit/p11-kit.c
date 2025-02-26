/*
 * Copyright (c) 2011, Collabora Ltd.
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
 * Author: Stef Walter <stefw@collabora.co.uk>
 */

#include "config.h"

#include "compat.h"
#include "debug.h"
#include "message.h"
#include "path.h"
#include "p11-kit.h"

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <getopt.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#ifdef ENABLE_NLS
#include <libintl.h>
#define _(x) dgettext(PACKAGE_NAME, x)
#else
#define _(x) (x)
#endif
#define N_(x) (x)

#include "tool.h"

int       p11_kit_list_modules     (int argc,
                                    char *argv[]);

int       p11_kit_list_tokens      (int argc,
                                    char *argv[]);

int       p11_kit_list_objects     (int argc,
                                    char *argv[]);

int       p11_kit_import_object    (int argc,
                                    char *argv[]);

int       p11_kit_export_object    (int argc,
                                    char *argv[]);

int       p11_kit_delete_object    (int argc,
                                    char *argv[]);

int       p11_kit_generate_keypair (int argc,
                                    char *argv[]);

int       p11_kit_list_profiles    (int argc,
                                    char *argv[]);

int       p11_kit_add_profile      (int argc,
                                    char *argv[]);

int       p11_kit_delete_profile   (int argc,
                                    char *argv[]);

int       p11_kit_list_mechanisms  (int argc,
                                    char *argv[]);

int       p11_kit_print_config     (int argc,
                                    char *argv[]);

int       p11_kit_trust            (int argc,
                                    char *argv[]);

int       p11_kit_external         (int argc,
                                    char *argv[]);

static const p11_tool_command commands[] = {
	{ "list-modules", p11_kit_list_modules, N_("List modules and tokens") },
	{ "list-tokens", p11_kit_list_tokens, N_("List tokens") },
	{ "list-objects", p11_kit_list_objects, N_("List objects of a token") },
	{ "import-object", p11_kit_import_object, N_("Import object into a token") },
	{ "export-object", p11_kit_export_object, N_("Export object matching PKCS#11 URI") },
	{ "delete-object", p11_kit_delete_object, N_("Delete objects matching PKCS#11 URI") },
	{ "generate-keypair", p11_kit_generate_keypair, N_("Generate key-pair on a PKCS#11 token") },
	{ "list-profiles", p11_kit_list_profiles, N_("List PKCS#11 profiles supported by the token") },
	{ "add-profile", p11_kit_add_profile, N_("Add PKCS#11 profile to the token") },
	{ "delete-profile", p11_kit_delete_profile, N_("Delete PKCS#11 profile from the token") },
	{ "print-config", p11_kit_print_config, N_("Print merged configuration") },
	{ "list-mechanisms", p11_kit_list_mechanisms, N_("List supported mechanisms") },
	{ "remote", p11_kit_external, N_("Run a specific PKCS#11 module remotely") },
	{ "server", p11_kit_external, N_("Run a server process that exposes PKCS#11 module remotely") },
	{ P11_TOOL_FALLBACK, p11_kit_external, NULL },
	{ 0, }
};

int
p11_kit_trust (int argc,
               char *argv[])
{
	char **args;

	args = calloc (argc + 2, sizeof (char *));
	return_val_if_fail (args != NULL, 1);

	args[0] = BINDIR "/trust" EXEEXT;
	memcpy (args + 1, argv, sizeof (char *) * argc);
	args[argc + 1] = NULL;

	execv (args[0], args);

	/* At this point we have no command */
	p11_message_err (errno, _("couldn't run trust tool"));

	free (args);
	return 2;
}

int
p11_kit_external (int argc,
                  char *argv[])
{
	const char *private_dir;
	char *filename;
	char *path;

	/* These are trust commands, send them to that tool */
	if (strcmp (argv[0], "extract") == 0) {
		return p11_kit_trust (argc, argv);
	} else if (strcmp (argv[0], "extract-trust") == 0) {
		argv[0] = "extract-compat";
		return p11_kit_trust (argc, argv);
	}

	if (asprintf (&filename, "p11-kit-%s%s", argv[0], EXEEXT) < 0)
		return_val_if_reached (1);

	private_dir = secure_getenv ("P11_KIT_PRIVATEDIR");
	if (!private_dir || !private_dir[0])
		private_dir = PRIVATEDIR;

	/* Add our libexec directory to the path */
	path = p11_path_build (private_dir, filename, NULL);
	return_val_if_fail (path != NULL, 1);

	/* Windows execv() requires the first element of ARGV must be
	 * the executable name */
#ifdef OS_WIN32
	argv[0] = path;
#endif
	argv[argc] = NULL;
	execv (path, argv);

	/* At this point we have no command */
	p11_message (_("'%s' is not a valid command. See 'p11-kit --help'"), argv[0]);

	free (filename);
	free (path);
	return 2;
}

int
main (int argc,
      char *argv[])
{
	return p11_tool_main (argc, argv, commands);
}
