/*
 * Copyright (C) 2007 Hans de Goede <j.w.r.degoede@gmail.com>
 *
 * This file is part of libmms a free mms protocol library
 *
 * libmms is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * libmss is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA
 */

/*
 * mmsx is a small wrapper around the mms and mmsh protocol implementations
 * in libmms. The mmsx functions provide transparent access to both protocols
 * so that programs who wish to support both can do so with a single code path
 * if desired.
 */

#include <stdlib.h>
#include "mmsx.h"
#include "mms.h"
#include "mmsh.h"

/********** logging **********/
#define lprintf(...) \
    if (getenv("LIBMMS_DEBUG")) \
        fprintf(stderr, "mmsx: " __VA_ARGS__)

struct mmsx_s {
  mms_t *connection;
  mmsh_t *connection_h;
};

#ifdef INCLUDE_LMF
int mmsx_set_play_rate(mmsx_t *mmsx, double rate)
{
  mmsh_t *mmsh;

  mmsh = mmsx->connection_h;
  if (!mmsh)
    return -1;

  mmsh_set_play_rate(mmsh, rate);

  return 0;
}
#endif

mmsx_t *mmsx_connect(mms_io_t *io, void *data, const char *url, int bandwidth)
{
  mmsx_t *mmsx = calloc(1, sizeof(mmsx_t));
  char *try_mms_first = getenv("LIBMMS_TRY_MMS_FIRST");
  
  if (!mmsx)
    return mmsx;

  /* Normally we try mmsh first, as mms: is a rollover protocol identifier
     according to microsoft and recent mediaplayer versions will try
     mmsh before mms for mms:// uris. Note that in case of a mmst:// or a
     mmsh:// url the mms[h]_connect function will directly exit if it cannot
     handle it. The LIBMMS_TRY_MMS_FIRST environment variable is there for
     testing the mms code against servers which accept both mmsh and mms. */
  if (try_mms_first) {
    lprintf("%s:Try by mmst - %s\n", __FUNCTION__, url);
    mmsx->connection = mms_connect(io, data, url, bandwidth);
    if (mmsx->connection)
      return mmsx;
  }

  lprintf("%s:Try by mmsh - %s\n", __FUNCTION__, url);
  mmsx->connection_h = mmsh_connect(io, data, url, bandwidth);
  if (mmsx->connection_h)
    return mmsx;

  if (!try_mms_first) {
    lprintf("%s:Try by mmst - %s\n", __FUNCTION__, url);
    mmsx->connection = mms_connect(io, data, url, bandwidth);
    if (mmsx->connection)
      return mmsx;
  }

  lprintf("%s:Fail to connect - %s\n", __FUNCTION__, url);

  free(mmsx);
  return NULL;
}

int mmsx_read (mms_io_t *io, mmsx_t *mmsx, char *data, int len)
{
  if(mmsx->connection)
    return mms_read(io, mmsx->connection, data, len);
  else
    return mmsh_read(io, mmsx->connection_h, data, len);
}

int mmsx_time_seek (mms_io_t *io, mmsx_t *mmsx, double time_sec)
{
  if(mmsx->connection)
    return mms_time_seek(io, mmsx->connection, time_sec);
  else
    return mmsh_time_seek(io, mmsx->connection_h, time_sec);
}

mms_off_t mmsx_seek (mms_io_t *io, mmsx_t *mmsx, mms_off_t offset, int origin)
{
  if(mmsx->connection)
    return mms_seek(io, mmsx->connection, offset, origin);
  else
    return mmsh_seek(io, mmsx->connection_h, offset, origin);
}

double mmsx_get_time_length (mmsx_t *mmsx)
{
  if(mmsx->connection)
    return mms_get_time_length(mmsx->connection);
  else
    return mmsh_get_time_length(mmsx->connection_h);
}

uint64_t mmsx_get_raw_time_length (mmsx_t *mmsx)
{
  if(mmsx->connection)
    return mms_get_raw_time_length(mmsx->connection);
  else
    return mmsh_get_raw_time_length(mmsx->connection_h);
}

uint64_t mmsx_get_file_time (mmsx_t *mmsx) {
  if(mmsx->connection)
    return mms_get_file_time(mmsx->connection);
  else
    return mmsh_get_file_time(mmsx->connection_h);
}

uint32_t mmsx_get_length (mmsx_t *mmsx)
{
  if(mmsx->connection)
    return mms_get_length(mmsx->connection);
  else
    return mmsh_get_length(mmsx->connection_h);
}

void mmsx_close (mmsx_t *mmsx)
{
  if(mmsx->connection)
    mms_close(mmsx->connection);
  else
    mmsh_close(mmsx->connection_h);

  free(mmsx);
}

int mmsx_peek_header (mmsx_t *mmsx, char *data, int maxsize)
{
  if(mmsx->connection)
    return mms_peek_header(mmsx->connection, data, maxsize);
  else
    return mmsh_peek_header(mmsx->connection_h, data, maxsize);
}

mms_off_t mmsx_get_current_pos (mmsx_t *mmsx)
{
  if(mmsx->connection)
    return mms_get_current_pos(mmsx->connection);
  else
    return mmsh_get_current_pos(mmsx->connection_h);
}

uint32_t mmsx_get_asf_header_len(mmsx_t *mmsx)
{
  if(mmsx->connection)
    return mms_get_asf_header_len(mmsx->connection);
  else
    return mmsh_get_asf_header_len(mmsx->connection_h);
}

uint64_t mmsx_get_asf_packet_len (mmsx_t *mmsx)
{
  if(mmsx->connection)
    return mms_get_asf_packet_len(mmsx->connection);
  else
    return mmsh_get_asf_packet_len(mmsx->connection_h);
}

int mmsx_get_seekable (mmsx_t *mmsx)
{
  if(mmsx->connection)
    return mms_get_seekable(mmsx->connection);
  else
    return mmsh_get_seekable(mmsx->connection_h);
}

#ifdef INCLUDE_LMF

int mmsx_unlock ()
{
  mms_disable_read();
  return 1;
}

int mmsx_unlock_stop ()
{
  mms_enable_read();
  return 1;
}

#endif