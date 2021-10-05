/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1982-2012 AT&T Intellectual Property          *
*                      and is licensed under the                       *
*                 Eclipse Public License, Version 1.0                  *
*                    by AT&T Intellectual Property                     *
*                                                                      *
*                A copy of the License is available at                 *
*          http://www.eclipse.org/org/documents/epl-v10.html           *
*         (with md5 checksum b35adb5213ca9657e911e9befb180842)         *
*                                                                      *
*              Information and Software Systems Research               *
*                            AT&T Research                             *
*                           Florham Park NJ                            *
*                                                                      *
*                  David Korn <dgk@research.att.com>                   *
*                                                                      *
***********************************************************************/
#pragma prototyped
/*
 * Array processing routines
 *
 *   David Korn
 *   AT&T Labs
 *   dgk@research.att.com
 *
 */

#include	"defs.h"
#include	<stak.h>
#include	"name.h"

#define NUMSIZE	11
#define is_associative(ap)	array_assoc((Namarr_t*)(ap))
#define array_setbit(cp, n, b)	(cp[n] |= (b))
#define array_clrbit(cp, n, b)	(cp[n] &= ~(b))
#define array_isbit(cp, n, b)	(cp[n] & (b))
#define NV_CHILD		NV_EXPORT
#define ARRAY_CHILD		1
#define ARRAY_NOFREE		2

struct index_array
{
        Namarr_t        header;
	void		*xp;	/* if set, subscripts will be converted */
        int		cur;    /* index of current element */
        int		maxi;   /* maximum index for array */
	unsigned char	*bits;	/* bit array for child subscripts */
        union Value	val[1]; /* array of value holders */
};

struct assoc_array
{
	Namarr_t	header;
	Namval_t	*pos;
	Namval_t	*nextpos;
	Namval_t	*cur;
};

#if SHOPT_FIXEDARRAY
   struct fixed_array
   {
	unsigned char	ndim;
	unsigned char	dim;
	unsigned char	level;
	unsigned char	ptr;
	short		size;
	int		nelem;
	int		curi;
	int		*max;
	int		*incr;
	int		*cur;
	char		*data;
   };
#  define array_fixed_data(ap)	((ap)?((struct fixed_array*)((ap)->fixed))->data:0)
   static void array_fixed_setdata(Namval_t*,Namarr_t*,struct fixed_array*);
#endif /* SHOPT_FIXEDARRAY */

static Namarr_t *array_scope(Namval_t *np, Namarr_t *ap, int flags)
{
	Namarr_t *aq;
#if SHOPT_FIXEDARRAY
	struct fixed_array *fp;
#endif /* SHOPT_FIXEDARRAY */
	struct index_array *ar;
	size_t size = ap->hdr.dsize;
	if(size==0)
		size = ap->hdr.disc->dsize;
        if(!(aq=newof(NIL(Namarr_t*),Namarr_t,1,size-sizeof(Namarr_t))))
                return(0);
        memcpy(aq,ap,size);
	aq->hdr.nofree &= ~1;
        aq->hdr.nofree |= (flags&NV_RDONLY)?1:0;
	if(is_associative(aq))
	{
		aq->scope = (void*)dtopen(&_Nvdisc,Dtoset);
		dtview((Dt_t*)aq->scope,aq->table);
		aq->table = (Dt_t*)aq->scope;
		return(aq);
	}
#if SHOPT_FIXEDARRAY
	else if(fp = (struct fixed_array*)ap->fixed)
	{
		aq->scope = (void*)ap;
		fp = (struct fixed_array*)(aq+1);
		aq->fixed = (void*)fp;
		fp->max = (int*)(fp+1);
		fp->incr = fp->max+fp->ndim;
		fp->cur = fp->incr+fp->ndim;
		return(aq);
	}
#endif /* SHOPT_FIXEDARRAY */
	aq->scope = (void*)ap;
	ar = (struct index_array*)aq;
	memset(ar->val, 0, ar->maxi*sizeof(char*));
	ar->bits =  (unsigned char*)&ar->val[ar->maxi];
	return(aq);
}

static int array_unscope(Namval_t *np,Namarr_t *ap)
{
	Namfun_t *fp;
	if(!ap->scope)
		return(0);
	if(is_associative(ap))
		(*ap->fun)(np, NIL(char*), NV_AFREE);
	if((fp = nv_disc(np,(Namfun_t*)ap,NV_POP)) && !(fp->nofree&1))
		free((void*)fp);
	nv_delete(np,(Dt_t*)0,0);
	return(1);
}

static void array_syncsub(Namarr_t *ap, Namarr_t *aq)
{
	((struct index_array*)ap)->cur = ((struct index_array*)aq)->cur;
}

static int array_covered(Namval_t *np, struct index_array *ap)
{
	struct index_array *aq = (struct index_array*)ap->header.scope;
	if(!ap->header.fun && aq)
#if SHOPT_FIXEDARRAY
		return (ap->header.fixed || ((ap->cur<aq->maxi) && aq->val[ap->cur].cp));
#else
		return ((ap->cur<aq->maxi) && aq->val[ap->cur].cp);
#endif /* SHOPT_FIXEDARRAY */
	return(0);
}

/*
 * replace discipline with new one
 */
static void array_setptr(register Namval_t *np, struct index_array *old, struct index_array *new)
{
	register Namfun_t **fp = &np->nvfun;
	while(*fp && *fp!= &old->header.hdr)
		fp = &((*fp)->next);
	if(*fp)
	{
		new->header.hdr.next = (*fp)->next;
		*fp = &new->header.hdr;
	}
	else sfprintf(sfstderr,"discipline not replaced\n");
}

/*
 *   Calculate the amount of space to be allocated to hold an
 *   indexed array into which <maxi> is a legal index.  The number of
 *   elements that will actually fit into the array (> <maxi>
 *   but <= ARRAY_MAX) is returned.
 *
 */
static int	arsize(struct index_array *ap, register int maxi)
{
	if(ap && maxi < 2*ap->maxi)
		maxi = 2*ap->maxi;
	maxi = roundof(maxi,ARRAY_INCR);
	return (maxi>ARRAY_MAX?ARRAY_MAX:maxi);
}

static struct index_array *array_grow(Namval_t*, struct index_array*,int);

/* return index of highest element of an array */
int array_maxindex(Namval_t *np)
{
	register struct index_array *ap = (struct index_array*)nv_arrayptr(np);
	register int i = ap->maxi;
	if(is_associative(ap))
		return(-1);
	while(i>0 && ap->val[--i].cp==0);
	return(i+1);
}

static union Value *array_getup(Namval_t *np, Namarr_t *arp, int update)
{
	register struct index_array *ap = (struct index_array*)arp;
	register union Value *up;
#if SHOPT_FIXEDARRAY
	struct fixed_array *fp;
#endif /* SHOPT_FIXEDARRAY */
	int	nofree=0;
	if(!arp)
		return(&np->nvalue);
	if(is_associative(ap))
	{
		Namval_t	*mp;
		mp = (Namval_t*)((*arp->fun)(np,NIL(char*),NV_ACURRENT));
		if(mp)
		{
			nofree = nv_isattr(mp,NV_NOFREE);
			up = &mp->nvalue;
		}
		else
			return((union Value*)((*arp->fun)(np,NIL(char*),0)));
	}
#if SHOPT_FIXEDARRAY
	else if(fp = (struct fixed_array*)arp->fixed)
	{
		if(!fp->data)
			array_fixed_setdata(np,arp,fp);
		up = &np->nvalue;
		if(fp->ptr)
			up->cp =  *(((char**)fp->data)+fp->curi);
		else
			up->cp = fp->data+fp->size*fp->curi;
        }
#endif /* SHOPT_FIXEDARRAY */
	else
	{
		if(ap->cur >= ap->maxi)
			errormsg(SH_DICT,ERROR_exit(1),e_subscript,nv_name(np));
		up = &(ap->val[ap->cur]);
		nofree = array_isbit(ap->bits,ap->cur,ARRAY_NOFREE);
	}
	if(update)
	{
		if(nofree)
			nv_onattr(np,NV_NOFREE);
		else
			nv_offattr(np,NV_NOFREE);
	}
	return(up);
}

