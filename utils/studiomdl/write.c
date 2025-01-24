/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
****/

//
// write.c: writes a studio .mdl file
//



#pragma warning( disable : 4244 )
#pragma warning( disable : 4237 )
#pragma warning( disable : 4305 )


#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "archtypes.h"
#include "cmdlib.h"
#include "lbmlib.h"
#include "scriplib.h"
#include "mathlib.h"
#include "..\..\engine\studio.h"
#include "studiomdl.h"


int totalframes = 0;
float totalseconds = 0;
extern int numcommandnodes;



/*
============
WriteModel
============
*/
byte *pData;
byte* pDatadummy;
byte *pStart;
studiohdr_t *phdr;
studioseqhdr_t *pseqhdr;

#define ALIGN( a ) a = (byte *)((int)((byte *)a + 3) & ~ 3)
void WriteBoneInfo( )
{
	int i, j;
	mstudiobone_t *pbone;
	mstudiobonecontroller_t *pbonecontroller;
	mstudiobbox_t *pbbox;

	// save bone info
	pbone = (mstudiobone_t *)pData;
	phdr->numbones = numbones;
	phdr->boneindex = (pData - pStart);

	for (i = 0; i < numbones; i++) 
	{
		strcpy( pbone[i].name, bonetable[i].name );
		pbone[i].parent			= bonetable[i].parent;
		pbone[i].unused[0] = 0;
		pbone[i].unused[1] = 0;
		pbone[i].unused[2] = 0;
		pbone[i].unused[3] = 0;
		pbone[i].unused[4] = 0;
		pbone[i].unused[5] = 0;
		pbone[i].unused[6] = 0;
	}
	pData += numbones * sizeof( mstudiobone_t );
	pDatadummy += numbones * sizeof(mstudiobone_t);
	ALIGN( pData );
	ALIGN( pDatadummy );


	// save bonecontroller info
	pbonecontroller = (mstudiobonecontroller_t *)pData;
	phdr->numbonecontrollers = numbonecontrollers;
	phdr->bonecontrollerindex = (pData - pStart);

	for (i = 0; i < numbonecontrollers; i++) {
		pbonecontroller[i].bone			= bonecontroller[i].bone;
		pbonecontroller[i].type			= bonecontroller[i].type;
		pbonecontroller[i].start		= bonecontroller[i].start;
		pbonecontroller[i].end			= bonecontroller[i].end;
	}
	pData += numbonecontrollers * sizeof( mstudiobonecontroller_t );
	pDatadummy += numbonecontrollers * sizeof(mstudiobonecontroller_t);
	ALIGN( pData );
	ALIGN( pDatadummy );

}


void WriteSequenceInfo( )
{
	int i, j, q;
	mstudiobonepos_t* pbonepos;
	mstudiobonerot_t* pbonerot;
	mstudioanim_t* panim;

	mstudioseqdesc_t	*pseqdesc;
	mstudioseqdesc_t	*pbaseseqdesc;
	mstudioevent_t		*pevent;
	mstudiopivot_t		*ppivot;

	// save sequence info
	pseqdesc = (mstudioseqdesc_t *)pData;
	pbaseseqdesc = pseqdesc;
	phdr->numseq = numseq;
	phdr->seqindex = (pData - pStart);
	pData += numseq * sizeof( mstudioseqdesc_t );

	for (i = 0; i < numseq; i++, pseqdesc++) 
	{
		strcpy( pseqdesc->label, sequence[i].name );
		pseqdesc->numframes		= sequence[i].numframes;
		pseqdesc->fps			= sequence[i].fps;
		pseqdesc->flags			= sequence[i].flags;

		pseqdesc->numblends		= sequence[i].numblends;

		pseqdesc->motiontype	= sequence[i].motiontype;
		pseqdesc->motionbone	= 0; // sequence[i].motionbone;
		VectorCopy( sequence[i].linearmovement, pseqdesc->linearmovement );

		totalframes				+= sequence[i].numframes;
		totalseconds			+= sequence[i].numframes / sequence[i].fps;

		// save events
		pevent					= (mstudioevent_t *)pData;
		pseqdesc->numevents		= sequence[i].numevents;
		pseqdesc->eventindex	= (pData - pStart);
		pData += pseqdesc->numevents * sizeof( mstudioevent_t );
		for (j = 0; j < sequence[i].numevents; j++)
		{
			pevent[j].frame		= sequence[i].event[j].frame - sequence[i].frameoffset;
			pevent[j].type = sequence[i].event[j].event;
		}
		ALIGN( pData );

		// save pivots
		ppivot					= (mstudiopivot_t *)pData;
		pseqdesc->numpivots		= sequence[i].numpivots;
		pseqdesc->pivotindex	= (pData - pStart);
		pData += pseqdesc->numpivots * sizeof( mstudiopivot_t );
		for (j = 0; j < sequence[i].numpivots; j++)
		{
			VectorCopy( sequence[i].pivot[j].org, ppivot[j].org );
			ppivot[j].start		= sequence[i].pivot[j].start - sequence[i].frameoffset;
			ppivot[j].end		= sequence[i].pivot[j].end - sequence[i].frameoffset;
		}
		ALIGN( pData );


		panim = (mstudioanim_t*)pData;
		sequence[i].animindex = (pData - pStart);
		pseqdesc->animindex = sequence[i].animindex;
		pData += numbones * sizeof(mstudioanim_t);
		// Write the panim header
		for (j = 0; j < numbones; j++)
		{
			panim[j].numpos = sequence[i].numframes;
			panim[j].numrot = sequence[i].numframes;
		}
		ALIGN(pData);

		panim->posindex = (pData - pStart);
		pbonepos = (mstudiobonepos_t*)pData;
		pData += sequence[i].numframes * sizeof(mstudiobonepos_t);
		for (q = 0; q < sequence[i].numframes; q++) {
			for (j = 0; j < numbones; j++) {
				pbonepos[j].pos[0] = sequence[i].panim->pos[j][q][0];
				pbonepos[j].pos[1] = sequence[i].panim->pos[j][q][1];
				pbonepos[j].pos[2] = sequence[i].panim->pos[j][q][2];
				pbonepos[j].frame = q;
			}
		}
		ALIGN(pData);
		panim->rotindex = (pData - pStart);
		pbonerot = (mstudiobonerot_t*)pData;
		pData += sequence[i].numframes * sizeof(mstudiobonerot_t);
		for (q = 0; q < sequence[i].numframes; q++) {
			for (j = 0; j < numbones; j++) {
				pbonerot[j].angle[0] = sequence[i].panim->rot[j][q][0];
				pbonerot[j].angle[1] = sequence[i].panim->rot[j][q][1];
				pbonerot[j].angle[2] = sequence[i].panim->rot[j][q][2];
				pbonerot[j].frame = q;
			}
		}
		ALIGN(pData);


	}
}

