f32: cos_approx(f32: t) {
    return (9.86960440109 - 4.0*t*t) / (9.86960440109 + t*t);
}

f32: cos(f32: t) {
    if t < 0.0 {
        t = 0.0 - t;
    }

    while t > 6.28318530718 {
        t = t - 6.28318530718;
    }

    if t < 3.14159265359 {
        return cos_approx(t); 
    }

    return 0.0 - cos_approx(0.0 - t + 3.14159265359);
}

f32: sin (f32: t) {
    return cos(t - 1.57079632679);
}

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
