#include "rpn.h"

#define stackx stack[0]
#define stacky stack[1]

rpn::rpn(void)
{
  altFn = alt_Norm;
}


void rpn::begin(display &dev)
{
  PrDev=&dev;
  push_en=true;
}

void rpn::key_input(char key)
{
  switch(altFn){
    case alt_Norm:
      key_norm(key);
      break;
    case alt_Edit:
      key_edit(key);
      break;  
    case alt_Shift:
      break;
    case alt_Alpha:
      break;
    case alt_Hyp:
      break;
  }
}

static void cdel(char *buf)
{
  for(;*buf;buf++)
    *(buf)=*(buf+1);
}

static void cins(char *buf, char c)
{
  for(;c;buf++){
    c^=*buf;
    *buf^=c;
    c^=*buf;
  }
  *buf='\0';
}

static void set_exp_sign(char *buf, bool neg)
{
  int8_t n = 0;
  
  while(buf[n] && buf[n]!='e')n++; // string MUST contain an 'e' 
  if(!buf[n])return;
  n++;
  if(neg && buf[n]=='-')return;
  if(!neg && buf[n]!='-')return;
  if(!neg)cdel(&buf[n]); //delete '-'    
  else cins(&buf[n],'-'); //insert '-'
}

void rpn::key_edit(char key)
{
  static char edln[18];
  static int8_t edpos=0,expos=0;
  static bool neg,point,ex, sexp=0;
  static int16_t exv=0;

  if(altFn != alt_Edit){
    altFn = alt_Edit;
    edln[0]='-'; //it is easier to insert the sign first and remove it later
    for(int8_t i=1;i<18;i++)edln[i]=0;
    PrDev->lcdclear(0);
    edpos=1;
    point=neg=false;
    exv=0;
    sexp=ex=false;
  }
  switch(key){
    case '0' ... '9':
      if(ex){
        if(exv*10+(key-'0')>=307)break;
        exv*=10;
        exv+=key-'0';
      }
      edln[edpos++]=key;
      PrDev->print(key);
      break;
    case ',': //sign
      if(ex){
        sexp^=1;
        set_exp_sign(edln+expos,sexp);
        edpos+=sexp?1:-1;
      }
      else{
        neg^=1; //toggle sign
      }
      PrDev->lcdclear(0);
      PrDev->print(&edln[!neg]);
      break;
    case '.':
      if(!point){
        if(edpos<2){
          PrDev->print('0');
          edln[edpos++]='0';
        }
        edln[edpos++]=key;
        PrDev->print(key);
      }
      point=true;
      break;
    case 'q':
      if(!ex){
        if(edpos==1){ /* eex cannot be first */
          PrDev->print('1');
          edln[edpos++]='1';
        }
        expos=edpos;
        edln[edpos++]='e';
        PrDev->print('e');
      }
      ex=true;
      exv=0;
      sexp=false;
      break;
    case '\177': //pc raw backspace
    case '\010': //backspace
    case 'r':
      edpos--;
      if(edln[edpos]=='.')point=false;
      if(edln[edpos]=='e'){
        ex=false;
      }
      if(ex && edln[edpos]=='-')sexp=false;
      exv=exv/10;
      edln[edpos]='\0';
      if(edpos<2){ //exit edit mode
        altFn=alt_Norm;
        PrDev->lcdprint(stackx.toString());
      }
      else{
        PrDev->lcdclear(0);
        PrDev->lcdprint(&edln[!neg]);
      }
      break;
    case '\n':
    case '\r':
      altFn=alt_Norm;
      edln[edpos]='\0';
      stackx = strtof64(&edln[!neg], NULL);
      push_en=false;
      key_norm(key);
      return;
      break;
    default:
      altFn=alt_Norm;
      edln[edpos]='\0';
      stackx = strtof64(&edln[!neg], NULL);
      key_norm(key);
  }
}

void rpn::key_norm(char key)
{
  switch(key){
    case 'q'://exp
    case '.':
    case '0' ... '9':
      stack_push();
      PrDev->lcdprint(stacky.toString(),1);
      PrDev->lcdclear(0);
      key_edit(key);
      return;
    case '+':
      stackx+=stack_pull();
      break;
    case '-':
      stackx-=stack_pull();
      break;
    case '*':
      stackx*=stack_pull();
      break;
    case '/':
      stackx/=stack_pull();
      break;
    case 'a':
      stackx=f64(1)/stackx;
      break;
    case 'b':
      stackx=sqrt64(stackx);
      break;
    case 'c':
      stackx=exp64(stack_pull() * log64(stacky));
      break;
    case 'd':
      stackx=log64(stackx);
      break;
    case 'e':
      stackx=log64(stackx)/log64(10);
      break;
    case 'f':
      stack_swapxy();
      break;
    case 'g':
      stack_pull();
      break;
    case 'h':
      stackx=sin64(stackx);
      break;
    case 'i':
      stackx=cos64(stackx);
      break;
    case 'j':
      stackx=tan64(stackx);
      break;
    case 'k':
    case 'l':
    case 'm':
    case 'n':
    case 'o':
      break;
    case 'p':
      stackx=-stackx;
      break;
    case '\177': //pc raw backspace
    case '\010': //backspace
    case 'r':
      stackx = 0;
      push_en=false;
      break;
    case '\r':
    case '\n':
      stack_push();
      break;

    default:
      PrDev->print((int)key);
      break;
  }

	PrDev->lcdprint(stacky.toString(),1);
	PrDev->lcdprint(stackx.toString());
	Serial.println();
	Serial.println("----------------");
	for(int i=STACK_TOP;i>=0;i--)Serial.println(stack[i]);
}

void rpn::stack_push(void)
{
  if(push_en){
    for(int8_t i=STACK_TOP;i>0;i--){
      stack[i] = stack[i-1];
    }
  }
  push_en=true;
}

f64 rpn::stack_pull(void)
{
  f64 result = stack[0];
  for(int8_t i=0;i<STACK_TOP;i++){
    stack[i] = stack[i+1];
  }
  stack[STACK_TOP]=0;
  return result;
}

void rpn::stack_swapxy(void)
{
  f64 z=stack[1];
  stack[1]=stack[0];
  stack[0]=z;
}

