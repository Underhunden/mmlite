/*-------------------------------------------------------------------------
 *
 * mmlite.h
 *		Logical Replication output plugin for lightweight
 *      multimaster environments
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
