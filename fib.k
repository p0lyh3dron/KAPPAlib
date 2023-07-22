u64: factorial(u64: n) {
    if n == 1 {
        return 1;
    }

    return n * factorial(n - 1);
}

u64: fib(u64: n) {
    u64: i = 0;
    u64: x = 0;
    u64: y = 1;
    u64: z;

    while i < n {
        z = x + y;
        x = y;
        y = z;

        i = i + 1;
    }

    return x;
}