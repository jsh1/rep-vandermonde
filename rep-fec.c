/* rep-fec.c -- fec wrapper

   Copyright (C) 2001 John Harper <jsh@unfactored.org>

   $Id: rep-fec.c,v 1.2 2001/11/26 05:46:19 jsh Exp $

   This file is part of librep.

   librep is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   librep is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with librep; see the file COPYING.  If not, write to
   the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.  */

#define _GNU_SOURCE

#include <rep.h>
#include "fec.h"
#include <unistd.h>

static rep_bool
rep_to_array (repv in, int **array_ret, int length, int undef)
{
    int *array;

    array = malloc (sizeof (int) * length);

    if (rep_CONSP (in))
    {
	int i;
	for (i = 0; i < length; i++)
	{
	    if (rep_CONSP (in))
	    {
		array[i] = rep_get_long_int (rep_CAR (in));
		in = rep_CDR (in);
	    }
	    else
		array[i] = undef;
	}
    }
    else if (rep_VECTORP (in))
    {
	int i;
	for (i = 0; i < length; i++)
	{
	    if (i < rep_VECT_SIZE (in))
		array[i] = rep_get_long_int (rep_VECTI (in, i));
	    else
		array[i] = undef;
	}
    }
    else
    {
	free (array);
	rep_signal_arg_error (in, 1);
	return rep_FALSE;
    }

    *array_ret = array;
    return rep_TRUE;
}

DEFUN ("vandermonde-encode-file", Fvandermonde_encode_file,
       Svandermonde_encode_file,
       (repv src_file, repv dest_file, repv k, repv n), rep_Subr4)
{
    void *code;
    size_t block_size;

    rep_DECLARE1 (src_file, rep_STRINGP);
    rep_DECLARE2 (dest_file, rep_STRINGP);
    rep_DECLARE3 (k, rep_INTP);
    rep_DECLARE (4, n, rep_INTP (n) && rep_INT (n) >= rep_INT (k));

    code = fec_new (rep_INT (k), rep_INT (n));
    if (code != 0)
    {
	int ret = fec_encode_file (rep_STR (src_file),
				   rep_STR (dest_file),
				   code, &block_size);
	if (ret == 0)
	    return rep_make_long_uint (block_size);
	else
	    return Fsignal (Qerror, rep_LIST_1 (rep_string_dup ("encoding error")));
    }
    else
	return Fsignal (Qerror, rep_LIST_1 (rep_string_dup ("can't make coder")));
}

DEFUN ("vandermonde-decode-file", Fvandermonde_decode_file,
       Svandermonde_decode_file, (repv args), rep_SubrN)
{
    repv k, n, block_size, extra, src_file, src_indices, dest_file;

    int *array;
    void *code;

    if (!rep_assign_args (args, 7, 7, &k, &n, &block_size, &extra,
			  &src_file, &src_indices, &dest_file))
	return rep_NULL;

    rep_DECLARE (1, k, rep_INTP (k));
    rep_DECLARE (2, n, rep_INTP (n) && rep_INT (n) >= rep_INT (k));
    rep_DECLARE (3, block_size, rep_INTP (block_size));
    rep_DECLARE (4, extra, rep_INTP (extra));
    rep_DECLARE (5, src_file, rep_STRINGP (src_file));
    rep_DECLARE (6, src_indices, rep_LISTP (src_indices) || rep_VECTORP (src_indices));
    rep_DECLARE (7, dest_file, rep_STRINGP (dest_file));

    if (!rep_to_array (src_indices, &array, rep_INT (n) + rep_INT (k), rep_INT (n)))
	return rep_signal_arg_error (src_indices, 5);

    code = fec_new (rep_INT (k), rep_INT (n));
    if (code != 0)
    {
	int ret = fec_decode_file (rep_STR (src_file),
				   rep_STR (dest_file),
				   array, code,
				   rep_INT (block_size));
	if (ret == 0)
	{
	    rep_long_long size = rep_INT (block_size);
	    size = size * rep_INT (k) - rep_INT (extra);
	    if (truncate (rep_STR (dest_file), size) == 0)
		return Qt;
	    else
		return rep_signal_file_error (dest_file);
	}
	else
	    return Fsignal (Qerror, rep_LIST_1 (rep_string_dup ("decoding error")));
    }
    else
	return Fsignal (Qerror, rep_LIST_1 (rep_string_dup ("can't make coder")));
}


/* init */

repv
rep_dl_init (void)
{
    repv tem = rep_push_structure ("vandermonde");

    rep_ADD_SUBR (Svandermonde_encode_file);
    rep_ADD_SUBR (Svandermonde_decode_file);

    return rep_pop_structure (tem);
}

void
rep_dl_kill (void)
{
}
