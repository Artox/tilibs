/* Hey EMACS -*- linux-c -*- */
/* $Id: packets.h 1179 2005-06-06 14:42:32Z roms $ */

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
 *  along with this program; if not, write to the Free Software Foundation,
 *  Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

// /!\ NOTE: for this file, backwards compatibility will not necessarily be maintained as strongly as it is for ticalcs.h !

#ifndef __TICALCS_DBUS__
#define __TICALCS_DBUS__


/*************/
/* Constants */
/*************/

// Machine IDs (host <-> target)

#define PC_TIXX   0x00

#define PC_TI73   0x07
#define TI73_PC   0x74
#define CBL_TI73  0x12
#define TI73_CBL  0x95

#define PC_TI80   0x00
#define TI80_PC   0x80

#define PC_TI82   0x02
#define TI82_PC   0x82
#define CBL_TI82  0x12
#define TI82_CBL  0x82

#define PC_TI83   0x03
#define TI83_PC   0x83
#define CBL_TI83  0x12
#define TI83_CBL  0x95

#define PC_TI83p  0x23
#define TI83p_PC  0x73
#define CBL_TI83p 0x12
#define TI83p_CBL 0x95

#define PC_TI84p  0x23
#define TI84p_PC  0x73
#define CBL_TI84p 0x12
#define TI84p_CBL 0x95

#define PC_TI85   0x05
#define TI85_PC   0x85
#define CBL_TI85  0x15
#define TI85_CBL  0x85

#define PC_TI86   0x06
#define TI86_PC   0x86
#define CBL_TI86  0x15
#define TI86_CBL  0x85

#define PC_TI89   0x08
#define TI89_PC   0x98
#define CBL_TI89  0x19
#define TI89_CBL  0x89

#define PC_TI89t  0x08
#define TI89t_PC  0x98
#define CBL_TI89t 0x19
#define TI89t_CBL 0x89

#define PC_TI92   0x09
#define TI92_PC   0x89
#define CBL_TI92  0x19
#define TI92_CBL  0x89

#define PC_TI92p  0x08
#define TI92p_PC  0x88
#define CBL_TI92p 0x19
#define TI92p_CBL 0x89

#define PC_V200   0x08
#define V200_PC   0x88
#define CBL_V200  0x19
#define V200_CBL  0x89

// Command IDs
/*
 * means there is data attached.
 # means the device id is ignored with this command.
 $ means it's a silent command
*/
// Official names found in TI's DBUS implementation in the activitygrapher TINCS Navigator plugin.
#define CMD_VAR  0x06		// [*  ] Variable Header - includes a std variable header (official name: RTS)
#define CMD_CTS  0x09		// [   ] Continue - used to signal OK to send a variable (official name: RDY)
#define CMD_XDP  0x15		// [*  ] Data packet - Pure data (official name: XDP)
#define CMD_ELD  0x29		// [  $] Enable Lockdown (official name: EBL)
#define CMD_DLD  0x2A		// [  $] Disable Lockdown (official name: DBL)
#define CMD_EKE  0x2B		// [  $] Enable Key Echo - start sending keypresses as scan codes in remote control packets (official name: EKE)
#define CMD_DKE  0x2C		// [  $] Disable Key Echo (official name: DKE)
#define CMD_VER  0x2D		// [  $] Request version (official name: RDI)
#define CMD_SKP  0x36		// [*  ] Skip/Exit - used when duplicate name is found (official name: EOR)
#define CMD_SID  0x47		// [*#$] Send calc ID (official name: SIOA)
#define CMD_ACK  0x56		// [   ] Acknowledge (official name: ACK)
#define CMD_ERR  0x5A		// [   ] Checksum error: send last packet again (official name: CHK)
#define CMD_RDY  0x68		// [ #$] Test if calc is ready (official name: RDC)
#define CMD_SCR  0x6D		// [  $] Request screenshot (official name: REQ)
#define CMD_GID  0x6E		// [  $] Get Calculator ID - on 83+ reads 9 bytes right from cert
#define CMD_DMP  0x6F		// [* $] Request dump of memory page
#define CMD_TG2  0x70		// [* $] Testguard2 packet - its long and complicated.
#define CMD_RID  0x74		// [ #$] Request calc ID (official name: RIOA)
#define CMD_CNT  0x78		// [  $] Continue (official name: RTC)
#define CMD_KEY  0x87		// [  $] Send key (official name: SKY)
#define CMD_DEL  0x88		// [* $] Delete variable/app (official name: DVL)
#define CMD_RUN  0x89		// [* $] Run variable/app (official name: EPL)
#define CMD_EOT  0x92		// [   ] End Of Transmission: no more variables to send (official name: DONE)
#define CMD_REQ  0xA2		// [* $] Request variable - includes a std var header (official name: GET)
#define CMD_ERR2 0xB4		// [   ] Checksum error: send last packet again
#define CMD_IND  0xB7		// [* $] Request variable index (official name: GTE)
#define CMD_RTS  0xC9		// [* $] Request to send - includes a padded var header (official name: SEND)
#define CMD_RSE  0xCA		// [* $] Request to send and execute - same as above but runs it after send (official name: SETP)

// Rejection codes (CMD82_SKIP)

#define REJ_EXIT     1
#define REJ_SKIP     2
#define REJ_MEMORY   3
#define REJ_VERSION  4

#endif
