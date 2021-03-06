/* vifm
 * Copyright (C) 2001 Ken Steen.
 * Copyright (C) 2011 xaizek.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 */

#include "path.h"

#ifdef _WIN32
#include <ctype.h>
#endif
#include <stddef.h> /* NULL size_t */
#include <stdio.h>  /* snprintf() */
#include <stdlib.h> /* malloc() free() */
#include <string.h> /* strcat() strcmp() strcasecmp() strdup() strncmp()
                       strncasecmp() strncat() strchr() strcpy() strlen()
                       strrchr() */

#ifndef _WIN32
#include <pwd.h> /* getpwnam() */
#endif

#include "../cfg/config.h"
#include "../path_env.h"
#include "env.h"
#include "fs.h"
#include "fs_limits.h"
#include "str.h"
#include "utils.h"

static int skip_dotdir_if_any(const char *path[], int fully);
static char * try_replace_tilde(const char path[]);
static char * find_ext_dot(const char path[]);

/* like chomp() but removes trailing slash */
void
chosp(char *path)
{
	size_t len;

	if(path[0] == '\0')
		return;

	len = strlen(path);
	if(path[len - 1] == '/')
		path[len - 1] = '\0';
}

int
ends_with_slash(const char *str)
{
	return str[0] != '\0' && str[strlen(str) - 1] == '/';
}

int
path_starts_with(const char *path, const char *begin)
{
	size_t len = strlen(begin);

	if(len > 0 && begin[len - 1] == '/')
		len--;

	if(strnoscmp(path, begin, len) != 0)
		return 0;

	return (path[len] == '\0' || path[len] == '/');
}

int
paths_are_equal(const char s[], const char t[])
{
	/* Some additional space is allocated for adding slashes. */
	char s_can[strlen(s) + 8];
	char t_can[strlen(t) + 8];

	canonicalize_path(s, s_can, sizeof(s_can));
	canonicalize_path(t, t_can, sizeof(t_can));

	return stroscmp(s_can, t_can) == 0;
}

void
canonicalize_path(const char directory[], char buf[], size_t buf_size)
{
	/* Source string pointer. */
	const char *p = directory;
	/* Destination string pointer. */
	char *q = buf - 1;

	buf[0] = '\0';

#ifdef _WIN32
	/* Handle first component of a UNC path. */
	if(p[0] == '/' && p[1] == '/' && p[2] != '/')
	{
		strcpy(buf, "//");
		q = buf + 1;
		p += 2;
		while(*p != '\0' && *p != '/')
		{
			*++q = *p++;
		}
		buf = q + 1;
	}
#endif

	while(*p != '\0' && (size_t)((q + 1) - buf) < buf_size - 1)
	{
		const int prev_dir_present = (q != buf - 1 && *q == '/');
		if(skip_dotdir_if_any(&p, prev_dir_present))
		{
			/* skip_dotdir_if_any() function did all job for us. */
		}
		else if(prev_dir_present &&
				(strncmp(p, "../", 3) == 0 || strcmp(p, "..") == 0) &&
				strcmp(buf, "../") != 0)
		{
			/* Remove the last path component added. */
#ifdef _WIN32
			/* Special handling of Windows disk name. */
			if(*(q - 1) != ':')
#endif
			{
				p++;
				q--;
				while(q >= buf && *q != '/')
				{
					q--;
				}
			}
		}
		else if(*p == '/')
		{
			/* Don't add more than one slash between path components. */
			if(!prev_dir_present)
			{
				*++q = '/';
			}
		}
		else
		{
			/* Copy current path component till the end. */
			*++q = *p;
			while(p[1] != '\0' && p[1] != '/' &&
					(size_t)((q + 1) - buf) < buf_size - 1)
			{
				*++q = *++p;
			}
		}

		p++;
	}

	if(*q != '/')
	{
		*++q = '/';
	}

	*++q = '\0';
}

