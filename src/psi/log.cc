/*!
 * \file log.cc
 * \brief Implementation of PSI Logging System.
 *
 * \b Changelog
 * 01-03-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - New thread safe implementation.
 */

#include "log.h"

#include <boost/date_time.hpp>

typedef std::map<psi::Color, std::string> ColorMap;

static ColorMap MakeColorMap()
{
    ColorMap m;
    m[psi::Black] = "\033[30m";
    m[psi::BlackB] = "\033[1;30m";
    m[psi::Red] = "\033[31m";
    m[psi::RedB] = "\033[1;31m";
    m[psi::Green] = "\033[32m";
    m[psi::GreenB] = "\033[1;32m";
    m[psi::Yellow] = "\033[33m";
    m[psi::YellowB] = "\033[1;33m";
    m[psi::Blue] = "\033[34m";
    m[psi::BlueB] = "\033[1;34m";
    m[psi::Pink] = "\033[35m";
    m[psi::PinkB] = "\033[1;35m";
    m[psi::Cyan] = "\033[36m";
    m[psi::CyanB] = "\033[1;36m";
    m[psi::White] = "\033[37m";
    m[psi::WhiteB] = "\033[1;37m";
    m[psi::DefaultColor] = "\033[0m";
    return m;
}

std::string psi::log::detail::ConsoleCommand::MakeString(const Color& c)
{
    static const ColorMap map = MakeColorMap();
    ColorMap::const_iterator iter = map.find(c);
    if(iter!= map.end())
        return iter->second;
    return "";
}

std::string psi::log::detail::DateTimeProvider::Now()
{
    boost::posix_time::ptime now = boost::date_time::microsec_clock<boost::posix_time::ptime>::local_time();
    return boost::posix_time::to_iso_extended_string(now);
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

void psi::log::detail::LogBaseImpl::write(const psi::log::detail::LogString& logString)
{
    if(!logString.isTerminalCommand && file)
        (*file) << logString.string;
}
