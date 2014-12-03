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


#endif
