/* 
   Copyright (C) 1994 Free Software Foundation

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2, or (at
   your option) any later version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA. */

#include "priv.h"
#include "fs_S.h"

/* Implement fs.defs:file_invoke_translator as described in <hurd/fs.defs>. */
kern_return_t
diskfs_S_file_invoke_translator (struct protid *cred,
				 int flags,
				 retry_type *retry,
				 char *retry_name,
				 mach_port_t *retrypt,
				 mach_msg_type_name_t *retrypttype)
{
  error_t error = 0;
  mode_t type;
  struct node *np;
  
  /* This code is very similar (but subtly different) from
     dir-pathtrans.c and fsys-getroot.c.  A way should be found to
     combine them. */

  if (!cred)
    return EOPNOTSUPP;
  
  flags &= O_HURD;

  np = cred->po->np;
  
  mutex_lock (&np->lock);
  
  type = np->dn_stat.st_mode & S_IFMT;
  
 repeat_transcheck:
  /* Ignore O_NOTRANS in the following check */
  if (diskfs_node_translated (np) || np->translator.control != MACH_PORT_NULL)
    {
      mach_port_t control = np->translator.control;

      if (control == MACH_PORT_NULL)
	{
	  /* This use of _diskfs_dotdot_file is completely and utterly
	     bogus. XXX */
	  if (error = diskfs_start_translator (np, _diskfs_dotdot_file))
	    {
	      mutex_unlock (&np->lock);
	      return error;
	    }
	  control = np->translator.control;
	}

      mach_port_mod_refs (mach_task_self (), control, MACH_PORT_RIGHT_SEND, 1);
      mutex_unlock (&np->lock);
      error = fsys_getroot (childcontrol, cred->uids, cred->nuids, 
			    cred->gids, cred->ngids, flags, retry,
			    retryname, retrypt);
      if (error == MACH_SEND_INVALID_DEST)
	{
	  mutex_lock (&np->lock);
	  if (np->translator.control == control)
	    fshelp_translator_drop (&np->translator);
	  mach_port_deallocate (mach_task_self (), control);
	  error = 0;
	  goto repeat_transcheck;
	}
      
      if (!error && *retrypt != MACH_PORT_NULL)
	*retrypttype = MACH_MSG_TYPE_MOVE_SEND;
      else
	*retrypttype = MACH_MSG_TYPE_COPY_SEND;
      
      return error;
    }
  
  /* Ignore O_NOTRANS here. */
  if (type == S_IFLNK && !(flags & O_NOLINK))
    
	  
