/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/




#ifndef _STUDIO_H_
#define _STUDIO_H_

/*
==============================================================================

STUDIO MODELS

Studio models are position independent, so the cache manager can move them.
==============================================================================
*/
 

#define MAXSTUDIOTRIANGLES	20000	// TODO: tune this
#define MAXSTUDIOVERTS		2048	// TODO: tune this
#define MAXSTUDIOSEQUENCES	2048	// total animation sequences -- KSH incremented
#define MAXSTUDIOSKINS		100		// total textures
#define MAXSTUDIOSRCBONES	512		// bones allowed at source movement
#define MAXSTUDIOBONES		128		// total bones actually used
#define MAXSTUDIOMODELS		32		// sub-models per model
#define MAXSTUDIOBODYPARTS	32
#define MAXSTUDIOGROUPS		16
#define MAXSTUDIOANIMATIONS	2048		
#define MAXSTUDIOMESHES		256
#define MAXSTUDIOEVENTS		1024
#define MAXSTUDIOPIVOTS		256
#define MAXSTUDIOCONTROLLERS 8


typedef struct
{
	int					id;
	int					version;

	char				name[64];
	// 48h
	int					length;

	// 4Ch
	int					numbones;				// bones
	int					boneindex;				// (->BCh)

	// 54h
	int					numbonecontrollers;		// bone controllers
	// TOMAS: turret.mdl has 2
	int					bonecontrollerindex;	// if num == 0 then this points to bones! not controlers!

	// 5Ch
	int					numseq;					// animation sequences
	int					seqindex;

	// 64h
	int					numtextures;			// raw textures
	int					textureindex;
	int					texturedataindex;

	// 70h
	int					numskinref;				// replaceable textures
	int					numskinfamilies;
	int					skinindex;

	// 7Ch
	int					numbodyparts;
	int					bodypartindex;			// (->mstudiobodyparts_t)

	int					unused[14];				// TOMAS: UNUSED (checked)

} studiohdr_t;

// header for demand loaded sequence group data
typedef struct 
{
	int					id;
	int					version;

	char				name[64];
	int					length;
} studioseqhdr_t;

// bones
typedef struct 
{
	char				name[32];	// bone name for symbolic links
	int		 			parent;		// parent bone
	int		 			unused[6];
} mstudiobone_t;


// bone controllers
typedef struct 
{
	int					bone;	// -1 == 0
	int					type;	// X, Y, Z, XR, YR, ZR, M
	float				start;
	float				end;
} mstudiobonecontroller_t;

// intersection boxes
typedef struct
{
	int					bone;
	int					group;			// intersection group
	vec3_t				bbmin;		// bounding box
	vec3_t				bbmax;		
} mstudiobbox_t;

#if !defined( CACHE_USER ) && !defined( QUAKEDEF_H )
#define CACHE_USER
typedef struct cache_user_s
{
	void *data;
} cache_user_t;
#endif

// sequence descriptions
typedef struct
{
	char				label[32];		// sequence label

	float				fps;			// frames per second
	int					flags;			// looping/non-looping flags

	int					numevents;		// TOMAS: USED (not always 0)
	int					eventindex;

	int					numframes;		// number of frames per sequence

	int					unused01;		// TOMAS: UNUSED (checked)

	int					numpivots;		// number of foot pivots
	// TOMAS: polyrobo.mdl use this (4)
	int					pivotindex;

	int					motiontype;		// TOMAS: USED (not always 0)
	int					motionbone;		// motion bone id (0)

	int					unused02;		// TOMAS: UNUSED (checked)
	vec3_t				linearmovement;	// TOMAS: USED (not always 0)

	int					numblends;		// TOMAS: UNUSED (checked)
	int					animindex;		// (->mstudioanim_t)

	int					unused03[2];	// TOMAS: UNUSED (checked)
} mstudioseqdesc_t;

// events
#include "studio_event.h"
/*
typedef struct 
{
	int 				frame;
	int					event;
	int					type;
	char				options[64];
} mstudioevent_t;
*/

// pivots
typedef struct 
{
	vec3_t				org;	// pivot point
	int					start;
	int					end;
} mstudiopivot_t;

// attachment
typedef struct 
{
	char				name[32];
	int					type;
	int					bone;
	vec3_t				org;	// attachment point
	vec3_t				vectors[3];
} mstudioattachment_t;

