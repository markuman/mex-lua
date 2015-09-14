# inline_lua
...the faster interpreter in the interpreter

    octave:11> tic, a = lua('a = {} a[1] = 1 for n = 1,1000 do a[1] = a[1] + 1 end return a'){:}, toc
    a =                 1001
    Elapsed time is 0.000264883 seconds.
    octave:12> tic, a = 1; for n = 1:1000, a = a + 1; end, a, toc
    a =                 1001
    Elapsed time is 0.00235295 seconds.
