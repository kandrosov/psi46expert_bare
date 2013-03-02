/*!
 * \file log.h
 * \brief Definition of PSI Logging System.
 *
 * \b Changelog
 * 02-03-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Added Log::PrintTimestamp method.
 * 01-03-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - New thread safe implementation.
 *
 * Logging System Implementation. Defined Info, Error and Debug Logs:
 * Each of them dumps log messages into output file. It is possible to set
 * message head (for example class name where message was posted from).
 *
 * Differences:
 *   Log<Debug>   Dump all messages into file
 *   Log<Info>    print messages on screen via std::cout, dump into output
 *              Info file, dump into Debug
 *   Log<Error>   print messages on screen via std::cerr, dump into output
 *              Errorr file, dump into Debug
 *
 * Nothing will be dummped into file unless it is opened. File will be automatically closed
 * at the end of program.
 *
 * Examples using LogInfo. LogDebug and LogError work in the same way:
 *
 *   psi::Log<Info>().open( "info.log"); // set output filename: works only once
 *
 *   psi::Log<Info>() << "This is a message into Info Log" << std::endl;
 *
 *   psi::Log<Info>() << "Analog of previous line: not head is output" << std::endl;
 *
 *   psi::Log<Info>( "Head" ) << "This is a message with head" << std::endl;
 *
 *   psi::Log<Info>( __func__) << "Message from some function" << std::endl;
 *
 *   psi::Log<Info>( __PRETTY_FUNCTION__ ) << "Message from some function" << std::endl;
 *
 *   psi::Log<Info>( "Test1") << "Voltage: " << _voltage << std::endl;
 */

#pragma once

#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <list>

#include <boost/thread/mutex.hpp>
#include <boost/date_time.hpp>

namespace psi {

class Info;

/*!
 * Log<Debug> should be used for all debugging (intermediate) information:
 * voltages, currents, 'hello world's, 'I am here', etc. Its output is
 * stored in separate file and not displayed on Monitor. Very useful for
 * later review by experts.
 */
class Debug;

class Error;

/// Colors
enum Color { DefaultColor,
             Black , Red , Green , Yellow , Blue , Pink , Cyan , White ,
             BlackB, RedB, GreenB, YellowB, BlueB, PinkB, CyanB, WhiteB };

namespace log {
namespace detail {
struct LogString
{
    std::string string;
    bool isTerminalCommand;
    LogString() : string(""), isTerminalCommand(false) {}
    LogString(const std::string& _string, bool _isTerminalCommand)
        : string(_string), isTerminalCommand(_isTerminalCommand) {}
};

template<typename L>
struct LogWriter;

template<typename L>
class LogBase
{
public:
    static LogBase<L>& Singleton()
    {
        static LogBase<L> log;
        return log;
    }

    void open(const std::string& fileName)
    {
        boost::lock_guard<boost::mutex> lock(mutex);
        if(file.is_open())
            file.close();
        file.open(fileName.c_str());
    }

    template<typename Iterator>
    void write(const Iterator& begin, const Iterator& end)
    {
        boost::lock_guard<boost::mutex> lock(mutex);
        for(Iterator iter = begin; iter != end; ++iter)
        {
            LogWriter<L>::terminal_write(iter->string);
            if(!iter->isTerminalCommand)
            {
                if(file.is_open())
                    file << iter->string;
            }
        }
        LogWriter<L>::repeat_write(begin, end);
    }

private:
    LogBase() {}

    std::ofstream file;
    boost::mutex mutex;
};

struct ConsoleCommands
{
    static std::string GetString(const Color& c);
};

template<>
struct LogWriter<Debug>
{
    static void terminal_write(const std::string&) {}

    template<typename Iterator>
    static void repeat_write(const Iterator& begin, const Iterator& end) {}
};

template<>
struct LogWriter<Info>
{
    static void terminal_write(const std::string& str)
    {
        std::cout << str;
    }

    template<typename Iterator>
    static void repeat_write(const Iterator& begin, const Iterator& end)
    {
        LogBase<Debug>::Singleton().write(begin, end);
    }
};

template<>
struct LogWriter<Error>
{
    static void terminal_write(const std::string& str)
    {
        std::cerr << str;
    }

    template<typename Iterator>
    static void repeat_write(const Iterator& begin, const Iterator& end)
    {
        LogBase<Debug>::Singleton().write(begin, end);
    }
};

} // detail
} // log

template<typename L>
class Log
{
public:
    typedef std::ostream& (*ostream_manipulator)(std::ostream&);

    explicit Log() {}
    explicit Log(const std::string& head)
    {
        std::stringstream ss;
        ss << "[" << head << "] ";
        strings.push_back(log::detail::LogString(ss.str(), false));
    }

    ~Log() { log::detail::LogBase<L>::Singleton().write(strings.begin(), strings.end()); }

    void open(const std::string& fileName)
    {
        log::detail::LogBase<L>::Singleton().open(fileName);
    }

    template<typename T>
    Log& operator<<(const T& t)
    {
        std::stringstream ss;
        ss << t;
        strings.push_back(log::detail::LogString(ss.str(), false));
        return *this;
    }

    Log& operator<<(ostream_manipulator manipulator)
    {
        std::stringstream ss;
        ss << manipulator;
        strings.push_back(log::detail::LogString(ss.str(), false));
        return *this;
    }

    Log& operator<<(const Color& c)
    {
        const std::string colorString = log::detail::ConsoleCommands::GetString(c);
        strings.push_back(log::detail::LogString(colorString, true));
    }

    void PrintTimestamp()
    {
        boost::posix_time::ptime now = boost::date_time::microsec_clock<boost::posix_time::ptime>::local_time();
        std::stringstream ss;
        ss << "Timestamp: " << boost::posix_time::to_iso_extended_string(now) << std::endl;
        strings.push_back(log::detail::LogString(ss.str(), false));
    }

private:
    std::list<log::detail::LogString> strings;
};

} // psi
