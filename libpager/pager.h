/* Definitions for multi-threaded pager library
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


#ifndef _HURD_PAGER_
#define _HURD_PAGER_

/* This declaration exists to place struct user_pager_info in the proper
   scope.  */
struct user_pager_info;

/* This de-muxer function is for use within libports_demuxer. */
/* INP is a message we've received; OUTP will be filled in with
   a reply message.  */
int pager_demuxer (mach_msg_header_t *inp,
		   mach_msg_header_t *outp);

/* Create a new pager.  The pager will have a port created for it
   (using libports) and will be immediately ready to receive requests.
   U_PAGER will be provided to later calls to pager_find_address.  
   The pager will have one user reference created.  */
struct pager *
pager_create (struct user_pager_info *u_pager);

/* Sync data from pager PAGER to backing store; wait for
   all the writes to complete iff WAIT is set. */
void
pager_sync (struct pager *pager,
	    int wait);

/* Sync some data (starting at START, for LEN bytes) from pager PAGER
   to backing store.  Wait for all the writes to complete iff WAIT is
   set.  */
void
pager_sync_some (struct pager *pager,
		 vm_address_t start,
		 vm_size_t len,
		 int wait);

/* Flush data from the kernel for pager PAGER and force any pending
   delayed copies.  Wait for all pages to be flushed iff WAIT is set. */
void
pager_flush (struct pager *pager,
	     int wait);


/* Flush some data (starting at START, for LEN bytes) for pager PAGER
   from the kernel.  Wait for all pages to be flushed iff WAIT is set.  */
void
pager_flush_some (struct pager *pager,
		  vm_address_t start,
		  vm_size_t len,
		  int wait);

/* Change the attributes of the memory object underlying pager PAGER.
   Args MAY_CACHE and COPY_STRATEGY are as for 
   memory_object_change_atributes.  Wait for the kernel to report completion
   off WAIT is set.*/
void
pager_change_attributes (struct pager *pager,
			 boolean_t may_cache,
			 memory_object_copy_strategy_t copy_strategy,
			 int wait);

/* Return the port (receive right) for requests to the pager.  It is
   absolutely necessary that a new send right be created from this
   receive right.  */
mach_port_t
pager_get_port (struct pager *pager);

/* Call this whenever a no-senders notification is called for a port
   of type pager_port_type.  The first argument is the result of a
   check_port_type.  */
void
pager_no_senders (struct pager *pager,
		  mach_port_seqno_t seqno,
		  mach_port_mscount_t mscount);

/* Force termination of a pager.  After this returns, no
   more paging requests on the pager will be honored, and the 
   pager will be deallocated.  (The actual deallocation might
   occur asynchronously if there are currently outstanding paging
   requests that will complete first.)  */
void
pager_shutdown (struct pager *pager);

/* This is a clean routine which should be called when a pager port 
   is being deallocated.  It is suitable to be installed directly into
   the libports_cleanroutines array.  ARG is the address of the
   struct pager referenced by the port.  */
void
pager_clean (void *arg);

/* Return the error code of the last page error for pager P at address ADDR;
   this will be deleted when the kernel interface is fixed.  */
error_t
pager_get_error (struct pager *p, vm_address_t addr);

/* Allocate a user reference to pager structure P */
void
pager_reference (struct pager *p);

/* Deallocate a user reference to pager structure P */
void
pager_unreference (struct pager *p);

/* The user must set this variable.  This will be the type used in calls
   to allocate_port by the pager system.  */
extern int pager_port_type;

/* The user must define this function.  Describe for pager PAGER 
   the may_cache and copy_strategy attributes for memory_object_ready. */
void
pager_report_attributes (struct user_pager_info *pager,
			 boolean_t *may_cache,
			 memory_object_copy_strategy_t *copy_strategy);

/* The user must define this function.  For pager PAGER, read one
   page from offset PAGE.  Set *BUF to be the address of the page,
   and set *WRITE_LOCK if the page must be provided read-only. 
   The only permissable error returns are EIO, EDQUOT, and ENOSPC. */
error_t
pager_read_page (struct user_pager_info *pager,
		 vm_offset_t page,
		 vm_address_t *buf,
		 int *write_lock);

/* The user must define this function.  For pager PAGER, synchronously
   write one page from BUF to offset PAGE.  In addition, vm_deallocate 
   (or equivalent) BUF.  The only permissable error returns are EIO,
   EDQUOT, and ENOSPC. */
error_t
pager_write_page (struct user_pager_info *pager,
		  vm_offset_t page,
		  vm_address_t buf);

/* The user must define this function.  A page should be made writable. */
error_t
pager_unlock_page (struct user_pager_info *pager,
		   vm_offset_t address);

/* The user must define this function.  It should report back (in
   *OFFSET and *SIZE the minimum valid address the pager will accept
   and the size of the object.   */
error_t
pager_report_extent (struct user_pager_info *pager,
		     vm_address_t *offset,
		     vm_size_t *size);

/* The user must define this function.  This is called when a pager is
   being deallocated after all extant send rights have been destroyed.  */
void
pager_clear_user_data (struct user_pager_info *pager);

#endif
