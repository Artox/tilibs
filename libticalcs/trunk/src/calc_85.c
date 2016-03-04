/* Hey EMACS -*- linux-c -*- */
/* $Id: link_nul.c 1059 2005-05-14 09:45:42Z roms $ */

/*  libticalcs2 - hand-helds support library, a part of the TiLP project
 *  Copyright (C) 1999-2005  Romain Lievin
 *  Copyright (C) 2006  Kevin Kofler
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

/*
	TI82/85 support.
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <ticonv.h>
#include "ticalcs.h"
#include "gettext.h"
#include "internal.h"
#include "logging.h"
#include "error.h"
#include "pause.h"
#include "macros.h"

#include "dbus_pkt.h"
#include "cmdz80.h"
#include "rom82.h"
#include "rom85.h"
#include "romdump.h"

#define SEND_SCR ((handle->model < CALC_TI85) ? ti82_send_SCR : ti85_send_SCR)
#define SEND_ACK ((handle->model < CALC_TI85) ? ti82_send_ACK : ti85_send_ACK)
#define SEND_VAR ((handle->model < CALC_TI85) ? ti82_send_VAR : ti85_send_VAR)
#define SEND_XDP ((handle->model < CALC_TI85) ? ti82_send_XDP : ti85_send_XDP)
#define SEND_CTS ((handle->model < CALC_TI85) ? ti82_send_CTS : ti85_send_CTS)
#define SEND_EOT ((handle->model < CALC_TI85) ? ti82_send_EOT : ti85_send_EOT)

#define RECV_ACK ((handle->model < CALC_TI85) ? ti82_recv_ACK : ti85_recv_ACK)
#define RECV_VAR ((handle->model < CALC_TI85) ? ti82_recv_VAR : ti85_recv_VAR)
#define RECV_XDP ((handle->model < CALC_TI85) ? ti82_recv_XDP : ti85_recv_XDP)
#define RECV_SKP ((handle->model < CALC_TI85) ? ti82_recv_SKP : ti85_recv_SKP)
#define RECV_ERR ti82_recv_ERR

// Screen coordinates of the TI-82
#define TI82_ROWS  64
#define TI82_COLS  96

// Screen coordinates of the TI-85
#define TI85_ROWS  64
#define TI85_COLS  128

static int		recv_screen	(CalcHandle* handle, CalcScreenCoord* sc, uint8_t** bitmap)
{
	int ret;

	*bitmap = (uint8_t *)ticalcs_alloc_screen(65537U);
	if (*bitmap == NULL)
	{
		return ERR_MALLOC;
	}

	if (handle->model < CALC_TI85)
	{
		sc->width = TI82_COLS;
		sc->height = TI82_ROWS;
		sc->clipped_width = TI82_COLS;
		sc->clipped_height = TI82_ROWS;
	}
	else
	{
		sc->width = TI85_COLS;
		sc->height = TI85_ROWS;
		sc->clipped_width = TI85_COLS;
		sc->clipped_height = TI85_ROWS;
	}
	sc->pixel_format = CALC_PIXFMT_MONO;

	ret = SEND_SCR(handle);
	if (!ret)
	{
		ret = RECV_ACK(handle, NULL);
		if (!ret)
		{
			uint16_t max_cnt;
			ret = RECV_XDP(handle, &max_cnt, *bitmap);
			if (!ret || ret == ERR_CHECKSUM) // problem with checksum
			{
				*bitmap = ticalcs_realloc_screen(*bitmap, (handle->model < CALC_TI85) ? TI82_COLS * TI82_ROWS / 8 : TI85_COLS * TI85_ROWS / 8);
				ret = SEND_ACK(handle);
			}
		}
	}

	if (ret)
	{
		ticalcs_free_screen(*bitmap);
		*bitmap = NULL;
	}

	return ret;
}

static int		get_memfree	(CalcHandle* handle, uint32_t* ram, uint32_t* flash)
{
	(void)handle;
	*ram = *flash = -1;
	return 0;
}

static int		send_backup	(CalcHandle* handle, BackupContent* content)
{
	int ret = 0;
	uint16_t length;
	char varname[9];
	uint8_t rej_code;
	uint16_t status;

	strncpy(update_->text, _("Waiting for user's action..."), sizeof(update_->text) - 1);
	update_->text[sizeof(update_->text) - 1] = 0;
	update_label();

	length = content->data_length1;
	varname[0] = LSB(content->data_length2);
	varname[1] = MSB(content->data_length2);
	varname[2] = LSB(content->data_length3);
	varname[3] = MSB(content->data_length3);
	varname[4] = LSB(content->mem_address);
	varname[5] = MSB(content->mem_address);

	do
	{
		ret = SEND_VAR(handle, content->data_length1, (handle->model < CALC_TI85) ? TI82_BKUP : TI85_BKUP, varname);
		if (!ret)
		{
			ret = RECV_ACK(handle, &status);
		}
		if (ret)
		{
			break;
		}

		do
		{
			// wait for user's action
			update_refresh();

			if (update_->cancel)
			{
				ret = ERR_ABORT;
				break;
			}

			ret = RECV_SKP(handle, &rej_code);
		}
		while (ret == ERROR_READ_TIMEOUT);

		if (!ret)
		{
			ret = SEND_ACK(handle);
		}
		if (ret)
		{
			break;
		}

		switch (rej_code)
		{
		case REJ_EXIT:
		case REJ_SKIP:
			ret = ERR_ABORT;
			break;
		case REJ_MEMORY:
			ret = ERR_OUT_OF_MEMORY;
			// Fall through.
		case 0:						// CTS
			break;
		default:
			ret = ERR_VAR_REJECTED;
			break;
		}

		if (ret)
		{
			break;
		}

		update_->text[0] = 0;
		update_label();

		update_->cnt2 = 0;
		update_->max2 = 3;
		update_->pbar();

		ret = SEND_XDP(handle, content->data_length1, content->data_part1);
		if (!ret)
		{
			ret = RECV_ACK(handle, &status);
		}
		if (ret)
		{
			break;
		}
		update_->cnt2++;
		update_->pbar();

		ret = SEND_XDP(handle, content->data_length2, content->data_part2);
		if (!ret)
		{
			ret = RECV_ACK(handle, &status);
		}
		if (ret)
		{
			break;
		}
		update_->cnt2++;
		update_->pbar();

		ret = SEND_XDP(handle, content->data_length3, content->data_part3);
		if (!ret)
		{
			ret = RECV_ACK(handle, &status);
		}
		if (ret)
		{
			break;
		}
		update_->cnt2++;
		update_->pbar();

		if (handle->model < CALC_TI85)
		{
			//ret = SEND_EOT(handle);
		}
		else
		{
			ret = SEND_EOT(handle);
		}

		break;
	} while(0);

	return ret;
}

static int		recv_backup	(CalcHandle* handle, BackupContent* content)
{
	int ret = 0;
	char varname[9];

	strncpy(update_->text, _("Waiting for backup..."), sizeof(update_->text) - 1);
	update_->text[sizeof(update_->text) - 1] = 0;
	update_label();

	content->model = handle->model;
	strncpy(content->comment, tifiles_comment_set_backup(), sizeof(content->comment) - 1);
	content->comment[sizeof(content->comment) - 1] = 0;

	varname[0] = 0;
	do
	{
		ret = RECV_VAR(handle, &(content->data_length1), &content->type, varname);
		if (ret)
		{
			break;
		}
		content->data_length2 = (uint8_t)varname[0] | (((uint16_t)(uint8_t)varname[1]) << 8);
		content->data_length3 = (uint8_t)varname[2] | (((uint16_t)(uint8_t)varname[3]) << 8);
		content->mem_address  = (uint8_t)varname[4] | (((uint16_t)(uint8_t)varname[5]) << 8);
		ret = SEND_ACK(handle);
		if (ret)
		{
			break;
		}

		ret = SEND_CTS(handle);
		if (!ret)
		{
			ret = RECV_ACK(handle, NULL);
		}
		if (ret)
		{
			break;
		}

		update_->text[0] = 0;
		update_label();

		update_->cnt2 = 0;
		update_->max2 = 3;
		update_->pbar();

		content->data_part1 = tifiles_ve_alloc_data(65536);
		ret = RECV_XDP(handle, &content->data_length1, content->data_part1);
		if (!ret)
		{
			ret = SEND_ACK(handle);
		}
		if (ret)
		{
			break;
		}
		update_->cnt2++;
		update_->pbar();

		content->data_part2 = tifiles_ve_alloc_data(65536);
		ret = RECV_XDP(handle, &content->data_length2, content->data_part2);
		if (!ret)
		{
			ret = SEND_ACK(handle);
		}
		if (ret)
		{
			break;
		}
		update_->cnt2++;
		update_->pbar();

		content->data_part3 = tifiles_ve_alloc_data(65536);
		ret = RECV_XDP(handle, &content->data_length3, content->data_part3);
		if (!ret)
		{
			ret = SEND_ACK(handle);
		}
		if (ret)
		{
			break;
		}
		update_->cnt2++;
		update_->pbar();

		content->data_part4 = NULL;
	} while(0);

	return ret;
}

static int		send_var_ns	(CalcHandle* handle, CalcMode mode, FileContent* content);
static int		send_var	(CalcHandle* handle, CalcMode mode, FileContent* content)
{
	return send_var_ns(handle, mode, content);
}

static int		send_var_ns	(CalcHandle* handle, CalcMode mode, FileContent* content)
{
	unsigned int i;
	int ret;
	uint8_t rej_code;
	uint16_t status;

	if ((mode & MODE_SEND_EXEC_ASM) && content->num_entries != 1)
	{
		ticalcs_critical("no variable to execute");
		return -1;
	}

	update_->cnt2 = 0;
	update_->max2 = content->num_entries;

	for (i = 0; i < content->num_entries; i++) 
	{
		VarEntry *entry = content->entries[i];

		ret = SEND_VAR(handle, (uint16_t)entry->size, entry->type, entry->name);
		if (!ret)
		{
			ret = RECV_ACK(handle, &status);
		}
		if (ret)
		{
			break;
		}

		strncpy(update_->text, _("Waiting for user's action..."), sizeof(update_->text) - 1);
		update_->text[sizeof(update_->text) - 1] = 0;
		update_label();

		do
		{
			// wait for user's action
			update_refresh();
			if (update_->cancel)
			{
				ret = ERR_ABORT;
				break;
			}

			ret = RECV_SKP(handle, &rej_code);
		}
		while (ret == ERROR_READ_TIMEOUT);

		if (!ret)
		{
			ret = SEND_ACK(handle);
		}
		if (ret)
		{
			break;
		}

		switch (rej_code)
		{
		case REJ_EXIT:
			ret = ERR_ABORT;
			break;
		case REJ_SKIP:
			if (mode & MODE_SEND_EXEC_ASM)
			{
				ret = ERR_ABORT;
				break;
			}
			continue;
		case REJ_MEMORY:
			ret = ERR_OUT_OF_MEMORY;
			// Fall through.
		case 0:						// CTS
			break;
		default:
			ret = ERR_VAR_REJECTED;
			break;
		}

		if (ret)
		{
			break;
		}

		ticonv_varname_to_utf8_sn(handle->model, entry->name, update_->text, sizeof(update_->text), entry->type);
		update_label();

		ret = SEND_XDP(handle, (uint16_t)entry->size, entry->data);
		if (!ret)
		{
			ret = RECV_ACK(handle, &status);
		}
		if (ret)
		{
			break;
		}

		update_->cnt2 = i+1;
		update_->max2 = content->num_entries;
		update_->pbar();
	}

	if (mode & MODE_SEND_EXEC_ASM)
	{
		ret = ti82_send_asm_exec(handle, content->entries[0]);
		if (!ret)
		{
			ret = RECV_ERR(handle, &status);
			if (!ret)
			{
				ret = SEND_ACK(handle);
			}
		}
	}
	else if ((mode & MODE_SEND_ONE_VAR) || (mode & MODE_SEND_LAST_VAR))
	{
		ret = SEND_EOT(handle);
		if (!ret)
		{
			ret = RECV_ACK(handle, NULL);
		}
	}

	return ret;
}

static int		recv_var_ns	(CalcHandle* handle, CalcMode mode, FileContent* content, VarEntry** vr)
{
	int nvar = 0;
	int ret = 0;
	uint16_t ve_size;

	strncpy(update_->text, _("Waiting for var(s)..."), sizeof(update_->text) - 1);
	update_->text[sizeof(update_->text) - 1] = 0;
	update_label();

	content->model = handle->model;

	for (nvar = 0;; nvar++)
	{
		VarEntry *ve;

		content->entries = tifiles_ve_resize_array(content->entries, nvar+1);
		ve = content->entries[nvar] = tifiles_ve_create();

		do
		{
			update_refresh();
			if (update_->cancel)
			{
				ret = ERR_ABORT;
				break;
			}

			ret = RECV_VAR(handle, &ve_size, &(ve->type), ve->name);
			ve->size = ve_size;
		}
		while (ret == ERROR_READ_TIMEOUT);

		if (ret)
		{
			break;
		}

		ret = SEND_ACK(handle);
		if (ret == ERR_EOT)
		{
			ret = 0;
			break;
		}
		if (ret)
		{
			break;
		}

		ret = SEND_CTS(handle);
		if (!ret)
		{
			ret = RECV_ACK(handle, NULL);
		}
		if (ret)
		{
			break;
		}

		ticonv_varname_to_utf8_sn(handle->model, ve->name, update_->text, sizeof(update_->text), ve->type);
		update_label();

		ve->data = tifiles_ve_alloc_data(ve->size);
		ret = RECV_XDP(handle, &ve_size, ve->data);
		if (ret)
		{
			break;
		}
		ve->size = ve_size;
		ret = SEND_ACK(handle);
		if (ret)
		{
			break;
		}
	}

	content->num_entries = nvar;
	if (nvar == 1)
	{
		strncpy(content->comment, tifiles_comment_set_single(), sizeof(content->comment) - 1);
		content->comment[sizeof(content->comment) - 1] = 0;
		*vr = tifiles_ve_dup(content->entries[0]);
	}
	else
	{
		strncpy(content->comment, tifiles_comment_set_group(), sizeof(content->comment) - 1);
		content->comment[sizeof(content->comment) - 1] = 0;
		*vr = NULL;
	}

	return ret;
}

static int		dump_rom_1	(CalcHandle* handle)
{
	// Send dumping program
	if (handle->model == CALC_TI82)
	{
		return rd_send(handle, "romdump.82p", romDumpSize82, romDump82);
	}
	else // if (handle->model == CALC_TI85)
	{
		return rd_send(handle, "romdump.85s", romDumpSize85, romDump85);
	}
}

static int		dump_rom_2	(CalcHandle* handle, CalcDumpSize size, const char *filename)
{
	// Get dump
	return rd_dump(handle, filename);
}

const CalcFncts calc_82 =
{
	CALC_TI82,
	"TI82",
	"TI-82",
	"TI-82",
	OPS_SCREEN | OPS_BACKUP | OPS_VARS | OPS_ROMDUMP |
	FTS_BACKUP | FTS_NONSILENT,
	PRODUCT_ID_NONE,
	{"",     /* is_ready */
	 "",     /* send_key */
	 "",     /* execute */
	 "1P",   /* recv_screen */
	 "",     /* get_dirlist */
	 "",     /* get_memfree */
	 "2P1L", /* send_backup */
	 "2P1L", /* recv_backup */
	 "",     /* send_var */
	 "",     /* recv_var */
	 "2P1L", /* send_var_ns */
	 "1P1L", /* recv_var_ns */
	 "",     /* send_app */
	 "",     /* recv_app */
	 "",     /* send_os */
	 "",     /* recv_idlist */
	 "2P",   /* dump_rom_1 */
	 "2P",   /* dump_rom_2 */
	 "",     /* set_clock */
	 "",     /* get_clock */
	 "",     /* del_var */
	 "",     /* new_folder */
	 "",     /* get_version */
	 "",     /* send_cert */
	 "",     /* recv_cert */
	 "",     /* rename */
	 "",     /* chattr */
	 "",     /* send_all_vars_backup */
	 ""      /* recv_all_vars_backup */ },
	&noop_is_ready,
	&noop_send_key,
	&noop_execute,
	&recv_screen,
	&noop_get_dirlist,
	&get_memfree,
	&send_backup,
	&recv_backup,
	&send_var,
	&noop_recv_var,
	&send_var_ns,
	&recv_var_ns,
	&noop_send_flash,
	&noop_recv_flash,
	&noop_send_os,
	&noop_recv_idlist,
	&dump_rom_1,
	&dump_rom_2,
	&noop_set_clock,
	&noop_get_clock,
	&noop_del_var,
	&noop_new_folder,
	&noop_get_version,
	&noop_send_cert,
	&noop_recv_cert,
	&noop_rename_var,
	&noop_change_attr,
	&noop_send_all_vars_backup,
	&noop_recv_all_vars_backup
};

