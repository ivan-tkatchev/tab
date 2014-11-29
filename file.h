#ifndef __TAB_FILE_H
#define __TAB_FILE_H

struct Linereader {

    std::istream& infile;

    char bufb[64*1024];
    char* bufe;
    char* bufi;
    char* bufi_p;
    bool done;

    bool did_init;
    
    Linereader(std::istream& i) :
        infile(i), bufe(bufb + sizeof(bufb)), bufi(bufe), bufi_p(bufi), done(false), did_init(false)
        {}

    void populate() {

        infile.read(bufb, bufe - bufb);
        bufi = bufb;
        bufi_p = bufi;

        if (!infile) {
            done = true;
            bufe = bufb + infile.gcount();
        }
    }
    
    void getline(std::string& s, bool& ok) {

        if (!did_init) {
            populate();
            did_init = true;
        }
        
        s.clear();
        
        if (done && bufi == bufe) {
            ok = false;
            return;
        }

        ok = true;
        bool stop = false;
        while (!stop) {

            if (*bufi == '\n') {
                s.append(bufi_p, bufi);
                bufi_p = bufi + 1;
                stop = true;
            }

            ++bufi;

            if (bufi == bufe) {
                s.append(bufi_p, bufi);
                populate();

                if (done && bufi == bufe) {
                    ok = false;
                    stop = true;
                }
            }
        }
    }
};

struct SeqFile : public obj::SeqBase {

    obj::String* holder;
    Linereader reader;
    
    SeqFile(std::istream& infile) : reader(infile) {
        holder = new obj::String;
    }

    obj::Object* next(bool& ok) {
        reader.getline(holder->v, ok);
        return holder;
    }
};


#endif
