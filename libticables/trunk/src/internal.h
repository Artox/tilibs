/* Hey EMACS -*- linux-c -*- */
/* $Id$ */

/*  libticables2 - link cable library, a part of the TiLP project
 *  Copyright (C) 2015 Benjamin Moody
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

#ifndef __TICABLES_INTERNAL__
#define __TICABLES_INTERNAL__

#define VALIDATE_NONNULL(ptr) \
	do \
	{ \
		if (ptr == NULL) \
		{ \
			ticables_critical("%s: " #ptr " is NULL", __FUNCTION__); \
			return ERR_ILLEGAL_ARG; \
		} \
	} while(0);
#define VALIDATE_HANDLE(handle) \
	do \
	{ \
		if (!ticables_validate_handle(handle)) \
		{ \
			ticables_critical("%s: " #handle " is invalid", __FUNCTION__); \
			return ERR_ILLEGAL_ARG; \
		} \
	} while(0);
#define VALIDATE_CABLEFNCTS(cable) \
	do \
	{ \
		if (!ticables_validate_cablefncts(cable)) \
		{ \
			ticables_critical("%s: " # cable " is invalid", __FUNCTION__); \
			return ERR_ILLEGAL_ARG; \
		} \
	} while(0);

#define RETURN_IF_HANDLE_OPEN(handle) \
	do \
	{ \
		if (handle->open) \
		{ \
			return ERR_BUSY; \
		} \
	} while(0);
#define RETURN_IF_HANDLE_NOT_OPEN(handle) \
	do \
	{ \
		if (!handle->open) \
		{ \
			return ERR_NOT_OPEN; \
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

static inline int ticables_validate_handle(CableHandle * handle)
{
	return handle != NULL;
}

static inline int ticables_validate_cablefncts(const CableFncts * cable)
{
	return cable != NULL;
}

#define FILL_CABLE_EVENT_DATA(event, type_, retval_, open_, data_, len_) \
	do \
	{ \
		event.version = 1; \
		event.type = type_; \
		event.retval = retval_; \
		event.open = open_; \
		event.data = data_; \
		event.len = len_; \
	} while(0);

typedef struct {
	uint16_t    vid;
	uint16_t    pid;
	uint16_t    version;
	char        product_str[65];
	void *dev;
} USBCableInfo;

int usb_probe_device_info(const USBCableInfo **list, int *count);
void translate_usb_device_info(CableDeviceInfo *info, const USBCableInfo *usbinfo);

int dbus_decomp(const char *filename, int resync);
int dusb_decomp(const char *filename);
int nsp_decomp(const char *filename);

// link_nul.c
int noop_prepare(CableHandle *h);
int noop_probe(CableHandle *h);
int noop_open(CableHandle *h);
int noop_close(CableHandle *h);
int noop_reset(CableHandle *h);
int noop_put(CableHandle *h, uint8_t *data, uint32_t len);
int noop_get(CableHandle *h, uint8_t *data, uint32_t len);
int noop_check(CableHandle *h, int *status);
int noop_set_red_wire(CableHandle *h, int b);
int noop_set_white_wire(CableHandle *h, int b);
int noop_get_red_wire(CableHandle *h);
int noop_get_white_wire(CableHandle *h);
int noop_set_device(CableHandle *h, const char * device);

#endif
