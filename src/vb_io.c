#include <string.h>
#include <stdlib.h>

#include "vb_internal.h"

/* -------------------------------------------------- */
/* Length field helpers (little-endian, defined)      */
/* -------------------------------------------------- */

static inline void
vb_write_len(uint8_t *p, uint32_t len, vb_lenfmt_t fmt)
{
    p[0] = (uint8_t)(len & 0xFF);
    p[1] = (uint8_t)((len >> 8) & 0xFF);

    if (fmt == VB_LEN32) {
        p[2] = (uint8_t)((len >> 16) & 0xFF);
        p[3] = (uint8_t)((len >> 24) & 0xFF);
    }
}

static inline uint32_t
vb_read_len(const uint8_t *p, vb_lenfmt_t fmt)
{
    uint32_t len = (uint32_t)p[0]
                 | ((uint32_t)p[1] << 8);

    if (fmt == VB_LEN32) {
        len |= ((uint32_t)p[2] << 16)
            |  ((uint32_t)p[3] << 24);
    }
    return len;
}

/* -------------------------------------------------- */
/* Block flush (write side)                            */
/* -------------------------------------------------- */

static int
vb_flush(vb_handle_t *vb)
{
    if (vb->block_used == 0)
        return 0;

    fwrite(&vb->block_used, 4, 1, vb->fp);   /* Output Blocksize */  

    if (fwrite(vb->block_buf, 1, vb->block_used, vb->fp) != vb->block_used) {
        vb->error = 1;
        return -1;
    }

    vb->block_used = 0;
    return 0;
}

/* -------------------------------------------------- */
/* Write a record                                     */
/* -------------------------------------------------- */

int
VB_Put(vb_handle_t *vb, const void *data, uint32_t len)
{
    uint8_t lenbuf[4];
    uint32_t lsz = (vb->lenfmt == VB_LEN16) ? 2 : 4;

    vb_write_len(lenbuf, len, vb->lenfmt);

    /* Unblocked */
    if (vb->block_size == 0) {
        if (fwrite(lenbuf, lsz, 1, vb->fp) != 1) return -1;
        if (fwrite(data, 1, len, vb->fp) != len) return -1;
        return 1;
    }

    /* Blocked */
    if (lsz + len > vb->block_size)
        return -1;  /* record too large */

    if (vb->block_used + lsz + len > vb->block_size) {
        if (vb_flush(vb) < 0)
            return -1;
    }

    memcpy(vb->block_buf + vb->block_used, lenbuf, lsz);
    vb->block_used += lsz;

    memcpy(vb->block_buf + vb->block_used, data, len);
    vb->block_used += len;

    return 1;
}

/* -------------------------------------------------- */
/* Read a record                                      */
/* -------------------------------------------------- */

int
VB_Get(vb_handle_t *vb, void *buf, uint32_t max_len, uint32_t *out_len)
{
    uint8_t lenbuf[4];
    uint32_t lsz = (vb->lenfmt == VB_LEN16) ? 2 : 4;

    /* Unblocked */
    if (vb->block_size == 0) {
        if (fread(lenbuf, lsz, 1, vb->fp) != 1)
            return 0;  /* EOF */

        uint32_t len = vb_read_len(lenbuf, vb->lenfmt);
        if (len > max_len)
            return -1;

        if (fread(buf, 1, len, vb->fp) != len)
            return -1;

        *out_len = len;
        return 1;
    }

    /* Blocked */
    if (vb->block_pos + lsz > vb->block_used) {
        fread(&vb->block_used, 4, 1, vb->fp);   /* Input Blocksize */  
        vb->block_used = fread(vb->block_buf, 1,
                               vb->block_used, vb->fp);
        vb->block_pos = 0;
        if (vb->block_used == 0)
            return 0;
    }

    memcpy(lenbuf, vb->block_buf + vb->block_pos, lsz);
    vb->block_pos += lsz;

    uint32_t len = vb_read_len(lenbuf, vb->lenfmt);
    if (len > max_len)
        return -1;

    memcpy(buf, vb->block_buf + vb->block_pos, len);
    vb->block_pos += len;

    *out_len = len;
    return 1;
}

/* -------------------------------------------------- */
/* Close                                              */
/* -------------------------------------------------- */

int
VB_Close(vb_handle_t *vb)
{
    if (!vb)
        return 0;

    if (vb->mode == VB_MODE_WRITE && vb->block_size)
        vb_flush(vb);

    fclose(vb->fp);
    free(vb->block_buf);
    free(vb);
    return 0;
}
