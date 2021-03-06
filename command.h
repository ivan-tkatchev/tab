#ifndef __TAB_COMMAND_H
#define __TAB_COMMAND_H

namespace tab {

namespace obj {

struct Object;

} // namespace obj

struct Command {

    enum cmd_t {
        VAL,
        VAW,
        VAR,

        EXP,

        MUL_I,
        MUL_R,
        DIV_I,
        DIV_R,
        MOD,
        ADD_I,
        ADD_R,
        SUB_I,
        SUB_R,

        NOT,
        AND,
        OR,
        XOR,

        I2R_1,
        I2R_2,
        U2R_1,
        U2R_2,

        EQ,
        LT,
        NEG,
        ROT,
        
        ARR,
        MAP,
        FUN,
        FUN0,
        SEQ,
        TUP,
        GEN,
        GEN_TRY,
        REC,
        LAMD
    };

    cmd_t cmd;

    Atom arg;

    struct Closure {
        std::vector<Command> code;

        void swap(Closure& c) {
            code.swap(c.code);
        }
    };
    
    std::vector< Closure > closure;

    Type type;
    obj::Object* object;
    void* function;
    
    Command(cmd_t c = VAL) : cmd(c), object(nullptr), function(nullptr) {}

    template <typename T>
    Command(cmd_t c, const T& t) : cmd(c), arg(t), object(nullptr), function(nullptr) {}

    static std::string print(cmd_t c) {
        switch (c) {
        case VAL: return "VAL";
        case VAW: return "VAW";
        case VAR: return "VAR";

        case EXP: return "EXP";
        case MUL_I: return "MUL_I";
        case MUL_R: return "MUL_R";
        case DIV_I: return "DIV_I";
        case DIV_R: return "DIV_R";
        case MOD: return "MOD";
        case ADD_I: return "ADD_I";
        case ADD_R: return "ADD_R";
        case SUB_I: return "SUB_I";
        case SUB_R: return "SUB_R";

        case NOT: return "NOT";
        case AND: return "AND";
        case  OR: return "OR";
        case XOR: return "XOR";
            
        case I2R_1: return "I2R_1";
        case I2R_2: return "I2R_2";
        case U2R_1: return "U2R_1";
        case U2R_2: return "U2R_2";

        case EQ: return "EQ";
        case LT: return "LT";
        case NEG: return "NEG";
        case ROT: return "ROT";
            
        case ARR: return "ARR";
        case MAP: return "MAP";
        case FUN: return "FUN";
        case FUN0: return "FUN0";
        case SEQ: return "SEQ";
        case TUP: return "TUP";
        case GEN: return "GEN";
        case GEN_TRY: return "GEN_TRY";
        case REC: return "REC";
        case LAMD: return "LAMD";
        }
        return ":~(";
    }
};

} // namespace tab

#endif
