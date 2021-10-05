#ifndef _STRTONUM_H_
#define _STRTONUM_H_

long long
strtonum(const char *numstr, long long minval, long long maxval,
    const char **errstrp);

#endif