void WriteAnimationsDummy(byte *pDatadummy, byte *pStart, int group) {
	int i, j, q;
	mstudiobonepos_t* pbonepos;
	mstudiobonerot_t* pbonerot;
	mstudioanim_t* panim;


	for (i = 0; i < numseq; i++) {
		// Write the panim header
		panim = (mstudioanim_t*)pDatadummy;
		sequence[i].animindex = (pDatadummy - pStart);
		pDatadummy += sizeof(mstudioanim_t);

		// Write bone positions
		panim->posindex = (pDatadummy - pStart);
		panim->numpos = sequence[i].numframes * numbones;
		pbonepos = (mstudiobonepos_t*)pDatadummy;
		pDatadummy += sequence[i].numframes * numbones;

		for (q = 0; q < sequence[i].numframes; q++) {
			for (j = 0; j < numbones; j++) {
				//pbonepos[j].pos[0] = sequence[i].panim->pos[j][q][0];
				//pbonepos[j].pos[1] = sequence[i].panim->pos[j][q][1];
				//pbonepos[j].pos[2] = sequence[i].panim->pos[j][q][2];
				//pbonepos[j].frame = q;
			}
		}
		ALIGN(pDatadummy);

		// Write bone rotations
		panim->rotindex = (pDatadummy - pStart);
		panim->numrot = sequence[i].numframes * numbones;
		pbonerot = (mstudiobonerot_t*)pDatadummy;
		pDatadummy += sequence[i].numframes * numbones;

		for (q = 0; q < sequence[i].numframes; q++) {
			for (j = 0; j < numbones; j++) {
				//pbonerot[j].angle[0] = sequence[i].panim->rot[j][q][0];
				//pbonerot[j].angle[1] = sequence[i].panim->rot[j][q][1];
				//pbonerot[j].angle[2] = sequence[i].panim->rot[j][q][2];
				//pbonerot[j].frame = q;
			}
		}
		ALIGN(pDatadummy);
	}
}


