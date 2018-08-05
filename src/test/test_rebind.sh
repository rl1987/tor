#!/bin/sh

set -x

exitcode=0

"${PYTHON:-python}" "${abs_top_srcdir:-.}/src/test/test_rebind.py" "${abs_top_srcdir:-.}/src/app/tor" || exitcode=1

exit ${exitcode}
