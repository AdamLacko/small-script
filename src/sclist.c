/*  Small compiler  - maintenance of various lists
 *
 *  Name list (aliases)
 *  Include path list
 *
 *  Copyright (c) ITB CompuPhase, 2001-2003
 *  This file may be freely used. No warranties of any kind.
 *
 *  Version: $Id: sclist.c 2349 2003-10-16 11:02:52Z thiadmer $
 */
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "sc.h"

static stringpair *insert_stringpair(stringpair *root,char *first,char *second)
{
  stringpair *cur;

  assert(root!=NULL);
  assert(first!=NULL);
  assert(second!=NULL);
  if ((cur=(stringpair*)malloc(sizeof(stringpair)))==NULL)
    return NULL;
  cur->first=strdup(first);
  cur->second=strdup(second);
  if (cur->first==NULL || cur->second==NULL) {
    if (cur->first!=NULL)
      free(cur->first);
    if (cur->second!=NULL)
      free(cur->second);
    free(cur);
    return NULL;
  } /* if */
  cur->next=root->next;
  root->next=cur;
  return cur;
}

static void delete_stringpairtable(stringpair *root)
{
  stringpair *cur, *next;

  assert(root!=NULL);
  cur=root->next;
  while (cur!=NULL) {
    next=cur->next;
    assert(cur->first!=NULL);
    assert(cur->second!=NULL);
    free(cur->first);
    free(cur->second);
    free(cur);
    cur=next;
  } /* while */
  memset(root,0,sizeof(stringpair));
}

static stringpair *find_stringpair(stringpair *root,char *first,int max)
{
  stringpair *cur;

  assert(root!=NULL);
  assert(max==-1 || max>0);     /* the function cannot handle zero-length comparison */
  cur=root->next;
  assert(first!=NULL);
  while (cur!=NULL) {
    if (max<0 && strcmp(first,cur->first)==0 || strncmp(first,cur->first,max)==0)
      return cur;
    cur=cur->next;
  } /* while */
  return NULL;
}

static int delete_stringpair(stringpair *root,stringpair *item)
{
  stringpair *cur;

  assert(root!=NULL);
  cur=root;
  while (cur->next!=NULL) {
    if (cur->next==item) {
      cur->next=item->next;     /* unlink from list */
      assert(item->first!=NULL);
      assert(item->second!=NULL);
      free(item->first);
      free(item->second);
      free(item);
      return TRUE;
    } /* if */
    cur=cur->next;
  } /* while */
  return FALSE;
}

/* ----- alias table --------------------------------------------- */
static stringpair alias_tab = {NULL, NULL, NULL};   /* alias table */

SC_FUNC stringpair *insert_alias(char *name,char *alias)
{
  stringpair *cur;

  assert(name!=NULL);
  assert(strlen(name)<=sNAMEMAX);
  assert(alias!=NULL);
  assert(strlen(alias)<=sEXPMAX);
  if ((cur=insert_stringpair(&alias_tab,name,alias))==NULL)
    error(103);       /* insufficient memory (fatal error) */
  return cur;
}

SC_FUNC stringpair *find_alias(char *name)
{
  return find_stringpair(&alias_tab,name,-1);
}

SC_FUNC int lookup_alias(char *target,char *name)
{
  stringpair *cur=find_stringpair(&alias_tab,name,-1);
  if (cur!=NULL) {
    assert(strlen(cur->second)<=sEXPMAX);
    strcpy(target,cur->second);
  } /* if */
  return cur!=NULL;
}

SC_FUNC void delete_aliastable(void)
{
  delete_stringpairtable(&alias_tab);
}

/* ----- include paths list -------------------------------------- */
static stringlist includepaths = {NULL, NULL};  /* directory list for include files */

SC_FUNC stringlist *insert_path(char *path)
{
  stringlist *cur;

  assert(path!=NULL);
  if ((cur=(stringlist*)malloc(sizeof(stringlist)))==NULL)
    error(103);       /* insufficient memory (fatal error) */
  if ((cur->line=strdup(path))==NULL)
    error(103);       /* insufficient memory (fatal error) */
  cur->next=includepaths.next;
  includepaths.next=cur;
  return cur;
}

SC_FUNC char *get_path(int index)
{
  stringlist *cur = includepaths.next;

  while (cur!=NULL && index-->0)
    cur=cur->next;
  if (cur!=NULL) {
    assert(cur->line!=NULL);
    return cur->line;
  } /* if */
  return NULL;
}

SC_FUNC void delete_pathtable(void)
{
  stringlist *cur=includepaths.next, *next;

  while (cur!=NULL) {
    next=cur->next;
    assert(cur->line!=NULL);
    free(cur->line);
    free(cur);
    cur=next;
  } /* while */
  memset(&includepaths,0,sizeof(stringlist));
}


/* ----- text substitution patterns ------------------------------ */
#if !defined NO_DEFINE

static stringpair substpair = { NULL, NULL, NULL};  /* list of substitution pairs */

SC_FUNC stringpair *insert_subst(char *pattern,char *substitution)
{
  stringpair *cur;

  assert(pattern!=NULL);
  assert(substitution!=NULL);
  if ((cur=insert_stringpair(&substpair,pattern,substitution))==NULL)
    error(103);       /* insufficient memory (fatal error) */
  return cur;
}

SC_FUNC int get_subst(int index,char **pattern,char **substitution)
{
  stringpair *cur = substpair.next;

  assert(pattern!=NULL);
  assert(substitution!=NULL);
  while (cur!=NULL && index-->0)
    cur=cur->next;
  if (cur!=NULL) {
    assert(cur->first!=NULL);
    assert(cur->second!=NULL);
    *pattern=cur->first;
    *substitution=cur->second;
    return TRUE;
  } /* if */
  return FALSE;
}

SC_FUNC stringpair *find_subst(char *name)
{
  return find_stringpair(&substpair,name,strlen(name));
}

SC_FUNC int delete_subst(char *name)
{
  stringpair *item=find_stringpair(&substpair,name,strlen(name));
  return (item!=NULL) ? delete_stringpair(&substpair,item) : FALSE;
}

SC_FUNC void delete_substtable(void)
{
  delete_stringpairtable(&substpair);
}

#endif /* !defined NO_SUBST */