/* Checks whether *path begins with current directory component ('./') and moves
 * *path to the last character of such component (to slash if present) if fully
 * is non-zero, otherwise to the previous of the last character. When fully is
 * zero the function normalizes '\.\.\.+/?' on Windows to '\./?'. Returns
 * non-zero if a path component was fully skipped. */
static int
skip_dotdir_if_any(const char *path[], int fully)
{
	size_t dot_count = 0;
	while((*path)[dot_count] == '.')
	{
		dot_count++;
	}
	if((dot_count == 1
#ifdef _WIN32
				|| dot_count > 2
#endif
				) &&
			strchr("/", (*path)[dot_count]) != NULL)
	{
		if(!fully)
		{
			dot_count--;
		}
		if((*path)[dot_count] == '\0')
		{
			*path += dot_count - 1;
		}
		else
		{
			*path += dot_count;
		}
		return fully;
	}
	return 0;
}

const char *
make_rel_path(const char *path, const char *base)
{
	static char buf[PATH_MAX];

	const char *p = path, *b = base;
	int i;
	int nslashes;

#ifdef _WIN32
	if(path[1] == ':' && base[1] == ':' && path[0] != base[0])
	{
		canonicalize_path(path, buf, sizeof(buf));
		return buf;
	}
#endif

	while(p[0] != '\0' && p[1] != '\0' && b[0] != '\0' && b[1] != '\0')
	{
		const char *op = p, *ob = b;
		if((p = strchr(p + 1, '/')) == NULL)
			p = path + strlen(path);
		if((b = strchr(b + 1, '/')) == NULL)
			b = base + strlen(base);
		if(p - path != b - base || strnoscmp(path, base, p - path) != 0)
		{
			p = op;
			b = ob;
			break;
		}
	}

	canonicalize_path(b, buf, sizeof(buf));
	chosp(buf);

	nslashes = 0;
	for(i = 0; buf[i] != '\0'; i++)
		if(buf[i] == '/')
			nslashes++;

	buf[0] = '\0';
	while(nslashes-- > 0)
		strcat(buf, "../");
	if(*p == '/')
		p++;
	canonicalize_path(p, buf + strlen(buf), sizeof(buf) - strlen(buf));
	chosp(buf);

	if(buf[0] == '\0')
		strcpy(buf, ".");

	return buf;
}

int
is_path_absolute(const char *path)
{
#ifdef _WIN32
	if(isalpha(path[0]) && path[1] == ':')
		return 1;
	if(path[0] == '/' && path[1] == '/')
		return 1;
#endif
	return (path[0] == '/');
}

int
is_root_dir(const char *path)
{
#ifdef _WIN32
	if(isalpha(path[0]) && stroscmp(path + 1, ":/") == 0)
		return 1;

	if(path[0] == '/' && path[1] == '/' && path[2] != '\0')
	{
		char *p = strchr(path + 2, '/');
		if(p == NULL || p[1] == '\0')
			return 1;
	}
#endif
	return (path[0] == '/' && path[1] == '\0');
}

int
is_unc_root(const char *path)
{
#ifdef _WIN32
	if(is_unc_path(path) && path[2] != '\0')
	{
		char *p = strchr(path + 2, '/');
		if(p == NULL || p[1] == '\0')
			return 1;
	}
	return 0;
#else
	return 0;
#endif
}

/*
 * Escape the filename for the purpose of inserting it into the shell.
 *
 * quote_percent means prepend percent sign with a percent sign
 *
 * Returns new string, caller should free it.
 */
