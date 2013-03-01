/*!
 * \file exception.h
 * \brief Definition of the base exception class for the psi namespace.
 *
 * \author Konstantin Androsov <konstantin.androsov@gmail.com>
 *
 * \b Changelog
 * 25-02-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - psi_exception renamed to exception and moved into psi namespace.
 * 23-01-2013 by Konstantin Androsov <konstantin.androsov@gmail.com>
 *      - First version.
 */

#pragma once

#include <exception>
#include <string>
#include <sstream>

#define THROW_PSI_EXCEPTION(msg)   {    \
    std::stringstream ss;               \
    ss << msg;                          \
    throw psi::exception(ss.str()); }

namespace psi {
class exception : public std::exception
{
public:
    exception(const std::string& _message) : message(_message) {}
    virtual ~exception() throw() {}
    virtual const char* what() const throw() { return message.c_str(); }
private:
    std::string message;
};
} // psi
