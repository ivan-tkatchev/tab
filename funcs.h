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
#include "funcs/map.h"
#include "funcs/sort.h"
#include "funcs/reverse.h"
#include "funcs/rand.h"
#include "funcs/misc.h"
#include "funcs/ngram.h"
#include "funcs/time.h"
#include "funcs/hist.h"

}

void register_functions(size_t seed) {

    funcs::get_rand_generator(seed);

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
    funcs::register_file(funs);
    funcs::register_sum(funs);
    funcs::register_minmax(funs);
    funcs::register_avg(funs);
    funcs::register_if(funs);
    funcs::register_array(funs);
    funcs::register_map(funs);
    funcs::register_sort(funs);
    funcs::register_reverse(funs);
    funcs::register_rand(funs);
    funcs::register_misc(funs);
    funcs::register_ngram(funs);
    funcs::register_time(funs);
    funcs::register_hist(funs);
}

#endif
