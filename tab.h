#ifndef __TAB_TAB_H
#define __TAB_TAB_H

#include <sys/time.h>
#include <string>
#include <iostream>

struct bm_scope {
    double sum;
    std::string name;
    
    bm_scope(const std::string& n) : sum(0), name(n) {}

    ~bm_scope() {
        std::cerr << name << " : " << sum << std::endl;
    }
};

struct bm {
    struct timeval b;
    double& _s;

    bm(bm_scope& s) : _s(s.sum) {
        //gettimeofday(&b, NULL);
    }

    ~bm() {
        //struct timeval e;
        //gettimeofday(&e, NULL);
        //size_t a = (e.tv_sec*1e6 + e.tv_usec);
        //size_t q = (b.tv_sec*1e6 + b.tv_usec);
        //_s += ((double)a-(double)q)/1e6;
    }
};


#include "deps.h"
#include "atom.h"
#include "type.h"
#include "command.h"
#include "infer.h"
#include "parse.h"
#include "object.h"
#include "funcs.h"
#include "file.h"
#include "exec.h"

#endif
