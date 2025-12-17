#include "mejiro_tables.h"

const struct mj_kv mj_commands[] = {
    {"-SKNA", "{^\\{\\}^}{#Left}{^}"},
    {"-STYI", "{^<>^}{#Left}{^}"},
    {"-KNA", "{^[]^}{#Left}{^}"},
    {"-SKN", "{^\\}^}"},
    {"-SNA", "{^\\{^}"},
    {"-STY", "{^>^}"},
    {"-SYI", "{^<^}"},
    {"-TYI", "{^()^}{#Left}{^}"},
    {"-AU", "{#BackSpace}"},
    {"-IU", "{#Delete}"},
    {"-KN", "{^]^}"},
    {"-KY", "{^%^}"},
    {"-NA", "{^[^}"},
    {"-TN", "{^/^}"},
    {"-TY", "{^)^}"},
    {"-YI", "{^(^}"},
    //{"tk#", "{PLOVER:TOGGLE}"},
    {"#k", "{#F13}"},
    {"#t", "{#F14}"},
    {"-A", "{^}{#Left}{^}"},
    {"-I", "{^}{#Home}{^}"},
    {"-K", "{^}{#Right}{^}"},
    {"-N", "{^}{#Down}{^}"},
    {"-S", "{#Escape}"},
    {"-T", "{^}{#End}{^}"},
    {"-U", "=undo"},
    {"-Y", "{^}{#Up}{^}"},
    //{"t#", "{PLOVER:SWITCH_SYSTEM:English Stenotype}{#F13}{^}"},
    {"#", "=repeat_last_translation"}
};
const size_t mj_commands_len = sizeof(mj_commands) / sizeof(mj_commands[0]);

/* Placeholders: keep linker happy even if core references them. */
const struct mj_kv mj_users[] = { };
const size_t mj_users_len = 0;

const struct mj_kv mj_abstract[] = { };
const size_t mj_abstract_len = 0;

const struct mj_kv mj_verbs[] = { };
const size_t mj_verbs_len = 0;
