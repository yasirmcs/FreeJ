#include <stdlib.h>
#include <string.h>
#include <freej.h>
#include <freej_plugin.h>

static char *name = "RandTrans";
static char *author = "Clifford Smith";
static char *info = "random positional translation";
static int version = 1;

static ScreenGeometry *geo;

/*
 * fastrand - fast fake random number generator
 * Warning: The low-order bits of numbers generated by fastrand()
 *          are bad as random numbers. For example, fastrand()%4
 *          generates 1,2,3,0,1,2,3,0...
 *          You should use high-order bits.
 */

unsigned int fastrand_val;
unsigned int fastrand() {
	return (fastrand_val=fastrand_val*1103515245+12345);
}
void fastsrand(unsigned int seed) {
	fastrand_val = seed;
}

int livemap(int x,int y) {
  int xd,yd;
  yd = y + (fastrand() >> 30)-2;
  xd = x + (fastrand() >> 30)-2;
  if (xd > geo->w) xd-=1;
  return xd + yd*geo->w;
}

void init_table(int *table, ScreenGeometry *sg) {
  /* always call livemap function */
  int c;
  geo = sg;
  for(c=0;c<geo->w*geo->h;c++) table[c] = -2;
  fastsrand(rand());
}
