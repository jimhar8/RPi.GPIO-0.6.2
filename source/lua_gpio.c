#include <stdlib.h>
#include <string.h>
 
#include <lua.h>
#include <lauxlib.h>
 
#include "lua_gpio.h"
 
/* Userdata object that will hold the counter and name. */
typedef struct {
    counter_t *c;
    char      *name;
	int gpio_direction[54];
} wxGPIO_userdata_t;
 
static int wxGPIO_new(lua_State *L)
{
    wxGPIO_userdata_t *cu;
    const char          *name;
    int                  start;
 
    /* Check the arguments are valid. */
    start = luaL_checkint(L, 1);
    name  = luaL_checkstring(L, 2);
    if (name == NULL)
        luaL_error(L, "name cannot be empty");
 
    /* Create the user data pushing it onto the stack. We also pre-initialize
     * the member of the userdata in case initialization fails in some way. If
     * that happens we want the userdata to be in a consistent state for __gc. */
    cu       = (wxGPIO_userdata_t *)lua_newuserdata(L, sizeof(*cu));
    cu->c    = NULL;
    cu->name = NULL;
 
    /* Add the metatable to the stack. */
    luaL_getmetatable(L, "wxGPIO");
    /* Set the metatable on the userdata. */
    lua_setmetatable(L, -2);
 
    /* Create the data that comprises the userdata (the counter). */
    cu->c    = counter_create(start);
    cu->name = strdup(name);
 
    return 1;
}
 
static int wxGPIO_add(lua_State *L)
{
    wxGPIO_userdata_t *cu;
    int                  amount;
 
    cu     = (wxGPIO_userdata_t *)luaL_checkudata(L, 1, "wxGPIO");
    amount = luaL_checkint(L, 2);
    counter_add(cu->c, amount);
 
    return 0;
}
 
static int wxGPIO_subtract(lua_State *L)
{
    wxGPIO_userdata_t *cu;
    int                  amount;
 
    cu     = (wxGPIO_userdata_t *)luaL_checkudata(L, 1, "wxGPIO");
    amount = luaL_checkint(L, 2);
    counter_subtract(cu->c, amount);
 
    return 0;
}
 
static int wxGPIO_increment(lua_State *L)
{
    wxGPIO_userdata_t *cu;
 
    cu = (wxGPIO_userdata_t *)luaL_checkudata(L, 1, "wxGPIO");
    counter_increment(cu->c);
 
    return 0;
}
 
static int wxGPIO_decrement(lua_State *L)
{
    wxGPIO_userdata_t *cu;
 
    cu = (wxGPIO_userdata_t *)luaL_checkudata(L, 1, "wxGPIO");
    counter_decrement(cu->c);
 
    return 0;
}
 
static int wxGPIO_getval(lua_State *L)
{
    wxGPIO_userdata_t *cu;
 
    cu = (wxGPIO_userdata_t *)luaL_checkudata(L, 1, "wxGPIO");
    lua_pushinteger(L, counter_getval(cu->c));
 
    return 1;
}
 
static int wxGPIO_getname(lua_State *L)
{
    wxGPIO_userdata_t *cu;
 
    cu = (wxGPIO_userdata_t *)luaL_checkudata(L, 1, "wxGPIO");
    lua_pushstring(L, cu->name);
 
    return 1;
}
 
static int wxGPIO_destroy(lua_State *L)
{
    wxGPIO_userdata_t *cu;
 
    cu = (wxGPIO_userdata_t *)luaL_checkudata(L, 1, "wxGPIO");
 
    if (cu->c != NULL)
        counter_destroy(cu->c);
    cu->c = NULL;
 
    if (cu->name != NULL)
        free(cu->name);
    cu->name = NULL;
     
    return 0;
}
 
static int wxGPIO_tostring(lua_State *L)
{
    wxGPIO_userdata_t *cu;
 
    cu = (wxGPIO_userdata_t *)luaL_checkudata(L, 1, "wxGPIO");
 
    lua_pushfstring(L, "%s(%d)", cu->name, counter_getval(cu->c));
 
    return 1;
}



static int mmap_gpio_mem(lua_State *L)
{
   int result;

   if (module_setup)
      return 0;

   result = setup();
   if (result == SETUP_DEVMEM_FAIL)
   {
      luaL_error(L, "No access to /dev/mem.  Try running as root!");
      return 1;
   } else if (result == SETUP_MALLOC_FAIL) {
      luaL_error(L, "No memory!");
      return 2;
   } else if (result == SETUP_MMAP_FAIL) {
      luaL_error(L, "Mmap of GPIO registers failed");
      return 3;
   } else if (result == SETUP_CPUINFO_FAIL) {
      luaL_error(L, "Unable to open /proc/cpuinfo");
      return 4;
   } else if (result == SETUP_NOT_RPI_FAIL) {
      luaL_error(L, "Not running on a RPi!");
      return 5;
   } else { // result == SETUP_OK
      module_setup = 1;
      return 0;
   }
}

 
static const struct luaL_Reg wxGPIO_methods[] = {
    { "add",         wxGPIO_add       },
    { "subtract",    wxGPIO_subtract  },
    { "increment",   wxGPIO_increment },
    { "decrement",   wxGPIO_decrement },
    { "getval",      wxGPIO_getval    },
    { "getname",     wxGPIO_getname   },
    { "__gc",        wxGPIO_destroy   },
    { "__tostring",  wxGPIO_tostring  },
    { NULL,          NULL               },
};
 
static const struct luaL_Reg wxGPIO_functions[] = {
    { "new", wxGPIO_new },
    { NULL,  NULL         }
};
 
int luaopen_wxGPIO(lua_State *L)
{
    /* Create the metatable and put it on the stack. */
    luaL_newmetatable(L, "wxGPIO");
    /* Duplicate the metatable on the stack (We know have 2). */
    lua_pushvalue(L, -1);
    /* Pop the first metatable off the stack and assign it to __index
     * of the second one. We set the metatable for the table to itself.
     * This is equivalent to the following in lua:
     * metatable = {}
     * metatable.__index = metatable
     */
    lua_setfield(L, -2, "__index");
 
    /* Set the methods to the metatable that should be accessed via object:func */
    luaL_setfuncs(L, wxGPIO_methods, 0);
 
    /* Register the object.func functions into the table that is at the top of the
     * stack. */
    luaL_newlib(L, wxGPIO_functions);
 
    return 1;
}