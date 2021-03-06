/***************************************************************************

 BetterString.mcc - A better String gadget MUI Custom Class
 Copyright (C) 1997-2000 Allan Odgaard
 Copyright (C) 2005-2019 BetterString.mcc Open Source Team

 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 2.1 of the License, or (at your option) any later version.

 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.

 TextEditor class Support Site:  http://www.sf.net/projects/texteditor-mcc

 $Id$

***************************************************************************/

#include <stdint.h>

#if !defined(__MORPHOS__)
// uncompressed ARGB data
#if defined(__AROS__)
extern const uint8_t icon32[];
#else
extern const uint32_t icon32[];
#endif

#define ICON32_WIDTH       24
#define ICON32_HEIGHT      20
#define ICON32_DEPTH       32
#else
// bzip2 compressed ARGB data
extern const uint8_t icon32[];
#endif

#ifdef USE_ICON8_COLORS
const ULONG icon8_colors[24] =
{
	0x00000000,0x00000000,0x00000000,
	0x43434343,0x5b5b5b5b,0x97979797,
	0x7b7b7b7b,0x7b7b7b7b,0x7b7b7b7b,
	0x86868686,0x90909090,0x8b8b8b8b,
	0xafafafaf,0xafafafaf,0xafafafaf,
	0xffffffff,0xffffffff,0xffffffff,
	0x00000000,0x00000000,0x00000000,
	0x00000000,0x00000000,0x00000000,
};
#endif

#define ICON8_WIDTH        24
#define ICON8_HEIGHT       14
#define ICON8_DEPTH         3
#define ICON8_COMPRESSION   0
#define ICON8_MASKING       2

#ifdef USE_ICON8_HEADER
const struct BitMapHeader icon8_header =
{ 24, 14, 186, 289, 3, 2, 0, 0, 0, 14, 14, 24, 14 };
#endif

#ifdef USE_ICON8_BODY
const UBYTE icon8_body[168] = {
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0xff,0xff,0xfc,0x00,0x00,0x00,0x02,0x00,0xff,
0xff,0xfc,0x00,0x80,0x00,0x02,0x00,0x7f,0xff,0xf8,0x00,0xff,0xff,0xfc,0x00,
0x80,0x00,0x04,0x00,0x40,0x00,0x02,0x00,0xff,0xff,0xfe,0x00,0x8c,0x00,0x04,
0x00,0x46,0x80,0x02,0x00,0xf1,0x7f,0xfe,0x00,0x85,0x60,0x04,0x00,0x4c,0x20,
0x02,0x00,0xf2,0x9f,0xfe,0x00,0x80,0x00,0x04,0x00,0x40,0x00,0x02,0x00,0xff,
0xff,0xfe,0x00,0xbf,0xff,0xfc,0x00,0x00,0x00,0x02,0x00,0xff,0xff,0xfe,0x00,
0x40,0x00,0x00,0x00,0xbf,0xff,0xfe,0x00,0x3f,0xff,0xfe,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00, };
#endif

#ifdef USE_ICON8_BITMAP
const struct BitMap icon8_bitmap =
{
  4, 14, 0, ICON8_DEPTH, 0,
  { (UBYTE *)icon8_body+(0*14*4),
    (UBYTE *)icon8_body+(1*14*4),
    (UBYTE *)icon8_body+(2*14*4),
    NULL, NULL, NULL, NULL, NULL }
};
#endif
