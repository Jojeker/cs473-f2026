/* Consolidated host-side verification of both number formats against double. */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "myflpt.h"
#include "fxpt_shim.h"

static double md(myflpt v){ if(!v) return 0; int s=v>>31; uint32_t m=(v<<1)>>9;
  int e=(int)((v<<24)>>24); double g=((double)m/(double)(1u<<23))*ldexp(1.0,e-250); return s?-g:g; }
static double xd(fxpt v){ return (double)v/(double)(1<<FXPT_FRAC); }

static long long fx_max_sum=0; static int fx_ovf=0, my_sat=0;

static int ref(double cx,double cy,int nm){ double x=cx,y=cy,xx,yy,t; int n=0;
  do{xx=x*x;yy=y*y;t=2*x*y;x=xx-yy+cx;y=t+cy;++n;}while(((xx+yy)<4.0)&&(n<nm)); return n;}
static int fxm(fxpt cx,fxpt cy,int nm){ fxpt x=cx,y=cy,xx,yy,t; int n=0;
  do{xx=fxmul(x,x);yy=fxmul(y,y);t=2*fxmul(x,y);
     long long s=(long long)xx+(long long)yy; if(s>fx_max_sum)fx_max_sum=s;
     if(s>2147483647LL||s<-2147483648LL)fx_ovf++;
     x=xx-yy+cx;y=t+cy;++n;}while(((xx+yy)<(4<<FXPT_FRAC))&&(n<nm)); return n;}
static int mym(myflpt cx,myflpt cy,int nm){ myflpt x=cx,y=cy,xx,yy,xy,t; int n=0;
  do{xx=mul(x,x);yy=mul(y,y);xy=mul(x,y);
     if(xx==0x7FFFFFFFu||yy==0x7FFFFFFFu||(xy&0x7FFFFFFFu)==0x7FFFFFFFu)my_sat++;
     t=add(xy,xy);x=add(sub(xx,yy),cx);y=add(t,cy);++n;}
  while(is_smaller(add(xx,yy),0x400000FD)&&(n<nm)); return n;}

int main(void){
  printf("=========== myflpt arithmetic accuracy (random operands in [-4,4]) ===========\n");
  double wc=0,wa=0,ws=0,wm=0;
  for(int i=0;i<1000000;i++){
    float a=(float)((drand48()-0.5)*8.0), b=(float)((drand48()-0.5)*8.0);
    myflpt A=float_to_myflpt(a),B=float_to_myflpt(b);
    double ra=md(A),rb=md(B),r;
    if(fabs(a)>1e-6){ r=fabs(ra-(double)a)/fabs((double)a); if(r>wc)wc=r; }
    r=fabs(md(add(A,B))-(ra+rb)); if(fabs(ra+rb)>1e-6&&(r/=fabs(ra+rb))>wa)wa=r;
    r=fabs(md(sub(A,B))-(ra-rb)); if(fabs(ra-rb)>1e-6&&(r/=fabs(ra-rb))>ws)ws=r;
    r=fabs(md(mul(A,B))-(ra*rb)); if(fabs(ra*rb)>1e-6&&(r/=fabs(ra*rb))>wm)wm=r;
  }
  printf("  float->myflpt conversion  worst rel. error = %.4e  (2^-22 = %.4e)\n",wc,ldexp(1,-22));
  printf("  add()                     worst rel. error = %.4e\n",wa);
  printf("  sub()                     worst rel. error = %.4e\n",ws);
  printf("  mul()                     worst rel. error = %.4e\n",wm);
  printf("  IEEE-754 float for comparison             = %.4e  (2^-23)\n",ldexp(1,-23));

  printf("\n=========== is_smaller() exhaustive sign/magnitude check ===========\n");
  const float V[]={-8,-4,-1.5,-0.25,0,0.25,1.5,4,8}; int n=9,bad=0;
  for(int i=0;i<n;i++)for(int j=0;j<n;j++){
    myflpt A=float_to_myflpt(V[i]),B=float_to_myflpt(V[j]);
    if(is_smaller(A,B)!=(md(A)<md(B)))bad++; }
  printf("  %d / %d ordered pairs incorrect\n",bad,n*n);

  printf("\n=========== full 512x512 image vs. double-precision reference ===========\n");
  int dx=0,dm=0,mx=0,mm=0; long itx=0,itm=0,itr=0;
  fxpt    xcy=FX(-1.5),  xdl=FX(3.0)/512;
  myflpt  mcy=float_to_myflpt(-1.5f), mdl=float_to_myflpt(3.0f/512.0f);
  for(int k=0;k<512;k++){
    fxpt xcx=FX(-2.0); myflpt mcx=float_to_myflpt(-2.0f);
    for(int i=0;i<512;i++){
      int a=fxm(xcx,xcy,64), b=ref(xd(xcx),xd(xcy),64);
      int c=mym(mcx,mcy,64), d=ref(md(mcx),md(mcy),64);
      itx+=a; itm+=c; itr+=b;
      if(a!=b){dx++; if(abs(a-b)>mx)mx=abs(a-b);}
      if(c!=d){dm++; if(abs(c-d)>mm)mm=abs(c-d);}
      xcx+=xdl; mcx=add(mcx,mdl);
    } xcy+=xdl; mcy=add(mcy,mdl); }
  printf("  fxpt   : %d / 262144 pixels differ (%.4f%%), max iteration delta = %d\n",dx,100.0*dx/262144,mx);
  printf("  myflpt : %d / 262144 pixels differ (%.4f%%), max iteration delta = %d\n",dm,100.0*dm/262144,mm);
  printf("  total iterations: fxpt=%ld  myflpt=%ld  double-ref=%ld\n",itx,itm,itr);

  printf("\n=========== dynamic-range headroom ===========\n");
  printf("  fxpt  : max observed xx+yy = %lld = %.3f  (Q8.24 saturates at 128.0)\n",
         fx_max_sum, (double)fx_max_sum/(double)(1<<FXPT_FRAC));
  printf("  fxpt  : int32 overflow events = %d\n",fx_ovf);
  printf("  myflpt: mul() saturation events = %d  (format saturates at 2^5 = 32)\n",my_sat);
  printf("  myflpt: representable magnitudes 2^-251 .. 2^5, i.e. %.3e .. %.1f\n",ldexp(1,-251),32.0);

  printf("\n=========== coordinate accumulation ===========\n");
  myflpt c=float_to_myflpt(-2.0f); for(int i=0;i<512;i++) c=add(c,mdl);
  fxpt   q=FX(-2.0);               for(int i=0;i<512;i++) q+=xdl;
  printf("  myflpt: cx after 512 add() = %.9f (exact 1.0), drift = %.3e\n",md(c),md(c)-1.0);
  printf("  fxpt  : cx after 512 +=    = %.9f (exact 1.0), drift = %.3e\n",xd(q),xd(q)-1.0);
  return 0;
}
