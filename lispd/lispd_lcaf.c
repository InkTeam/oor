/*
 * lispd_lcaf.c
 *
 * This file is part of LISP Mobile Node Implementation.
 * Necessary logic to handle incoming map replies.
 *
 * Copyright (C) 2012 Cisco Systems, Inc, 2012. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 * Please send any bug reports or fixes you make to the email address(es):
 *    LISP-MN developers <devel@lispmob.org>
 *
 * Written or modified by:
 *    Florin Coras  <fcoras@ac.upc.edu>
 *
 */

#include "defs.h"
#include "lispd_lcaf.h"


typedef void    (*del_fct)(void *);
typedef int     (*read_from_pkt_fct)(void *, void *);
typedef char    *(*to_char_fct)(void *);
typedef void    (*copy_fct)(void **, void *);
typedef int     (*cmp_fct)(void *, void *);
typedef uint8_t *(*write_to_pkt_fct)(uint8_t *, void *);

del_fct del_fcts[MAX_LCAFS] = {
        0, 0,
        iid_type_del,
        0, 0, 0,
        geo_type_del, 0, 0,
        mc_type_del, 0, 0,
        rle_type_del, 0, 0, 0};

read_from_pkt_fct read_from_pkt_fcts[MAX_LCAFS] = {
        0, 0,
        iid_type_read_from_pkt, 0, 0, 0,
        geo_type_read_from_pkt, 0, 0,
        mc_type_read_from_pkt, 0, 0,
        rle_type_read_from_pkt, 0, 0, 0};

to_char_fct to_char_fcts[MAX_LCAFS] = {
        0, 0,
        iid_type_to_char, 0, 0, 0,
        geo_type_to_char, 0, 0,
        mc_type_to_char, 0, 0,
        rle_type_to_char, 0, 0, 0
};

write_to_pkt_fct write_to_pkt_fcts[MAX_LCAFS] = {
        0, 0,
        iid_type_copy_to_pkt, 0, 0, 0,
        0, 0, 0,
        mc_type_copy_to_pkt, 0, 0,
        0, 0, 0, 0
};

copy_fct copy_fcts[MAX_LCAFS] = {
        0, 0,
        iid_type_copy, 0, 0, 0,
        geo_type_copy, 0, 0,
        mc_type_copy, 0, 0,
        rle_type_copy, 0, 0, 0
};

cmp_fct cmp_fcts[MAX_LCAFS] = {
        0, 0,
        iid_type_cmp, 0, 0, 0,
        0, 0, 0,
        mc_type_cmp, 0, 0,
        0, 0, 0, 0
};

static inline lcaf_t _get_type(lcaf_addr_t *lcaf) {
    assert(lcaf);
    return(lcaf->type);
}

static inline void *_get_addr(lcaf_addr_t *lcaf) {
    assert(lcaf);
    return(lcaf->addr);
}



lcaf_addr_t *lcaf_addr_new() {
    return(calloc(1, sizeof(lcaf_addr_t)));
}

lcaf_addr_t *lcaf_addr_new_type(uint8_t type) {
    lcaf_addr_t *lcaf;

    lcaf = calloc(1, sizeof(lcaf_addr_t));
    lcaf_addr_set_type(lcaf, type);

    switch(type) {
        case LCAF_IID:
            lcaf->addr = iid_type_new();
            break;
        case LCAF_MCAST_INFO:
            lcaf->addr = mc_type_new();
            break;
        case LCAF_GEO:
            break;
        default:
            break;
    }

    return(lcaf);
}

void lcaf_addr_del(lcaf_addr_t *lcaf) {
    assert(lcaf);
    if (!del_fcts[_get_type(lcaf)]) {
        return;
    }
    (*del_fcts[_get_type(lcaf)])(lcaf_addr_get_addr(lcaf));
    free(lcaf);
}

/*
 * lcaf_addr_t functions
 */

