/* @(#) $Id$ */

/* Copyright (C) 2005-2006 Daniel B. Cid <dcid@ossec.net>
 * All right reserved.
 *
 * This program is a free software; you can redistribute it
 * and/or modify it under the terms of the GNU General Public
 * License (version 2) as published by the FSF - Free Software
 * Foundation
 */


/* v0.1: 2005/06/21
 */
 
#include "shared.h"
#include "os_regex/os_regex.h"
#include "os_xml/os_xml.h"


#include "eventinfo.h"
#include "decoder.h"



/* DecodeEvent.
 * Will use the osdecoders to decode the received event.
 */
void DecodeEvent(Eventinfo *lf)
{
    OSDecoderNode *node;
    OSDecoderNode *child_node;
    OSDecoderInfo *nnode;

    char *llog;
    char *pmatch;
    char *cmatch;
    char *regex_prev = NULL;


    node = OS_GetFirstOSDecoder(lf->program_name);


    /* Return if no node...
     * This shouldn't happen here anyways.
     */
    if(!node)
        return;

    do 
    {
        nnode = node->osdecoder;


        /* First checking program name */
        if(lf->program_name)
        {
            if(!OSMatch_Execute(lf->program_name, lf->p_name_size, 
                        nnode->program_name))
            {
                continue;
            }
            pmatch = lf->log;
        }


        /* If prematch fails, go to the next osdecoder in the list */
        else if(nnode->prematch)
        {
            if(!(pmatch = OSRegex_Execute(lf->log, nnode->prematch)))
            {
                continue;
            }

            /* Next character */
            if(*pmatch != '\0')
                pmatch++;
        }
        else
        {
            continue;
        }


        lf->decoder_info = nnode;
        

        child_node = node->child;


        /* If no child node is set, set the child node
         * as if it were the child (ugh)
         */
        if(!child_node)
        {
            child_node = node;
        }

        else
        {
            /* Check if we have any child osdecoder */
            while(child_node)
            {
                nnode = child_node->osdecoder;


                /* If we have a pre match and it matches, keep
                 * going. If we don't have a prematch, stop
                 * and go for the regexes.
                 */
                if(nnode->prematch)
                {
                    char *llog;

                    /* If we have an offset set, use it */     
                    if(nnode->prematch_offset & AFTER_PARENT)
                    {
                        llog = pmatch;
                    }
                    else
                    {
                        llog = lf->log;
                    }

                    if((cmatch = OSRegex_Execute(llog, nnode->prematch)))
                    {
                        if(*cmatch != '\0')
                            cmatch++;

                        lf->decoder_info = nnode;

                        break;
                    }
                }
                else
                {
                    break;
                }


                /* If we have multiple regex-only childs,
                 * do not attempt to go any further with them.
                 */
                if(child_node->osdecoder->get_next)
                {
                    do
                    {
                        child_node = child_node->next;
                    }while(child_node && child_node->osdecoder->get_next);

                    if(!child_node)
                        return;

                    child_node = child_node->next;
                    nnode = NULL;   
                }
                else
                {
                    child_node = child_node->next;
                    nnode = NULL;
                }
            }
        }

        /* Nothing matched */
        if(!nnode)
            return;


        /* If we have a external decoder, execute it */
        if(nnode->plugindecoder)
        {
            nnode->plugindecoder(lf);
            return;
        }
        
        
        /* Getting the regex */
        while(child_node)
        {
            if(nnode->regex)
            {
                int i = 0;

                /* With regex we have multiple options
                 * regarding the offset:
                 * after the prematch,
                 * after the parent,
                 * after some previous regex,
                 * or any offset
                 */
                if(nnode->regex_offset)
                {
                    if(nnode->regex_offset & AFTER_PARENT)
                    {
                        llog = pmatch;
                    }
                    else if(nnode->regex_offset & AFTER_PREMATCH)
                    {
                        llog = cmatch;
                    }
                    else if(nnode->regex_offset & AFTER_PREVREGEX)
                    {
                        if(!regex_prev)
                            llog = cmatch;
                        else
                            llog = regex_prev;
                    }
                }
                else
                {
                    llog = lf->log;
                }

                /* If Regex does not match, return */
                if(!(regex_prev = OSRegex_Execute(llog, nnode->regex)))
                {
                    if(nnode->get_next)
                    {
                        child_node = child_node->next;
                        nnode = child_node->osdecoder;
                        continue;
                    }
                    return;
                }


                /* Fixing next pointer */
                if(*regex_prev != '\0')
                    regex_prev++;

                while(nnode->regex->sub_strings[i])
                {
                    if(nnode->order[i])
                    {
                        nnode->order[i](lf, nnode->regex->sub_strings[i]);
                        nnode->regex->sub_strings[i] = NULL;
                        i++;
                        continue;
                    }

                    /* We do not free any memory used above */
                    os_free(nnode->regex->sub_strings[i]);
                    nnode->regex->sub_strings[i] = NULL;
                    i++;
                }

                /* If we have a next regex, try getting it */
                if(nnode->get_next)
                {
                    child_node = child_node->next;
                    nnode = child_node->osdecoder;
                    continue;
                }

                break;
            }

            /* If we don't have a regex, we may leave now */
            return;
        }

        /* ok to return  */
        return;         
    }while((node=node->next) != NULL);
}


/*** Event decoders ****/
void *DstUser_FP(Eventinfo *lf, char *field)
{
    lf->dstuser = field;
    return(NULL);
}
void *User_FP(Eventinfo *lf, char *field)
{
    lf->user = field;
    return(NULL);
}
void *SrcIP_FP(Eventinfo *lf, char *field)
{
    lf->srcip = field;
    return(NULL);
}
void *DstIP_FP(Eventinfo *lf, char *field)
{
    lf->dstip = field;
    return(NULL);
}
void *SrcPort_FP(Eventinfo *lf, char *field)
{
    lf->srcport = field;
    return(NULL);
}
void *DstPort_FP(Eventinfo *lf, char *field)
{
    lf->dstport = field;
    return(NULL);
}
void *Protocol_FP(Eventinfo *lf, char *field)
{
    lf->protocol = field;
    return(NULL);
}
void *Action_FP(Eventinfo *lf, char *field)
{
    lf->action = field;
    return(NULL);
}
void *ID_FP(Eventinfo *lf, char *field)
{
    lf->id = field;
    return(NULL);
}
void *Url_FP(Eventinfo *lf, char *field)
{
    lf->url = field;
    return(NULL);
}
void *Data_FP(Eventinfo *lf, char *field)
{
    lf->data = field;
    return(NULL);
}
void *Status_FP(Eventinfo *lf, char *field)
{
    lf->status = field;
    return(NULL);
}
void *SystemName_FP(Eventinfo *lf, char *field)
{
    lf->systemname = field;
    return(NULL);
}
void *None_FP(Eventinfo *lf, char *field)
{
    free(field);
    return(NULL);
}


/* EOF */