char *
escape_filename(const char *string, int quote_percent)
{
	size_t len;
	size_t i;
	char *ret, *dup;

	len = strlen(string);

	dup = ret = malloc(len*2 + 2 + 1);

	if(*string == '-')
	{
		*dup++ = '.';
		*dup++ = '/';
	}
	else if(*string == '~')
	{
		*dup++ = *string++;
	}

	for(i = 0; i < len; i++, string++, dup++)
	{
		switch(*string)
		{
			case '%':
				if(quote_percent)
					*dup++ = '%';
				break;

			/* Escape the following characters anywhere in the line. */
			case '\'':
			case '\\':
			case '\r':
			case '\n':
			case '\t':
			case '"':
			case ';':
			case ' ':
			case '?':
			case '|':
			case '[':
			case ']':
			case '{':
			case '}':
			case '<':
			case '>':
			case '`':
			case '!':
			case '$':
			case '&':
			case '*':
			case '(':
			case ')':
			case '#':
				*dup++ = '\\';
				break;

			/* Escape the following characters only at the beginning of the line. */
			case '~':
				if(dup == ret)
					*dup++ = '\\';
				break;
		}
		*dup = *string;
	}
	*dup = '\0';
	return ret;
}

char *
replace_home_part(const char directory[])
{
	static char buf[PATH_MAX];
	size_t len;

	len = strlen(cfg.home_dir) - 1;
	if(strnoscmp(directory, cfg.home_dir, len) == 0 &&
			(directory[len] == '\0' || directory[len] == '/'))
	{
		strncat(strcpy(buf, "~"), directory + len, sizeof(buf) - strlen(buf) - 1);
	}
	else
	{
		copy_str(buf, sizeof(buf), directory);
	}

	if(!is_root_dir(buf))
		chosp(buf);

	return buf;
}

char *
expand_tilde(const char path[])
{
	char *const expanded_path = try_replace_tilde(path);
	if(expanded_path == path)
	{
		return strdup(path);
	}
	return expanded_path;
}

char *
replace_tilde(char path[])
{
	char *const expanded_path = try_replace_tilde(path);
	if(expanded_path != path)
	{
		free(path);
	}
	return expanded_path;
}

/* Tries to expands tilde in the front of the path.  Returns the path or newly
 * allocated string without tilde. */
static char *
try_replace_tilde(const char path[])
{
#ifndef _WIN32
	char name[NAME_MAX];
	const char *p;
	char *result;
	struct passwd *pw;
#endif

	if(path[0] != '~')
	{
		return (char *)path;
	}

	if(path[1] == '\0' || path[1] == '/')
	{
		char *const result = format_str("%s%s", cfg.home_dir,
				(path[1] == '/') ? (path + 2) : "");
		return result;
	}

#ifndef _WIN32
	if((p = strchr(path, '/')) == NULL)
	{
		p = path + strlen(path);
		copy_str(name, sizeof(name), path + 1);
	}
	else
	{
		snprintf(name, p - (path + 1) + 1, "%s", path + 1);
		p++;
	}

	if((pw = getpwnam(name)) == NULL)
	{
		return (char *)path;
	}

	chosp(pw->pw_dir);
	result = format_str("%s/%s", pw->pw_dir, p);

	return result;
#else
	return (char *)path;
#endif
}

char *
get_last_path_component(const char path[])
{
	char *slash = strrchr(path, '/');
	if(slash == NULL)
	{
		slash = (char *)path;
	}
	else if(slash[1] == '\0')
	{
		while(slash > path && slash[0] == '/')
			slash--;
		while(slash > path + 1 && slash[-1] != '/')
			slash--;
	}
	else
	{
		slash++;
	}
	return slash;
}

void
remove_last_path_component(char path[])
{
	char *slash;

	while(ends_with_slash(path))
	{
		chosp(path);
	}

	slash = strrchr(path, '/');
	if(slash != NULL)
	{
		const int offset = is_root_dir(path) ? 1 : 0;
		slash[offset] = '\0';
	}
}

int
is_path_well_formed(const char *path)
{
#ifndef _WIN32
	return strchr(path, '/') != NULL;
#else
	return is_unc_path(path) || (strlen(path) >= 2 && path[1] == ':' &&
			drive_exists(path[0]));
#endif
}

void
ensure_path_well_formed(char *path)
{
	if(is_path_well_formed(path))
	{
		return;
	}

#ifndef _WIN32
	strcpy(path, "/");
#else
	strcpy(path, env_get("SYSTEMDRIVE"));
	strcat(path, "/");
#endif
}

