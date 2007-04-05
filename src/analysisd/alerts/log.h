/* @(#) $Id$ */

/* Copyright (C) 2004,2005 Daniel B. Cid <dcid@ossec.net>
 * All right reserved.
 *
 * This program is a free software; you can redistribute it
 * and/or modify it under the terms of the GNU General Public
 * License (version 2) as published by the FSF - Free Software 
 * Foundation
 */

/* Basic logging operations */

#ifndef __LOG_H
#define __LOG_H

#include "eventinfo.h"

#define FWDROP "drop"
#define FWALLOW "accept"

void OS_Log(Eventinfo *lf);
void OS_Store(Eventinfo *lf);
int FW_Log(Eventinfo *lf);

#endif


