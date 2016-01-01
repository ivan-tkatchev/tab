#ifndef __TAB_FUNCS_FILE_H
#define __TAB_FUNCS_FILE_H

struct Linereader {

    std::istream& infile;

    char bufb[64*1024];
    char* bufe;
    char* bufi;
    char* bufi_p;

    Linereader(std::istream& i) :
        infile(i), bufe(bufb + sizeof(bufb)), bufi(bufe), bufi_p(bufi)
        {}

    void populate() {

        infile.read(bufb, bufe - bufb);
        bufi = bufb;
        bufi_p = bufi;

        if (!infile) {
            bufe = bufb + infile.gcount();
        }
    }
    
    bool getline(std::string& s) {
        
        s.clear();

        if (bufi == bufe) {
            populate();
        }

        if (bufi == bufe) {
            return false;
        }
        
        while (1) {

            if (*bufi == '\n') {
                s.append(bufi_p, bufi);
                ++bufi;
                bufi_p = bufi;
                return true;
            }

            ++bufi;

            if (bufi == bufe) {
                s.append(bufi_p, bufi);
                populate();

                if (bufi == bufe) {
                    return !(s.empty());
                }
            }
        }

        return true;
    }
};

struct SeqFile : public obj::SeqBase {

    obj::String* holder;
    Linereader reader;
    
    SeqFile(std::istream& infile) : reader(infile) {
        holder = new obj::String;
    }

    ~SeqFile() {
        delete holder;
    }

    obj::Object* next() {
        bool ok = reader.getline(holder->v);

        if (!ok) return nullptr;

        return holder;
    }
};

struct SeqFileV : public obj::SeqBase {

    obj::String* holder;
    Linereader* reader;
    std::ifstream file;

    SeqFileV() {
        holder = new obj::String;
        reader = nullptr;
    }

    ~SeqFileV() {
        delete holder;
        delete reader;
    }

    void open(const std::string& fname) {

        if (file.is_open())
            file.close();

        file.open(fname);

        if (!file)
            throw std::runtime_error("Could not open input file: " + fname);

        if (reader)
            delete reader;

        reader = new Linereader(file);
    }

    obj::Object* next() {
        bool ok = reader->getline(holder->v);

        if (!ok) return nullptr;

        return holder;
    }
};


void file(const obj::Object* in, obj::Object*& out) {

    const std::string& fname = obj::get<obj::String>(in).v;
    SeqFileV& file = obj::get<SeqFileV>(out);

    file.open(fname);
}

Functions::func_t file_checker(const Type& args, Type& ret, obj::Object*& obj) {

    if (!check_string(args))
        return nullptr;

    ret = Type(Type::SEQ);
    ret.push(Type::STRING);

    obj = new SeqFileV;
    
    return file;
}

void register_file(Functions& funcs) {

    funcs.add_poly("file", file_checker);
    funcs.add_poly("open", file_checker);
}

#endif