int nv_arrayisset(Namval_t *np, Namarr_t *arp)
{
	register struct index_array *ap = (struct index_array*)arp;
	union Value *up;
	if(is_associative(ap))
		return((np = nv_opensub(np)) && !nv_isnull(np));
	if(ap->cur >= ap->maxi)
		return(0);
	up = &(ap->val[ap->cur]);
	if(up->cp==Empty)
	{
		Namfun_t *fp = &arp->hdr;
		for(fp=fp->next; fp; fp = fp->next)
		{
			if(fp->disc && (fp->disc->getnum || fp->disc->getval))
				return(1);
		}
	}
	return(up->cp && up->cp!=Empty);
}

/*
 * Get the Value pointer for an array.
 * Delete space as necessary if flag is ARRAY_DELETE
 * After the lookup is done the last @ or * subscript is incremented
 */
static Namval_t *array_find(Namval_t *np,Namarr_t *arp, int flag)
{
	register struct index_array *ap = (struct index_array*)arp;
	register union Value	*up;
	Namval_t		*mp;
	int			wasundef;
#if SHOPT_FIXEDARRAY
	struct fixed_array	*fp=(struct fixed_array*)(arp->fixed);
#endif /* SHOPT_FIXEDARRAY */
	if(flag&ARRAY_LOOKUP)
		ap->header.nelem &= ~ARRAY_NOSCOPE;
	else
		ap->header.nelem |= ARRAY_NOSCOPE;
	if(wasundef = ap->header.nelem&ARRAY_UNDEF)
	{
		ap->header.nelem &= ~ARRAY_UNDEF;
		/* delete array is the same as delete array[@] */
		if(flag&ARRAY_DELETE)
		{
#if SHOPT_FIXEDARRAY
			nv_putsub(np, NIL(char*), ARRAY_SCAN|ARRAY_NOSCOPE|(ap->header.fixed?(ARRAY_UNDEF|ARRAY_FIXED):0));
#else
			nv_putsub(np, NIL(char*), ARRAY_SCAN|ARRAY_NOSCOPE);
#endif /* SHOPT_FIXEDARRAY */
			ap->header.nelem |= ARRAY_SCAN;
		}
		else /* same as array[0] */
		{
			if(is_associative(ap))
				(*ap->header.fun)(np,"0",flag==ARRAY_ASSIGN?NV_AADD:0);
#if SHOPT_FIXEDARRAY
			else if(fp)
			{
				int n=fp->ndim;
				fp->curi = 0;
				while(--n>=0)
					fp->cur[n] = 0;
			}
#endif /* SHOPT_FIXEDARRAY */
			else
				ap->cur = 0;
		}
	}
	if(is_associative(ap))
	{
		mp = (Namval_t*)((*arp->fun)(np,NIL(char*),NV_ACURRENT));
		if(!mp)
			up = (union Value*)&mp;
		else if(nv_isarray(mp))
		{
			if(wasundef)
				nv_putsub(mp,NIL(char*),ARRAY_UNDEF);
			return(mp);
		}
		else
		{
			up =  &mp->nvalue;
			if(nv_isvtree(mp))
			{
				if(!up->cp && flag==ARRAY_ASSIGN)
				{
					nv_arraychild(np,mp,0);
					ap->header.nelem++;
				}
				return(mp);
			}
		}
	}
#if SHOPT_FIXEDARRAY
	else if(fp)
	{
		char	*data = array_fixed_data((Namarr_t*)ap->header.scope);
		if(flag==ARRAY_ASSIGN && data==fp->data)
		{
			if(data)
			{
				fp->data = (char*)malloc(fp->nelem*fp->size);
				memcpy(fp->data,data,fp->nelem*fp->size);
			}
			else
				array_fixed_setdata(np,&ap->header,fp);
		}
		if(fp->ptr)
		{
			if(!fp->data)
				array_fixed_setdata(np,&ap->header,fp);
			np->nvalue.cp =  *(((char**)fp->data)+fp->curi);
		}
		else
			np->nvalue.cp =  fp->data+fp->size*fp->curi;
		return(np);
	}
#endif /* SHOPT_FIXEDARRAY */
	else
	{
		if(!(ap->header.nelem&ARRAY_SCAN) && ap->cur >= ap->maxi)
			ap = array_grow(np, ap, (int)ap->cur);
		if(ap->cur>=ap->maxi)
			errormsg(SH_DICT,ERROR_exit(1),e_subscript,nv_name(np));
		up = &(ap->val[ap->cur]);
		if((!up->cp||up->cp==Empty) && nv_type(np) && nv_isvtree(np))
		{
			char *cp;
			if(!ap->header.table)
				ap->header.table = dtopen(&_Nvdisc,Dtoset);
			sfprintf(sh.strbuf,"%d",ap->cur);
			cp = sfstruse(sh.strbuf);
			mp = nv_search(cp, ap->header.table, NV_ADD);
			mp->nvenv = (char*)np;
			nv_arraychild(np,mp,0);
		}
		if(up->np && array_isbit(ap->bits,ap->cur,ARRAY_CHILD))
		{
			if(wasundef && nv_isarray(up->np))
				nv_putsub(up->np,NIL(char*),ARRAY_UNDEF);
			return(up->np);
		}
	}
	np->nvalue.cp = up->cp;
	if(!up->cp)
	{
			char *xp = nv_setdisc(np,"get",np,(Namfun_t*)np);
		if(flag!=ARRAY_ASSIGN)
			return(xp && xp!=(char*)np?np:0);
		if(!array_covered(np,ap))
			ap->header.nelem++;
	}
	return(np);
}

#if SHOPT_TYPEDEF
int nv_arraysettype(Namval_t *np, Namval_t *tp, const char *sub, int flags)
{
	Namval_t	*nq;
	char		*av[2];
	int		rdonly = nv_isattr(np,NV_RDONLY);
	int		xtrace = sh_isoption(SH_XTRACE);
	Namarr_t	*ap = nv_arrayptr(np);
	av[1] = 0;
	sh.last_table = 0;
	if(!ap->table)
		ap->table = dtopen(&_Nvdisc,Dtoset);
	if(nq = nv_search(sub, ap->table, NV_ADD))
	{
		if(!nq->nvfun && nq->nvalue.cp && *nq->nvalue.cp==0)
			_nv_unset(nq,NV_RDONLY);
		nv_arraychild(np,nq,0);
		if(!nv_isattr(tp,NV_BINARY))
		{
			sfprintf(sh.strbuf,"%s=%s",nv_name(nq),nv_getval(np));
			av[0] = strdup(sfstruse(sh.strbuf));
		}
		if(!nv_clone(tp,nq,flags|NV_NOFREE))
			return(0);
		ap->nelem |= ARRAY_SCAN;
		if(!rdonly)
			nv_offattr(nq,NV_RDONLY);
		if(!nv_isattr(tp,NV_BINARY))
		{
			if(xtrace)
				sh_offoption(SH_XTRACE);
			ap->nelem &= ~ARRAY_SCAN;
			sh_eval(sh_sfeval(av),0);
			ap->nelem |= ARRAY_SCAN;
			free((void*)av[0]);
			if(xtrace)
				sh_onoption(SH_XTRACE);
		}
		return(1);
	}
	return(0);
}
#endif /* SHOPT_TYPEDEF */


