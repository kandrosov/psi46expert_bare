#!/bin/bash

ASTYLE_OPTIONS="--style=ansi --indent=spaces=4 --indent-preprocessor --pad-oper"

find src -name "*.cc" -exec astyle $ASTYLE_OPTIONS '{}' \; | grep Formatted
find src -name "*.h" -exec astyle $ASTYLE_OPTIONS '{}' \; | grep Formatted
find src -name "*.cpp" -exec astyle $ASTYLE_OPTIONS '{}' \; | grep Formatted
find src -name "*.orig" -exec rm '{}' +