int lcaf_addr_read_from_pkt(void *offset, lcaf_addr_t *lcaf_addr) {

    int len = 0;

    lcaf_addr_set_type(lcaf_addr, ntohs(((lispd_pkt_lcaf_t *)offset)->type));
    if (!read_from_pkt_fcts[lcaf_addr_get_type(lcaf_addr)])
        return(BAD);
    len = read_from_pkt_fcts[lcaf_addr_get_type(lcaf_addr)](offset, lcaf_addr_get_addr(lcaf_addr));
    if (len != ntohs(((lispd_pkt_lcaf_t *)offset)->len)) {
        lispd_log_msg(LISP_LOG_DEBUG_3, "lcaf_addr_read_from_pkt: len field and the number of bytes read are different!");
        return(BAD);
    }

    return(len);

//    switch(lcaf_addr_get_type(lcaf_addr)) {
//        case LCAF_IID:
//            return(iid_addr_read_from_pkt(offset, lcaf_addr_get_iid(lcaf_addr)));
//            break;
//        case LCAF_MCAST_INFO:
//            return(mc_addr_read_from_pkt(offset, lcaf_addr_get_mc(lcaf_addr)));
//            break;
//        case LCAF_GEO:
//            return(geo_addr_read_from_pkt(offset, lcaf_addr_get_geo(lcaf_addr)));
//            break;
//        default:
//            lispd_log_msg(LISP_LOG_DEBUG_2,"lcaf_addr_read_from_pkt:  Unknown LCAF type %d in EID",
//                    lcaf_addr_get_type(lcaf_addr));
//            break;
//    }
//    return(0);

}


char *lcaf_addr_to_char(lcaf_addr_t *lcaf) {
    assert(lcaf);
    return((*to_char_fcts[_get_type(lcaf)])(lcaf_addr_get_addr(lcaf)));
}


inline lcaf_t lcaf_addr_get_type(lcaf_addr_t *lcaf) {
    assert(lcaf);
    return(lcaf->type);
}

inline mc_t *lcaf_addr_get_mc(lcaf_addr_t *lcaf) {
    assert(lcaf);
    return((mc_t *)lcaf_addr_get_addr(lcaf));
}

inline geo_t *lcaf_addr_get_geo(lcaf_addr_t *lcaf) {
    assert(lcaf);
    return((geo_t *)lcaf_addr_get_addr(lcaf));
}

inline iid_t *lcaf_addr_get_iid(lcaf_addr_t *lcaf) {
    assert(lcaf);
    return((iid_t *)lcaf_addr_get_addr(lcaf));
}

inline void *lcaf_addr_get_addr(lcaf_addr_t *lcaf) {
    assert(lcaf);
    return(lcaf->addr);
}

inline int lcaf_addr_is_mc(lcaf_addr_t *lcaf) {
    if (lcaf_addr_get_type(lcaf) == LCAF_MCAST_INFO)
        return(1);
    else
        return(0);
}


inline void lcaf_addr_set(lcaf_addr_t *lcaf, void *newaddr, uint8_t type) {
    void *addr;
    addr = lcaf_addr_get_addr(lcaf);
    if (addr)
        lcaf_addr_del(addr);
    lcaf_addr_set_type(lcaf, type);
    lcaf_addr_set_addr(lcaf, newaddr);
}

inline void lcaf_addr_set_addr(lcaf_addr_t *lcaf, void *addr) {
    assert(lcaf);
    assert(addr);
    lcaf->addr = addr;
}
inline void lcaf_addr_set_type(lcaf_addr_t *lcaf, uint8_t type) {
    assert(lcaf);
    lcaf->type = type;
}

inline uint32_t lcaf_addr_get_size_in_pkt(lcaf_addr_t *lcaf) {
    /*NOTE: This returns size in packet of the lcaf */
    switch(lcaf_addr_get_type(lcaf)) {
        case LCAF_IID:
            return(iid_type_get_size_in_pkt(lcaf_addr_get_iid(lcaf)));
            break;
        case LCAF_MCAST_INFO:
            return(mc_type_get_size_in_pkt(lcaf_addr_get_mc(lcaf)));
        /* TODO: to be finished */
        case LCAF_GEO:
            break;
        default:
            break;
    }

    return(0);
}

