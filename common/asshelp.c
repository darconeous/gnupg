/* asshelp.c - Helper functions for Assuan
 * Copyright (C) 2002, 2004 Free Software Foundation, Inc.
 *
 * This file is part of GnuPG.
 *
 * GnuPG is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * GnuPG is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#ifdef HAVE_LOCALE_H
#include <locale.h>
#endif

#include "util.h"

#include "asshelp.h"

/* Send the assuan command pertaining to the pinenry environment.  The
   OPT_* arguments are optional and may be used to overide the
   defaults taken from the current locale. */
gpg_error_t
send_pinentry_environment (assuan_context_t ctx,
                           const char *opt_display,
                           const char *opt_ttyname,
                           const char *opt_ttytype,
                           const char *opt_lc_ctype,
                           const char *opt_lc_messages)
{
  int rc = 0;
  char *dft_display = NULL;
  char *dft_ttyname = NULL;
  char *dft_ttytype = NULL;
  char *old_lc = NULL; 
  char *dft_lc = NULL;

  dft_display = getenv ("DISPLAY");
  if (opt_display || dft_display)
    {
      char *optstr;
      if (asprintf (&optstr, "OPTION display=%s",
		    opt_display ? opt_display : dft_display) < 0)
	return gpg_error_from_errno (errno);
      rc = assuan_transact (ctx, optstr, NULL, NULL, NULL, NULL, NULL,
			    NULL);
      free (optstr);
      if (rc)
	return map_assuan_err (rc);
    }
  if (!opt_ttyname)
    {
      dft_ttyname = getenv ("GPG_TTY");
      if ((!dft_ttyname || !*dft_ttyname) && ttyname (0))
        dft_ttyname = ttyname (0);
    }
  if (opt_ttyname || dft_ttyname)
    {
      char *optstr;
      if (asprintf (&optstr, "OPTION ttyname=%s",
		    opt_ttyname ? opt_ttyname : dft_ttyname) < 0)
	return gpg_error_from_errno (errno);
      rc = assuan_transact (ctx, optstr, NULL, NULL, NULL, NULL, NULL,
			    NULL);
      free (optstr);
      if (rc)
	return map_assuan_err (rc);
    }
  dft_ttytype = getenv ("TERM");
  if (opt_ttytype || (dft_ttyname && dft_ttytype))
    {
      char *optstr;
      if (asprintf (&optstr, "OPTION ttytype=%s",
		    opt_ttyname ? opt_ttytype : dft_ttytype) < 0)
	return gpg_error_from_errno (errno);
      rc = assuan_transact (ctx, optstr, NULL, NULL, NULL, NULL, NULL,
			    NULL);
      free (optstr);
      if (rc)
	return map_assuan_err (rc);
    }
#if defined(HAVE_SETLOCALE) && defined(LC_CTYPE)
  old_lc = setlocale (LC_CTYPE, NULL);
  if (old_lc)
    {
      old_lc = strdup (old_lc);
      if (!old_lc)
        return gpg_error_from_errno (errno);
    }
  dft_lc = setlocale (LC_CTYPE, "");
#endif
  if (opt_lc_ctype || (dft_ttyname && dft_lc))
    {
      char *optstr;
      if (asprintf (&optstr, "OPTION lc-ctype=%s",
		    opt_lc_ctype ? opt_lc_ctype : dft_lc) < 0)
	rc = gpg_error_from_errno (errno);
      else
	{
	  rc = assuan_transact (ctx, optstr, NULL, NULL, NULL, NULL, NULL,
				NULL);
	  free (optstr);
	  if (rc)
	    rc = map_assuan_err (rc);
	}
    }
#if defined(HAVE_SETLOCALE) && defined(LC_CTYPE)
  if (old_lc)
    {
      setlocale (LC_CTYPE, old_lc);
      free (old_lc);
    }
#endif
  if (rc)
    return rc;
#if defined(HAVE_SETLOCALE) && defined(LC_MESSAGES)
  old_lc = setlocale (LC_MESSAGES, NULL);
  if (old_lc)
    {
      old_lc = strdup (old_lc);
      if (!old_lc)
        return gpg_error_from_errno (errno);
    }
  dft_lc = setlocale (LC_MESSAGES, "");
#endif
  if (opt_lc_messages || (dft_ttyname && dft_lc))
    {
      char *optstr;
      if (asprintf (&optstr, "OPTION lc-messages=%s",
		    opt_lc_messages ? opt_lc_messages : dft_lc) < 0)
	rc = gpg_error_from_errno (errno);
      else
	{
	  rc = assuan_transact (ctx, optstr, NULL, NULL, NULL, NULL, NULL,
				NULL);
	  free (optstr);
	  if (rc)
	    rc = map_assuan_err (rc);
	}
    }
#if defined(HAVE_SETLOCALE) && defined(LC_MESSAGES)
  if (old_lc)
    {
      setlocale (LC_MESSAGES, old_lc);
      free (old_lc);
    }
#endif

  return rc;
}
