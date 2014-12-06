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

    ~SeqZip() {
        delete holder;
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

void zip_seq(const obj::Object* in, obj::Object*& out) {

    obj::Tuple& t = obj::get<obj::Tuple>(in);
    SeqZip& z = obj::get<SeqZip>(out);
    
    for (size_t i = 0; i < t.v.size(); ++i) {
        z.seqs[i] = t.v[i];
    }
}

void zip_val(const obj::Object* in, obj::Object*& out) {

    obj::Tuple& t = obj::get<obj::Tuple>(in);
    SeqZip& z = obj::get<SeqZip>(out);
    
    for (size_t i = 0; i < t.v.size(); ++i) {
        z.seqs[i]->wrap(t.v[i]);
    }
}

Functions::func_t zip_checker(const Type& args, Type& ret, obj::Object*& obj) {

    if (args.type != Type::TUP || !args.tuple)
        return nullptr;

    Type tmp(Type::TUP);

    Type::types_t type = Type::NONE;
    
    for (const Type& t : *(args.tuple)) {

        if (type == Type::NONE) {

            if (t.type != Type::SEQ && t.type != Type::ARR)
                return nullptr;

            type = t.type;

        } else if (t.type != type) {

            return nullptr;
        }

        tmp.push(t.tuple->at(0));
    }

    ret = Type(Type::SEQ);
    ret.push(tmp);

    size_t n = args.tuple->size();
    
    if (type == Type::SEQ) {
    
        obj = new SeqZip(n);        
        return zip_seq;

    } else if (type == Type::ARR) {

        obj = new SeqZip(n);
        SeqZip& sz = obj::get<SeqZip>(obj);

        for (size_t i = 0; i < n; ++i) {
            sz.seqs[i] = obj::make_seq_from(args.tuple->at(i));
        }

        return zip_val;
        
    } else {
        return nullptr;
    }
}


void register_zip(Functions& funcs) {

    funcs.add_poly("zip", zip_checker);
}


#endif
