/* 
   Copyright (C) 1995 Free Software Foundation, Inc.
   Written by Michael I. Bushnell, p/BSG.

   This file is part of the GNU Hurd.

   The GNU Hurd is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2, or (at
   your option) any later version.

   The GNU Hurd is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111, USA. */

#include "netfs.h"
#include "io_S.h"

error_t
netfs_S_io_readable (struct protid *user,
		     mach_msg_type_number_t *amount)
{
  if (!user)
    return EOPNOTSUPP;
  
  if (!(cred->po->openstat & O_READ))
    return EINVAL;
  
  mutex_lock (&user->po->np->lock);
  err = netfs_validate_stat (user->po->np, user);
  if (!err)
    {
      if (user->po->np->nn_stat.st_size > user->po->filepointer)
	*amount = user->po->np->nn_stat.st_size - user->po->filepointer;
      else
	*amount = 0;
    }
  mutex_unlock (&user->po->np->lock);
  return err;
}
