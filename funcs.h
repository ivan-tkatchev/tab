#ifndef __TAB_FUNCS_H
#define __TAB_FUNCS_H

namespace tab {

namespace funcs {

#include "funcs/index.h"
#include "funcs/flatten.h"
#include "funcs/filter.h"
#include "funcs/explode.h"
#include "funcs/count.h"
#include "funcs/math.h"
#include "funcs/head.h"
#include "funcs/cutgrep.h"
#include "funcs/zip.h"
#include "funcs/combo.h"
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
#include "funcs/uniques.h"
#include "funcs/url.h"
#include "funcs/unflatten.h"

} // namespace funcs

template <bool SORTED>
void register_functions(size_t seed) {

    funcs::get_rand_generator(seed);

    Functions& funs = functions_init();

    funs.add_seqmaker(obj::make_seq_from<SORTED>);

    funcs::register_index<SORTED>(funs);
    funcs::register_flatten<SORTED>(funs);
    funcs::register_filter(funs);
    funcs::register_explode(funs);
    funcs::register_count<SORTED>(funs);
    funcs::register_math(funs);
    funcs::register_head<SORTED>(funs);
    funcs::register_cutgrep(funs);
    funcs::register_zip<SORTED>(funs);
    funcs::register_combo(funs);
    funcs::register_file(funs);
    funcs::register_sum(funs);
    funcs::register_minmax(funs);
    funcs::register_avg(funs);
    funcs::register_if<SORTED>(funs);
    funcs::register_array<SORTED>(funs);
    funcs::register_map<SORTED>(funs);
    funcs::register_sort<SORTED>(funs);
    funcs::register_reverse(funs);
    funcs::register_rand(funs);
    funcs::register_misc(funs);
    funcs::register_ngram(funs);
    funcs::register_time(funs);
    funcs::register_hist(funs);
    funcs::register_uniques(funs);
    funcs::register_url(funs);
    funcs::register_unflatten(funs);
}

} // namespace tab

#endif