int lcaf_addr_copy(lcaf_addr_t **dst, lcaf_addr_t *src) {

    void *addr;

    assert(src);
    if (!copy_fcts[lcaf_addr_get_type(src)]) {
        lispd_log_msg(LISP_LOG_WARNING, "lcaf_addr_copy: copy not implemented for LCAF type %s",lcaf_addr_get_type(src));
        return(BAD);
    }

    if (!(*dst))
        *dst = lcaf_addr_new();
    else if (_get_type(*dst) != _get_type(src)) {
        lcaf_addr_del(*dst);
        *dst = lcaf_addr_new();
    }

    lcaf_addr_set_type(*dst, _get_type(src));
    addr = _get_addr(*dst);
    (*copy_fcts[_get_type(src)])(&addr, src);

    return(GOOD);
}

inline uint8_t *lcaf_addr_write_to_pkt(void *offset, lcaf_addr_t *lcaf) {

    assert(lcaf);
    if (!write_to_pkt_fcts[_get_type(lcaf)]) {
        lispd_log_msg(LISP_LOG_WARNING, "lcaf_addr_write_to_pkt: write not implemented for LCAF type %s",
                _get_type(lcaf));
        return(BAD);
    }

    return((*write_to_pkt_fcts[_get_type(lcaf)])(offset, _get_addr(lcaf)));
}

inline int lcaf_addr_cmp(lcaf_addr_t *addr1, lcaf_addr_t *addr2) {
    if (lcaf_addr_get_type(addr1) != lcaf_addr_get_type(addr2))
        return(-1);
    if (!(cmp_fcts[lcaf_addr_get_type(addr1)])) {
        lispd_log_msg(LISP_LOG_DEBUG_1, "lcaf_addr_cmp: cmp not implemented for type %d", lcaf_addr_get_type(addr1));
        return(-1);
    }
    return((*cmp_fcts[lcaf_addr_get_type(addr1)])(lcaf_addr_get_addr(addr1), lcaf_addr_get_addr(addr2)));

}

inline uint8_t lcaf_addr_cmp_iids(lcaf_addr_t *addr1, lcaf_addr_t *addr2) {
    if (_get_type(addr1) != _get_type(addr2))
        return(0);
    switch(_get_type(addr1)) {
        case LCAF_IID:
            return(lcaf_iid_get_iid(addr1) == lcaf_iid_get_iid(addr2));
        case LCAF_MCAST_INFO:
            return(mc_type_get_iid(_get_addr(addr1)) == mc_type_get_iid(addr2));
        default:
            return(0);
    }
}

uint8_t is_lcaf_mcast_info(uint8_t *cur_ptr) {
    uint16_t    lisp_afi;

    cur_ptr  = CO(cur_ptr, sizeof(lisp_afi));
    lisp_afi = ntohs(*(uint16_t *)cur_ptr);

    return(lisp_afi == LISP_AFI_LCAF && ntohs(((lispd_pkt_lcaf_t *)cur_ptr)->type) == LCAF_MCAST_INFO);
}

inline mrsignaling_flags_t lcaf_mcinfo_get_flags(uint8_t *cur_ptr) {
    mrsignaling_flags_t  flags;

    cur_ptr = CO(cur_ptr, sizeof(uint16_t));
    flags.jbit = ((lispd_lcaf_mcinfo_hdr_t *)cur_ptr)->jbit;
    flags.lbit = ((lispd_lcaf_mcinfo_hdr_t *)cur_ptr)->lbit;
    flags.rbit = ((lispd_lcaf_mcinfo_hdr_t *)cur_ptr)->rbit;

    return(flags);
}





/*
 * mc_addr_t functions
 */

