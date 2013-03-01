/*!
 * \file PsiShell.cc
 * \brief Implementation of psi::Shell class.
 *
 * \author Konstantin Androsov <konstantin.androsov@gmail.com>
 *
 * \b Changelog
 * 28-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - First version.
 */

#include <boost/algorithm/string.hpp>
#include <readline/readline.h>
#include <readline/history.h>

#include "psi/log.h"
#include "PsiShell.h"

using namespace psi::control;

Shell::Shell(const std::string& aHistoryFileName)
    : historyFileName(aHistoryFileName), prompt("psi46expert> "), runNext(true)
{
    rl_bind_key('\t', &rl_insert);
    read_history(historyFileName.c_str());
}

Shell::~Shell()
{
    write_history(historyFileName.c_str());
}

void Shell::Run()
{
    std::cout << "Please enter a command or 'help' to see a list of the available commands." << std::endl;

    while(runNext)
    {
        const std::string p = ReadLine();
        psi::Log<psi::Debug>() << "psi46expert> " << p << std::endl;
        std::vector<std::string> commandLineArguments;
        boost::algorithm::split(commandLineArguments, p, boost::algorithm::is_any_of(" "),
                                boost::algorithm::token_compress_on);
        boost::shared_ptr<Command> command;
        const bool result = FindAndCreateCommand(*this, commandLineArguments, command);
        if(result)
        {
            try
            {
                command->Execute();
            }
            catch(incorrect_command_exception&)
            {

            }
        }
    }
}

std::string Shell::ReadLine()
{
    char* line = readline (prompt.c_str());
    if (line && *line)
        add_history (line);
    const std::string str = std::string(line);
    free(line);
    return str;
}

void Shell::Execute(const commands::Exit&)
{
    std::cout << "Exiting.." << std::endl;
    runNext = false;
}

void Shell::Execute(const commands::Help&)
{
    std::cout << "List of the available commands:" << std::endl;
    std::cout << "exit - exit from the program." << std::endl;
    std::cout << "help - print out this list." << std::endl;
    std::cout << "IV - run an IV test." << std::endl;
}
