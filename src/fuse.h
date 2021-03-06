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

#ifndef VIFM__FUSE_H__
#define VIFM__FUSE_H__

#include "ui/ui.h"
#include "utils/test_helpers.h"

/* Won't mount same file twice */
void fuse_try_mount(FileView *view, const char program[]);

/* Unmounts all FUSE mounded filesystems. */
void fuse_unmount_all(void);

/* Returns non-zero on successful leaving mount point directory. */
int fuse_try_updir_from_a_mount(const char path[], FileView *view);

int fuse_is_in_mounted_dir(const char path[]);

/* Return value:
 *   -1 error occurred.
 *   0  not mount point.
 *   1  left FUSE mount directory. */
int fuse_try_unmount(FileView *view);

/* Returns non-zero in case string is a FUSE mount string. */
int fuse_is_mount_string(const char string[]);

/* Removes fuse mount prefixes from the string. */
void fuse_strip_mount_metadata(char string[]);

TSTATIC_DEFS(
	void format_mount_command(const char mount_point[], const char file_name[],
			const char param[], const char format[], size_t buf_size, char buf[],
			int *foreground);
)

#endif /* VIFM__FUSE_H__ */

/* vim: set tabstop=2 softtabstop=2 shiftwidth=2 noexpandtab cinoptions-=(0 : */
/* vim: set cinoptions+=t0 : */