inline mc_t *mc_type_new() {
    mc_t *mc = calloc(1, sizeof(mc_t));
    mc->src = lisp_addr_new();
    mc->grp = lisp_addr_new();
    return(mc);
}

inline void mc_type_del(void *mc) {
    lisp_addr_del(mc_type_get_src(mc));
    lisp_addr_del(mc_type_get_grp(mc));

    free(mc);
}

inline void mc_addr_set_src_plen(mc_t *mc, uint8_t plen) {
    assert(mc);
    mc->src_plen = plen;
}

inline void mc_addr_set_grp_plen(mc_t *mc, uint8_t plen) {
    assert(mc);
    mc->grp_plen = plen;
}

inline void mc_type_set_iid(mc_t *mc, uint32_t iid) {
    assert(mc);
    mc->iid = iid;
}

inline void mc_type_set_src(void *mc, lisp_addr_t *src) {
    assert(mc);
    assert(src);
    lisp_addr_copy(mc_type_get_src(mc), src);
}

inline void mc_type_set_grp(mc_t *mc, lisp_addr_t *grp) {
    assert(mc);
    assert(grp);
    lisp_addr_copy(mc_type_get_grp(mc), grp);
}

inline void mc_type_copy(void **dst, void *src) {
    assert(src);
    if (!(*dst))
        *dst = mc_type_new();
    mc_type_set_iid(*dst, mc_type_get_iid(src));
    mc_addr_set_src_plen(*dst, mc_type_get_src_plen(src));
    mc_addr_set_grp_plen(*dst, mc_type_get_grp_plen(src));
    lisp_addr_copy(mc_type_get_src(*dst), mc_type_get_src(src));
    lisp_addr_copy(mc_type_get_grp(*dst), mc_type_get_grp(src));
}

inline int mc_type_cmp(void *mc1, void *mc2) {
    if (    (mc_type_get_iid(mc1) != mc_type_get_iid(mc2)) ||
            (mc_type_get_src_plen(mc1) != mc_type_get_src_plen(mc2)) ||
            (mc_type_get_grp_plen(mc1) != mc_type_get_grp_plen(mc2)))
        return(-1);

    /* XXX: rushed implementation
     * (S, G) comparison
     * First compare S and then G*/
    int res = lisp_addr_cmp(mc_type_get_src(mc1), mc_type_get_src(mc2));
    if (res == 0)
        return(lisp_addr_cmp(mc_type_get_grp(mc1), mc_type_get_grp(mc2)));
    else
        return(res);

}

inline void mc_type_set(mc_t *dst, lisp_addr_t *src, lisp_addr_t *grp, uint8_t splen, uint8_t gplen, uint32_t iid) {
    assert(src);
    assert(dst);
    assert(grp);
    mc_type_set_src(dst, src);
    mc_type_set_grp(dst, grp);
    mc_addr_set_src_plen(dst, splen);
    mc_addr_set_grp_plen(dst, gplen);
    mc_type_set_iid(dst, iid);
}

/**
 * mc_addr_init - makes an mc_addr_t from the parameters passed
 * @ src: source ip
 * @ grp: group ip
 * @ splen: source prefix length
 * @ gplen: group prefix length
 * @ iid: iid of the address
 */
mc_t *mc_type_init(lisp_addr_t *src, lisp_addr_t *grp, uint8_t splen, uint8_t gplen, uint32_t iid) {
    mc_t *mc;

    assert(src);
    assert(grp);

    mc = mc_type_new();
    lisp_addr_copy(mc_type_get_src(mc), src);
    lisp_addr_copy(mc_type_get_grp(mc), grp);
    mc_addr_set_src_plen(mc, splen);
    mc_addr_set_grp_plen(mc, gplen);
    mc_type_set_iid(mc, iid);
    return(mc);
}


inline lisp_addr_t *lcaf_mc_get_src(lcaf_addr_t *mc) {
    assert(mc);
    if (lcaf_addr_get_type(mc) != LCAF_MCAST_INFO)
        return(NULL);
    return(((mc_t *)mc->addr)->src);
}

