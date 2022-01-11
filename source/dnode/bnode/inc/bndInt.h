/*
 * Copyright (c) 2019 TAOS Data, Inc. <jhtao@taosdata.com>
 *
 * This program is free software: you can use, redistribute, and/or modify
 * it under the terms of the GNU Affero General Public License, version 3
 * or later ("AGPL"), as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _TD_BNODE_INT_H_
#define _TD_BNODE_INT_H_

#include "os.h"

#include "tarray.h"
#include "tlog.h"
#include "tmsg.h"
#include "trpc.h"

#include "bnode.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct SBnode {
  int32_t           dnodeId;
  int64_t           clusterId;
  SBnodeCfg         cfg;
  SendReqToDnodeFp  sendReqToDnodeFp;
  SendReqToMnodeFp  sendReqToMnodeFp;
  SendRedirectRspFp sendRedirectRspFp;
} SBnode;

#ifdef __cplusplus
}
#endif

#endif /*_TD_BNODE_INT_H_*/