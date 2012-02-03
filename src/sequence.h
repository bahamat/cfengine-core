/*
   Copyright (C) Cfengine AS

   This file is part of Cfengine 3 - written and maintained by Cfengine AS.

   This program is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by the
   Free Software Foundation; version 3.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA

  To the extent this program is licensed as part of the Enterprise
  versions of Cfengine, the applicable Commerical Open Source License
  (COSL) may apply to this file if you as a licensee so wish it. See
  included file COSL.txt.
*/

#ifndef CFENGINE_SEQUENCE_H
#define CFENGINE_SEQUENCE_H

#include "cf.defs.h"

typedef struct
   {
   void **data;
   size_t length;
   size_t capacity;
   void (*ItemDestroy)(void *item);
   } Sequence;

Sequence *SequenceCreate(size_t initialCapacity, void (*ItemDestroy)(void *item));
void SequenceDestroy(Sequence **seq);

void SequenceAppend(Sequence *seq, void *item);
void SequenceRemoveRange(Sequence *seq, size_t start, size_t end);
void SequenceSort(Sequence *seq, __compar_fn_t Compare);


#endif
