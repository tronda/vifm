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

#ifndef VIFM__FILELIST_H__
#define VIFM__FILELIST_H__

#include <sys/types.h> /* ssize_t */

#include <stddef.h> /* size_t */
#include <stdint.h> /* uint64_t */
#include <stdio.h> /* FILE */

#include "ui/ui.h"
#include "utils/test_helpers.h"

/* Default value of case sensitivity for filters. */
#ifdef _WIN32
#define FILTER_DEF_CASE_SENSITIVITY 0
#else
#define FILTER_DEF_CASE_SENSITIVITY 1
#endif

/* Type of contiguous area of file list. */
typedef enum
{
	FLS_SELECTION, /* Of selected entries. */
	FLS_MARKING,   /* Of marked entries. */
}
FileListScope;

/* Initialization/termination functions. */

/* Prepares views for the first time. */
void init_filelists(void);
/* Reinitializes views. */
void reset_views(void);
/* Loads view file list for the first time. */
void load_initial_directory(FileView *view, const char *dir);

/* Position related functions. */

/* Find index of the file within list of currently visible files of the view.
 * Returns file entry index or -1, if file wasn't found. */
int find_file_pos_in_list(const FileView *const view, const char file[]);
/* Recalculates difference of two panes scroll positions. */
void update_scroll_bind_offset(void);
/* Tries to move cursor by pos_delta positions.  A wrapper for
 * correct_list_pos_on_scroll_up() and correct_list_pos_on_scroll_down()
 * functions. */
void correct_list_pos(FileView *view, ssize_t pos_delta);
/* Returns non-zero if doing something makes sense. */
int correct_list_pos_on_scroll_down(FileView *view, size_t lines_count);
/* Returns new list position after making correction for scrolling down. */
int get_corrected_list_pos_down(const FileView *view, size_t pos_delta);
/* Returns non-zero if doing something makes sense. */
int correct_list_pos_on_scroll_up(FileView *view, size_t lines_count);
/* Returns new list position after making correction for scrolling up. */
int get_corrected_list_pos_up(const FileView *view, size_t pos_delta);
/* Returns non-zero if all files are visible, so no scrolling is needed. */
int all_files_visible(const FileView *view);
void move_to_list_pos(FileView *view, int pos);
/* Ensures that cursor is moved outside of entries of certain type. */
void move_cursor_out_of(FileView *view, FileListScope scope);
/* Adds inactive cursor mark to the view. */
void put_inactive_mark(FileView *view);
/* Returns non-zero in case view can be scrolled up (there are more files). */
int can_scroll_up(const FileView *view);
/* Returns non-zero in case view can be scrolled down (there are more files). */
int can_scroll_down(const FileView *view);
/* Scrolls view down or up at least by specified number of files.  Updates both
 * top and cursor positions.  A wrapper for scroll_up() and scroll_down()
 * functions. */
void scroll_by_files(FileView *view, ssize_t by);
/* Scrolls view up at least by specified number of files.  Updates both top and
 * cursor positions. */
void scroll_up(FileView *view, size_t by);
/* Scrolls view down at least by specified number of files.  Updates both top
 * and cursor positions. */
void scroll_down(FileView *view, size_t by);
/* Returns non-zero if cursor is on the first line. */
int at_first_line(const FileView *view);
/* Returns non-zero if cursor is on the last line. */
int at_last_line(const FileView *view);
/* Returns non-zero if cursor is on the first column. */
int at_first_column(const FileView *view);
/* Returns non-zero if cursor is on the last column. */
int at_last_column(const FileView *view);
/* Returns window top position adjusted for 'scrolloff' option. */
size_t get_window_top_pos(const FileView *view);
/* Returns window middle position adjusted for 'scrolloff' option. */
size_t get_window_middle_pos(const FileView *view);
/* Returns window bottom position adjusted for 'scrolloff' option. */
size_t get_window_bottom_pos(const FileView *view);
/* Moves cursor to the first file in a row. */
void go_to_start_of_line(FileView *view);
/* Returns position of the first file in current line. */
int get_start_of_line(const FileView *view);
/* Returns position of the last file in current line. */
int get_end_of_line(const FileView *view);
/* Returns index of last visible file in the view.  Value returned may be
 * greater than or equal to number of files in the view, which should be
 * threated correctly. */
size_t get_last_visible_file(const FileView *view);
/* Updates current and top line of a view according to scrolloff option value.
 * Returns non-zero if redraw is needed. */