const CalcFncts calc_85 =
{
	CALC_TI85,
	"TI85",
	"TI-85",
	"TI-85",
	OPS_SCREEN | OPS_BACKUP | OPS_VARS | OPS_ROMDUMP |
	FTS_BACKUP | FTS_NONSILENT,
	PRODUCT_ID_NONE,
	{"",     /* is_ready */
	 "",     /* send_key */
	 "",     /* execute */
	 "1P",   /* recv_screen */
	 "",     /* get_dirlist */
	 "",     /* get_memfree */
	 "2P1L", /* send_backup */
	 "2P1L", /* recv_backup */
	 "",     /* send_var */
	 "",     /* recv_var */
	 "2P1L", /* send_var_ns */
	 "1P1L", /* recv_var_ns */
	 "",     /* send_app */
	 "",     /* recv_app */
	 "",     /* send_os */
	 "",     /* recv_idlist */
	 "2P",   /* dump_rom_1 */
	 "2P",   /* dump_rom_2 */
	 "",     /* set_clock */
	 "",     /* get_clock */
	 "",     /* del_var */
	 "",     /* new_folder */
	 "",     /* get_version */
	 "",     /* send_cert */
	 "",     /* recv_cert */
	 "",     /* rename */
	 "",     /* chattr */
	 "",     /* send_all_vars_backup */
	 ""      /* recv_all_vars_backup */ },
	&noop_is_ready,
	&noop_send_key,
	&noop_execute,
	&recv_screen,
	&noop_get_dirlist,
	&get_memfree,
	&send_backup,
	&recv_backup,
	&send_var,
	&noop_recv_var,
	&send_var_ns,
	&recv_var_ns,
	&noop_send_flash,
	&noop_recv_flash,
	&noop_send_os,
	&noop_recv_idlist,
	&dump_rom_1,
	&dump_rom_2,
	&noop_set_clock,
	&noop_get_clock,
	&noop_del_var,
	&noop_new_folder,
	&noop_get_version,
	&noop_send_cert,
	&noop_recv_cert,
	&noop_rename_var,
	&noop_change_attr,
	&noop_send_all_vars_backup,
	&noop_recv_all_vars_backup
};