int
contains_slash(const char *path)
{
	char *slash_pos = strchr(path, '/');
#ifdef _WIN32
	if(slash_pos == NULL)
		slash_pos = strchr(path, '\\');
#endif
	return slash_pos != NULL;
}

char *
find_slashr(const char *path)
{
	char *result = strrchr(path, '/');
#ifdef _WIN32
	if(result == NULL)
		result = strrchr(path, '\\');
#endif
	return result;
}

char *
cut_extension(char path[])
{
	char *e;
	char *ext;

	if((ext = strrchr(path, '.')) == NULL)
		return path + strlen(path);

	*ext = '\0';
	if((e = strrchr(path, '.')) != NULL && stroscmp(e + 1, "tar") == 0)
	{
		*ext = '.';
		ext = e;
	}
	*ext = '\0';

	return ext + 1;
}

void
split_ext(char path[], int *root_len, const char **ext_pos)
{
	char *const dot = find_ext_dot(path);

	if(dot == NULL)
	{
		const size_t len = strlen(path);

		*ext_pos = path + len;
		*root_len = len;

		return;
	}

	*dot = '\0';
	*ext_pos = dot + 1;
	*root_len = dot - path;
}

char *
get_ext(const char path[])
{
	char *const dot = find_ext_dot(path);

	if(dot == NULL)
	{
		return (char *)path + strlen(path);
	}

	return dot + 1;
}

/* Gets extension for file path.  Returns pointer to the dot or NULL if file
 * name has no extension. */
static char *
find_ext_dot(const char path[])
{
	const char *const slash = strrchr(path, '/');
	char *const dot = strrchr(path, '.');

	const int no_ext = dot == NULL
	                || (slash != NULL && dot < slash)
	                || dot == path
	                || dot == slash + 1;

	return no_ext ? NULL : dot;
}

void
exclude_file_name(char path[])
{
	if(path_exists(path, DEREF) && !is_valid_dir(path))
	{
		remove_last_path_component(path);
	}
}

int
is_parent_dir(const char path[])
{
	return path[0] == '.' && path[1] == '.' &&
		((path[2] == '/' && path[3] == '\0') || path[2] == '\0');
}

int
is_builtin_dir(const char name[])
{
	return name[0] == '.'
	    && (name[1] == '\0' || (name[1] == '.' && name[2] == '\0'));
}

int
find_cmd_in_path(const char cmd[], size_t path_len, char path[])
{
	size_t i;
	size_t paths_count;
	char **paths;

	paths = get_paths(&paths_count);
	for(i = 0; i < paths_count; i++)
	{
		char tmp_path[PATH_MAX];
		snprintf(tmp_path, sizeof(tmp_path), "%s/%s", paths[i], cmd);

		/* Need to check for executable, not just a file, as this additionally
		 * checks for path with different executable extensions on Windows. */
		if(executable_exists(tmp_path))
		{
			if(path != NULL)
			{
				copy_str(path, path_len, tmp_path);
			}
			return 0;
		}
	}
	return 1;
}

void
generate_tmp_file_name(const char prefix[], char buf[], size_t buf_len)
{
	snprintf(buf, buf_len, "%s/%s", get_tmpdir(), prefix);
#ifdef _WIN32
	to_forward_slash(buf);
#endif
	copy_str(buf, buf_len, make_name_unique(buf));
}

const char *
get_tmpdir(void)
{
	return env_get_one_of_def("/tmp/", "TMPDIR", "TEMP", "TEMPDIR", "TMP", NULL);
}

#ifdef _WIN32

int
is_unc_path(const char path[])
{
	return (path[0] == '/' && path[1] == '/' && path[2] != '/');
}

void
to_forward_slash(char path[])
{
	replace_char(path, '\\', '/');
}

void
to_back_slash(char path[])
{
	replace_char(path, '/', '\\');
}

#endif

/* vim: set tabstop=2 softtabstop=2 shiftwidth=2 noexpandtab cinoptions-=(0 : */
/* vim: set cinoptions+=t0 : */