static Namfun_t *array_clone(Namval_t *np, Namval_t *mp, int flags, Namfun_t *fp)
{
	Namarr_t		*ap = (Namarr_t*)fp;
	Namval_t		*nq, *mq;
	char			*name, *sub=0;
	int			nelem, skipped=0;
	Dt_t			*otable=ap->table;
	struct index_array	*aq = (struct index_array*)ap, *ar;
	Shell_t			*shp = sh_getinterp();
	if(flags&NV_MOVE)
	{
		if((flags&NV_COMVAR) && nv_putsub(np,NIL(char*),ARRAY_SCAN))
		{
			do
			{
				if(nq=nv_opensub(np))
					nq->nvenv = (void*)mp;
			}
			while(nv_nextsub(np));
		}
		return(fp);
	}
	nelem = ap->nelem;
	if(nelem&ARRAY_NOCLONE)
		return(0);
	if((flags&NV_TYPE) && !ap->scope)
	{
		ap = array_scope(np,ap,flags);
		return(&ap->hdr);
	}
	ap = (Namarr_t*)nv_clone_disc(fp,0);
	if(flags&NV_COMVAR)
	{
		ap->scope = 0;
		ap->nelem = 0;
		sh.prev_table = sh.last_table;
		sh.prev_root = sh.last_root;
	}
	if(ap->table)
	{
		ap->table = dtopen(&_Nvdisc,Dtoset);
		if(ap->scope && !(flags&NV_COMVAR))
		{
			ap->scope = ap->table;
			dtview(ap->table, otable->view);
		}
	}
	mp->nvfun = (Namfun_t*)ap;
	mp->nvflag &= NV_MINIMAL;
	mp->nvflag |= (np->nvflag&~(NV_MINIMAL|NV_NOFREE));
	if(!(nelem&(ARRAY_SCAN|ARRAY_UNDEF)) && (sub=nv_getsub(np)))
		sub = strdup(sub);
	ar = (struct index_array*)ap;
	if(!is_associative(ap))
		ar->bits = (unsigned char*)&ar->val[ar->maxi];
	if(!nv_putsub(np,NIL(char*),ARRAY_SCAN|((flags&NV_COMVAR)?0:ARRAY_NOSCOPE)))
	{
		if(ap->fun)
			(*ap->fun)(np,(char*)np,0);
		skipped=1;
		goto skip;
	}
	do
	{
		name = nv_getsub(np);
		nv_putsub(mp,name,ARRAY_ADD|ARRAY_NOSCOPE);
		mq = 0;
		if(nq=nv_opensub(np))
			mq = nv_search(name,ap->table,NV_ADD);
		if(nq && (((flags&NV_COMVAR) && nv_isvtree(nq)) || nv_isarray(nq)))
		{
			mq->nvalue.cp = 0;
			if(!is_associative(ap))
				ar->val[ar->cur].np = mq;
			nv_clone(nq,mq,flags);
		}
		else if(flags&NV_ARRAY)
		{
			if((flags&NV_NOFREE) && !is_associative(ap))
				array_setbit(aq->bits,aq->cur,ARRAY_NOFREE);
			else if(nq && (flags&NV_NOFREE))
			{
				mq->nvalue = nq->nvalue;
				nv_onattr(nq,NV_NOFREE);
			}
		}
		else if(nv_isattr(np,NV_INTEGER))
		{
			Sfdouble_t d= nv_getnum(np);
			if(!is_associative(ap))
				ar->val[ar->cur].cp = 0;
			nv_putval(mp,(char*)&d,NV_LDOUBLE);
		}
		else
		{
			if(!is_associative(ap))
				ar->val[ar->cur].cp = 0;
			nv_putval(mp,nv_getval(np),NV_RDONLY);
		}
		aq->header.nelem |= ARRAY_NOSCOPE;
	}
	while(nv_nextsub(np));
skip:
	if(sub)
	{
		if(!skipped)
			nv_putsub(np,sub,0L);
		free((void*)sub);
	}
	aq->header.nelem = ap->nelem = nelem;
	return(&ap->hdr);
}

static char *array_getval(Namval_t *np, Namfun_t *disc)
{
	register Namarr_t *aq,*ap = (Namarr_t*)disc;
	register Namval_t *mp;
	register char	  *cp=0;
	if((mp=array_find(np,ap,ARRAY_LOOKUP))!=np)
	{
		if(!mp && !is_associative(ap) && (aq=(Namarr_t*)ap->scope))
		{
			array_syncsub(aq,ap);
			if((mp=array_find(np,aq,ARRAY_LOOKUP))==np)
				return(nv_getv(np,&aq->hdr));
		}
		if(mp)
		{
			cp = nv_getval(mp);
			nv_offattr(mp,NV_EXPORT);
		}
		return(cp);
	}
#if SHOPT_FIXEDARRAY
	if(ap->fixed && nv_isattr(np,NV_INT16P) == NV_INT16)
		np->nvalue.s = *np->nvalue.sp;
#endif /* SHOPT_FIXEDARRAY */
	return(nv_getv(np,&ap->hdr));
}

static Sfdouble_t array_getnum(Namval_t *np, Namfun_t *disc)
{
	register Namarr_t *aq,*ap = (Namarr_t*)disc;
	register Namval_t *mp;
	if((mp=array_find(np,ap,ARRAY_LOOKUP))!=np)
	{
		if(!mp && !is_associative(ap) && (aq=(Namarr_t*)ap->scope))
		{
			array_syncsub(aq,ap);
			if((mp=array_find(np,aq,ARRAY_LOOKUP))==np)
				return(nv_getn(np,&aq->hdr));
		}
		return(mp?nv_getnum(mp):0);
	}
	return(nv_getn(np,&ap->hdr));
}

static void array_putval(Namval_t *np, const char *string, int flags, Namfun_t *dp)
{
	register Namarr_t	*ap = (Namarr_t*)dp;
	register union Value	*up;
	register Namval_t	*mp;
	register struct index_array *aq = (struct index_array*)ap;
	int			scan,nofree = nv_isattr(np,NV_NOFREE);
#if SHOPT_FIXEDARRAY
	struct fixed_array	*fp;
#endif /* SHOPT_FIXEDARRAY */
	do
	{
		int xfree = (ap->fixed||is_associative(ap))?0:array_isbit(aq->bits,aq->cur,ARRAY_NOFREE);
		mp = array_find(np,ap,string?ARRAY_ASSIGN:ARRAY_DELETE);
		scan = ap->nelem&ARRAY_SCAN;
		if(mp && mp!=np)
		{
			if(!is_associative(ap) && string && !(flags&NV_APPEND) && !nv_type(np) && nv_isvtree(mp) && !(ap->nelem&ARRAY_TREE))

			{
				if(!nv_isattr(np,NV_NOFREE))
					_nv_unset(mp,flags&NV_RDONLY);
				array_clrbit(aq->bits,aq->cur,ARRAY_CHILD);
				aq->val[aq->cur].cp = 0;
				if(!nv_isattr(mp,NV_NOFREE))
					nv_delete(mp,ap->table,0);
				goto skip;
			}
			if(!xfree)
				nv_putval(mp, string, flags);
			if(string)
			{
#if SHOPT_TYPEDEF
				if(ap->hdr.type && ap->hdr.type!=nv_type(mp))
					nv_arraysettype(np,ap->hdr.type,nv_getsub(np),0);
#endif /* SHOPT_TYPEDEF */
				continue;
			}
			ap->nelem |= scan;
		}
		if(!string)
		{
			if(mp)
			{
				if(is_associative(ap))
				{
					(*ap->fun)(np,NIL(char*),NV_ADELETE);
					np->nvalue.cp = 0;
				}
				else
				{
					if(mp!=np)
					{
						array_clrbit(aq->bits,aq->cur,ARRAY_CHILD);
						aq->val[aq->cur].cp = 0;
						if(!xfree)
							nv_delete(mp,ap->table,0);
					}
					if(!array_covered(np,(struct index_array*)ap))
					{
						if(array_elem(ap))
							ap->nelem--;
					}
#if SHOPT_FIXEDARRAY
					else if(fp=(struct fixed_array*)ap->fixed)
					{
						char *data = array_fixed_data((Namarr_t*)ap->scope);
						int n = fp->size*fp->curi;
						if(data)
						{
							memcpy(fp->data+n,data+n,fp->size);
							continue;
						}
					}
#endif /* SHOPT_FIXEDARRAY */
				}
			}
			if(array_elem(ap)==0 && (ap->nelem&ARRAY_SCAN))
			{
				if(is_associative(ap))
					(*ap->fun)(np, NIL(char*), NV_AFREE);
				else if(ap->table)
					dtclose(ap->table);
				nv_offattr(np,NV_ARRAY);
			}
			if(!mp || mp!=np || is_associative(ap))
				continue;
		}
	skip:
		/* prevent empty string from being deleted */
		up = array_getup(np,ap,!nofree);
		if(up->cp ==  Empty)
			up->cp = 0;
#if SHOPT_FIXEDARRAY
		if(nv_isarray(np) && !ap->fixed)
#else
		if(nv_isarray(np))
#endif /* SHOPT_FIXEDARRAY */
			np->nvalue.up = up;
		nv_putv(np,string,flags,&ap->hdr);
#if SHOPT_FIXEDARRAY
		if(fp = (struct fixed_array*)ap->fixed)
		{
			if(fp->ptr)
			{
				char **cp = (char**)fp->data;
				cp[fp->curi] = (char*)(np->nvalue.cp?np->nvalue.cp:Empty);
			}
		}
		else
#endif /* SHOPT_FIXEDARRAY */
		if(!is_associative(ap))
		{
			if(string)
				array_clrbit(aq->bits,aq->cur,ARRAY_NOFREE);
			else if(mp==np)
				aq->val[aq->cur].cp = 0;
		}
#if SHOPT_TYPEDEF
		if(string && ap->hdr.type && nv_isvtree(np))
			nv_arraysettype(np,ap->hdr.type,nv_getsub(np),0);
#endif /* SHOPT_TYPEDEF */
	}
	while(!string && nv_nextsub(np));
	if(ap)
		ap->nelem &= ~ARRAY_NOSCOPE;
	if(nofree)
		nv_onattr(np,NV_NOFREE);
	else
		nv_offattr(np,NV_NOFREE);
	if(!string && !nv_isattr(np,NV_ARRAY))
	{
		Namfun_t *nfp;
#if SHOPT_FIXEDARRAY
		char	*data = array_fixed_data((Namarr_t*)ap->scope);
		fp = (struct fixed_array*)ap->fixed;
		if(fp && (!ap->scope || data!=fp->data))
		{
			if(fp->ptr)
			{
				int n = fp->nelem;
				char **cp = (char**)fp->data;
				while(n-->0)
				{
					if(cp && *cp!=Empty)
						free(*cp);
					cp++;
				}
			}
			free((void*)fp->data);
			if(data)
				fp->data = data;
		}
		else
#endif /* SHOPT_FIXEDARRAY */
		if(!is_associative(ap) && aq->xp)
		{
			_nv_unset(nv_namptr(aq->xp,0),NV_RDONLY);
			free((void*)aq->xp);
		}
		if((nfp = nv_disc(np,(Namfun_t*)ap,NV_POP)) && !(nfp->nofree&1))
			free((void*)nfp);
		if(!nv_isnull(np))
		{
			if(!np->nvfun)
				nv_onattr(np,NV_NOFREE);
			_nv_unset(np,flags);
		}
		else
			nv_offattr(np,NV_NOFREE);
		if(np->nvalue.cp==Empty)
			np->nvalue.cp = 0;
	}
	if(!string && (flags&NV_TYPE))
		array_unscope(np,ap);
}

