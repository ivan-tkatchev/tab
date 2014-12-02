#ifndef __TAB_FUNCS_H
#define __TAB_FUNCS_H

namespace funcs {

#include "funcs/index.h"
#include "funcs/flatten.h"
#include "funcs/filter.h"
#include "funcs/count.h"
#include "funcs/math.h"
#include "funcs/head.h"
#include "funcs/cutgrep.h"
#include "funcs/zip.h"
#include "funcs/file.h"
#include "funcs/sum.h"
#include "funcs/minmax.h"
#include "funcs/avg.h"
#include "funcs/if.h"
#include "funcs/array.h"
#include "funcs/sort.h"
#include "funcs/misc.h"

}

void register_functions() {

    Functions& funs = functions_init();

    funs.add_seqmaker(obj::make_seq_from);

    funcs::register_index(funs);
    funcs::register_flatten(funs);
    funcs::register_filter(funs);
    funcs::register_count(funs);
    funcs::register_math(funs);
    funcs::register_head(funs);
    funcs::register_cutgrep(funs);
    funcs::register_zip(funs);
    funcs::register_sum(funs);
    funcs::register_minmax(funs);
    funcs::register_avg(funs);
    funcs::register_if(funs);
    funcs::register_array(funs);
    funcs::register_sort(funs);
    funcs::register_misc(funs);
}

#endif
