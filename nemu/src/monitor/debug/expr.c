#include <isa.h>

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>
#include <stdlib.h>
#include <ctype.h>
#include "/home/kylin/ics2020/nemu/include/memory/vaddr.h"

#define min(a,b) ( (a)<(b) ? (a):(b) )

enum {
  TK_NOTYPE = 256, 

  /* TODO: Add more token types */
  TK_DECIMAL,  TK_HEX,  TK_REG,
  TK_EQ,  TK_NEQ,  TK_AND
};

static struct rule {
  char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" +", TK_NOTYPE},    // spaces

  {"\\+", '+'},         // plus
  {"-", '-'},         // minus
  {"\\*", '*'},         // mult
  {"/", '/'},         // div
  {"\\(",'('},
  {"\\)",')'},

  {"==", TK_EQ},
  {"!=",TK_NEQ},
  {"&&",TK_AND},

  {"\\$[a-zA-Z]+",TK_REG},
  {"0x[0-9a-fA-F]+",TK_HEX},
  {"[0-9]+",TK_DECIMAL}
};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]) )

static regex_t re[NR_REGEX] = {};


void strTOupper(char *str)
{
  while(*str){
    if(islower(*str))
      *str += 'A' - 'a';
    str++;
  }
}

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[32];
} Token;

static Token tokens[32] __attribute__((used)) = {};
static int nr_token __attribute__((used))  = 0;

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);

        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */

        switch (rules[i].token_type) {
          case '+':
                tokens[nr_token].type = '+'; 
                tokens[nr_token].str[0]='+';
                nr_token++; break;
          case '-':
                tokens[nr_token].type = '-'; 
                tokens[nr_token].str[0]='-';
                nr_token++; break;
          case '*':
                tokens[nr_token].type = '*'; 
                tokens[nr_token].str[0]='*';
                nr_token++; break;
          case '/':
                tokens[nr_token].type = '/'; 
                tokens[nr_token].str[0]='/';
                nr_token++; break;
          case '(':
                tokens[nr_token].type = '('; 
                tokens[nr_token].str[0]='(';
                nr_token++; break;
          case ')':
                tokens[nr_token].type = ')'; 
                tokens[nr_token].str[0]=')';
                nr_token++; break;
          case TK_DECIMAL:
                tokens[nr_token].type = TK_DECIMAL; 
                strncpy(tokens[nr_token].str, substr_start, min(substr_len,31));
                tokens[nr_token].str[min(substr_len,31)] = '\0';
                nr_token++; break;
          case TK_HEX:
                tokens[nr_token].type = TK_HEX; 
                strncpy(tokens[nr_token].str, substr_start, min(substr_len,31));
                tokens[nr_token].str[min(substr_len,31)] = '\0';
                strTOupper(tokens[nr_token].str);
                nr_token++; break;
          case TK_REG:
                tokens[nr_token].type = TK_REG; 
                strncpy(tokens[nr_token].str, substr_start, min(substr_len,31));
                tokens[nr_token].str[min(substr_len,31)] = '\0';
                strTOupper(tokens[nr_token].str);
                nr_token++; break;
          case TK_EQ:
                tokens[nr_token].type = TK_EQ; 
                strncpy(tokens[nr_token].str, substr_start, min(substr_len,31));
                tokens[nr_token].str[min(substr_len,31)] = '\0';
                nr_token++; break;
          case TK_NEQ:
                tokens[nr_token].type = TK_NEQ; 
                strncpy(tokens[nr_token].str, substr_start, min(substr_len,31));
                tokens[nr_token].str[min(substr_len,31)] = '\0';
                nr_token++; break;
          case TK_AND:
                tokens[nr_token].type = TK_AND; 
                strncpy(tokens[nr_token].str, substr_start, min(substr_len,31));
                tokens[nr_token].str[min(substr_len,31)] = '\0';
                nr_token++; break;      
          case TK_NOTYPE: break;
          default: break;
        }

        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}

bool match_parentheses(int p, int q)
{
  int a = 0;
  for(int i = p; i <= q; i++){
    if(tokens[i].type == '(')
      a++;
    else if(tokens[i].type == ')')
      a--;
    if( a < 0 )
      return false;
  }
  return a==0;
}

bool surround_parentheses(int p, int q)
{
  if(p+1 == q)
    return tokens[p].type=='(' && tokens[q].type==')';
  else
    return tokens[p].type=='(' && tokens[q].type==')' && match_parentheses(p+1,q-1);
}