static const Namdisc_t array_disc =
{
	sizeof(Namarr_t),
	array_putval,
	array_getval,
	array_getnum,
	0,
	0,
	array_clone
};

static void array_copytree(Namval_t *np, Namval_t *mp)
{
	Namfun_t	*fp = nv_disc(np,NULL,NV_POP);
	nv_offattr(np,NV_ARRAY);
	nv_clone(np,mp,0);
	if(np->nvalue.cp && !nv_isattr(np,NV_NOFREE))
		free((void*)np->nvalue.cp);
	np->nvalue.cp = 0;
	np->nvalue.up = &mp->nvalue;
	fp->nofree  &= ~1;
	nv_disc(np,(Namfun_t*)fp, NV_FIRST);
	fp->nofree |= 1;
	nv_onattr(np,NV_ARRAY);
	mp->nvenv = (char*)np;
}

/*
 *        Increase the size of the indexed array of elements in <arp>
 *        so that <maxi> is a legal index.  If <arp> is 0, an array
 *        of the required size is allocated.  A pointer to the 
 *        allocated Namarr_t structure is returned.
 *        <maxi> becomes the current index of the array.
 */
static struct index_array *array_grow(Namval_t *np, register struct index_array *arp,int maxi)
{
	register struct index_array *ap;
	register int i;
	register int newsize = arsize(arp,maxi+1);
	if (maxi >= ARRAY_MAX)
		errormsg(SH_DICT,ERROR_exit(1),e_subscript, fmtbase((long)maxi,10,0));
	i = (newsize-1)*sizeof(union Value*)+newsize;
	ap = new_of(struct index_array,i);
	memset((void*)ap,0,sizeof(*ap)+i);
	ap->maxi = newsize;
	ap->cur = maxi;
	ap->bits =  (unsigned char*)&ap->val[newsize];
	memset(ap->bits, 0, newsize);
	if(arp)
	{
		ap->header = arp->header;
		ap->header.hdr.dsize = sizeof(*ap) + i;
		for(i=0;i < arp->maxi;i++)
		{
			ap->bits[i] = arp->bits[i];
			ap->val[i].cp = arp->val[i].cp;
		}
		memcpy(ap->bits, arp->bits, arp->maxi);
		array_setptr(np,arp,ap);
		free((void*)arp);
	}
	else
	{
		Namval_t *mp=0;
		ap->header.hdr.dsize = sizeof(*ap) + i;
		i = 0;
		ap->header.fun = 0;
		if((nv_isnull(np)||np->nvalue.cp==Empty) && nv_isattr(np,NV_NOFREE))
		{
			i = ARRAY_TREE;
			nv_offattr(np,NV_NOFREE);
		}
		if(np->nvalue.cp==Empty)
			np->nvalue.cp=0;
		if(nv_hasdisc(np,&array_disc) || (nv_type(np) && nv_isvtree(np)))
		{
			ap->header.table = dtopen(&_Nvdisc,Dtoset);
			mp = nv_search("0", ap->header.table,NV_ADD);
			if(mp && nv_isnull(mp))
			{
				Namfun_t *fp;
				ap->val[0].np = mp;
				array_setbit(ap->bits,0,ARRAY_CHILD);
				for(fp=np->nvfun; fp && !fp->disc->readf; fp=fp->next);
				if(fp && fp->disc && fp->disc->readf)
					(*fp->disc->readf)(mp,(Sfio_t*)0,0,fp);
				i++;
			}
		}
		else
		if((ap->val[0].cp=np->nvalue.cp))
			i++;
		else if(nv_isattr(np,NV_INTEGER) && !nv_isnull(np))
		{
			Sfdouble_t d= nv_getnum(np);
			i++;
		}
		ap->header.nelem = i;
		ap->header.hdr.disc = &array_disc;
		nv_disc(np,(Namfun_t*)ap, NV_FIRST);
		nv_onattr(np,NV_ARRAY);
		if(mp)
		{
			array_copytree(np,mp);
			ap->header.hdr.nofree &= ~1;
		}
	}
	for(;i < newsize;i++)
		ap->val[i].cp = 0;
	return(ap);
}

int nv_atypeindex(Namval_t *np, const char *tname)
{
	Namval_t	*tp;
	int		offset = staktell();
	size_t		n = strlen(tname)-1;
	sfprintf(stkstd,"%s.%.*s%c",NV_CLASS,n,tname,0);
	tp = nv_open(stakptr(offset), sh.var_tree, NV_NOADD|NV_VARNAME);
	stakseek(offset);
	if(tp)
	{
		struct index_array *ap = (struct index_array*)nv_arrayptr(np);
		if(!nv_hasdisc(tp,&ENUM_disc))
			errormsg(SH_DICT,ERROR_exit(1),e_notenum,tp->nvname);
		if(!ap)
			ap = array_grow(np,ap,1);
		ap->xp = calloc(NV_MINSZ,1);
		np = nv_namptr(ap->xp,0);
		np->nvname = tp->nvname;
		nv_onattr(np,NV_MINIMAL);
		nv_clone(tp,np,NV_NOFREE);
		nv_offattr(np,NV_RDONLY);
		return(1);
	}
	errormsg(SH_DICT,ERROR_exit(1),e_unknowntype, n,tname);
	return(0);
}

Namarr_t *nv_arrayptr(register Namval_t *np)
{
	if(nv_isattr(np,NV_ARRAY))
		return((Namarr_t*)nv_hasdisc(np, &array_disc));
	return(0);
}

