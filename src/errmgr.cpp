#include <errmgr.hh>
#include <cstdio>
#include <cstdlib>

void ErrorMgr::err(const char *s)
{
    std::perror(s);
    std::exit(1);
}
