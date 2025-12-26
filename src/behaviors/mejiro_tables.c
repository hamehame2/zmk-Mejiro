#include <stddef.h>
#include "mejiro/mejiro_tables.h"

/* users / abstract / verbs は今は空でOK（Lookup順序の器だけ用意） */
const struct mj_kv mj_users[] = {};
const size_t mj_users_len = 0;

const struct mj_kv mj_abstract[] = {};
const size_t mj_abstract_len = 0;

const struct mj_kv mj_verbs[] = {};
const size_t mj_verbs_len = 0;

/* commands: mejiro_commands.json より */
const struct mj_kv mj_commands[] = {
    {"#", "{`"},
    {"k#", "{^}"},
    {"n#", "{^}"},
    {"t#", "{^}"},
    {"tk#", "{^}"},
    {"-T", "{"},
    {"-Y", "{&}"},
    {"-n", "{Enter}"},
    {"-t", "{Tab}"},
};
const size_t mj_commands_len = sizeof(mj_commands) / sizeof(mj_commands[0]);