/*
 * Verify that argument is an indexed array and convert to associative,
 * freeing relevant storage
 */
static Namarr_t *nv_changearray(Namval_t *np, void *(*fun)(Namval_t*,const char*,int))
{
	register Namarr_t *ap;
	char numbuff[NUMSIZE+1];
	unsigned dot, digit, n;
	union Value *up;
	struct index_array *save_ap;
	register char *string_index=&numbuff[NUMSIZE];
	numbuff[NUMSIZE]='\0';

	if(!fun || !(ap = nv_arrayptr(np)) || is_associative(ap))
		return(NIL(Namarr_t*));

	nv_stack(np,&ap->hdr);
	save_ap = (struct index_array*)nv_stack(np,0);
	ap = (Namarr_t*)((*fun)(np, NIL(char*), NV_AINIT));
	ap->nelem = 0;
	ap->fun = fun;
	nv_onattr(np,NV_ARRAY);

	for(dot = 0; dot < (unsigned)save_ap->maxi; dot++)
	{
		if(save_ap->val[dot].cp)
		{
			if ((digit = dot)== 0)
				*--string_index = '0';
			else while( n = digit )
			{
				digit /= 10;
				*--string_index = '0' + (n-10*digit);
			}
			nv_putsub(np, string_index, ARRAY_ADD);
			up = (union Value*)((*ap->fun)(np,NIL(char*),0));
			up->cp = save_ap->val[dot].cp;
			save_ap->val[dot].cp = 0;
		}
		string_index = &numbuff[NUMSIZE];
	}
	free((void*)save_ap);
	return(ap);
}

/*
 * set the associative array processing method for node <np> to <fun>
 * The array pointer is returned if sucessful.
 */
Namarr_t *nv_setarray(Namval_t *np, void *(*fun)(Namval_t*,const char*,int))
{
	register Namarr_t *ap;
	char		*value=0;
	Namfun_t	*fp;
	int		nelem = 0;
	if(fun && (ap = nv_arrayptr(np)))
	{
		/*
		 * if it's already an indexed array, convert to 
		 * associative structure
		 */
		if(!is_associative(ap))
			ap = nv_changearray(np, fun);
		return(ap);
	}
	if(nv_isnull(np) && nv_isattr(np,NV_NOFREE))
	{
		nelem = ARRAY_TREE;
		nv_offattr(np,NV_NOFREE);
	}
	if(!(fp=nv_isvtree(np)))
		value = nv_getval(np);
	if(fun && !ap && (ap = (Namarr_t*)((*fun)(np, NIL(char*), NV_AINIT))))
	{
		/* check for preexisting initialization and save */
		ap->nelem = nelem;
		ap->fun = fun;
		nv_onattr(np,NV_ARRAY);
		if(fp || value)
		{
			nv_putsub(np, "0", ARRAY_ADD);
			if(value)
				nv_putval(np, value, 0);
			else
			{
				Namval_t *mp = (Namval_t*)((*fun)(np,NIL(char*),NV_ACURRENT));
				array_copytree(np,mp);
			}
		}
		return(ap);
	}
	return(NIL(Namarr_t*));
}

/*
 * move parent subscript into child
 */
Namval_t *nv_arraychild(Namval_t *np, Namval_t *nq, int c)
{
	Namfun_t		*fp;
	register Namarr_t	*ap = nv_arrayptr(np);
	union Value		*up;
	Namval_t		*tp;
	if(!nq)
		return(ap?array_find(np,ap, ARRAY_LOOKUP):0);
	if(!ap)
	{
		nv_putsub(np, NIL(char*), ARRAY_FILL);
		ap = nv_arrayptr(np);
	}
	if(!(up = array_getup(np,ap,0)))
		return((Namval_t*)0);
	np->nvalue.cp = up->cp;
	if((tp=nv_type(np)) || c)
	{
		ap->nelem |= ARRAY_NOCLONE;
		nq->nvenv = (char*)np;
		if(c=='t')
			nv_clone(tp,nq, 0);
		else
			nv_clone(np, nq, NV_NODISC);
		nv_offattr(nq,NV_ARRAY);
		ap->nelem &= ~ARRAY_NOCLONE;
	}
	nq->nvenv = (char*)np;
	if((fp=nq->nvfun) && fp->disc && fp->disc->setdisc && (fp = nv_disc(nq,fp,NV_POP)))
		free((void*)fp);
	if(!ap->fun)
	{
		struct index_array *aq = (struct index_array*)ap;
		array_setbit(aq->bits,aq->cur,ARRAY_CHILD);
		if(c=='.' && !nq->nvalue.cp)
			ap->nelem++;
		up->np = nq;
	}
	if(c=='.')
		nv_setvtree(nq);
	return(nq);
}

/*
 * This routine sets subscript of <np> to the next element, if any.
 * The return value is zero, if there are no more elements
 * Otherwise, 1 is returned.
 */
int nv_nextsub(Namval_t *np)
{
	register struct index_array	*ap = (struct index_array*)nv_arrayptr(np);
	register unsigned		dot;
	struct index_array		*aq=0, *ar=0;
#if SHOPT_FIXEDARRAY
	struct fixed_array		*fp;
#endif /* SHOPT_FIXEDARRAY */
	if(!ap || !(ap->header.nelem&ARRAY_SCAN))
		return(0);
	if(is_associative(ap))
	{
		if((*ap->header.fun)(np,NIL(char*),NV_ANEXT))
			return(1);
		ap->header.nelem &= ~(ARRAY_SCAN|ARRAY_NOCHILD);
		return(0);
	}
#if SHOPT_FIXEDARRAY
	else if(fp = (struct fixed_array*)ap->header.fixed)
	{
		if(ap->header.nelem&ARRAY_FIXED)
		{
			while(++fp->curi < fp->nelem)
			{
				nv_putsub(np,0,fp->curi|ARRAY_FIXED|ARRAY_SCAN);
				if(fp->ptr && *(((char**)fp->data)+fp->curi))
				return(1);
			}
			ap->header.nelem &= ~ARRAY_FIXED;
			return(0);
		}
		dot = fp->dim;
		if((fp->cur[dot]+1) < fp->max[dot])
		{
			fp->cur[dot]++;
			for(fp->curi=0,dot=0; dot < fp->ndim; dot++)
				fp->curi +=  fp->incr[dot]*fp->cur[dot];
			return(1);
		}
		if(fp->level)
		{
			dot= --fp->dim;
			while((dot+1) < fp->ndim)
				fp->cur[++dot] = 0;
			fp->level--;
			fp->curi = 0;
		}
		else
		ap->header.nelem &= ~(ARRAY_SCAN|ARRAY_NOCHILD);
		return(0);
	}
#endif /* SHOPT_FIXEDARRAY */
	if(!(ap->header.nelem&ARRAY_NOSCOPE))
		ar = (struct index_array*)ap->header.scope;
	for(dot=ap->cur+1; dot <  (unsigned)ap->maxi; dot++)
	{
		aq = ap;
		if(!ap->val[dot].cp && !(ap->header.nelem&ARRAY_NOSCOPE))
		{
			if(!(aq=ar) || dot>=(unsigned)aq->maxi)
				continue;
		}
		if(aq->val[dot].cp==Empty && array_elem(&aq->header) < nv_aimax(np)+1)		{
			ap->cur = dot;
			if(nv_getval(np)==Empty)
				continue;
		}
		if(aq->val[dot].cp)
		{
			ap->cur = dot;
			if(array_isbit(aq->bits, dot,ARRAY_CHILD))
			{
				Namval_t *mp = aq->val[dot].np;			
				if((aq->header.nelem&ARRAY_NOCHILD) && nv_isvtree(mp) && !mp->nvfun->dsize)
					continue;
				if(nv_isarray(mp))
					nv_putsub(mp,NIL(char*),ARRAY_SCAN);
			}
			return(1);
		}
	}
	ap->header.nelem &= ~(ARRAY_SCAN|ARRAY_NOCHILD);
	ap->cur = 0;
	return(0);
}

