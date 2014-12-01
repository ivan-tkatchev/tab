#ifndef __TAB_FUNCS_ZIP_H
#define __TAB_FUNCS_ZIP_H

struct SeqZip : public obj::SeqBase {

    obj::Tuple* holder;
    std::vector<obj::Object*> seqs;

    SeqZip(size_t n) {
        seqs.resize(n);
        holder = new obj::Tuple;
        holder->v.resize(n);
    }

    obj::Object* next() {

        for (size_t i = 0; i < seqs.size(); ++i) {

            obj::Object* o = seqs[i]->next();

            if (!o) return nullptr;

            holder->v[i] = o;
        }

        return holder;
    }
};

void zip(const obj::Object* in, obj::Object*& out) {

    obj::Tuple& t = obj::get<obj::Tuple>(in);
    SeqZip& z = obj::get<SeqZip>(out);
    
    for (size_t i = 0; i < t.v.size(); ++i) {
        z.seqs[i] = t.v[i];
    }
}

Functions::func_t zip_checker(const Type& args, Type& ret, obj::Object*& obj) {

    if (args.type != Type::TUP || !args.tuple)
        return nullptr;

    ret = Type(Type::TUP);
    
    for (const Type& t : *(args.tuple)) {

        if (t.type != Type::SEQ)
            return nullptr;

        ret.push(unwrap_seq(t));
    }

    ret = wrap_seq(ret);
    
    obj = new SeqZip(args.tuple->size());

    return zip;
}


void register_zip(Functions& funcs) {

    funcs.add_poly("zip", zip_checker);
}


#endif
