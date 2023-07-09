$
*    fib.k    --    test source for KAPPA
*
*    Computes the nth Fibonacci number.
$

$u64: _global = 0;$

u64: fib($u64: n$) {
    u64: n;
    u64: i;
    u64: x;
    u64: y;
    u64: z;

    n = 10;
    i = 0;
    x = 0;
    y = 1;

    while i < n {
        z = x + y;
        x = y;
        y = z;

        i = i + 1;
    }

    return x;
}