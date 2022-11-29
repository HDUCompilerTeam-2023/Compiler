#include <parser.h>
#include <stdlib.h>

#include <frontend/syntaxtree_printer.h>

pExtraInfo frontend_init_extra() {
    pExtraInfo ret = malloc(sizeof(*ret));
    *ret = (ExtraInfo) { .root = NULL };
    return ret;
}

void frontend_drop_extra(pExtraInfo ExtraInfo) {
    if (!ExtraInfo) return;
    if (ExtraInfo->root) {
        frontend_print_syntaxtree(ExtraInfo->root);
        frontend_drop_syntaxtree(ExtraInfo->root);
    }
    free(ExtraInfo);
}