int findop(int p, int q)
{
  int res = 0;
  int flag = 0;
  for(int i=p; i<=q; i++){
    if(tokens[i].type=='(')
      flag++;
    else if(tokens[i].type==')')
      flag--;
    if(flag > 0)
      continue;
    if(tokens[i].type=='+'||tokens[i].type=='-'
     ||tokens[i].type=='*'||tokens[i].type=='/'
     ||tokens[i].type==TK_EQ||tokens[i].type==TK_NEQ
     ||tokens[i].type==TK_AND
     ){
       res = i;
       break;
     }
  }
  for(int i = res+1; i <= q; i++){
    if(tokens[i].type=='(')
      flag++;
    else if(tokens[i].type==')')
      flag--;
    if(flag > 0)
      continue;
    if(tokens[i].type==TK_AND)
    {
      res = i;
    }
    else if( (tokens[i].type==TK_EQ||tokens[i].type==TK_NEQ) &&
        (tokens[res].type==TK_EQ||tokens[res].type==TK_NEQ
        ||tokens[res].type=='+'||tokens[res].type=='-'
        ||tokens[res].type=='*'||tokens[res].type=='/')
     ){
       res = i;
     }
    else if( (tokens[i].type=='+'||tokens[i].type=='-') &&
        (tokens[res].type=='+'||tokens[res].type=='-'
       ||tokens[res].type=='*'||tokens[res].type=='/')
     ){
       res = i;
     }
     else if( (tokens[i].type=='*'||tokens[i].type=='/') &&
              (tokens[res].type=='*'||tokens[res].type=='/')
     ){
       res = i;
     }
  }
  return res;
}

word_t eval(int p, int q)
{
  if(p > q)
    return 0;

  else if(p==q){
    word_t s;
    switch(tokens[p].type){
      case TK_DECIMAL:
              return atoi(tokens[p].str);
      case TK_HEX:
              //printf("eval HEX!\n");
              //printf("%s\n",tokens[p].str);
              s = 0;
              char *ptr = tokens[p].str + 2;
              while(*ptr!='\0'){
                if(isdigit(*ptr))
                  s = s * 16 + (*ptr) - '0';
                else
                  s = s * 16 + (*ptr) - 'A' + 10;
                ptr++;
              }
              return s;
      case TK_REG:
            //printf("Reg is %s\n",tokens[p].str);
            if(!strcmp(tokens[p].str, "$EAX"))
              return cpu.eax;
            else if(!strcmp(tokens[p].str, "$AX"))
              return cpu.gpr[0]._16;
            else if(!strcmp(tokens[p].str, "$AH"))
              return cpu.gpr[0]._8[1];
            else if(!strcmp(tokens[p].str, "$AL"))
              return cpu.gpr[0]._8[0];
            else if(!strcmp(tokens[p].str, "$EBX"))
              return cpu.ebx;
            else if(!strcmp(tokens[p].str, "$BX"))
              return cpu.gpr[3]._16;
            else if(!strcmp(tokens[p].str, "$BH"))
              return cpu.gpr[3]._8[1];
            else if(!strcmp(tokens[p].str, "$BL"))
              return cpu.gpr[3]._8[0];
            else if(!strcmp(tokens[p].str, "$ECX"))
              return cpu.ecx;
            else if(!strcmp(tokens[p].str, "$CX"))
              return cpu.gpr[1]._16;
            else if(!strcmp(tokens[p].str, "$CH"))
              return cpu.gpr[1]._8[1];
            else if(!strcmp(tokens[p].str, "$CL"))
              return cpu.gpr[1]._8[0];
            else if(!strcmp(tokens[p].str, "$EDX"))
              return cpu.edx;
            else if(!strcmp(tokens[p].str, "$DX"))
              return cpu.gpr[2]._16;
            else if(!strcmp(tokens[p].str, "$DH"))
              return cpu.gpr[2]._8[1];
            else if(!strcmp(tokens[p].str, "$DL"))
              return cpu.gpr[2]._8[0];  
            else if(!strcmp(tokens[p].str, "$ESP"))
              return cpu.esp;
            else if(!strcmp(tokens[p].str, "$SP"))
              return cpu.gpr[4]._16;
            else if(!strcmp(tokens[p].str, "$EBP"))
              return cpu.ebp;
            else if(!strcmp(tokens[p].str, "$BP"))
              return cpu.gpr[5]._16;
            else if(!strcmp(tokens[p].str, "$ESI"))
              return cpu.esi;
            else if(!strcmp(tokens[p].str, "$SI"))
              return cpu.gpr[6]._16;
            else if(!strcmp(tokens[p].str, "$EDI"))
              return cpu.edi;
            else if(!strcmp(tokens[p].str, "$DI"))
              return cpu.gpr[7]._16;
            else if(!strcmp(tokens[p].str, "$PC"))
              return cpu.pc;
      default: return 0;
    }
  }

  else if(surround_parentheses(p,q))
    return eval(p+1, q-1);
  
  else if(tokens[p].type == '*'){
    word_t address = eval(p+1, q);
    return vaddr_read(address, 4);
  }

  else{
    int op = findop(p,q);
    word_t v1 = eval(p, op-1);
    word_t v2 = eval(op+1, q);
    //printf("%d %s %d\n",v1,tokens[op].str,v2);
    switch(tokens[op].type){
      case '+': return v1 + v2;
      case '-': return v1 - v2;
      case '*': return v1 * v2;
      case '/': return v2 ? v1 / v2 : 0;
      case TK_EQ: return v1==v2;
      case TK_NEQ: return v1!=v2;
      case TK_AND: return v1&&v2;
      default: return 0;
    }
  }
}

word_t expr(char *e, bool *success) {

  if (!make_token(e)) {
    *success = false;
    return 0;
  }

  /* TODO: Insert codes to evaluate the expression. */

  //for(int i=0; i<nr_token; i++)
    //printf("%s\n",tokens[i].str);

  if(!match_parentheses(0,nr_token-1)){
    *success = false;
    return 0;
  }

  *success = true;
  return eval(0, nr_token-1);
}
