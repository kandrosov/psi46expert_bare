/*!
 * \file exception.h
 * \brief Definition of the base exception class for the psi namespace.
 * \author Konstantin Androsov <konstantin.androsov@gmail.com>
 */

#pragma once

#include <exception>
#include <string>
#include <sstream>

#define THROW_PSI_EXCEPTION(msg)   {    \
        std::stringstream ss;               \
        ss << msg;                          \
        throw psi::exception(__PRETTY_FUNCTION__, ss.str()); }

namespace psi {
class exception : public std::exception {
public:
    exception(const std::string& header, const std::string& message) : hdr(header), msg(message) {}
    virtual ~exception() throw() {}
    virtual const char* what() const throw() {
        return ("[" + hdr + "] " + msg).c_str();
    }
    const std::string& header() const {
        return hdr;
    }
    const std::string& message() const {
        return msg;
    }
private:
    std::string hdr;
    std::string msg;
};
} // psi
