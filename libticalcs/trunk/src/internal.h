/*  libticalcs - Ti Calculator library, a part of the TiLP project
 *  Copyright (C) 1999-2012  Romain Liévin
 *  Copyright (C) 2012       Lionel Debroux
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software Foundation,
 *  Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/**
 * \file internal.h
 * \brief Definitions for internal (libticalcs) usage.
 */

#ifndef __TICALCS_INTERNAL__
#define __TICALCS_INTERNAL__

#include "error.h"
#include "pause.h"
#include "macros.h"

#define VALIDATE_NONNULL(ptr) \
	do \
	{ \
		if (ptr == NULL) \
		{ \
			ticalcs_critical("%s: " #ptr " is NULL", __FUNCTION__); \
			return ERR_INVALID_PARAMETER; \
		} \
	} while(0);
#define VALIDATE_HANDLE(handle) \
	do \
	{ \
		if (!ticalcs_validate_handle(handle)) \
		{ \
			ticalcs_critical("%s: " #handle " is invalid", __FUNCTION__); \
			return ERR_INVALID_HANDLE; \
		} \
	} while(0);
#define VALIDATE_CALCFNCTS(calc) \
	do \
	{ \
		if (!ticalcs_validate_calcfncts(calc)) \
		{ \
			ticalcs_critical("%s: " # calc " is invalid", __FUNCTION__); \
			return ERR_INVALID_HANDLE; \
		} \
	} while(0);
#define VALIDATE_BACKUPCONTENT(content) \
	do \
	{ \
		if (!ticalcs_validate_backupcontent(content)) \
		{ \
			ticalcs_critical("%s: " #content " is NULL", __FUNCTION__); \
			return ERR_INVALID_PARAMETER; \
		} \
	} while(0);
#define VALIDATE_FILECONTENT(content) \
	do \
	{ \
		if (!ticalcs_validate_filecontent(content)) \
		{ \
			ticalcs_critical("%s: " #content " is NULL", __FUNCTION__); \
			return ERR_INVALID_PARAMETER; \
		} \
	} while(0);
#define VALIDATE_FILECONTENT_ENTRIES(content) \
	do \
	{ \
		if (!ticalcs_validate_filecontent_entries(content)) \
		{ \
			ticalcs_critical("%s: " #content "->entries is NULL", __FUNCTION__); \
			return ERR_INVALID_PARAMETER; \
		} \
	} while(0);
#define VALIDATE_FLASHCONTENT(content) \
	do \
	{ \
		if (!ticalcs_validate_flashcontent(content)) \
		{ \
			ticalcs_critical("%s: " #content " is NULL", __FUNCTION__); \
			return ERR_INVALID_PARAMETER; \
		} \
	} while(0);
#define VALIDATE_VARENTRY(var) \
	do \
	{ \
		if (!ticalcs_validate_varentry(var)) \
		{ \
			ticalcs_critical("%s: " #var " is NULL", __FUNCTION__); \
			return ERR_INVALID_PARAMETER; \
		} \
	} while(0);
#define VALIDATE_VARREQUEST(var) \
	do \
	{ \
		if (!ticalcs_validate_varrequest(var)) \
		{ \
			ticalcs_critical("%s: " #var " is NULL", __FUNCTION__); \
			return ERR_INVALID_PARAMETER; \
		} \
	} while(0);
#define VALIDATE_CALCUPDATE(upd) \
	do \
	{ \
		if (!ticalcs_validate_calcupdate(upd)) \
		{ \
			ticalcs_critical("%s: " #upd " is NULL", __FUNCTION__); \
			return ERR_INVALID_PARAMETER; \
		} \
	} while(0);
#define VALIDATE_SCREENWIDTH(width) \
	do \
	{ \
		if (width > 320) \
		{ \
			ticalcs_critical("%s: no calculator model known to this library has screens of width > 320 pixels", __FUNCTION__); \
			return ERR_INVALID_PARAMETER; \
		} \
	} while(0);
#define VALIDATE_SCREENHEIGHT(height) \
	do \
	{ \
		if (height > 240) \
		{ \
			ticalcs_critical("%s: no calculator model known to this library has screens of height > 240 pixels", __FUNCTION__); \
			return ERR_INVALID_PARAMETER; \
		} \
	} while(0);

#define RETURN_IF_HANDLE_NOT_ATTACHED(handle) \
	do \
	{ \
		if (!handle->attached) \
		{ \
			return ERR_NO_CABLE; \
		} \
	} while(0);
#define RETURN_IF_HANDLE_NOT_OPEN(handle) \
	do \
	{ \
		if (!handle->open) \
		{ \
			return ERR_NO_CABLE; \
		} \
	} while(0);
#define RETURN_IF_HANDLE_BUSY(handle) \
	do \
	{ \
		if (handle->busy) \
		{ \
			return ERR_BUSY; \
		} \
	} while(0);

static inline int ticalcs_validate_handle(CalcHandle * handle)
{
	return handle != NULL;
}

static inline int ticalcs_validate_calcfncts(const CalcFncts * calc)
{
	return calc != NULL;
}

static inline int ticalcs_validate_backupcontent(BackupContent * content)
{
	return content != NULL;
}

static inline int ticalcs_validate_filecontent(FileContent * content)
{
	return content != NULL;
}

static inline int ticalcs_validate_filecontent_entries(FileContent * content)
{
	return content->num_entries == 0 || content->entries != NULL;
}

static inline int ticalcs_validate_flashcontent(FlashContent * content)
{
	return content != NULL;
}

static inline int ticalcs_validate_varentry(VarEntry * var)
{
	return var != NULL;
}

static inline int ticalcs_validate_varrequest(VarRequest * var)
{
	return var != NULL;
}

static inline int ticalcs_validate_calcupdate(CalcUpdate * upd)
{
	return upd != NULL && upd->start != NULL && upd->stop != NULL && upd->refresh != NULL && upd->label != NULL && upd->pbar != NULL;
}

static inline void * ticalcs_alloc_screen(size_t len)
{
	return g_malloc(len);
}

static inline void * ticalcs_realloc_screen(void * mem, size_t len)
{
	return g_realloc(mem, len);
}

static inline int dirlist_init_tree(CalcHandle * handle, GNode ** tree, const char * type)
{
	int ret = ERR_MALLOC;

	(*tree) = g_node_new(NULL);
	if (*tree != NULL)
	{
		TreeInfo *ti = (TreeInfo *)g_malloc(sizeof(TreeInfo));
		if (ti != NULL)
		{
			ti->model = handle->model;
			ti->type = type;
			(*tree)->data = ti;
			ret = 0;
		}
	}

	return ret;
}

static inline int dirlist_init_trees(CalcHandle * handle, GNode ** vars, GNode ** apps)
{
	int ret = dirlist_init_tree(handle, vars, VAR_NODE_NAME);

	if (!ret)
	{
		ret = dirlist_init_tree(handle, apps, APP_NODE_NAME);
	}

	return ret;
}

static inline GNode * dirlist_create_append_node(void * data, GNode ** tree)
{
	GNode * node = g_node_new(data);
	if (node != NULL)
	{
		g_node_append(*tree, node);
	}
	return node;
}

#define ticalcs_slprintf(str, size, format, ...) \
	snprintf(str, size - 1, format, ##__VA_ARGS__); \
	str[size - 1] = 0;

#define ticalcs_strlcpy(dst, src, size) \
	strncpy(dst, src, size - 1); \
	dst[size - 1] = 0;

#define ticalcs_update_start(handle)     handle->updat->start()
#define ticalcs_update_stop(handle)      handle->updat->stop()
#define ticalcs_update_refresh(handle)   handle->updat->refresh()
#define ticalcs_update_pbar(handle)      handle->updat->pbar()
#define ticalcs_update_label(handle)     handle->updat->label()

#define ticalcs_update_canceled(handle)  (handle->updat->cancel)

#define FILL_CALC_EVENT_DATA(event, type_, retval_, attached_, open_, operation_) \
	do \
	{ \
		event.version = 1; \
		event.type = type_; \
		event.retval = retval_; \
		event.attached = attached_; \
		event.open = open_; \
		event.operation = operation_; \
	} while(0);

// backup.c
int tixx_recv_all_vars_backup(CalcHandle* handle, FileContent* content);


// calc_00.c

int noop_is_ready (CalcHandle* handle);
int noop_send_key (CalcHandle* handle, uint32_t key);
int noop_execute (CalcHandle* handle, VarEntry *ve, const char* args);
int noop_recv_screen (CalcHandle* handle, CalcScreenCoord* sc, uint8_t** bitmap);
int noop_get_dirlist (CalcHandle* handle, GNode** vars, GNode** apps);
int noop_get_memfree (CalcHandle* handle, uint32_t* ram, uint32_t* flash);
int noop_send_backup (CalcHandle* handle, BackupContent* content);
int noop_recv_backup (CalcHandle* handle, BackupContent* content);
int noop_send_var (CalcHandle* handle, CalcMode mode, FileContent* content);
int noop_recv_var (CalcHandle* handle, CalcMode mode, FileContent* content, VarRequest* vr);
int noop_send_var_ns (CalcHandle* handle, CalcMode mode, FileContent* content);
int noop_recv_var_ns (CalcHandle* handle, CalcMode mode, FileContent* content, VarEntry** ve);
int noop_send_flash (CalcHandle* handle, FlashContent* content);
int noop_recv_flash (CalcHandle* handle, FlashContent* content, VarRequest* vr);
int noop_send_os (CalcHandle* handle, FlashContent* content);
int noop_recv_idlist (CalcHandle* handle, uint8_t* idlist);
int noop_dump_rom_1 (CalcHandle* handle);
int noop_dump_rom_2 (CalcHandle* handle, CalcDumpSize size, const char *filename);
int noop_set_clock (CalcHandle* handle, CalcClock* _clock);
int noop_get_clock (CalcHandle* handle, CalcClock* _clock);
int noop_del_var (CalcHandle* handle, VarRequest* vr);
int noop_new_folder (CalcHandle* handle, VarRequest* vr);
int noop_get_version (CalcHandle* handle, CalcInfos* infos);
int noop_send_cert (CalcHandle* handle, FlashContent* content);
int noop_recv_cert (CalcHandle* handle, FlashContent* content);
int noop_rename_var (CalcHandle* handle, VarRequest* oldname, VarRequest* newname);
int noop_change_attr (CalcHandle* handle, VarRequest* vr, FileAttr attr);
int noop_send_all_vars_backup (CalcHandle* handle, FileContent* content);
int noop_recv_all_vars_backup (CalcHandle* handle, FileContent* content);


// cmdz80.c

int ti82_send_asm_exec(CalcHandle*, VarEntry * var);


// dusb_cmd.c

int dusb_check_cmd_data(CalcModel model, const uint8_t * data, uint32_t len, uint32_t vtl_size, uint16_t vtl_type);
int dusb_dissect_cmd_data(CalcModel model, FILE *f, const uint8_t * data, uint32_t len, uint32_t vtl_size, uint16_t vtl_type);


// dusb_cmd.c

#define CA(x)   (const DUSBCalcAttr **)(x)
#define CP(x)   (const DUSBCalcParam **)(x)

#define VALIDATE_ATTRS(nattrs, attrs) \
	if (nattrs != 0 && attrs == NULL) \
	{ \
		ticalcs_critical("%s: " #attrs " is NULL", __FUNCTION__); \
		return ERR_INVALID_PARAMETER; \
	}

#endif // __TICALCS_INTERNAL__