typedef struct
{
	int					numpos;			// count of mstudiobnonepos_t
	int					posindex;		// (->mstudiobnonepos_t)
	int					numrot;			// count of mstudiobonerot_t
	int					rotindex;		// (->mstudiobonerot_t)
} mstudioanim_t;

// animation frames data (16B)
// TOMAS: "New" in MDL v6
typedef struct
{
	int					frame;			// frame id (frame <= numframes)
	vec3_t				pos;
} mstudiobonepos_t;

// animation frames data (8B)
// TOMAS: "New" in MDL v6
typedef struct
{
	short 				frame;			// frame id (frame <= numframes)
	short 				angle[3];		// (values: +/-18000; 18000 = 180deg)
} mstudiobonerot_t;

typedef union
{
	struct {
		byte	valid;
		byte	total;
	} num;
	short		value;
} mstudioanimvalue_t;

// body part index
typedef struct
{
	char				name[64];
	int					nummodels;
	int					base;
	int					modelindex; // index into models array (->mstudiomodel_t)
} mstudiobodyparts_t;



// skin info
typedef struct
{
	char					name[64];
	int						flags;
	int						width;
	int						height;
	int						index;
} mstudiotexture_t;


// skin families
// short	index[skinfamilies][skinref]

// studio models
typedef struct
{
	char				name[64];

	int					type;

	int					unk01;			// TOMAS: (==1)
	int					unused01;		// TOMAS: UNUSED (checked)

	int					nummesh;
	int					meshindex;

	// vertex bone info
	int					numverts;		// number of unique vertices
	int					vertinfoindex;	// vertex bone info

	// normal bone info
	int					numnorms;		// number of unique surface normals
	int					norminfoindex;	// normal bone info

	// TOMAS: NEW IN MDL v6
	int					unused02;		// TOMAS: UNUSED (checked)
	int					modeldataindex;	// (->mstudiomodeldata_t)
} mstudiomodel_t;

// TOMAS: NEW IN MDL v6
// studio models data (28B)
typedef struct
{
	// TOMAS: UNDONE:
	int					unk01;
	int					unk02;
	int					unk03;

	int					numverts;		// number of unique vertices
	int					vertindex;		// vertex vec3_t (data)

	int					numnorms;		// number of unique surface normals
	int					normindex;		// normal vec3_t (data)

} mstudiomodeldata_t;



// meshes
typedef struct 
{
	int					numtris;
	int					triindex;
	int					skinref;
	int					numnorms;		// per mesh normals
	int					normindex;		// normal vec3_t
} mstudiomesh_t;

// triangles
typedef struct 
{
	short				vertindex;		// index into vertex array
	short				normindex;		// index into normal array
	short				s,t;			// s,t position on skin
} mstudiotrivert_t;

// lighting options
#define STUDIO_NF_FLATSHADE		0x0001
#define STUDIO_NF_CHROME		0x0002
#define STUDIO_NF_FULLBRIGHT	0x0004
#define STUDIO_NF_NOMIPS        0x0008
#define STUDIO_NF_ALPHA         0x0010
#define STUDIO_NF_ADDITIVE      0x0020
#define STUDIO_NF_MASKED        0x0040

// motion flags
#define STUDIO_X		0x0001
#define STUDIO_Y		0x0002	
#define STUDIO_Z		0x0004
#define STUDIO_XR		0x0008
#define STUDIO_YR		0x0010
#define STUDIO_ZR		0x0020
#define STUDIO_LX		0x0040
#define STUDIO_LY		0x0080
#define STUDIO_LZ		0x0100
#define STUDIO_AX		0x0200
#define STUDIO_AY		0x0400
#define STUDIO_AZ		0x0800
#define STUDIO_AXR		0x1000
#define STUDIO_AYR		0x2000
#define STUDIO_AZR		0x4000
#define STUDIO_TYPES	0x7FFF
#define STUDIO_RLOOP	0x8000	// controller that wraps shortest distance

// sequence flags
#define STUDIO_LOOPING	0x0001

// bone flags
#define STUDIO_HAS_NORMALS	0x0001
#define STUDIO_HAS_VERTICES 0x0002
#define STUDIO_HAS_BBOX		0x0004
#define STUDIO_HAS_CHROME	0x0008	// if any of the textures have chrome on them

#define RAD_TO_STUDIO		(32768.0/M_PI)
#define STUDIO_TO_RAD		(M_PI/32768.0)

#endif
