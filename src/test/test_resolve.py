from __future__ import print_function

import sys
import subprocess
import os
import platform

if sys.hexversion < 0x02070000:
    sys.exit("ERROR: unsupported Python version (should be >= 2.7)")

if sys.hexversion > 0x03000000 and sys.hexversion < 0x03010000:
    sys.exit("ERROR: unsupported Python3 version (should be >= 3.1)")

libfakesocket_path = sys.argv[1]
tor_resolve_path = sys.argv[2]

test_env = os.environ.copy()

if platform.system() == 'Darwin':
    test_env["DYLD_INSERT_LIBRARIES"] = libfakesocket_path
    test_env["DYLD_FORCE_FLAT_NAMESPACE"] = "1"
else:
    test_env["LD_PRELOAD"] = libfakesocket_path

child_process = subprocess.Popen([tor_resolve_path,
                                  '-5', 'mit.edu'],
                                  env=test_env,
                                  stdout=subprocess.PIPE,
                                  stderr=subprocess.PIPE,
                                  stdin=subprocess.PIPE)

l = child_process.stderr.readline().decode('utf8')

assert l == "connect() 127.0.0.1\n"

l = child_process.stderr.readline().decode('utf8')

assert l == "05 01 00 \n"

child_process.stdin.write('05 00\n'.encode())
child_process.stdin.flush()

l = child_process.stderr.readline().decode('utf8')

assert l == "05 f0 00 03 07 6d 69 74 2e 65 64 75 00 00 \n"

child_process.stdin.write(str.encode('05 00 00 04 2a 02 26 f0 00 10 02 95 00 00 00 00 00 00 25 5e 00 00 \n'))
child_process.stdin.flush()

l = child_process.stdout.readline().decode('utf8')

assert l == "2a02:26f0:10:295::255e\n"