int consider_scroll_offset(FileView *view);

/* Appearance related functions. */

/* Reinitializes view columns. */
void reset_view_sort(FileView *view);
/* Inverts primary key sorting order. */
void invert_sorting_order(FileView *view);
void draw_dir_list(FileView *view);
void erase_current_line_bar(FileView *view);
/* Updates view (maybe postponed) on the screen (redraws file list and
 * cursor) */
void redraw_view(FileView *view);
/* Updates view immediately on the screen (redraws file list and cursor). */
void redraw_view_imm(FileView *view);
/* Updates current view (maybe postponed) on the screen (redraws file list and
 * cursor) */
void redraw_current_view(void);
/* Returns non-zero in case view is visible and shows list of files at the
 * moment. */
int window_shows_dirlist(const FileView *const view);
void change_sort_type(FileView *view, char type, char descending);
/* Returns non-zero if redraw is needed */
int move_curr_line(FileView *view);
/* Returns number of columns in the view. */
size_t calculate_columns_count(FileView *view);

/* Directory traversing functions. */

/* Changes current directory of the view to the path if it's possible and in
 * case of success reloads filelist of the view and sets its cursor position
 * according to directory history of the view. */
void navigate_to(FileView *view, const char path[]);
/* The directory can either be relative to the current
 * directory - ../
 * or an absolute path - /usr/local/share
 * The *directory passed to change_directory() cannot be modified.
 * Symlink directories require an absolute path
 *
 * Return value:
 *  -1  if there were errors.
 *   0  if directory successfully changed and we didn't leave FUSE mount
 *      directory.
 *   1  if directory successfully changed and we left FUSE mount directory. */
int change_directory(FileView *view, const char path[]);
/* Changes pane directory handling path just like cd command does. */
int cd(FileView *view, const char *base_dir, const char *path);
/* Ensures that current directory of the view is a valid one.  Modifies
 * view->curr_dir. */
void leave_invalid_dir(FileView *view);
/* Checks if the view is the directory specified by the path.  Returns non-zero
 * if so, otherwise zero is returned. */
int pane_in_dir(const FileView *view, const char path[]);

/* Selection related functions. */

/* Cleans selection possibly saving it for later use. */
void clean_selected_files(FileView *view);
/* Erases selection not saving anything. */
void erase_selection(FileView *view);
/* Inverts selection of files in the view. */
void invert_selection(FileView *view);
/* Collects currently selected files (or current file only if no selection
 * present) in view->selected_filelist array.  Use free_file_capture() to clean
 * up memory allocated by this function. */
void capture_target_files(FileView *view);
/* Collects count items located at specified indices in view->selected_filelist
 * array.  Use free_file_capture() to clean up memory allocated by this
 * function. */
void capture_files_at(FileView *view, int count, const int indexes[]);
/* Counts number of selected files and writes saves the number in
 * view->selected_files. */
void recount_selected_files(FileView *view);
/* Frees memory from list of captured files. */
void free_file_capture(FileView *view);
/* Remove dot and regexp filters if it's needed to make file visible.  Returns
 * non-zero if file was found. */
int ensure_file_is_selected(FileView *view, const char name[]);

/* Filters related functions. */

void set_dot_files_visible(FileView *view, int visible);
void toggle_dot_files(FileView *view);

void filter_selected_files(FileView *view);
void remove_filename_filter(FileView *view);
void restore_filename_filter(FileView *view);
/* Toggles filter inversion state of the view.  Reloads filelist and resets
 * cursor position. */
void toggle_filter_inversion(FileView *view);

/* Sets regular expression of the local filter for the view.  First call of this
 * function initiates filter set process, which should be ended by call to
 * local_filter_accept() or local_filter_cancel(). */
void local_filter_set(FileView *view, const char filter[]);
/* Updates cursor position and top line of the view according to interactive
 * local filter in progress. */
void local_filter_update_view(FileView *view, int rel_pos);
/* Accepts current value of local filter. */
void local_filter_accept(FileView *view);
/* Sets local filter non-interactively. */
void local_filter_apply(FileView *view, const char filter[]);
/* Cancels local filter set process.  Restores previous values of the filter. */
void local_filter_cancel(FileView *view);
/* Removes local filter after storing its current value to make restore
 * operation possible. */
void local_filter_remove(FileView *view);
/* Restores previously removed local filter. */
void local_filter_restore(FileView *view);