byte *WriteAnimations(byte *pData, byte *pStart, int group) {
    int i, j, q;
    mstudiobonepos_t *pbonepos;
    mstudiobonerot_t *pbonerot;
    mstudioanim_t *panim;

    for (i = 0; i < numseq; i++) {
        // Write the panim header
        panim = (mstudioanim_t *)pData;
        sequence[i].animindex = (pData - pStart);
        pData += sizeof(mstudioanim_t);

        // Write bone positions
        panim->posindex = (pData - pStart);
        panim->numpos = sequence[i].numframes * numbones;
        pbonepos = (mstudiobonepos_t *)pData;
		pData += sequence[i].numframes * numbones;

        for (q = 0; q < sequence[i].numframes; q++) {
            for (j = 0; j < numbones; j++) {
                pbonepos[j].pos[0] = sequence[i].panim->pos[j][q][0];
                pbonepos[j].pos[1] = sequence[i].panim->pos[j][q][1];
                pbonepos[j].pos[2] = sequence[i].panim->pos[j][q][2];
				pbonepos[j].frame = q;
            }
        }
        ALIGN(pData);

        // Write bone rotations
        panim->rotindex = (pData - pStart);
        panim->numrot = sequence[i].numframes * numbones;
        pbonerot = (mstudiobonerot_t *)pData;
		pData += sequence[i].numframes * numbones;

        for (q = 0; q < sequence[i].numframes; q++) {
            for (j = 0; j < numbones; j++) {
                pbonerot[j].angle[0] = sequence[i].panim->rot[j][q][0];
                pbonerot[j].angle[1] = sequence[i].panim->rot[j][q][1];
                pbonerot[j].angle[2] = sequence[i].panim->rot[j][q][2];
                pbonerot[j].frame = q;
            }
        }
        ALIGN(pData);
    }

    return pData;
}
	
void WriteTextures( )
{
	int i, j;
	mstudiotexture_t *ptexture;
	short	*pref;

	// save bone info
	ptexture = (mstudiotexture_t *)pData;
	phdr->numtextures = numtextures;
	phdr->textureindex = (pData - pStart);
	pData += numtextures * sizeof( mstudiotexture_t );
	ALIGN( pData );

	phdr->skinindex = (pData - pStart);
	phdr->numskinref = numskinref;
	phdr->numskinfamilies = numskinfamilies;
	pref	= (short *)pData;

	for (i = 0; i < phdr->numskinfamilies; i++) 
	{
		for (j = 0; j < phdr->numskinref; j++) 
		{
			*pref = skinref[i][j];
			pref++;
		}
	}
	pData = (byte *)pref;
	ALIGN( pData );

	phdr->texturedataindex = (pData - pStart); 	// must be the end of the file!

	for (i = 0; i < numtextures; i++) {
		strcpy( ptexture[i].name, texture[i].name );
		ptexture[i].flags		= texture[i].flags;
		ptexture[i].width		= texture[i].skinwidth;
		ptexture[i].height		= texture[i].skinheight;
		ptexture[i].index		= (pData - pStart);
		memcpy( pData, texture[i].pdata, texture[i].size );
		pData += texture[i].size;
	}
	ALIGN( pData );
}


