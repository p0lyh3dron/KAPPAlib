return 0.0;

f32: cos_approx(f32: t) { return (9.86960440109 - 4.0*t*t) / (9.86960440109 + t*t); };

f32: cos(f32: t) {
    if t < 0.0 do t = 0.0 - t;

    while t > 6.28318530718 do t = t - 6.28318530718;

    if t < 1.57079632679 do return cos_approx(t);

    return 0.0 - cos_approx(t - 3.14159265359);
};

f32: sin (f32: t) { return cos(t - 1.57079632679); };

f32: ipow(f32: t, u32: p) {
    f32: ret = 1.0;

    while p > 0 do {
        ret = ret * t;

        p = p - 1;
    };

    return ret;
};

u32: fact(u32: t) {
    if t == 0 do return 1;

    return t * fact(t - 1);
};

f32: exp(f32: t) {
    f32: ret  = 1.0;
    f32: frac = 0.0;
    u32: i    = 0;

    while t > 1.0 do {
        t   = t - 1.0;
        ret = ret * 2.71828182846;
    };

    while t < 0.0 do {
        t   = t + 1.0;
        ret = ret * 0.367879441171;
    };

    while i < 8 do {
        frac = frac + ipow(t, i) / fact(i);

        i = i + 1;
    };

    return ret * frac;
};

f32: log(f32: t) {
    f32: ret  = 0.0;
    f32: frac = 0.0;
    u32: i    = 1;

    while t > 1.0 do {
        t   = t / 2.71828182846;
        ret = ret + 1.0;
    };

    t = t - 1;

    while i < 8 do {
        frac = frac + ipow(0.0 - 1.0, i) * ipow(t, i) / i;

        i = i + 1;
    };

    return ret - frac;
};

f32: pow(f32: b, f32: e) {
    return exp(e * log(b));
};

f32: cosh(f32: t) {
    return (exp(t) + exp(0.0 - t)) / 2.0;
};

f32: sinh(f32: t) {
    return (exp(t) - exp(0.0 - t)) / 2.0;
};

f32: sinz(f32: re, f32: im) {
    f32: tempr = *re;
    f32: tempi = *im;

    *re = sin(tempr) * cosh(tempi);
    *im = cos(tempr) * sinh(tempi);

    return 0.0;
};

f32: rmin() {
    return 0.0 - 2.3;
};

f32: rmax() {
    return 1.0;
};

f32: imin() {
    return 0.0 - 1.5;
};

f32: imax() {
    return 1.5;
};

f32: abs(f32: t) {
    if t < 0.0 do return 0.0 - t;

    return t;
};

f32: z(f32: re, f32: im) {
    f32: tempr = *re;
    f32: tempi = *im;

    *re = (tempr * tempr) - (tempi * tempi);
    *im = 2 * tempr * tempi;

    return 0.0;
};