/*
 * Set an array subscript for node <np> given the subscript <sp>
 * An array is created if necessary.
 * <mode> can be a number, plus or more of symbolic constants
 *    ARRAY_SCAN, ARRAY_UNDEF, ARRAY_ADD
 * The node pointer is returned which can be NULL if <np> is
 *    not already array and the ARRAY_ADD bit of <mode> is not set.
 * ARRAY_FILL sets the specified subscript to the empty string when
 *   ARRAY_ADD is specified and there is no value or sets all
 * the elements up to the number specified if ARRAY_ADD is not specified
 */
Namval_t *nv_putsub(Namval_t *np,register char *sp,register long mode)
{
	register struct index_array *ap = (struct index_array*)nv_arrayptr(np);
	register int size = (mode&ARRAY_MASK);
#if SHOPT_FIXEDARRAY
	struct fixed_array	*fp;
	if(!ap || (!ap->header.fixed && !ap->header.fun))
#else
	if(!ap || !ap->header.fun)
#endif /* SHOPT_FIXEDARRAY */
	{
		if(sp)
		{
			Shell_t	*shp = sh_getinterp();
			if(ap && ap->xp && !strmatch(sp,"+([0-9])"))
			{
				Namval_t *mp = nv_namptr(ap->xp,0);
				nv_putval(mp, sp,0);
				size = nv_getnum(mp);
			}
			else
				size = (int)sh_arith(shp,(char*)sp);
		}
		if(size <0 && ap)
			size += array_maxindex(np);
		if(size >= ARRAY_MAX || (size < 0))
		{
			errormsg(SH_DICT,ERROR_exit(1),e_subscript, nv_name(np));
			return(NIL(Namval_t*));
		}
		if(!ap || size>=ap->maxi)
		{
			if(size==0 && !(mode&ARRAY_FILL))
				return(NIL(Namval_t*));
			if(sh.subshell)
				np = sh_assignok(np,1);
			ap = array_grow(np, ap,size);
		}
		ap->header.nelem &= ~ARRAY_UNDEF;
		ap->header.nelem |= (mode&(ARRAY_SCAN|ARRAY_NOCHILD|ARRAY_UNDEF|ARRAY_NOSCOPE));
#if 0
		if(array_isbit(ap->bits,oldsize,ARRAY_CHILD))
			mp = ap->val[oldsize].np;
		if(size != oldsize && mp->nvalue.cp)
		{
			Namfun_t *nfp;
			for(nfp=np->nvfun; nfp; nfp=nfp->next)
			{
				if(nfp->disc && nfp->disc->readf)
				{
					(*nfp->disc->readf)(mp,(Sfio_t*)0,0,nfp);
					break;
				}
			}
		}
#endif
		ap->cur = size;
		if((mode&ARRAY_SCAN) && (ap->cur--,!nv_nextsub(np)))
			np = 0;
		if(mode&(ARRAY_FILL|ARRAY_ADD))
		{
			if(!(mode&ARRAY_ADD))
			{
				int n;
				if(mode&ARRAY_SETSUB)
				{
					for(n=0; n <= ap->maxi; n++)
						ap->val[n].cp = 0;
					ap->header.nelem = 0;
				}
				for(n=0; n <= size; n++)
				{
					if(!ap->val[n].cp)
					{
						ap->val[n].cp = Empty;
						if(!array_covered(np,ap))
							ap->header.nelem++;
					}
				}
				if(n=ap->maxi-ap->maxi)
					memset(&ap->val[size],0,n*sizeof(union Value));
			}
			else if(!(sp=(char*)ap->val[size].cp) || sp==Empty)
			{
				if(sh.subshell)
					np = sh_assignok(np,1);
				if(ap->header.nelem&ARRAY_TREE)
				{
					char *cp;
					Namval_t *mp;
					if(!ap->header.table)
						ap->header.table = dtopen(&_Nvdisc,Dtoset);
					sfprintf(sh.strbuf,"%d",ap->cur);
					cp = sfstruse(sh.strbuf);
					mp = nv_search(cp, ap->header.table, NV_ADD);
					mp->nvenv = (char*)np;
					nv_arraychild(np,mp,0);
					nv_setvtree(mp);
				}
				else
					ap->val[size].cp = Empty;
				if(!sp && !array_covered(np,ap))
					ap->header.nelem++;
			}
		}
		else if(!(mode&ARRAY_SCAN))
		{
			ap->header.nelem &= ~ARRAY_SCAN;
			if(array_isbit(ap->bits,size,ARRAY_CHILD))
				nv_putsub(ap->val[size].np,NIL(char*),ARRAY_UNDEF);
			if(sp && !(mode&ARRAY_ADD) && !ap->val[size].cp)
				np = 0;
		}
		return((Namval_t*)np);
	}
#if SHOPT_FIXEDARRAY
	if(fp=(struct fixed_array*)ap->header.fixed)
	{
		if(!fp->data)
			return(np);
		if(mode&ARRAY_UNDEF)
		{
			fp->dim = 0;
			fp->curi = 0;
			for(size=fp->ndim;--size>=0;)
				fp->cur[size] = 0;
			ap->header.nelem &= ~ARRAY_MASK;
			if(mode&ARRAY_FIXED)
			{
				mode &= ~ARRAY_UNDEF;
				ap->header.nelem |= (ARRAY_FIXED|fp->nelem);
			}
			else
				ap->header.nelem |=  fp->max[0];
		}
		else if(mode&ARRAY_FIXED)
		{
			size = (mode&ARRAY_MASK)&~(ARRAY_FIXED);
			fp->curi = size;
			for(fp->dim=0;size>0 && fp->dim<fp->ndim; fp->dim++)
			{
				fp->cur[fp->dim] = size/fp->incr[fp->dim];
				size -= fp->incr[fp->dim]*fp->cur[fp->dim];
			}
			while(fp->dim < fp->ndim)
				fp->cur[fp->dim++] = 0;
			fp->dim = ap->header.nelem;
			ap->header.nelem |= ARRAY_FIXED;
		}
		else if(fp->dim< fp->ndim)
		{
			fp->curi += (size-fp->cur[fp->dim])*fp->incr[fp->dim];
			fp->cur[fp->dim] = size;
		}
	}
#endif /* SHOPT_FIXEDARRAY */
	ap->header.nelem &= ~ARRAY_UNDEF;
	if(!(mode&ARRAY_FILL))
		ap->header.nelem &= ~ARRAY_SCAN;
	ap->header.nelem |= (mode&(ARRAY_SCAN|ARRAY_NOCHILD|ARRAY_UNDEF|ARRAY_NOSCOPE));
#if SHOPT_FIXEDARRAY
	if(fp)
		return(np);
	else
#endif /* SHOPT_FIXEDARRAY */
	if(sp)
	{
		if(mode&ARRAY_SETSUB)
		{
			(*ap->header.fun)(np, sp, NV_ASETSUB);
			return(np);
		}
		(*ap->header.fun)(np, sp, (mode&ARRAY_ADD)?NV_AADD:0);
		if(!(mode&(ARRAY_SCAN|ARRAY_ADD)) && !(*ap->header.fun)(np,NIL(char*),NV_ACURRENT))
			np = 0;
	}
	else if(mode&ARRAY_SCAN)
		(*ap->header.fun)(np,(char*)np,0);
	else if(mode&ARRAY_UNDEF)
		(*ap->header.fun)(np, "",0);
	if((mode&ARRAY_SCAN) && !nv_nextsub(np))
		np = 0;
	return(np);
}

#if SHOPT_FIXEDARRAY
int nv_arrfixed(Namval_t *np, Sfio_t *out, int flag, char *dim)
{
	Namarr_t		*ap =  nv_arrayptr(np);
	struct fixed_array	*fp = (struct fixed_array*)ap->fixed;
	int			n;
	if(flag)
	{
		if(out)
		{
			for(n=0; n < fp->dim; n++)
				sfprintf(out,"[%d]",fp->cur[n]);
		}
		if(dim)
			*dim = fp->dim;
		return(fp->curi);
	}
	if(out)
	{
		for(n=0; n < fp->ndim; n++)
			sfprintf(out,"[%d]",fp->max[n]);
	}
	fp->dim = 0;
	return(fp->curi);
}

