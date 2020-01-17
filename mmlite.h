/*-------------------------------------------------------------------------
 *
 * mmlite.h
 *		Logical Replication output plugin for lightweight
 *      multimaster environments
 *
 * Portions Copyright (c) 2020, Jesper St John
 * Portions Copyright (c) 2012-2019, PostgreSQL Global Development Group
 *
 * IDENTIFICATION
 *		mmlite.h
 *
 *-------------------------------------------------------------------------
 */
#ifndef MMLITE_H
#define MMLITE_H

#include "nodes/pg_list.h"

typedef struct MMLiteData
{
	MemoryContext context;		/* private memory context for transient
								 * allocations */

	/* client info */
	uint32		protocol_version;

	List	   *publication_names;
	List	   *publications;
}			MMLiteData;

#endif							/* MMLITE_H */