inline lisp_addr_t *lcaf_mc_get_grp(lcaf_addr_t *mc) {
    assert(mc);
    if (lcaf_addr_get_type(mc) != LCAF_MCAST_INFO)
        return(NULL);
    return(((mc_t *)mc->addr)->grp);
}

inline uint32_t lcaf_mc_get_iid(lcaf_addr_t *mc) {
    assert(mc);
    return(((mc_t *)mc->addr)->iid);
}

inline uint8_t lcaf_mc_get_src_plen(lcaf_addr_t *mc) {
    assert(mc);
    return(((mc_t *)mc->addr)->src_plen);
}

inline uint8_t lcaf_mc_get_grp_plen(lcaf_addr_t *mc) {
    assert(mc);
    return(((mc_t *)mc->addr)->grp_plen);
}


/* these shouldn't be called from outside */

inline lisp_addr_t *mc_type_get_src(mc_t *mc) {
    assert(mc);
    return(mc->src);
}

inline lisp_addr_t *mc_type_get_grp(mc_t *mc) {
    assert(mc);
    return(mc->grp);
}

inline uint32_t mc_type_get_iid(void *mc) {
    assert(mc);
    return(((mc_t *)mc)->iid);
}

inline uint8_t mc_type_get_src_plen(mc_t *mc) {
    assert(mc);
    return(mc->src_plen);
}

inline uint8_t mc_type_get_grp_plen(mc_t *mc) {
    assert(mc);
    return(mc->grp_plen);
}

/* set functions common to all types */

char *mc_type_to_char(void *mc){
    static char address[INET6_ADDRSTRLEN*2+4];
    sprintf(address, "(%s/%d,%s/%d)",
            lisp_addr_to_char(mc_type_get_src((mc_t *)mc)),
            mc_type_get_src_plen((mc_t *)mc),
            lisp_addr_to_char(mc_type_get_grp((mc_t *)mc)),
            mc_type_get_src_plen((mc_t *)mc));
    return(address);
}

inline uint32_t mc_type_get_size_in_pkt(mc_t *mc) {
    return( sizeof(lispd_lcaf_mcinfo_hdr_t)+
            lisp_addr_get_size_in_pkt(mc_type_get_src(mc)) +
            sizeof(uint16_t)+ /* grp afi */
            lisp_addr_get_size_in_pkt(mc_type_get_grp(mc)) );
}

inline uint8_t *mc_type_copy_to_pkt(uint8_t *offset, void *mc) {
    uint8_t *cur_ptr;
    ((lispd_lcaf_mcinfo_hdr_t *)offset)->rsvd1 = 0;
    ((lispd_lcaf_mcinfo_hdr_t *)offset)->flags = 0;
    ((lispd_lcaf_mcinfo_hdr_t *)offset)->type = LCAF_MCAST_INFO;
    ((lispd_lcaf_mcinfo_hdr_t *)offset)->rsvd2 = 0;
    ((lispd_lcaf_mcinfo_hdr_t *)offset)->rbit = 0;
    ((lispd_lcaf_mcinfo_hdr_t *)offset)->lbit = 0;
    ((lispd_lcaf_mcinfo_hdr_t *)offset)->jbit = 0;
    ((lispd_lcaf_mcinfo_hdr_t *)offset)->len = htons(mc_type_get_size_in_pkt(mc));
    ((lispd_lcaf_mcinfo_hdr_t *)offset)->iid = htonl(mc_type_get_iid(mc));
    ((lispd_lcaf_mcinfo_hdr_t *)offset)->reserved = 0;
    ((lispd_lcaf_mcinfo_hdr_t *)offset)->src_mlen = mc_type_get_src_plen(mc);
    ((lispd_lcaf_mcinfo_hdr_t *)offset)->grp_mlen = mc_type_get_grp_plen(mc);
    cur_ptr = (uint8_t *)&(((lispd_lcaf_mcinfo_hdr_t *)offset)->src_afi);
    cur_ptr = lisp_addr_write_to_pkt(cur_ptr, mc_type_get_src(mc));
    cur_ptr = lisp_addr_write_to_pkt(cur_ptr, mc_type_get_grp(mc));
    return(cur_ptr);
}

