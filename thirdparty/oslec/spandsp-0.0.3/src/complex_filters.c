/*
 * SpanDSP - a series of DSP components for telephony
 *
 * complex_filters.c
 *
 * Written by Steve Underwood <steveu@coppice.org>
 *
 * Copyright (C) 2003 Steve Underwood
 *
 * All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * $Id: complex_filters.c,v 1.8 2006/10/24 13:45:25 steveu Exp $
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>

#include "spandsp/complex.h"
#include "spandsp/complex_filters.h"

filter_t *filter_create(fspec_t *fs)
{
    int i;
    filter_t *fi;

    fi = (filter_t *) malloc(sizeof (filter_t) + sizeof (float)*(fs->np + 1));
    if (fi)
    {
    	fi->fs = fs;
    	fi->sum = 0.0;
    	fi->ptr = 0;	    /* mvg avg filters only */
        for (i = 0;  i <= fi->fs->np;  i++)
            fi->v[i] = 0.0;
    }
    return fi;
}

void filter_delete(filter_t *fi)
{
    if (fi)
    	free(fi);
}

float filter_step(filter_t *fi, float x)
{
    return fi->fs->fsf(fi, x);
}

cfilter_t *cfilter_create(fspec_t *fs)
{
    cfilter_t *cfi;

    cfi = (cfilter_t *) malloc(sizeof (cfilter_t));
    cfi->ref = filter_create(fs);
    cfi->imf = filter_create(fs);
    return  cfi;
}

void cfilter_delete(cfilter_t *cfi)
{
    if (cfi)
    {
        filter_delete(cfi->ref);
        filter_delete(cfi->imf);
    }
}

complexf_t cfilter_step(cfilter_t *cfi, const complexf_t *z)
{
    complexf_t cc;
    
    cc.re = filter_step(cfi->ref, z->re);
    cc.im = filter_step(cfi->imf, z->im);
    return cc;
}
