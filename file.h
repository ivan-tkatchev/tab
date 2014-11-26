#ifndef __TAB_FILE_H
#define __TAB_FILE_H

struct Linereader {

    std::istream& infile;

    char bufb[64*1024];
    char* bufe;
    char* bufi;
    bool done;

    Linereader(std::istream& i) : infile(i), bufe(bufb + sizeof(bufb)), bufi(bufe), done(false)
        {
            populate();
        }

    void populate() {

        infile.read(bufb, bufe - bufb);
        bufi = bufb;

        if (!infile) {
            done = true;
            bufe = bufb + infile.gcount();
        }
    }
    
    void getline(std::string& s, bool& ok) {

        s.clear();

        if (done && bufi == bufe) {
            ok = false;
            return;
        }

        ok = true;
        bool stop = false;
        while (!stop) {

            if (*bufi == '\n') {
                stop = true;

            } else {
                s += *bufi;
            }

            ++bufi;

            if (bufi == bufe) {
                populate();

                if (done && bufi == bufe) {
                    ok = false;
                    stop = true;
                }
            }
        }
    }
};

namespace obj {

struct SequencerFile : public Sequencer {

    Linereader reader;
    
    SequencerFile(std::istream& infile) : reader(infile) {

        holder = new String;

        v = [this](Object* i, bool& ok) {

            String& x = get<String>(i);
            reader.getline(x.v, ok);

            return i;
        };
    }
};

}

#endif