int mc_type_read_from_pkt(void *offset, void *mc) {
    mc = calloc(1, sizeof(mc_t));
    mc_type_set_iid(mc, ((lispd_lcaf_mcinfo_hdr_t *)offset)->iid);
    mc_addr_set_src_plen(mc, ((lispd_lcaf_mcinfo_hdr_t *)offset)->src_mlen);
    mc_addr_set_grp_plen(mc, ((lispd_lcaf_mcinfo_hdr_t *)offset)->grp_mlen);

    offset = CO(offset, sizeof(lispd_lcaf_mcinfo_hdr_t));

    return(sizeof(lispd_lcaf_mcinfo_hdr_t) +
            lisp_addr_read_from_pkt(offset, mc_type_get_src(mc)) +
            lisp_addr_read_from_pkt(offset, mc_type_get_grp(mc)));
}


/* Function that builds mc packets from packets on the wire. */
lcaf_addr_t *lcaf_addr_init_mc(lisp_addr_t *src, lisp_addr_t *grp, uint8_t splen, uint8_t gplen, uint32_t iid) {
    assert(src);
    assert(grp);

    mc_t            *mc;
    lcaf_addr_t     *lcaf;

    mc  = mc_type_init(src, grp, splen, gplen, iid);
    lcaf = lcaf_addr_new();
    lcaf_addr_set_type(lcaf, LCAF_MCAST_INFO);
    lcaf_addr_set_addr(lcaf, mc);
    return(lcaf);
}





/*
 * iid_addr_t functions
 */
inline iid_t *iid_type_new() {
    iid_t *iid;
    iid = (iid_t *)calloc(1, sizeof(iid_t));
    iid->iidaddr = lisp_addr_new();
    return(iid);
}

inline void iid_type_del(void *iid) {
    lisp_addr_del(iid_type_get_addr((iid_t *)iid));
    free(iid);
}

inline uint8_t iid_type_get_mlen(iid_t *iid) {
    assert(iid);
    return(iid->mlen);
}

inline uint32_t lcaf_iid_get_iid(lcaf_addr_t *iid) {
    return(iid_type_get_iid(_get_addr(iid)));
}

inline uint32_t iid_type_get_iid(iid_t *iid) {
    assert(iid);
    return(iid->iid);
}

inline lisp_addr_t *iid_type_get_addr(void *iid) {
    assert(iid);
    return(((iid_t *)iid)->iidaddr);
}



inline void iid_type_set_iid(iid_t *iidt, uint32_t iid) {
    assert(iidt);
    iidt->iid = iid;
}

inline void iid_type_set_addr(iid_t *iidt, lisp_addr_t *iidaddr) {
    assert(iidt);
    assert(iidaddr);
    iidt->iidaddr = iidaddr;
}

inline void iid_type_set_mlen(iid_t *iid, uint8_t mlen) {
    assert(iid);
    iid->mlen = mlen;
}

inline int iid_type_cmp(void *iid1, void *iid2) {
    if ((iid_type_get_iid((iid_t *)iid1) != iid_type_get_iid((iid_t *)iid2)) ||
            (iid_type_get_mlen((iid_t *)iid1) != iid_type_get_mlen((iid_t *)iid2)))
        return(-1);
    return(lisp_addr_cmp(iid_type_get_addr((iid_t *)iid1), iid_type_get_addr((iid_t *)iid2)));
}

inline uint32_t iid_type_get_size_in_pkt(iid_t *iid) {
    return( sizeof(lispd_pkt_lcaf_t)+
            sizeof(lispd_pkt_lcaf_iid_t)+
            lisp_addr_get_size_in_pkt(iid_type_get_addr(iid)));
}

