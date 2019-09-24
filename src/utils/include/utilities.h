#define sgn(x) ((x) > (0) ?  1 : ((x) < 0 ? -1 : 0))

#define sgnplus(x) ((x) >= (0) ? 1 : -1)

#define clip(x, xmin, xmax) (((x) < (xmin)) ? (xmin) : ((x) > (xmax) ? xmax : x))

long long modR(long long x, long long R);