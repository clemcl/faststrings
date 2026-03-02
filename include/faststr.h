#ifndef FASTSTR_H
#define FASTSTR_H

#include <stdint.h>
#include <string.h>

typedef struct {
    uint32_t max_len;
    uint32_t cur_len;
} vb_meta_t;

/* Declaration */
#define DCL(name, size) \
    char name[size]; \
    vb_meta_t dv_##name = { (size) - 1, 0 }

/* Length */
#define LEN(name) (dv_##name.cur_len)

/* Internal copy implementation */
static inline void
vb_cpy_impl(char *dst,
            vb_meta_t *dmeta,
            const char *src,
            const vb_meta_t *smeta)
{
    uint32_t len = smeta->cur_len;

    if (len > dmeta->max_len)
        len = dmeta->max_len;

    memcpy(dst, src, len);
    dst[len] = '\0';

    dmeta->cur_len = len;
}

/* Copy macro */
#define CPY(dst, src) \
    vb_cpy_impl(dst, &dv_##dst, src, &dv_##src)

/* Set literal */
static inline void
vb_set_literal(char *dst,
               vb_meta_t *dmeta,
               const char *lit)
{
    uint32_t len = (uint32_t)strlen(lit);

    if (len > dmeta->max_len)
        len = dmeta->max_len;

    memcpy(dst, lit, len);
    dst[len] = '\0';

    dmeta->cur_len = len;
}

#define SET(dst, literal) \
    vb_set_literal(dst, &dv_##dst, literal)

/* Concatenate */
static inline void
vb_cat_impl(char *dst,
            vb_meta_t *dmeta,
            const char *src,
            const vb_meta_t *smeta)
{
    uint32_t space = dmeta->max_len - dmeta->cur_len;
    uint32_t add   = smeta->cur_len;

    if (add > space)
        add = space;

    memcpy(dst + dmeta->cur_len, src, add);

    dmeta->cur_len += add;
    dst[dmeta->cur_len] = '\0';
}

#define CAT(dst, src) \
    vb_cat_impl(dst, &dv_##dst, src, &dv_##src)

/* Compare */
static inline int
vb_cmp_impl(const char *a,
            const vb_meta_t *am,
            const char *b,
            const vb_meta_t *bm)
{
    uint32_t min = am->cur_len < bm->cur_len
                 ? am->cur_len
                 : bm->cur_len;

    int r = memcmp(a, b, min);
    if (r != 0)
        return r;

    if (am->cur_len < bm->cur_len) return -1;
    if (am->cur_len > bm->cur_len) return 1;
    return 0;
}

#define CMP(a,b) \
    vb_cmp_impl(a, &dv_##a, b, &dv_##b)

#endif
