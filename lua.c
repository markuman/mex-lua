// compile hint

/*
 compiling directly in matlab
 mex -llua -I/Users/markus/Downloads/lua-5.3.1/src/ CFLAGS='-Wall -Wextra -fPIC -std=c99 -O4 -pedantic -g'  lua.c

 compiling on linux with tcc, gcc or clang for octave or matlab (just change the file extension
 tcc -llua -I /path/to/mex.h -shared lua.c -o lua.mex
 gcc -I/usr/include/octave-4.0.0/octave/ -lm -llua -shared -fPIC -std=c99 -O4 lua.c -o lua.mex
 clang -I/usr/include/octave-4.0.0/octave/ -lm -llua -shared -fPIC -std=c99 -O4 lua.c -o lua.mex

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

//#define DEBUG

int top;
mxArray *cell_array_ptr;


void mexFunction (int nlhs, mxArray *plhs[],
                  int nrhs, const mxArray *prhs[])
{

  // --- input checks
  // more than one
  if(nrhs != 1) {
    mexErrMsgIdAndTxt( "MATLAB:lua:invalidNumInputs",
                      "One input is required.");
    // first input have to be a string which calls the lua file
    // furthermore: filename = function name
  } else if ( nlhs != 1) {
    mexErrMsgIdAndTxt("MATLAB:lua:nlhs", "You have to define outputs!");
  } else if ( !mxIsChar(prhs[0])) {
    mexErrMsgIdAndTxt("MATLAB:lua:nrhs", "Input must be a string.");
  } else {

    char *luacode;
    luacode = (char *) mxCalloc(mxGetN(prhs[0])+1, sizeof(char));   
    mxGetString(prhs[0], luacode, mxGetN(prhs[0])+1);

    // try to open lua stack
    lua_State *L = luaL_newstate();
    luaL_openlibs(L);
    // try to run the luacode
    luaL_dostring(L, luacode);


    double *data;
    int columns, rows, len;
    char *string;
  
    #ifdef DEBUG
    top = lua_gettop(L);
    mexPrintf("lua stack top %d\n", top);
    #endif

    if (lua_istable(L, 1)) {
        #ifdef DEBUG
        printf("top is table\n");
        #endif
        
        
        int elements = 0;
        lua_len(L, 1);
        rows = lua_tonumber(L, -1);
        lua_pop(L, 1);
        lua_rawgeti(L, 1, 1);
        if (lua_istable(L, -1)) {
            // a = {{1, 3}, {7, 0}}
            #ifdef DEBUG
            printf("top is table again\n");
            #endif
            lua_len(L, -1);
            int columns = lua_tonumber(L, -1);
            lua_pop(L, 1);
            #ifdef DEBUG
            printf("rows %d, cols %d\n", rows, columns);
            #endif
            cell_array_ptr = mxCreateCellMatrix(rows,columns);                      
            
            for (int r = 1; r <= rows; ++r) {
                
                if ( r > 1 ){
                    lua_rawgeti(L, 1, r);
                    lua_len(L, -1);
                    columns = lua_tonumber(L, -1);
                    lua_pop(L, 1);
                }
                
                for (int c = 1; c <= columns; ++c) {
                    lua_rawgeti(L, -1, c);
                    // column major order
                    #ifdef DEBUG                    
                    printf("index %d, row %d, col %d, nbrows %d\n", ( (c - 1) * rows + (r - 1) + 1), r, c, rows); 
                    #endif
                    if (lua_isnumber(L, -1)) {
                        mxSetCell(cell_array_ptr,( (c - 1) * rows + (r - 1) + 1) - 1, mxCreateDoubleScalar(lua_tonumber(L, -1)));
                    } else {
                        mxSetCell(cell_array_ptr,( (c - 1) * rows + (r - 1) + 1) - 1, mxCreateString(lua_tostring(L, -1)));
                    }
                    lua_pop(L, 1);
                    ++elements;
                }
                lua_pop(L, 1);
            }
            lua_remove(L, 1);
        } else {
            // a = {1, 3, 7, 0}
            cell_array_ptr = mxCreateCellMatrix(1,rows);
            #ifdef DEBUG
            printf("rows %d\n", rows);
            #endif
            lua_pop(L, 1);
            lua_pushnil(L);
            for (int v = 1; v <= rows; ++v) {
                lua_next(L, -2);
                if (lua_isnumber(L, -1)) {
                    mxSetCell(cell_array_ptr, v - 1, mxCreateDoubleScalar(lua_tonumber(L, -1)));
                } else {
                    mxSetCell(cell_array_ptr, v - 1, mxCreateString(lua_tostring(L, -1)));
                }
                lua_pop(L, 1);
            }
            
            lua_remove(L, 1);
           
        }
        plhs[0] = cell_array_ptr;

    } else {
        mexErrMsgIdAndTxt("MATLAB:lua:nlhs", "return always a table!");
    }
        
        
    lua_close(L);
    return;

  }
}