static void array_fixed_setdata(Namval_t *np,Namarr_t* ap,struct fixed_array* fp)
{
	int n = ap->nelem;
	ap->nelem = 1;
	fp->size = fp->ptr?sizeof(void*):nv_datasize(np,0);
	ap->nelem = n;
	fp->data = (char*)calloc(fp->nelem,fp->size);
	if(fp->ptr)
	{
		char **cp = (char**)fp->data;
		for(n=fp->nelem; n-->0;)
			*cp++ = Empty;
	}
}

static int array_fixed_init(Namval_t *np, char *sub, char *cp)
{
	Shell_t			*shp=sh_getinterp();
	Namarr_t		*ap;
	struct fixed_array	*fp;
	int			n=1,sz;
	char			*ep=cp;
	while(*ep=='[')
	{
		ep = nv_endsubscript(np,ep,0);
		n++;
	}
	if(*ep)
		return(0);
	sz = sizeof(struct fixed_array)+ 3*n*sizeof(int);
        if(!(ap=newof(NIL(Namarr_t*),Namarr_t,1,sz)))
                return(0);
	ap->hdr.disc = &array_disc;
	ap->hdr.dsize = sizeof(Namarr_t)+sz;
	ap->hdr.nofree &= ~1;
	fp = (struct fixed_array*)(ap+1);
	ap->fixed = (void*)fp;
	fp->ndim = n;
	fp->max = (int*)(fp+1);
	fp->incr = fp->max+n;
	fp->cur = fp->incr+n;
	fp->max[0] = (int)sh_arith(shp,(char*)sub);
	for(n=1,ep=cp;*ep=='['; ep=cp)
	{
		cp = nv_endsubscript(np,ep,0);
		cp[-1]=0;
		fp->max[n++] = sz = (int)sh_arith(shp,(char*)ep+1);
		if(sz<0)
		{
			free((void*)ap);
			errormsg(SH_DICT,ERROR_exit(1),e_subscript, nv_name(np));
		}
		cp[-1] = ']';
	}
	nv_disc(np,(Namfun_t*)ap, NV_FIRST);
	fp->ptr = !np->nvsize;
	nv_onattr(np,NV_ARRAY|(fp->ptr?0:NV_NOFREE));
	fp->incr[n=fp->ndim-1] = 1;
	for(sz=1; --n>=0;)
		sz = fp->incr[n] = sz*fp->max[n+1];
	fp->nelem = sz*fp->max[0];
	ap->nelem = fp->max[0];
	return(1);
}

static char *array_fixed(Namval_t *np, char *sub, char *cp,int mode)
{
	Shell_t			*shp=sh_getinterp();
	Namarr_t		*ap = nv_arrayptr(np);
	struct fixed_array	*fp = (struct fixed_array*)ap->fixed;
	char			*ep;
	int			size,n=0,sz;
	if(!fp->data)
		array_fixed_setdata(np,ap,fp);
	ap->nelem &= ~ARRAY_UNDEF;
	if(ap->nelem&ARRAY_FIXED)
	{
		ap->nelem &= ~ARRAY_FIXED;
		n = fp->dim;
		sz = fp->curi;
		if(*sub==0)
			goto skip;
	}
	else
		fp->curi = 0;
	size = (int)sh_arith(shp,(char*)sub);
	fp->cur[n] = size;
	if(size >= fp->max[n] || (size < 0))
		errormsg(SH_DICT,ERROR_exit(1),e_subscript, nv_name(np));
	*cp++ = ']';
	sz = fp->curi + fp->cur[n]*fp->incr[n];
	for(n++,ep=cp;*ep=='['; ep=cp,n++)
	{
		if(n >= fp->ndim)
			errormsg(SH_DICT,ERROR_exit(1),e_subscript, nv_name(np));
		cp = nv_endsubscript(np,ep,0);
		cp[-1]=0;
		size = (int)sh_arith(shp,(char*)ep+1);
		if(size >= fp->max[n] || (size < 0))
			errormsg(SH_DICT,ERROR_exit(1),e_subscript, nv_name(np));
		fp->cur[n] = size;
		cp[-1] = ']';
		sz += fp->cur[n]*fp->incr[n];
	}
skip:
	fp->dim = n;
	ap->nelem &= ~ARRAY_MASK;
	ap->nelem |= fp->max[n];
	while(n < fp->ndim)
		fp->cur[n++] = 0;
	fp->curi = sz;
	return(cp-1);
}
#endif /* SHOPT_FIXEDARRAY */

/*
 * process an array subscript for node <np> given the subscript <cp>
 * returns pointer to character after the subscript
 */
char *nv_endsubscript(Namval_t *np, register char *cp, int mode)
{
	register int count=1, quoted=0, c;
	register char *sp = cp+1;
	/* first find matching ']' */
	while(count>0 && (c= *++cp))
	{
		if(c=='\\' && (!(mode&NV_SUBQUOTE) || (c=cp[1])=='[' || c==']' || c=='\\' || c=='*' || c=='@'))
		{
			quoted=1;
			cp++;
		}
		else if(c=='[')
			count++;
		else if(c==']')
			count--;
	}
	*cp = 0;
	if(quoted)
	{
		/* strip escape characters */
		count = staktell();
		stakwrite(sp,1+cp-sp);
		sh_trim(sp=stakptr(count));
	}
	if(mode && np)
	{
		Namarr_t *ap = nv_arrayptr(np);
		int scan = 0;
#if SHOPT_FIXEDARRAY
		if((mode&NV_FARRAY) && !nv_isarray(np))
		{
			if(array_fixed_init(np,sp,cp+1))
			{
				*cp++ = c;
				return(strchr(cp,0));
			}
		}
#endif /* SHOPT_FIXEDARRAY */
		if(ap)
			scan = ap->nelem&ARRAY_SCAN;
		if((mode&NV_ASSIGN) && (cp[1]=='=' || cp[1]=='+'))
			mode |= NV_ADD;
		else if(ap && cp[1]=='.' && (mode&NV_FARRAY))
			mode |= NV_ADD;
#if SHOPT_FIXEDARRAY
		if(ap && ap->fixed)
			cp = array_fixed(np,sp,cp,mode);
		else
#endif /* SHOPT_FIXEDARRAY */
		nv_putsub(np, sp, ((mode&NV_ADD)?ARRAY_ADD:0)|(cp[1]&&(mode&NV_ADD)?ARRAY_FILL:mode&ARRAY_FILL));
		if(scan)
			ap->nelem |= scan;
	}
	if(quoted)
		stakseek(count);
	*cp++ = c;
	return(cp);
}


Namval_t *nv_opensub(Namval_t* np)
{
	register struct index_array *ap = (struct index_array*)nv_arrayptr(np);
#if SHOPT_FIXEDARRAY
	struct fixed_array *fp;
#endif /* SHOPT_FIXEDARRAY */
	if(ap)
	{
		if(is_associative(ap))
			return((Namval_t*)((*ap->header.fun)(np,NIL(char*),NV_ACURRENT)));
#if SHOPT_FIXEDARRAY
		else if(!(fp=(struct fixed_array*)ap->header.fixed) && array_isbit(ap->bits,ap->cur,ARRAY_CHILD))
#else
		else if(array_isbit(ap->bits,ap->cur,ARRAY_CHILD))
#endif /* SHOPT_FIXEDARRAY */
		{
			return(ap->val[ap->cur].np);
		}
#if SHOPT_FIXEDARRAY
		else if(fp)
		{
			int n = fp->dim;
			if((fp->dim+1) < fp->ndim)
			{
				fp->dim++;
				if(ap->header.nelem&ARRAY_SCAN)
				{
					while(++n < fp->ndim)
						fp->cur[n] = 0;
					fp->level++;
				}
				return(np);
			}
		}
#endif /* SHOPT_FIXEDARRAY */
	}
	return(NIL(Namval_t*));
}

