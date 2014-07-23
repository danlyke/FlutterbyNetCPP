#!/usr/bin/perl -w

open my $I, '<', 'parsertokens.txt'
    || die "Unable to open parsertokens.txt";

open my $OH, '>', 'parsertokens.h'
    || die "Unable to open parsertokens.h for writing";
open my $OC, '>', 'parsertokens.cpp'
    || die "Unable to open parsertokens.cpp for writing";

print $OH <<'EOF';
#include <map>
#include <string>

EOF

print $OC <<'EOF';
#include "parsertokens.h"
using namespace std;

map<int, std::string> parserTokenNames;

void InitParserTokenNames()
{
EOF

my $token_num = 0;
while (<$I>)
{
    if (/^(\w+)/)
    {
        ++$token_num;
        print $OH "#define $1 $token_num\n";
        print $OC "    parserTokenNames[$token_num] = \"$1\";\n";
    }
}

print $OH <<'EOF';


extern std::map<int, std::string> parserTokenNames;

extern void InitParserTokenNames();
EOF

print $OC <<'EOF';
}
EOF