inline uint8_t *iid_type_copy_to_pkt(uint8_t *offset, void *iid) {
    ((lispd_pkt_iid_hdr_t *)offset)->rsvd1 = 0;
    ((lispd_pkt_iid_hdr_t *)offset)->flags = 0;
    ((lispd_pkt_iid_hdr_t *)offset)->type = LCAF_IID;
    ((lispd_pkt_iid_hdr_t *)offset)->mlen = iid_type_get_mlen(iid);
    ((lispd_pkt_iid_hdr_t *)offset)->len = htons(iid_type_get_size_in_pkt(iid));
    ((lispd_pkt_iid_hdr_t *)offset)->iid = htonl(iid_type_get_iid(iid));
    return(lisp_addr_write_to_pkt(&(((lispd_pkt_iid_hdr_t *)offset)->afi),iid_type_get_addr(iid)));
}

int iid_type_read_from_pkt(void *offset, void *iid) {
    iid = calloc(1, sizeof(iid_t));
    iid_type_set_mlen(iid, ((lispd_pkt_iid_hdr_t *)offset)->mlen);
    iid_type_set_iid(iid, ((lispd_pkt_iid_hdr_t *)offset)->iid);

    offset = CO(offset, sizeof(lispd_pkt_iid_hdr_t));
    return(lisp_addr_read_from_pkt(offset, iid_type_get_addr(iid)) + sizeof(lispd_pkt_iid_hdr_t));
}

char *iid_type_to_char(void *iid) {
    static char buf[INET6_ADDRSTRLEN*2+4];
    sprintf(buf, "(IID %d/%d, EID %s)",
            iid_type_get_iid(iid),
            iid_type_get_mlen(iid),
            lisp_addr_to_char(iid_type_get_addr(iid)));
    return(buf);
}

void iid_type_copy(void **dst, void *src) {
    if (!(*dst))
        *dst = iid_type_new();
    lisp_addr_copy(iid_type_get_addr((iid_t *)*dst), iid_type_get_addr((iid_t *)src));
    iid_type_set_iid((iid_t *)*dst, iid_type_get_iid((iid_t *)src));
    iid_type_set_mlen((iid_t*)*dst, iid_type_get_mlen(src));
}






/*
 * geo_addr_t functions
 */

inline geo_t *geo_type_new() {
    geo_t *geo;
    geo = (geo_t *)calloc(1, sizeof(geo_t));
    geo->addr = lisp_addr_new();
    return(geo);
}

inline void geo_type_del(void *geo) {
    lisp_addr_del(geo_type_get_addr((geo_t *)geo));
    free(geo);
}

inline void geo_type_set_addr(geo_t *geo, lisp_addr_t *addr){
    assert(addr);
    assert(geo);
    geo->addr = addr;
}

inline void geo_type_set_lat(geo_t *geo, uint8_t dir, uint16_t deg, uint8_t min, uint8_t sec) {
    assert(geo);
    geo->latitude.dir = dir;
    geo->latitude.deg = deg;
    geo->latitude.min = min;
    geo->latitude.sec = sec;
}

inline void geo_type_set_long(geo_t *geo, uint8_t dir, uint16_t deg, uint8_t min, uint8_t sec) {
    assert(geo);
    geo->longitude.dir = dir;
    geo->longitude.deg = deg;
    geo->longitude.min = min;
    geo->longitude.sec = sec;
}

inline void geo_type_set_lat_from_coord(geo_t *geo, geo_coordinates *coord) {
    assert(geo);
    geo->latitude.dir = coord->dir;
    geo->latitude.deg = coord->deg;
    geo->latitude.min = coord->min;
    geo->latitude.sec = coord->sec;
}

inline void geo_type_set_long_from_coord(geo_t *geo, geo_coordinates *coord) {
    assert(geo);
    geo->longitude.dir = coord->dir;
    geo->longitude.deg = coord->deg;
    geo->longitude.min = coord->min;
    geo->longitude.sec = coord->sec;
}