void WriteModel( )
{
	int i, j, k;

	mstudiobodyparts_t	*pbodypart;
	mstudiomodel_t	*pmodel;
	mstudiomodeldata_t* pmodeldata;
	// vec3_t			*bbox;
	byte			*pbone;
	vec3_t			*pvert;
	vec3_t			*pnorm;
	mstudiomesh_t	*pmesh;
	s_trianglevert_t *psrctri;
	int				cur;
	int				total_tris = 0;
	int				total_strips = 0;

	pbodypart = (mstudiobodyparts_t *)pData;
	phdr->numbodyparts = numbodyparts;
	phdr->bodypartindex = (pData - pStart);
	pData += numbodyparts * sizeof( mstudiobodyparts_t );

	pmodel = (mstudiomodel_t *)pData;
	pData += nummodels * sizeof( mstudiomodel_t );

	for (i = 0; i < nummodels; i++)
	{
		pmodel[i].modeldataindex = (pData - pStart);
	}

	pmodeldata = (mstudiomodeldata_t*)pData;
	pData += sizeof(mstudiomodeldata_t);

	for (i = 0, j = 0; i < numbodyparts; i++)
	{
		strcpy( pbodypart[i].name, bodypart[i].name );
		pbodypart[i].nummodels		= bodypart[i].nummodels;
		pbodypart[i].base			= bodypart[i].base;
		pbodypart[i].modelindex		= ((byte *)&pmodel[j]) - pStart;
		j += bodypart[i].nummodels;
	}
	ALIGN( pData );

	cur = (int)pData;
	for (i = 0; i < nummodels; i++) 
	{
		int normmap[MAXSTUDIOVERTS];
		int normimap[MAXSTUDIOVERTS];
		int n = 0;

		strcpy( pmodel[i].name, model[i]->name );

		// save bbox info

		// remap normals to be sorted by skin reference
		for (j = 0; j < model[i]->nummesh; j++)
		{
			for (k = 0; k < model[i]->numnorms; k++)
			{
				if (model[i]->normal[k].skinref == model[i]->pmesh[j]->skinref)
				{
					normmap[k] = n;
					normimap[n] = k;
					n++;
					model[i]->pmesh[j]->numnorms++;
				}
			}
		}
		
		// save vertice bones
		pbone = pData;
		pmodel[i].numverts	= model[i]->numverts;
		pmodel[i].vertinfoindex = (pData - pStart);
		for (j = 0; j < pmodel[i].numverts; j++)
		{
			*pbone++ = model[i]->vert[j].bone;
		}
		ALIGN( pbone );

		// save normal bones
		pmodel[i].numnorms	= model[i]->numnorms;
		pmodel[i].norminfoindex = ((byte *)pbone - pStart);
		for (j = 0; j < pmodel[i].numnorms; j++)
		{
			*pbone++ = model[i]->normal[normimap[j]].bone;
		}
		ALIGN( pbone );

		pData = pbone;

		// save group info
		pvert = (vec3_t *)pData;
		pData += model[i]->numverts * sizeof( vec3_t );
		pmodeldata[i].vertindex		= ((byte *)pvert - pStart); 
		ALIGN( pData );			

		pnorm = (vec3_t *)pData;
		pData += model[i]->numnorms * sizeof( vec3_t );
		pmodeldata[i].normindex		= ((byte *)pnorm - pStart); 
		ALIGN( pData );


		for (j = 0; j < model[i]->numverts; j++)
		{
			VectorCopy( model[i]->vert[j].org, pvert[j] );
		}

		for (j = 0; j < model[i]->numnorms; j++)
		{
			VectorCopy( model[i]->normal[normimap[j]].org, pnorm[j] );
		}
		printf("vertices  %6d bytes (%d vertices, %d normals)\n", pData - cur, model[i]->numverts, model[i]->numnorms);
		cur = (int)pData;

		// save mesh info
		pmesh = (mstudiomesh_t *)pData;
		pmodel[i].nummesh		= model[i]->nummesh;
		pmodel[i].meshindex		= (pData - pStart);
		pData += pmodel[i].nummesh * sizeof( mstudiomesh_t );
		ALIGN( pData );

		total_tris = 0;
		total_strips = 0;
		for (j = 0; j < model[i]->nummesh; j++)
		{
			int numCmdBytes;
			byte *pCmdSrc;

			pmesh[j].numtris	= model[i]->pmesh[j]->numtris;
			pmesh[j].skinref	= model[i]->pmesh[j]->skinref;
			pmesh[j].numnorms	= model[i]->pmesh[j]->numnorms;

			psrctri				= (s_trianglevert_t *)(model[i]->pmesh[j]->triangle);
			for (k = 0; k < pmesh[j].numtris * 3; k++) 
			{
				psrctri->normindex	= normmap[psrctri->normindex];
				psrctri++;
			}

			numCmdBytes = BuildTris( model[i]->pmesh[j]->triangle, model[i]->pmesh[j], &pCmdSrc );

			pmesh[j].triindex	= (pData - pStart);
			memcpy( pData, pCmdSrc, numCmdBytes );
			pData += numCmdBytes;
			ALIGN( pData );
			total_tris += pmesh[j].numtris;
			total_strips += numcommandnodes;
		}
		printf("mesh      %6d bytes (%d tris, %d strips)\n", pData - cur, total_tris, total_strips);
		cur = (int)pData;
	}	
}



#define FILEBUFFER ( 16 * 1024 * 1024)
	

void WriteFile (void)
{
	FILE		*modelouthandle;
	int			total = 0;
	int			i;

	pStart = kalloc( 1, FILEBUFFER );

	StripExtension (outname);
//
// write the model output file
//
	strcat (outname, ".mdl");
	
	printf ("---------------------\n");
	printf ("writing %s:\n", outname);
	modelouthandle = SafeOpenWrite (outname);

	phdr = (studiohdr_t *)pStart;

	phdr->id = IDSTUDIOHEADER;
	phdr->version = STUDIO_VERSION;
	strcpy( phdr->name, outname );

	pData = (byte *)phdr + sizeof( studiohdr_t );
	pDatadummy = (byte*)phdr + sizeof(studiohdr_t);

	WriteBoneInfo( );
	printf("bones     %6d bytes (%d)\n", pData - pStart - total, numbones );
	total = pData - pStart;

	//WriteAnimationsDummy(pDatadummy, pStart, 0);

	WriteSequenceInfo();
	printf("sequences %6d bytes (%d frames) [%d:%02d]\n", pData - pStart - total, totalframes, (int)totalseconds / 60, (int)totalseconds % 60);
	total = pData - pStart;

	//pData = WriteAnimations( pData, pStart, 0 );

	WriteModel( );
	printf("models    %6d bytes\n", pData - pStart - total );
	total  = pData - pStart;

	if (!split_textures)
	{
		WriteTextures( );
		printf("textures  %6d bytes\n", pData - pStart - total );
	}

	phdr->length = pData - pStart;

	printf("total     %6d\n", phdr->length );

	SafeWrite( modelouthandle, pStart, phdr->length );

	fclose (modelouthandle);
}



