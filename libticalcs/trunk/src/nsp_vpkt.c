/* Hey EMACS -*- linux-c -*- */
/* $Id: cmd84p.c 2077 2006-03-31 21:16:19Z roms $ */

/*  libticalcs - Ti Calculator library, a part of the TiLP project
 *  Copyright (C) 1999-2005  Romain Li�vin
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
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

/*
	This unit manages virtual packets from/to NSPire (DirectLink).
	Virtual packets are fragmented into one or more raw packets.
*/

#include <stdlib.h>
#include <string.h>

#include "ticalcs.h"
#include "logging.h"
#include "error.h"
#include "macros.h"

#include "nsp_rpkt.h"
#include "nsp_vpkt.h"

#define VPKT_DBG	1	// 1 = verbose, 2 = more verbose

// Type to string

static const ServiceName sid_types[] = 
{
	{ 0x00FE, "Reception Acknowledgment" },
	{ 0x00FF, "Reception Ack" },
	{ 0x4001, "Null" },
	{ 0x4002, "Echo" },
	{ 0x4003, "Device Address Request/Assignment" },
	{ 0x4020, "Device Information" },
	{ 0x4021, "Screen Capture" },
	{ 0x4024, "Screen Capture w/ RLE" },
	{ 0x4050, "Login" },
	{ 0x4060, "File Management" },
	{ 0x4080, "OS Installation" },
	{ 0x40DE, "Service Disconnect" },
	{ -1, NULL},
};

const char* nsp_sid2name(uint16_t id)
{
	const ServiceName *p;

	for(p = sid_types; p->name != NULL; p++)
		if(p->id == id)
			return p->name;

	return "unknown: not listed";
}

// Buffer allocation

static GList *vtl_pkt_list = NULL;

VirtualPacket*  nsp_vtl_pkt_new_ex(uint32_t size, uint16_t src_addr, uint16_t src_port, uint16_t dst_addr, uint16_t dst_port)
{
	VirtualPacket* vtl = g_malloc0(sizeof(VirtualPacket));

	vtl->src_addr = src_addr;
	vtl->src_port = src_port;
	vtl->dst_addr = dst_addr;
	vtl->dst_port = dst_port;
	vtl->size = size;
	vtl->data = g_malloc0(size);

	vtl_pkt_list = g_list_append(vtl_pkt_list, vtl);

	return vtl;
}

VirtualPacket*  nsp_vtl_pkt_new(void)
{
	return nsp_vtl_pkt_new_ex(0, 0, 0, 0, 0);
}

void			nsp_vtl_pkt_del(VirtualPacket* vtl)
{
	vtl_pkt_list = g_list_remove(vtl_pkt_list, vtl);

	g_free(vtl->data);
	g_free(vtl);
}

void			nsp_vtl_pkt_purge(void)
{
	g_list_foreach(vtl_pkt_list, (GFunc)nsp_vtl_pkt_del, NULL);
	g_list_free(vtl_pkt_list);
	vtl_pkt_list = NULL;
}

// Fragmenting of packets

int nsp_send_data(CalcHandle *h, VirtualPacket *vtl)
{
	RawPacket raw = { 0 };
	int i, r, q;
	long offset = 0;

	raw.src_addr = vtl->src_addr;
	raw.src_port = vtl->src_port;
	raw.dst_addr = vtl->dst_addr;
	raw.dst_port = vtl->dst_port;

	q = (vtl->size - offset) / DATA_SIZE;
	r = (vtl->size - offset) % DATA_SIZE;

#if (VPKT_DBG == 2)
#elif (VPKT_DBG == 1)
		ticalcs_info("  %04x:%04x->%04x:%04x AK=%02x SQ=%02x (%i bytes)", 
			vtl->src_addr, vtl->src_port, vtl->dst_addr, vtl->dst_port, 0, 0, vtl->size);
#endif

	for(i = 1; i <= q; i++)
	{
		raw.data_size = DATA_SIZE;
		memcpy(raw.data, vtl->data + offset, DATA_SIZE);
		offset += DATA_SIZE;

		TRYF(nsp_send(h, &raw));
		//TRYF(nsp_recv_acknowledge(h));

		h->updat->max1 = vtl->size;
		h->updat->cnt1 += DATA_SIZE;
		h->updat->pbar();
	}

	if(r)
	{
		raw.data_size = r;
		memcpy(raw.data, vtl->data + offset, r);
		offset += r;
		
		TRYF(nsp_send(h, &raw));
		//TRYF(nsp_recv_acknowledge(h));
	}

	return 0;
}

// beware: data field may be re-allocated in size !
int nsp_recv_data(CalcHandle* h, VirtualPacket* vtl)
{
	RawPacket raw = { 0 };
	int i = 0;
	long offset = 0;

	vtl->size = 0;
	vtl->data = malloc(DATA_SIZE);

	do
	{
		TRYF(nsp_recv(h, &raw));
		vtl->size += raw.data_size;

		vtl->data = realloc(vtl->data, vtl->size);
		memcpy(vtl->data + offset, &raw.data, raw.data_size);
		offset += raw.data_size;

		h->updat->max1 = vtl->size;
		h->updat->cnt1 += DATA_SIZE;
		h->updat->pbar();
	} while(raw.data_size >= DATA_SIZE);

	vtl->src_addr = raw.src_addr;
	vtl->src_port = raw.src_port;
	vtl->dst_addr = raw.dst_addr;
	vtl->dst_port = raw.dst_port;

	vtl->ack = raw.ack;
	vtl->seq = raw.seq;

#if (VPKT_DBG == 2)
#elif (VPKT_DBG == 1)
		ticalcs_info("  %04x:%04x->%04x:%04x AK=%02x SQ=%02x (%i bytes)", 
			vtl->src_addr, vtl->src_port, vtl->dst_addr, vtl->dst_port, vtl->ack, vtl->seq, vtl->size);
#endif

	return 0;
}