inline void geo_type_set_altitude(geo_t *geo, uint32_t altitude) {
    assert(geo);
    geo->altitude = altitude;
}

inline geo_coordinates *geo_type_get_lat(geo_t *geo) {
    assert(geo);
    return(&(geo->latitude));
}

inline geo_coordinates *geo_type_get_long(geo_t *geo) {
    assert(geo);
    return(&(geo->longitude));
}

inline uint32_t geo_type_get_altitude(geo_t *geo) {
    assert(geo);
    return(geo->altitude);
}

inline int geo_type_read_from_pkt(void *offset, void *geo) {
    geo = calloc(1, sizeof(geo_t));
    geo_type_set_lat((geo_t *)geo,
            ((lispd_lcaf_geo_hdr_t *)offset)->latitude_dir,
            ((lispd_lcaf_geo_hdr_t *)offset)->latitude_deg,
            ((lispd_lcaf_geo_hdr_t *)offset)->latitude_min,
            ((lispd_lcaf_geo_hdr_t *)offset)->latitude_sec);
    geo_type_set_long((geo_t *)geo,
            ((lispd_lcaf_geo_hdr_t *)offset)->longitude_dir,
            ((lispd_lcaf_geo_hdr_t *)offset)->longitude_deg,
            ((lispd_lcaf_geo_hdr_t *)offset)->longitude_min,
            ((lispd_lcaf_geo_hdr_t *)offset)->longitude_sec);
    geo_type_set_altitude((geo_t *)geo, ((lispd_lcaf_geo_hdr_t *)offset)->altitude);

    offset = CO(offset, sizeof(lispd_lcaf_geo_hdr_t));
    return(sizeof(lispd_lcaf_geo_hdr_t) +
            lisp_addr_read_from_pkt(offset, geo_type_get_addr((geo_t *)geo)));
}

inline lisp_addr_t *geo_type_get_addr(geo_t *geo) {
    assert(geo);
    return(geo->addr);
}


char *geo_type_to_char(void *geo) {
    static char buf[INET6_ADDRSTRLEN*2+4];
    sprintf(buf, "(latitude: %s | longitude: %s | altitude: %d, EID %s)",
            geo_coord_to_char(geo_type_get_lat(geo)),
            geo_coord_to_char(geo_type_get_long(geo)),
            geo_type_get_altitude(geo),
            lisp_addr_to_char(geo_type_get_addr(geo)));
    return(buf);
}

char *geo_coord_to_char(geo_coordinates *coord) {
    static char buf[INET6_ADDRSTRLEN*2+4];
    sprintf(buf, "dir %d deg %d min %d sec %d",
            coord->dir, coord->deg, coord->min, coord->sec);
    return(buf);
}

void geo_type_copy(void **dst, void *src) {
    assert(src);
    if(!(*dst))
        *dst = geo_type_new();
    geo_type_set_lat_from_coord((geo_t *)*dst, geo_type_get_lat(src));
    geo_type_set_long_from_coord((geo_t *)*dst, geo_type_get_long(src));
    geo_type_set_altitude((geo_t *)*dst, geo_type_get_altitude(src));
    lisp_addr_copy(geo_type_get_addr((geo_t *)*dst), geo_type_get_addr(src));
}


/*
 * rle_addr_t functions
 */
inline rle_t *rle_type_new() {
    return((rle_t *)calloc(1, sizeof(iid_t)));
}

inline void rle_type_del(void *rleaddr) {
    /* TODO: finish */
//    uint32_t lvls;
//    lvls = rle_addr_get_nb_levels((rle_t*)rleaddr);

//    free(rleaddr);
}

int rle_type_read_from_pkt(void *offset, void *rle) {

    // XXX: to implement
    return(0);
}

char *rle_type_to_char(void *rle) {
    static char buf[INET6_ADDRSTRLEN*2+4];
    /* XXX: to implement */
    return(buf);
}

void rle_type_copy(void **dst, void *src) {
    /* XXX: to implement */
}

