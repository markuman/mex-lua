// compile hint

/*
 compiling directly in matlab
 mex -llua -I/Users/markus/Downloads/lua-5.2.3/src/ CFLAGS='-Wall -Wextra -fPIC -std=c99 -O4 -pedantic -g'  lua.c

 compiling on linux with tcc, gcc or clang for octave or matlab (just change the file extension
 tcc -llua -I /path/to/mex.h -shared lua.c -o lua.mex
 gcc -I/usr/include/octave-3.8.1/octave/ -lm -llua -shared -fPIC -std=c99 -O4 lua.c -o lua.mex
 clang -I/usr/include/octave-3.8.1/octave/ -lm -llua -shared -fPIC -std=c99 -O4 lua.c -o lua.mex

 */

// C specific includes
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

// lua specific includes
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

// mac os x
/*
#include "/usr/local/include/lua.h"
#include "/usr/local/include/lualib.h"
#include "/usr/local/include/lauxlib.h"
*/

// Matlab specific includes
#include "mex.h"

void mexFunction (int nlhs, mxArray *plhs[],
                  int nrhs, const mxArray *prhs[])
{

  // --- input checks
  // more than one
  if(nrhs < 1) {
    mexErrMsgIdAndTxt( "MATLAB:lua:invalidNumInputs",
                      "One or more inputs are required.");
    // first input have to be a string which calls the lua file
    // furthermore: filename = function name
  } else if ( nlhs < 1) {
    mexErrMsgIdAndTxt("MATLAB:lua:nrhs", "You have to define outputs!");
  } else if ( !mxIsChar(prhs[0])) {
    mexErrMsgIdAndTxt("MATLAB:lua:nrhs", "First Input must be a string.");
  } else {

    // get string from nrhs
    // while funcname == filename + .lua extension
    char *luafunc, *luafile;
    luafunc = (char *) mxCalloc(mxGetN(prhs[0])+1, sizeof(char));
    luafile = (char *) mxCalloc(mxGetN(prhs[0])+5, sizeof(char));
    mxGetString(prhs[0], luafunc, mxGetN(prhs[0])+1);
    mxGetString(prhs[0], luafile, mxGetN(prhs[0])+1);
    char file_extension[] = ".lua";
    strcat(luafile, file_extension);
    mexPrintf("well done %s in %s\n", luafunc, luafile);

    // try to open lua stack
    lua_State *L = luaL_newstate();
    luaL_openlibs(L);
    // Tell Lua to load & run the file sample_script.lua
    luaL_dofile(L, luafile);
    // Push function to the Lua stack
    lua_getglobal(L, luafunc);
    // Checks if top of the Lua stack is a function.
    lua_isfunction(L, -1);

    double *data;
    int columns, rows, len;
    char *string;
    // put all inputs to lua stack
    for(int in = 1; in < nrhs ; in++){
      // is double array?
      if (mxIsDouble(prhs[in])){

        data = mxGetPr (prhs[in]);

        // check dimension
        columns = mxGetN(prhs[in]);
        rows = mxGetM(prhs[in]);

        // if one double, push one number to stack
        if (columns == 1 && rows == 1){
          if ( mxIsNumeric(prhs[in])) {
            lua_pushnumber(L, data[0]);
          }
        } else {
          // TODO, create/set table
          mexErrMsgIdAndTxt("MATLAB:lua:nrhs", "Multidimension Matrix is not supported atm\n.");

        }
        // is string?
      } else if (mxIsChar(prhs[in])){

        string = (char *) mxCalloc(mxGetN(prhs[in])+1, sizeof(char));
        mxGetString(prhs[in], string, mxGetN(prhs[in])+1);

        lua_pushstring(L, string);
      }

    }


    // Perform the function call (nrhs-1, nlhs) */
    if (lua_pcall(L, nrhs-1, nlhs, 0) != 0) {
      mexErrMsgIdAndTxt("MATLAB:lua:nrhs", "Failed to load lua file\n.");
    }

    int top = lua_gettop(L);
    mexPrintf("lua stack top %d\n", top);

    // Let's retrieve the function result
    // Check if the top of the Lua Stack has a numeric value
    double *mexoutput;
    double luaout;
    const char* luastring;

    for(int n=0; n<nlhs ; n++){

      // is table
      if (lua_istable(L, -1)) {
        top = lua_gettop(L);
        mexPrintf("%d is table, lua stack top %d\n", n, top);
        int elements = 0;
        rows = luaL_len(L, 1);
        lua_rawgeti(L, 1, 1);
        if (lua_istable(L, -1)) {
          // a = {{1, 3}, {7, 0}}
          columns = luaL_len(L, -1);
          plhs[n] = mxCreateNumericMatrix(rows, columns, mxDOUBLE_CLASS, mxREAL);
          mexoutput = mxGetPr(plhs[n]);

          for (int r = 1; r <= rows; ++r) {

            if ( r > 1 ){
              lua_rawgeti(L, 1, r);
              columns = luaL_len(L, -1);
            }

            for (int c = 1; c <= columns; ++c) {
              lua_rawgeti(L, -1, c);
              mexoutput[elements] = lua_tonumber(L, -1);
              lua_pop(L, 1);
              ++elements;
            }
            lua_pop(L, 1);
          }
          lua_remove(L, 1);
        } else {
          // a = {1, 3, 7, 0}
          top = lua_gettop(L);
          mexPrintf("%d is vector, lua stack top %d\n", n, top);
          plhs[n] = mxCreateNumericMatrix(1, rows, mxDOUBLE_CLASS, mxREAL);
          mexoutput = mxGetPr(plhs[n]);
          lua_pop(L, 1);
          for (int v = 1; v <= rows; ++v) {
            lua_rawgeti(L, -1, v);
            mexoutput[v-1] = lua_tonumber(L, -1);
            lua_pop(L, 1);
          }
          top = lua_gettop(L);
          mexPrintf("after vec %d lua stack top %d\n", n, top);
          lua_remove(L, 1);
          top = lua_gettop(L);
          mexPrintf("after remove %d lua stack top %d\n", n, top);
        }

      } else if (lua_isnumber(L, -1)) {

        top = lua_gettop(L);
        mexPrintf("%d is number, lua stack top %d\n", n, top);
        luaout = lua_tonumber(L, -1); // retrieve the result
        lua_pop(L, 1); // Pop retrieved value from the Lua stack
        plhs[n] = mxCreateNumericMatrix(1, 1, mxDOUBLE_CLASS, mxREAL);
        mexoutput = mxGetPr(plhs[n]);
        mexoutput[0] = luaout;

      } else if (lua_isstring(L, -1)) {
        top = lua_gettop(L);
        mexPrintf("%d is string, lua stack top %d\n", n, top);
        luastring = lua_tostring(L, -1);
        plhs[n] = mxCreateString(luastring);
        lua_pop(L, 1);
      } else {

        mexPrintf("%d is unkown\n", n);

      }


    }

    top = lua_gettop(L);
    mexPrintf("all done, lua stack top %d\n", top);
    lua_close(L);
    return;

    //mexPrintf("well done %s in %s\n", luafunc, luafile);

  }
}


