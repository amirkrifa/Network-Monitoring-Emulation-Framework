/*	$Id: flowd-pytypes.h,v 1.1.1.1 2009/01/28 14:23:59 akrifa Exp $	*/

/*
 * Copyright (c) 2005 Damien Miller <djm@mindrot.org>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef _FLOWD_PYTYPES_H
#define _FLOWD_PYTYPES_H

/*
 * Python's structmember.h doesn't provide width-oriented types, 
 * so we do our own
 */

#define FL_T_U8		T_UBYTE
#define FL_T_U16	T_USHORT
#define FL_T_U32	T_UINT
#define FL_T_AF		T_USHORT

#endif /* _FLOWD_PYTYPES_H */

