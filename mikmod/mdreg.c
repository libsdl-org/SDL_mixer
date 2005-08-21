/*	MikMod sound library
	(c) 1998, 1999 Miodrag Vallat and others - see file AUTHORS for
	complete list.

	This library is free software; you can redistribute it and/or modify
	it under the terms of the GNU Library General Public License as
	published by the Free Software Foundation; either version 2 of
	the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU Library General Public License for more details.

	You should have received a copy of the GNU Library General Public
	License along with this library; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
	02111-1307, USA.
*/

/*==============================================================================

  $Id$

  Routine for registering all drivers in libmikmod for the current platform.

==============================================================================*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "mikmod_internals.h"

void _mm_registeralldrivers(void)
{
	_mm_registerdriver(&drv_nos);
}

void MikMod_RegisterAllDrivers(void)
{
	MUTEX_LOCK(lists);
	_mm_registeralldrivers();
	MUTEX_UNLOCK(lists);
}

void MikMod_UnregisterAllDrivers(void)
{
	MUTEX_LOCK(lists);
	_mm_unregisterdrivers();
	MUTEX_UNLOCK(lists);
}

/* ex:set ts=4: */
