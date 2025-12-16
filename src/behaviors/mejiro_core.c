#include "mejiro/mejiro_core.h"
#include "mejiro/mejiro_tables.h"

#include <string.h>
#include <ctype.h>
#include <stdio.h>

#include <zephyr/logging/log.h>
#include <zmk/events/keycode_state_changed.h>

typedef struct { uint32_t keycode; const char *sym; } mj_keymap_ent;

/* Edit-only-here: keycode -> Mejiro symbol mapping (NO keymap/DTS changes). */
static const mj_keymap_ent mj_keymap[] = {
    /* left thumb */
    { 'C', "n" }, { 'V', "t" }, { 'B', "k" },
    /* right thumb */
    { ',', "nR" }, { 'M', "tR" }, { 'N', "kR" },
};
static const size_t mj_keymap_len = sizeof(mj_keymap) / sizeof(mj_keymap[0]);

static const char *lookup_kv(const struct mj_kv *tbl, size_t n, const char *k) {
    for (size_t i = 0; i < n; i++) if (strcmp(tbl[i].k, k) == 0) return tbl[i].v;
    return NULL;
}

static bool map_keycode_to_symbol(uint32_t keycode, const char **out_sym) {
    for (size_t i = 0; i < mj_keymap_len; i++) {
        if (mj_keymap[i].keycode == keycode) { *out_sym = mj_keymap[i].sym; return true; }
    }
    return false;
}

/* Minimal builder for ntk only. Extend later to STKN/YIAU/#/* when you add mapping entries. */
static bool build_stroke_from_nglist(const NGList *chord, char *out, size_t out_sz) {
    char l[4] = {0}, r[4] = {0};
    for (int i = 0; i < chord->size; i++) {
        const char *sym = NULL;
        if (!map_keycode_to_symbol(chord->elements[i], &sym)) return false;
        if (strcmp(sym, "n") == 0 || strcmp(sym, "t") == 0 || strcmp(sym, "k") == 0) {
            if (!strchr(l, sym[0])) { size_t x=strlen(l); if (x<3){ l[x]=sym[0]; l[x+1]=0; } }
        } else if (strcmp(sym, "nR") == 0 || strcmp(sym, "tR") == 0 || strcmp(sym, "kR") == 0) {
            char c = sym[0];
            if (!strchr(r, c)) { size_t x=strlen(r); if (x<3){ r[x]=c; r[x+1]=0; } }
        }
    }
    const char order[4]="ntk";
    char lc[4]={0}, rc[4]={0};
    for(int i=0;i<3;i++){ if(strchr(l,order[i])) strncat(lc,&order[i],1); }
    for(int i=0;i<3;i++){ if(strchr(r,order[i])) strncat(rc,&order[i],1); }
    snprintf(out,out_sz,"%s-%s",lc,rc);
    return true;
}

static void tap_keycode(uint32_t kc, int64_t timestamp) {
    raise_zmk_keycode_state_changed_from_encoded(kc, true, timestamp);
    raise_zmk_keycode_state_changed_from_encoded(kc, false, timestamp);
}

static void emit_ascii(const char *s, int64_t timestamp) {
    for (const unsigned char *p=(const unsigned char*)s; *p; p++) {
        unsigned char c=*p;
        if (c>='A'&&c<='Z') c = (unsigned char)tolower(c); /* keep simple */
        if ((c>='a'&&c<='z') || c==' ' || c=='.' || c==',' || c=='/' || c=='-' || c=='\'') {
            tap_keycode((uint32_t)c, timestamp);
        }
    }
}

static void emit_utf8_as_romaji(const char *utf8, int64_t timestamp) {
    const char *p = utf8;
    while (*p) {
        const char *best_v = NULL;
        size_t best_len = 0;
        for (size_t i = 0; i < mj_hepburn_len; i++) {
            size_t kl = strlen(mj_hepburn[i].k);
            if (kl > best_len && strncmp(p, mj_hepburn[i].k, kl) == 0) {
                best_len = kl;
                best_v = mj_hepburn[i].v;
            }
        }
        if (best_v) {
            emit_ascii(best_v, timestamp);
            p += best_len;
        } else {
            /* skip unknown UTF-8 byte */
            p++;
        }
    }
}

static void emit_command_string(const char *cmd, int64_t timestamp) {
    if (!cmd || cmd[0] == '=') return;
    /* supports {#BackSpace} only in this minimal version */
    if (strcmp(cmd, "{#BackSpace}") == 0) { tap_keycode(0x08, timestamp); return; }
    if (strcmp(cmd, "{#Return}") == 0) { tap_keycode(0x0D, timestamp); return; }
    if (strcmp(cmd, "{#Space}") == 0) { tap_keycode(' ', timestamp); return; }
}

static const struct mj_verb_ent *find_verb(const struct mj_verb_ent *tbl, size_t n, const char *stroke) {
    for (size_t i = 0; i < n; i++) if (strcmp(tbl[i].stroke, stroke) == 0) return &tbl[i];
    return NULL;
}

bool mejiro_try_emit_from_nginput(const NGListArray *nginput, int64_t timestamp) {
    if (!nginput || nginput->size <= 0) return false;

    /* validate whole nginput first (avoid partial emits) */
    for (int i=0;i<nginput->size;i++){
        char stroke[64];
        if (!build_stroke_from_nglist(&nginput->elements[i], stroke, sizeof(stroke))) return false;
    }

    for (int i=0;i<nginput->size;i++){
        char stroke[64];
        build_stroke_from_nglist(&nginput->elements[i], stroke, sizeof(stroke));

        const char *cmd = lookup_kv(mj_commands, mj_commands_len, stroke);
        if (cmd) { emit_command_string(cmd, timestamp); continue; }

        const char *abbr = lookup_kv(mj_users, mj_users_len, stroke);
        if (!abbr) abbr = lookup_kv(mj_abstract, mj_abstract_len, stroke);
        if (abbr) { emit_utf8_as_romaji(abbr, timestamp); continue; }

        const struct mj_verb_ent *v = find_verb(mj_verb_godan, mj_verb_godan_len, stroke);
        if (!v) v = find_verb(mj_verb_kami, mj_verb_kami_len, stroke);
        if (!v) v = find_verb(mj_verb_simo, mj_verb_simo_len, stroke);
        if (v) { emit_utf8_as_romaji(v->stem, timestamp); continue; }

        LOG_DBG("MEJIRO no-op stroke=%s", stroke);
    }

    return true;
}

LOG_MODULE_REGISTER(mejiro, CONFIG_ZMK_LOG_LEVEL);
