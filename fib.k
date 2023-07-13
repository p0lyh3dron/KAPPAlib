u64: ret() {
    return 1;
}

u64: fib(u64: n) {
    u64: i = 0;
    u64: x = 0;
    u64: y = ret();
    u64: z;

    while i < n {
        z = x + y;
        x = y;
        y = z;

        i = i + 1;
    }

    return x;
}