/*
 *
 * Copyright © 2013 Serge Hallyn <serge.hallyn@ubuntu.com>.
 * Copyright © 2013 Canonical Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "../lxc/lxccontainer.h"

#include <stdio.h>
#include <libgen.h>
#include <unistd.h>
#include <sys/types.h>

#include <lxc/lxc.h>
#include <lxc/log.h>

#include "arguments.h"
#include "utils.h"

lxc_log_define(lxc_destroy, lxc);

static int my_parser(struct lxc_arguments* args, int c, char* arg)
{
	switch (c) {
	case 'f': args->force = 1; break;
	}
	return 0;
}

static const struct option my_longopts[] = {
	{"force", no_argument, 0, 'f'},
	LXC_COMMON_OPTIONS
};

static struct lxc_arguments my_args = {
	.progname = "lxc-destroy",
	.help     = "\
--name=NAME [-f] [-P lxcpath]\n\
\n\
lxc-destroy destroys a container with the identifier NAME\n\
\n\
Options :\n\
  -n, --name=NAME   NAME for name of the container\n\
  -f, --force       wait for the container to shut down\n",
	.options  = my_longopts,
	.parser   = my_parser,
	.checker  = NULL,
};

int main(int argc, char *argv[])
{
	struct lxc_container *c;

	/* this is a short term test.  We'll probably want to check for
	 * write access to lxcpath instead */
	if (geteuid()) {
		fprintf(stderr, "%s must be run as root\n", argv[0]);
		exit(1);
	}

	if (lxc_arguments_parse(&my_args, argc, argv))
		exit(1);

	if (lxc_log_init(my_args.name, my_args.log_file, my_args.log_priority,
			 my_args.progname, my_args.quiet, my_args.lxcpath[0]))
		exit(1);

	c = lxc_container_new(my_args.name, my_args.lxcpath[0]);
	if (!c) {
		fprintf(stderr, "System error loading container\n");
		exit(1);
	}

	if (c->is_running(c)) {
		if (!my_args.force) {
			fprintf(stderr, "%s is running\n", my_args.name);
			exit(1);
		}
		c->stop(c);
	}

	exit(c->destroy(c) ? 0 : 1);
}
