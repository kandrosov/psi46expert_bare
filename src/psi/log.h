/*!
 * \file log.h
 * \brief Definition of PSI Logging System.
 *
 * \b Changelog
 * 09-03-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Log,Info,Debug,Error moved into log::detail namespace and typedefs created to use outside.
 * 08-03-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - Fixed ostream manipulator output.
 *      - Enum Color moved into psi::colors namespace.
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
 *   LogDebug   Dump all messages into file
 *   LogInfo    print messages on screen via std::cout, dump into output
 *              Info file, dump into Debug
 *   LogError   print messages on screen via psi::LogError(), dump into output
 *              Errorr file, dump into Debug
 *
 * Nothing will be dummped into file unless it is opened. File will be automatically closed
 * at the end of program.
 *
 * Examples using LogInfo. LogDebug and LogError work in the same way:
 *
 *   psi::LogInfo().open( "info.log"); // set output filename: works only once
 *
 *   psi::LogInfo() << "This is a message into Info Log" << std::endl;
 *
 *   psi::LogInfo() << "Analog of previous line: not head is output" << std::endl;
 *
 *   psi::LogInfo( "Head" ) << "This is a message with head" << std::endl;
 *
 *   psi::LogInfo( __func__) << "Message from some function" << std::endl;
 *
 *   psi::LogInfo( __PRETTY_FUNCTION__ ) << "Message from some function" << std::endl;
 *
 *   psi::LogInfo( "Test1") << "Voltage: " << _voltage << std::endl;
 */

#pragma once

#include <string>
#include <sstream>
#include <list>

#include <boost/thread/mutex.hpp>

namespace psi {
namespace colors
{
/// Colors
enum Color { Default,
             Black , Red , Green , Yellow , Blue , Pink , Cyan , White ,
             BlackB, RedB, GreenB, YellowB, BlueB, PinkB, CyanB, WhiteB };
} // colors

namespace log {
namespace detail {
typedef std::ostream& (*ostream_manipulator)(std::ostream&);

class Info;
class Debug;
class Error;

template<typename L>
struct LogWriter;

class LogBaseImpl
{
public:
    void open(const std::string& fileName);
    void write(const std::string& logString);
private:
    boost::shared_ptr<std::ostream> file;
};

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
        logImpl.open(fileName);
    }

    void write(const std::string& logString, const std::string& terminalString)
    {
        boost::lock_guard<boost::mutex> lock(mutex);
        LogWriter<L>::terminal_write(terminalString);
        logImpl.write(logString);
        LogWriter<L>::repeat_write(logString, terminalString);
    }

private:
    LogBase() {}

    LogBaseImpl logImpl;
    boost::mutex mutex;
};

struct ConsoleCommand
{
    static std::string MakeString(const colors::Color& c);
};

struct DateTimeProvider
{
    static std::string Now();
};

class ConsoleWriter
{
protected:
    static void Write_cout(const std::string& str);
    static void Write_cerr(const std::string& str);
};

template<>
struct LogWriter<Debug>
{
    static void terminal_write(const std::string&) {}
    static void repeat_write(const std::string& logString, const std::string& terminalString) {}
};

template<>
struct LogWriter<Info> : private ConsoleWriter
{
    static void terminal_write(const std::string& str)
    {
        ConsoleWriter::Write_cout(str);
    }

    static void repeat_write(const std::string& logString, const std::string& terminalString)
    {
        LogBase<Debug>::Singleton().write(logString, terminalString);
    }
};

template<>
struct LogWriter<Error> : private ConsoleWriter
{
    static void terminal_write(const std::string& str)
    {
        ConsoleWriter::Write_cerr(str);
    }

    static void repeat_write(const std::string& logString, const std::string& terminalString)
    {
        LogBase<Debug>::Singleton().write(logString, terminalString);
    }
};

template<typename L>
class Log
{
public:
    explicit Log() {}
    explicit Log(const std::string& head)
    {
        std::ostringstream ss;
        ss << "[" << head << "] ";
        logStream << ss.str();
        terminalStream << ss.str();
    }

    ~Log()
    {
        log::detail::LogBase<L>::Singleton().write(logStream.str(), terminalStream.str());
    }

    void open(const std::string& fileName)
    {
        log::detail::LogBase<L>::Singleton().open(fileName);
    }

    template<typename T>
    Log& operator<<(const T& t)
    {
        logStream << t;
        terminalStream << t;
        return *this;
    }

    Log& operator<<(log::detail::ostream_manipulator manipulator)
    {
        logStream << manipulator;
        terminalStream << manipulator;
        return *this;
    }

    Log& operator<<(const colors::Color& c)
    {
        const std::string colorString = log::detail::ConsoleCommand::MakeString(c);
        terminalStream << colorString;
        return *this;
    }

    void PrintTimestamp()
    {
        std::ostringstream ss;
        ss << "Timestamp: " << log::detail::DateTimeProvider::Now() << "." << std::endl;
        logStream << ss.str();
        terminalStream << ss.str();
    }

private:
    std::ostringstream logStream;
    std::ostringstream terminalStream;
};

} // detail
} // log

typedef log::detail::Log<log::detail::Info> LogInfo;

/*!
 * LogDebug should be used for all debugging (intermediate) information:
 * voltages, currents, 'hello world's, 'I am here', etc. Its output is
 * stored in separate file and not displayed on Monitor. Very useful for
 * later review by experts.
 */
typedef log::detail::Log<log::detail::Debug> LogDebug;

typedef log::detail::Log<log::detail::Error> LogError;

} // psi
