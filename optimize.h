#ifndef __TAB_OPTIMIZE_H
#define __TAB_OPTIMIZE_H

namespace tab {

void optimize(std::vector<Command>& commands, const TypeRuntime& typer) {

    if (commands.empty())
        return;

    // Find unnecessary variables and remove them.
    // 'Unnecessary' means variables that are accessed only once, immediately after assignment.

    std::vector<size_t> var_reads;

    var_reads.resize(typer.num_vars());

    std::unordered_set<size_t> to_erase;

    size_t i = commands.size();
    while (i > 1) {

        --i;
        auto& cmd = commands[i];

        for (auto& j : cmd.closure) {
            optimize(j.code, typer);
        }

        if (cmd.cmd != Command::VAR)
            continue;

        UInt var = cmd.arg.uint;
        var_reads[var]++;

        auto& cmdprev = commands[i - 1];

        if (cmdprev.cmd == Command::VAW && cmdprev.arg.uint == var && var_reads[var] == 1) {

            to_erase.insert(i);
            to_erase.insert(i - 1);
            var_reads[var]--;
        }
    }

    if (to_erase.empty())
        return;

    std::vector<Command> ret;
    ret.reserve(commands.size());

    for (size_t i = 0; i < commands.size(); ++i) {
        if (to_erase.count(i) == 0) {
            ret.push_back(commands[i]);
        }
    }

    commands.swap(ret);
}

}

#endif