/* Directory history related functions. */

/* Changes current directory of the view to next location backward in
 * history, if available. */
void navigate_backward_in_history(FileView *view);
/* Changes current directory of the view to next location forward in history, if
 * available. */
void navigate_forward_in_history(FileView *view);
void save_view_history(FileView *view, const char *path, const char *file,
		int pos);
int is_in_view_history(FileView *view, const char *path);
void clean_positions_in_history(FileView *view);

/* Typed (with trailing slash for directories) file name functions. */

/* Gets typed filename (not path, just name) for current entry of the view.
 * Allocates memory, that should be freed by the caller. */
char * get_typed_current_fname(const FileView *view);
/* Gets typed filename (not path, just name) for the entry.  Allocates memory,
 * that should be freed by the caller. */
char * get_typed_entry_fname(const dir_entry_t *entry);
/* Gets typed filename (not path, just name).  Allocates memory, that should be
 * freed by the caller. */
char * get_typed_fname(const char path[]);

/* Other functions. */

FILE * use_info_prog(const char *viewer);
/* Loads filelist for the view, but doesn't redraw the view.  The reload
 * parameter should be set in case of view refresh operation. */
void populate_dir_list(FileView *view, int reload);
/* Loads file list for the view and redraws the view.  The reload parameter
 * should be set in case of view refresh operation. */
void load_dir_list(FileView *view, int reload);
/* Resorts view without reloading it and preserving currently file under cursor
 * along with its relative position in the list.  msg parameter controls whether
 * to show "Sorting..." statusbar message. */
void resort_dir_list(int msg, FileView *view);
void load_saving_pos(FileView *view, int reload);
char * get_current_file_name(FileView *view);
/* Checks whether content in the current directory of the view changed and
 * reloads the view if so. */
void check_if_filelist_have_changed(FileView *view);
/* Checks whether cd'ing into path is possible. Shows cd errors to a user.
 * Returns non-zero if it's possible, zero otherwise. */
int cd_is_possible(const char *path);
/* Checks whether directory list was loaded at least once since startup. */
int is_dir_list_loaded(FileView *view);
/* Checks whether view can and should be navigated to the path (no need to do
 * anything if already there).  Returns non-zero if path should be changed. */
int view_needs_cd(const FileView *view, const char path[]);
/* Sets view's current directory from path value. */
void set_view_path(FileView *view, const char path[]);
/* Returns possible cached or calculated value of file size. */
uint64_t get_file_size_by_entry(const FileView *view, size_t pos);
/* Checks whether entry corresponds to a directory.  Returns non-zero if so,
 * otherwise zero is returned. */
int is_directory_entry(const dir_entry_t *entry);
/* Loads pointer to the next selected entry in file list of the view.  *entry
 * should be NULL for the first call and result of previous call otherwise.
 * Returns zero when there is no more entries to supply, otherwise non-zero is
 * returned.  List of entries shouldn't be reloaded between invocations of this
 * function. */
int iter_selected_entries(FileView *view, dir_entry_t **entry);
/* Same as iter_selected_entries() function, but checks for marks. */
int iter_marked_entries(FileView *view, dir_entry_t **entry);
/* Maps one of file list entries to its position in the list.  Returns the
 * position or -1 on wrong entry. */
int entry_to_pos(const FileView *view, const dir_entry_t *entry);
/* Fills the buffer with the full path to file under cursor. */
void get_current_full_path(const FileView *view, size_t buf_len, char buf[]);
/* Fills the buffer with the full path to file at specified position. */
void get_full_path_at(const FileView *view, int pos, size_t buf_len,
		char buf[]);
/* Fills the buffer with the full path to file of specified file list entry. */
void get_full_path_of(const dir_entry_t *entry, size_t buf_len, char buf[]);
/* Ensures that either entries at specified positions, selected entries or file
 * under cursor is marked. */
void check_marking(FileView *view, int count, const int indexes[]);
/* Marks files at positions specified in the indexes array of size count. */
void mark_files_at(FileView *view, int count, const int indexes[]);
/* Marks selected files of the view. */
void mark_selected(FileView *view);

TSTATIC_DEFS(
	int file_is_visible(FileView *view, const char filename[], int is_dir);
)

#endif /* VIFM__FILELIST_H__ */

/* vim: set tabstop=2 softtabstop=2 shiftwidth=2 noexpandtab cinoptions-=(0 : */
/* vim: set cinoptions+=t0 : */
