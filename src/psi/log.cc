/*!
 * \file log.cc
 * \brief Implementation of PSI Logging System.
 */

#include <iostream>
#include <fstream>

#include "log.h"

typedef std::map<psi::colors::Color, std::string> ColorMap;

static ColorMap MakeColorMap()
{
    using namespace psi::colors;
    ColorMap m;
    m[Default] = "\033[0m";
    m[Black] = "\033[30m";
    m[BlackB] = "\033[1;30m";
    m[Red] = "\033[31m";
    m[RedB] = "\033[1;31m";
    m[Green] = "\033[32m";
    m[GreenB] = "\033[1;32m";
    m[Yellow] = "\033[33m";
    m[YellowB] = "\033[1;33m";
    m[Blue] = "\033[34m";
    m[BlueB] = "\033[1;34m";
    m[Pink] = "\033[35m";
    m[PinkB] = "\033[1;35m";
    m[Cyan] = "\033[36m";
    m[CyanB] = "\033[1;36m";
    m[White] = "\033[37m";
    m[WhiteB] = "\033[1;37m";
    return m;
}

std::string psi::log::detail::ConsoleCommand::MakeString(const colors::Color& c)
{
    static const ColorMap map = MakeColorMap();
    ColorMap::const_iterator iter = map.find(c);
    if(iter != map.end())
        return iter->second;
    return "";
}

void psi::log::detail::ConsoleWriter::Write_cout(const std::string& str)
{
    std::cout << str;
}

void psi::log::detail::ConsoleWriter::Write_cerr(const std::string& str)
{
    std::cerr << str;
}

void psi::log::detail::LogBaseImpl::open(const std::string& fileName)
{
    file = boost::shared_ptr<std::ostream>(new std::ofstream(fileName.c_str()));
}

void psi::log::detail::LogBaseImpl::write(const std::string& logString)
{
    if(file) {
        (*file) << logString;
        file->flush();
    }
}
