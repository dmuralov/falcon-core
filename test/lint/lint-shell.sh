#!/usr/bin/env bash
#
# Copyright (c) 2018 The Bitcoin Core developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.
#
# Check for shellcheck warnings in shell scripts.

export LC_ALL=C

# The shellcheck binary segfault/coredumps in Travis with LC_ALL=C
# It does not do so in Ubuntu 14.04, 16.04, 18.04 in versions 0.3.3, 0.3.7, 0.4.6
# respectively. So export LC_ALL=C is set as required by lint-shell-locale.sh
# but unset here in case of running in Travis.
if [ "$TRAVIS" = "true" ]; then
  unset LC_ALL
fi

if ! command -v shellcheck > /dev/null; then
    echo "Skipping shell linting since shellcheck is not installed."
    exit 0
fi

# Disabled warnings:
# SC1087: Use braces when expanding arrays, e.g. ${array[idx]} (or ${var}[.. to quiet).
# SC1117: Backslash is literal in "\.". Prefer explicit escaping: "\\.".
# SC2001: See if you can use ${variable//search/replace} instead.
# SC2004: $/${} is unnecessary on arithmetic variables.
# SC2005: Useless echo? Instead of 'echo $(cmd)', just use 'cmd'.
# SC2006: Use $(..) instead of legacy `..`.
# SC2016: Expressions don't expand in single quotes, use double quotes for that.
# SC2028: echo won't expand escape sequences. Consider printf.
# SC2046: Quote this to prevent word splitting.
# SC2048: Use "$@" (with quotes) to prevent whitespace problems.
# SC2066: Since you double quoted this, it will not word split, and the loop will only run once.
# SC2086: Double quote to prevent globbing and word splitting.
# SC2116: Useless echo? Instead of 'cmd $(echo foo)', just use 'cmd foo'.
# SC2148: Tips depend on target shell and yours is unknown. Add a shebang.
# SC2162: read without -r will mangle backslashes.
# SC2164: Use 'cd ... || exit' or 'cd ... || return' in case cd fails
# SC2166: Prefer [ p ] && [ q ] as [ p -a q ] is not well defined.
# SC2166: Prefer [ p ] || [ q ] as [ p -o q ] is not well defined.
# SC2181: Check exit code directly with e.g. 'if mycmd;', not indirectly with $?.
# SC2206: Quote to prevent word splitting, or split robustly with mapfile or read -a.
# SC2207: Prefer mapfile or read -a to split command output (or quote to avoid splitting).
# SC2230: which is non-standard. Use builtin 'command -v' instead.
# SC2236: Don't force -n instead of ! -z.
shellcheck -e SC1087,SC1117,SC2001,SC2004,SC2005,SC2006,SC2016,SC2028,SC2046,SC2048,SC2066,SC2086,SC2116,SC2148,SC2162,SC2166,SC2181,SC2206,SC2207,SC2230,SC2236,SC2164 \
    $(git ls-files -- "*.sh" | grep -vE 'src/(secp256k1|univalue)/')
