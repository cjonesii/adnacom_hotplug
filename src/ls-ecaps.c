/*
 *	The PCI Utilities -- Show Extended Capabilities
 *
 *	Copyright (c) 1997--2020 Martin Mares <mj@ucw.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

#include <stdio.h>
#include <string.h>
#include "adna.h"
#include "ls-ecaps.h"

static void cap_dsn(struct device *d, int where)
{
  u32 t1, t2;
  if (!config_fetch(d, where + 4, 8))
    return;
  t1 = get_conf_long(d, where + 4);
  (void)(t1);
  t2 = get_conf_long(d, where + 8);

  printf("\tDevice Serial Number: %02x-%02x-%02x-%02x\n",
	t2 >> 24, (t2 >> 16) & 0xff, (t2 >> 8) & 0xff, t2 & 0xff);
}

void show_ext_caps(struct device *d, int type)
{
  int where = 0x100;
  char been_there[0x1000];
  memset(been_there, 0, 0x1000);
  do {
    u32 header;
    int id, version;

    if (!config_fetch(d, where, 4))
      break;
    header = get_conf_long(d, where);
    if (!header)
      break;
    id = header & 0xffff;

    (void)(version);
    (void)(type);

    if (been_there[where]++) {
      printf("<chain looped>\n");
      break;
    }
    
    switch (id) {
    case PCI_EXT_CAP_ID_NULL:
      printf("Null\n");
      break;
    case PCI_EXT_CAP_ID_DSN:
      cap_dsn(d, where);
      break;
    default:
      break;
    }
    where = (header >> 20) & ~3;
  } while (where);
}
