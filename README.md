# mex-lua
...the faster interpreter in the interpreter

## MATLAB

compiling directly in matlab

    >> mex -llua -I /Users/markus/Downloads/lua-5.2.3/src/ CFLAGS='-Wall -Wextra -fPIC -std=c99 -O4 -pedantic -g'  lua.c

## GNU OCTAVE

compiling on linux with `tcc`, `gcc` or `clang` for octave or matlab _(just change the file extension)_

    tcc -llua -I /path/to/mex.h/ -shared lua.c -o lua.mex
    gcc -I /usr/include/octave-3.8.1/octave/ -lm -llua -shared -fPIC -std=c99 -O4 lua.c -o lua.mex
    clang -I /usr/include/octave-3.8.1/octave/ -lm -llua -shared -fPIC -std=c99 -O4 lua.c -o lua.mex

## Limitations

1. Multidimension _(matrix)_ input is currently missing ...see Issue 1
2. The order of returns can crash the stack ... see Issue 2
3. Currently mex-lua needs at least one output ... see Issue 3
4. Currently debug outputs are hardcoded

## Usage

create you lua function _(make sure filename and function name are the same. e.g. sum() and sum.lua)_

    function sum (param1, param2)
        return = param1 + param2
    end

call lua script from Matlab/GNU Octave

    octave:1> ret = lua('sum', pi, e)
    well done sum in sum.lua
    lua stack top 1
    0 is number, lua stack top 1
    all done, lua stack top 0
    ret =     5.85987448204884


### BENCHMARKING

You can easily use it for [sumdata](https://github.com/markuman/sumdata). This will bring the speed of lua _(< 5 sec)_ into GNU Octave _(> 180 sec)_ and Matlab e.g. without touching C/C++.

#### octave benchmark

    octave:1> tic, sumdata, toc
    Original size 117261679.920000kb
    Compressed size 96727557.137000kb
    Elapsed time is 182.445 seconds.

#### mex-lua benchmark

    function sumdata(file)
      local o, ot, c, ct, l = 0, 0, 0, 0, 1

      for line in io.lines(file) do
        for n in string.gmatch(line, "%S+") do
          if l == 5 then
            ot = n
          elseif l == 6 then
            ct = n
            break
          end
          l = l + 1
        end
        l, o, c = 1, o + ot, c + ct
      end
      return (string.format("Original Size: %.0f kb\nCompressed Size: %.0f kb\n",o/1000,c/1000))
    end

    octave:2> tic, ret = lua('sumdata', 'data.log'), toc
    well done sumdata in sumdata.lua
    lua stack top 1
    0 is string, lua stack top 1
    all done, lua stack top 0
    ret = Original Size: 117261680 kb
    Compressed Size: 96727557 kb

    Elapsed time is 4.61084 seconds.


