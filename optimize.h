#ifndef __TAB_OPTIMIZE_H
#define __TAB_OPTIMIZE_H

namespace tab {

namespace {

struct var_access {
    size_t reads;
    size_t writes;
    size_t reads_after_write;
};

void unneeded_variable(std::vector<Command>& commands, size_t var, var_access& ret) {

    if (commands.empty())
        return;

    // Find unnecessary variables and remove them.
    // 'Unnecessary' means variables that are accessed only once, immediately after assignment.

    size_t i = commands.size();
    while (i > 0) {

        --i;
        auto& cmd = commands[i];

        for (auto& j : cmd.closure) {
            unneeded_variable(j.code, var, ret);
        }

        if (cmd.cmd == Command::VAR && cmd.arg.uint == var) {

            ret.reads++;

            if (i > 0) {
                auto& cmdprev = commands[i - 1];

                if (cmdprev.cmd == Command::VAW && cmdprev.arg.uint == var) {
                    ret.reads_after_write++;
                }
            }

        } else if (cmd.cmd == Command::VAW && cmd.arg.uint == var) {

            ret.writes++;
        }
    }
}

void remove_variable(std::vector<Command>& commands, size_t var) {

    std::vector<Command> ret;
    ret.reserve(commands.size());

    for (auto& cmd : commands) {

        for (auto& j : cmd.closure) {
            remove_variable(j.code, var);
        }

        bool bad = ((cmd.cmd == Command::VAR || cmd.cmd == Command::VAW) && cmd.arg.uint == var);

        if (!bad) {
            ret.push_back(cmd);
        }
    }

    commands.swap(ret);
}

}

#include <iostream>

void optimize(std::vector<Command>& commands, const TypeRuntime& typer) {

    if (commands.empty())
        return;

    for (size_t var = 0; var < typer.num_vars(); ++var) {

        var_access va{0, 0, 0};

        unneeded_variable(commands, var, va);

        if (va.reads == 1 && va.writes == 1 && va.reads_after_write == 1) {
            remove_variable(commands, var);
        }
    }
}

}

#endif
