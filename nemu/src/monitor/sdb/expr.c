/***************************************************************************************
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

/*
正则表达式中负号的匹配；
计算结果错误
*/

#include <isa.h>

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
/*提供了 POSIX 正则表达式函数的声明和定义
regcomp()：编译正则表达式。
regexec()：执行正则表达式匹配。
regerror()：获取正则表达式错误信息。
regfree()：释放编译后的正则表达式。*/
#include <regex.h>



//定义了一个枚举类型，用于表示不同的标记类型。每个标记类型都与一个整数值关联。
//这种设置是为了确保 TK_NOTYPE 的值大于 255，以便在后续代码中可以与 ASCII 字符一起使用，而不会与 ASCII 字符冲突。
enum {
  TK_NOTYPE = 256, TK_EQ, TK_NUMBER,TK_NEGATIVE,

  /* TODO: Add more token types */

};

static struct rule {
  const char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" +", TK_NOTYPE},    // spaces
  {"\\+", '+'},         // plus
  {"==", TK_EQ},        // equal
  {"-",'-'},            // minus
  {"\\*",'*'},          // multiplication
  {"/",'/'},            // divisions
  {"\\b[0-9]+\\b", TK_NUMBER},   //number 
  {"\\(", '('},         // 左括号
  {"\\)", ')'},         // 右括号
  //{"(?<![0-9\\d)])-",TK_NEGATIVE},  //负号
};


//ARRLEN(rules) 是一个宏函数，用于计算数组的长度。NR_REGEX 是一个宏定义，用于计算规则数组 rules 的元素个数。
#define NR_REGEX ARRLEN(rules)  

/*这个数组用于存储编译后的正则表达式,struct re_pattern_buffer (regex_t)是一个用于存储编译后正则表达式模式的数据结构*/
static regex_t re[NR_REGEX] = {};
/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */



//这些规则会在简易调试器初始化的时候通过init_regex()被编译成一些用于进行pattern匹配的内部信息, 这些内部信息是被库函数使用的
//通过这个函数，规则数组中的每个正则表达式都会被编译成内部表示形式，存储在 re 数组的相应位置，供后续使用。
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

//使用Token结构体来记录token的信息
typedef struct token {
  int type;
  char str[32];
} Token;



/*展示了两个全局变量的定义：tokens 和 nr_token。

tokens 是一个 Token 结构体类型的静态数组，长度为 32。Token 结构体用于表示词法分析中的标记（token），
包含一个整数类型的 type 字段和一个长度为 32 的字符串 str 字段。通过声明 Token tokens[32]，我们创建了一个可以存储
最多 32 个标记的数组。

nr_token 是一个整数类型的静态变量，用于记录已识别的标记数量。nr_token 的初始值为 0。

__attribute__((used)) 是一个编译器特定的属性（attribute），在给定的上下文中，它用于告知编译器保留这些变量即使它们没有被显式
地使用。这样做是为了防止编译器优化掉这些变量，确保它们在链接阶段能够正确地被访问到。*/
static Token tokens[32] __attribute__((used)) = {};
static int nr_token __attribute__((used))  = 0;


////////////////////////////////////////////make_tokenu函数//////////////////////////////////////////////////////////////

/*make_token()函数的工作方式十分直接, 它用position变量来指示当前处理到的位置, 
并且按顺序尝试用不同的规则来匹配当前位置的字符串. 当一条规则匹配成功, 并且匹配出的子串正好
是position所在位置的时候, 我们就成功地识别出一个token, Log()宏会输出识别成功的信息.*/
static bool make_token(char *e) {
  int position = 0;
  int i;

 //可以准确地记录正则表达式匹配的子字符串在原始字符串中的位置信息。
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

        Token token;

        switch (rules[i].token_type) 
        {
          case TK_NOTYPE:
          break;
          default: 
           strncpy(token.str, substr_start, substr_len);
           token.str[substr_len] = '\0'; 
           token.type=rules[i].token_type;
           tokens[nr_token++] = token;
           break;
        }
        regmatch_t pmatch2;

        //排查负号
        for (i=0;i<nr_token;i++)
        {
          if(regexec(&re[NR_REGEX-1], tokens[i].str, 1, &pmatch2, 0) == 0)
          {
            tokens[i].type=TK_NEGATIVE;
          }
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


/**********************************************check_parentheses函数判断括号是否合法*******************************************************************/
bool check_parentheses(word_t p,word_t q)
{
  bool flag=false;
  if(tokens[p].type=='(' && tokens[q].type == ')')
  {
    for(int i =p+1;i<q;)
    {
      
      if (tokens[i].type==')')
      {
        
        break;
      }

      else if (tokens[i].type=='(')
      {
        while(tokens[i+1].type!=')' )
        {
          i++;
          if(i==q-1)
          {
            
            break;
          }
        }
        i+=2;
      }

      else i++;
        
    }
    flag=true;
  }
  return flag;
}


/********************************************find_major函数寻找主运算符**********************************************************************/
word_t find_major(word_t p,word_t q)
{
  word_t ret=0;
  word_t par=0;//括号的数量
  word_t op_type=0; //当前找到的最高优先级的运算符类型
  word_t tmp_type=0; //相应运算符类型的等级
  for(word_t i=p;i<=q;i++)
  {
    if (tokens[i].type ==TK_NUMBER)
    {
      continue;
    }

    else if (tokens[i].type=='(')
    {
      par++;
      continue;
    }

    else if (tokens[i].type==')')
    {
      if (par==0)
      {
        return -1;
      }
      par--;
    }
    else if (par>0)
    {
      continue;
    }
    else
    {
      switch (tokens[i].type) 
      {
      case '*': case '/': tmp_type = 1; break;
      case '+': case '-': tmp_type = 2; break;
      default: assert(0);
      }
      if (tmp_type>=op_type)
      {
        op_type=tmp_type;
        ret=i;
      }
      
    }

  }
    if(par>0)
    {return -1;}
    return ret;
}


/**********************************************evalh求值函数*******************************************************************/
word_t eval(word_t p,word_t q)
{
    if(p>q)
    {
      assert(0);
    }
    else if (p==q)
    {
      word_t num;
      sscanf(tokens[p].str,"%d",&num);
      return num;
    }
    else if (check_parentheses(p, q) == true) 
    {
    /* The expression is surrounded by a matched pair of parentheses.
     * If that is the case, just throw away the parentheses.
     */
    return eval(p + 1, q - 1);
    }
    else
    {
      word_t op=find_major(p,q);  //主运算符的索引
      word_t val1 = eval(p, op - 1);
      word_t val2 = eval(op + 1, q);
      if (tokens[op].type==TK_NEGATIVE)
      {
        return -val2;
      }
       switch (tokens[op].type) 
      {
      case '+': return val1 + val2;
      case '-': return val1 - val2;
      case '*': return val1 * val2;
      case '/': return val1 / val2;
      default: assert(0);
      }
    }
    
  
}





word_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }
  return eval(0,nr_token-1);
}