char	*nv_getsub(Namval_t* np)
{
	static char numbuff[NUMSIZE+1];
	register struct index_array *ap;
	register unsigned dot, n;
	register char *cp = &numbuff[NUMSIZE];
	if(!np || !(ap = (struct index_array*)nv_arrayptr(np)))
		return(NIL(char*));
	if(is_associative(ap))
		return((char*)((*ap->header.fun)(np,NIL(char*),NV_ANAME)));
	if(ap->xp)
	{
		np = nv_namptr(ap->xp,0);
		np->nvalue.s = ap->cur;
		return(nv_getval(np));
	}
	if((dot = ap->cur)==0)
		*--cp = '0';
	else while(n=dot)
	{
		dot /= 10;
		*--cp = '0' + (n-10*dot);
	}
	return(cp);
}

/*
 * If <np> is an indexed array node, the current subscript index
 * returned, otherwise returns -1
 */
int nv_aindex(register Namval_t* np)
{
	Namarr_t *ap = nv_arrayptr(np);
	if(!ap)
		return(0);
	else if(is_associative(ap))
		return(-1);
#if SHOPT_FIXEDARRAY
	else if(ap->fixed)
		return(-1);
#endif /* SHOPT_FIXEDARRAY */
	return(((struct index_array*)(ap))->cur&ARRAY_MASK);
}

int nv_arraynsub(register Namarr_t* ap)
{
	return(array_elem(ap));
}

int nv_aimax(register Namval_t* np)
{
	struct index_array *ap = (struct index_array*)nv_arrayptr(np);
	int sub = -1;
#if SHOPT_FIXEDARRAY
	if(!ap || is_associative(&ap->header) || ap->header.fixed)
#else
	if(!ap || is_associative(&ap->header))
#endif /* SHOPT_FIXEDARRAY */
		return(-1);
	sub = ap->maxi;
	while(--sub>0 && ap->val[sub].cp==0);
	return(sub);
}

/*
 *  This is the default implementation for associative arrays
 */
void *nv_associative(register Namval_t *np,const char *sp,int mode)
{
	register struct assoc_array *ap = (struct assoc_array*)nv_arrayptr(np);
	register int type;
	switch(mode)
	{
	    case NV_AINIT:
		if(ap = (struct assoc_array*)calloc(1,sizeof(struct assoc_array)))
		{
			ap->header.table = dtopen(&_Nvdisc,Dtoset);
			ap->cur = 0;
			ap->pos = 0;
			ap->header.hdr.disc = &array_disc;
			nv_disc(np,(Namfun_t*)ap, NV_FIRST);
			ap->header.hdr.dsize = sizeof(struct assoc_array);
			ap->header.hdr.nofree &= ~1;
		}
		return((void*)ap);
	    case NV_ADELETE:
		if(ap->cur)
		{
			if(!ap->header.scope || (Dt_t*)ap->header.scope==ap->header.table || !nv_search(ap->cur->nvname,(Dt_t*)ap->header.scope,0))
				ap->header.nelem--;
			_nv_unset(ap->cur,NV_RDONLY);
			nv_delete(ap->cur,ap->header.table,0);
			ap->cur = 0;
		}
		return((void*)ap);
	    case NV_AFREE:
		ap->pos = 0;
		if(ap->header.scope)
		{
			ap->header.table = dtview(ap->header.table,(Dt_t*)0);
			dtclose(ap->header.scope);
			ap->header.scope = 0;
		}
		else
			dtclose(ap->header.table);
		return((void*)ap);
	    case NV_ANEXT:
		if(!ap->pos)
		{
			if((ap->header.nelem&ARRAY_NOSCOPE) && ap->header.scope && dtvnext(ap->header.table))
			{
				ap->header.scope = dtvnext(ap->header.table);
				ap->header.table->view = 0;
			}
			if(!(ap->pos=ap->cur))
				ap->pos = (Namval_t*)dtfirst(ap->header.table);
		}
		else
			ap->pos = ap->nextpos;
		for(;ap->cur=ap->pos; ap->pos=ap->nextpos)
		{
			ap->nextpos = (Namval_t*)dtnext(ap->header.table,ap->pos);
			if(!nv_isnull(ap->cur))
			{
				if((ap->header.nelem&ARRAY_NOCHILD) && nv_isattr(ap->cur,NV_CHILD))
					continue;
				return((void*)ap);
			}
		}
		if((ap->header.nelem&ARRAY_NOSCOPE) && ap->header.scope && !dtvnext(ap->header.table))
		{
			ap->header.table->view = (Dt_t*)ap->header.scope;
			ap->header.scope = ap->header.table;
		}
		return(NIL(void*));
	    case NV_ASETSUB:
		ap->cur = (Namval_t*)sp;
		return((void*)ap->cur);
	    case NV_ACURRENT:
		if(ap->cur)
			ap->cur->nvenv = (char*)np;
		return((void*)ap->cur);
	    case NV_ANAME:
		if(ap->cur)
		{
			Shell_t *shp = sh_getinterp();
			if(!shp->instance && nv_isnull(ap->cur))
				return(NIL(void*));
			return((void*)ap->cur->nvname);
		}
		return(NIL(void*));
	    default:
		if(sp)
		{
			Namval_t *mp=0;
			ap->cur = 0;
			if(sp==(char*)np)
				return(0);
			type = nv_isattr(np,NV_PUBLIC&~(NV_ARRAY|NV_CHILD|NV_MINIMAL));
			if(mode)
				mode = NV_ADD|HASH_NOSCOPE;
			else if(ap->header.nelem&ARRAY_NOSCOPE)
				mode = HASH_NOSCOPE;
			if(*sp==0 && sh_isoption(SH_XTRACE) && (mode&NV_ADD))
				errormsg(SH_DICT,ERROR_warn(0),"adding empty subscript"); 
			if(sh.subshell && (mp=nv_search(sp,ap->header.table,0)) && nv_isnull(mp))
				ap->cur = mp;
			if((mp || (mp=nv_search(sp,ap->header.table,mode))) && nv_isnull(mp) && (mode&NV_ADD))
			{
				nv_onattr(mp,type);
				mp->nvenv = (char*)np;
				if((mode&NV_ADD) && nv_type(np)) 
					nv_arraychild(np,mp,0);
				if(sh.subshell)
					np = sh_assignok(np,1);
				if(!ap->header.scope || !nv_search(sp,dtvnext(ap->header.table),0))
					ap->header.nelem++;
				if(nv_isnull(mp))
				{
					if(ap->header.nelem&ARRAY_TREE)
						nv_setvtree(mp);
					mp->nvalue.cp = Empty;
				}
			}
			else if(ap->header.nelem&ARRAY_SCAN)
			{
				Namval_t fake;
				fake.nvname = (char*)sp;
				ap->pos = mp = (Namval_t*)dtprev(ap->header.table,&fake);
				ap->nextpos = (Namval_t*)dtnext(ap->header.table,mp);
			}
			else if(!mp && *sp && mode==0)
				mp = nv_search(sp,ap->header.table,NV_ADD|HASH_NOSCOPE);
			np = mp;
			if(ap->pos && ap->pos==np)
				ap->header.nelem |= ARRAY_SCAN;
			else if(!(ap->header.nelem&ARRAY_SCAN))
				ap->pos = 0;
			ap->cur = np;
		}
		if(ap->cur)
			return((void*)(&ap->cur->nvalue));
		else
			return((void*)(&ap->cur));
	}
}

/*
 * Assign values to an array
 */
void nv_setvec(register Namval_t *np,int append,register int argc,register char *argv[])
{
	int arg0=0;
	struct index_array *ap=0,*aq;
	if(nv_isarray(np))
	{
		ap = (struct index_array*)nv_arrayptr(np);
		if(ap && is_associative(ap))
			errormsg(SH_DICT,ERROR_exit(1),"cannot append index array to associative array %s",nv_name(np));
	}
	if(append)
	{
		if(ap)
		{
			if(!(aq = (struct index_array*)ap->header.scope))
				aq = ap;
			arg0 = ap->maxi;
			while(--arg0>0 && ap->val[arg0].cp==0 && aq->val[arg0].cp==0);
			arg0++;
		}
		else
		{
			nv_offattr(np,NV_ARRAY);
			if(!nv_isnull(np) && np->nvalue.cp!=Empty)
				arg0=1;
		}
	}
	while(--argc >= 0)
	{
		nv_putsub(np,NIL(char*),(long)argc+arg0|ARRAY_FILL|ARRAY_ADD);
		nv_putval(np,argv[argc],0);
	}
}

