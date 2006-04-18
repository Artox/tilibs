/* Hey EMACS -*- linux-c -*- */
/* $Id$ */

/*  libCables - Ti Link Cable library, a part of the TiLP project
 *  Copyright (C) 1999-2005  Romain Lievin
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

/* This unit allows to trace bytes which are transferred between PC
   and TI calculator.
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "stdints.h"
#include "gettext.h"
#include "logging.h"
#include "data_log.h"

#define LOG_FILE  "ticables.log"
#define TIME_FILE "ticables.tim"

static char *fn1 = NULL;
static char *fn2 = NULL;

static FILE *log1 = NULL;
static FILE *log2 = NULL;

GTimeVal tv_start, tv;

int log_start(void)
{
  // build filenames
#ifdef __WIN32__
	fn1 = g_strconcat("C:\\", LOG_FILE, NULL);
	fn2 = g_strconcat("C:\\", TIME_FILE, NULL);
#else
	fn1 = g_strconcat(g_get_home_dir(), "/", LOG_FILE, NULL);
	fn2 = g_strconcat(g_get_home_dir(), "/", TIME_FILE, NULL);
#endif

  	//ticables_info(_("Logging started."));

  	log1 = fopen(fn1, "wt");
  	if (log1 == NULL) 
	{
    		ticables_error(_("Unable to open <%s> for logging.\n"), fn1);
    		return -1;
  	}

  	log2 = fopen(fn2, "wt");
  	if (log2 == NULL) 
	{
    		ticables_error(_("Unable to open <%s> for logging.\n"), fn2);
    		return -1;
  	}

	g_get_current_time(&tv_start);

  	return 0;
}

int log_1(int dir, uint8_t d)
{
  	static int array[16];
  	static int i = 0;
  	int j;
  	int c;
  	static int k = 0;
	long sec, us;

  	if (log1 == NULL)
    		return -1;
  	array[i++] = d;

  	fprintf(log1, "%02X ", d);
  	if (!(i % 16) && (i > 1)) 
	{
    	fprintf(log1, "| ");
    	for (j = 0; j < 16; j++) 
		{
      		c = array[j];
      		if ((c < 32) || (c > 127))
			fprintf(log1, " ");
      		else
			fprintf(log1, "%c", c);
    	}
    	fprintf(log1, "\n");
    	i = 0;
  	}

	g_get_current_time(&tv);
  	k++;
	sec = tv.tv_sec - tv_start.tv_sec;
	us = tv.tv_usec - tv_start.tv_usec;
	if(us < 0)
	{
		us += 1000000;
		sec++;
	}
  	fprintf(log2, "%i: %li.%2li\n", k, sec, us);

  	return 0;
}

int log_N(int dir, uint8_t* d, int n)
{
	int i;

	for(i = 0; i < n; i++)
		log_1(dir, d[i]);
  	
  	return 0;
}

int log_stop()
{
  	//ticables_info("Logging stopped.");

  	if (log1 != NULL)
    		fclose(log1);
  	if (log2 != NULL)
    		fclose(log2);

  	g_free(fn1);
  	g_free(fn2);

  	return 0;
}
