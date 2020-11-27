#include <iostream>
#include <string>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <cstring>
using namespace std;
//分析表达式使用的函数
bool analyseLowExpr(int funtionPos,int rangePos,int *retType);
bool analyseMediumExpr(int funtionPos,int rangePos,int *retType);
bool analyseHighExpr(int funtionPos,int rangePos,int *retType);
bool analyseExpr(int funtionPos,int rangePos,int *retType);
//分析语句使用的函数
bool analyseStmt(int funtionPos,int rangePos);
bool analyseExprStmt(int funtionPos,int rangePos);
bool analyseDeclStmt(int funtionPos,int rangePos);
bool analyseIfStmt(int funtionPos,int rangePos);
bool analyseWhileStmt(int funtionPos,int rangePos);
bool analyseBreakStmt(int funtionPos,int rangePos);
bool analyseContinueStmt(int funtionPos,int rangePos);
bool analyseReturnStmt(int funtionPos,int rangePos);
bool analyseBlockStmt(int funtionPos,int rangePos);
bool analyseEmptyStmt(int funtionPos,int rangePos);
//TODO:按指令使用情况检查
uint64_t doubleToRawBits(double x) {
    uint64_t bits;
    memcpy(&bits, &x, sizeof bits);
    return bits;
}
void intToTwoBits(int x,unsigned char *str){
    str[1]=x;
    str[2]=x>>8;
}
void intToFourBits(int x,unsigned char *str){
    str[1]=x;
    str[2]=x>>8;
    str[3]=x>>16;
    str[4]=x>>24;
}
void intToEightBits(int64_t x,unsigned char *str){
    str[1]=x;
    str[2]=x>>8;
    str[3]=x>>16;
    str[4]=x>>24;
    str[5]=x>>32;
    str[6]=x>>40;
    str[7]=x>>48;
    str[8]=x>>56;
}
void pushIns(int x,vector<unsigned char> &instructions){
    unsigned char str[5];
    memset(str,0,sizeof(str));
    intToFourBits(x,str);
    instructions.push_back(str[4]);
    instructions.push_back(str[3]);
    instructions.push_back(str[2]);
    instructions.push_back(str[1]);
}
void pushIns(int64_t x,vector<unsigned char> &instructions){
    unsigned char str[9];
    memset(str,0,sizeof(str));
    intToEightBits(x,str);
    instructions.push_back(str[8]);
    instructions.push_back(str[7]);
    instructions.push_back(str[6]);
    instructions.push_back(str[5]);
    instructions.push_back(str[4]);
    instructions.push_back(str[3]);
    instructions.push_back(str[2]);
    instructions.push_back(str[1]);
}
void pushIns(int16_t x,vector<unsigned char> &instructions){
    unsigned char str[3];
    memset(str,0,sizeof(str));
    intToTwoBits(x,str);
    instructions.push_back(str[2]);
    instructions.push_back(str[1]);
}
enum TokenType{//TODO:标准库函数
    NULL_TOKEN,
    FN_KW,
    LET_KW,
    CONST_KW,
    AS_KW,
    WHILE_KW,
    IF_KW,
    ELSE_KW,
    RETURN_KW,
    BREAK_KW,
    CONTINUE_KW,
    GETINT,
    GETDOUBLE,
    GETCHAR,
    PUTINT,
    PUTDOUBLE,
    PUTCHAR,
    PUTSTR,
    PUTLN,
    IDENT,
    UINT_LITERAL,
    DOUBLE_LITERAL,
    STRING_LITERAL,
    CHAR_LITERAL,
    PLUS,
    MINUS,
    MUL,
    DIV,
    ASSIGN,
    EQ,
    NEQ,
    LT,
    GT,
    LE,
    GE,
    L_PAREN,
    R_PAREN,
    L_BRACE,
    R_BRACE,
    ARROW,
    COMMA,
    COLON,
    SEMICOLON,
};
enum DFAState{
    INITIAL_STATE,
    IDENTIFIER_STATE,
    NUMBER_STATE,
    STRING_LITERAL_STATE,
    CHAR_LITERAL_STATE,
    PLUS_STATE,
    MINUS_SIGN_STATE,
    MUL_STATE,
    DIV_SIGN_STATE,
    EQUAL_SIGN_STATE,
    LESS_SIGN_STATE,
    GREATER_SIGN_STATE,
    LP_STATE,
    RP_STATE,
    LB_STATE,
    RB_STATE,
    COMMA_STATE,
    COLON_STATE,
    SEMICOLON_STATE,
};
int line=1,offset=0,preline=1,preoffset=0;
typedef struct Pos{
    int prePos;
    int endPos;
    Pos():prePos(line),endPos(offset){}
}Postion;
typedef struct TK{
    string value;
    TokenType type;
    bool success;
    Postion postion;
    TK():value(""),type(TokenType::NULL_TOKEN),success(false),postion(){}
    TK(Postion pos):value(""),type(TokenType::NULL_TOKEN),success(false),postion(pos){}
    TK(string val,TokenType ty,Postion pos):value(),type(ty),success(true),postion(pos){
        value=val;
        }
}Token;
FILE* file=NULL;
FILE* outFile=NULL;
bool isEOF=false;
//必须在调用后检查isEOF的值
char getChar(){
    char c=fgetc(file);
    if(c==EOF) {
        isEOF=true;
        return c;
    }
    if(c=='\n'){
        preline=line;
        line++;
        preoffset=offset;
        offset=0;
    }
    else {
        preoffset=offset;
        offset++;
    }
    return c;
}
void unGetCh(){
    fseek(file,-1,SEEK_CUR);
    offset=preoffset;
    line=preline;
}
Token checkIDENTIFIER(string s,Postion pos){
    if(s=="fn") return Token(s,TokenType::FN_KW,pos);
    else if(s=="let") return Token(s,TokenType::LET_KW,pos);
    else if(s=="const") return Token(s,TokenType::CONST_KW,pos);
    else if(s=="as") return Token(s,TokenType::AS_KW,pos);
    else if(s=="while") return Token(s,TokenType::WHILE_KW,pos);
    else if(s=="break") return Token(s,TokenType::BREAK_KW,pos);
    else if(s=="continue") return Token(s,TokenType::CONTINUE_KW,pos);
    else if(s=="if") return Token(s,TokenType::IF_KW,pos);
    else if(s=="else") return Token(s,TokenType::ELSE_KW,pos);
    else if(s=="return") return Token(s,TokenType::RETURN_KW,pos);
    else if(s=="getint") return Token(s,TokenType::GETINT,pos);
    else if(s=="getdouble") return Token(s,TokenType::GETDOUBLE,pos);
    else if(s=="getchar") return Token(s,TokenType::GETCHAR,pos);
    else if(s=="putint") return Token(s,TokenType::PUTINT,pos);
    else if(s=="putdouble") return Token(s,TokenType::PUTDOUBLE,pos);
    else if(s=="putchar") return Token(s,TokenType::PUTCHAR,pos);
    else if(s=="putstr") return Token(s,TokenType::PUTSTR,pos);
    else if(s=="putln") return Token(s,TokenType::PUTLN,pos);
    else return Token(s,TokenType::IDENT,pos);
}
Token NT(){
    string str="";
    Postion startPostion = Postion();
    DFAState currentState = DFAState::INITIAL_STATE;
    bool doubleFlag = false;
    bool eEFlag = false;
    while(true){
        char currentChar = getChar();
        switch (currentState)
        {
        case INITIAL_STATE:{
            if(isEOF) return Token();
            bool invalid = false;
            if(isspace(currentChar)) break;
            else if(!isprint(currentChar)) invalid = true;
            else if(isdigit(currentChar)) currentState = DFAState::NUMBER_STATE;
            else if(isalpha(currentChar) || currentChar=='_') currentState = DFAState::IDENTIFIER_STATE;
            else{
                switch(currentChar){
                    case '\"':
                        currentState = DFAState::STRING_LITERAL_STATE;
                        break;
                    case '\'':
                        currentState = DFAState::CHAR_LITERAL_STATE;
                        break;
                    case '+':
                        currentState = DFAState::PLUS_STATE;
                        break;
                    case '-':
                        currentState = DFAState::MINUS_SIGN_STATE;
                        break;
                    case '*':
                        currentState = DFAState::MUL_STATE;
                        break;
                    case '=':
                        currentState = DFAState::EQUAL_SIGN_STATE;
                        break;
                    case '<':
                        currentState = DFAState::LESS_SIGN_STATE;
                        break;
                    case '>':
                        currentState = DFAState::GREATER_SIGN_STATE;
                        break;
                    case '(':
                        currentState = DFAState::LP_STATE;
                        break;
                    case ')':
                        currentState = DFAState::RP_STATE;
                        break;
                    case '{':
                        currentState = DFAState::LB_STATE;
                        break;
                    case '}':
                        currentState = DFAState::RB_STATE;
                        break;
                    case ',':
                        currentState = DFAState::COMMA_STATE;
                        break;
                    case ':':
                        currentState = DFAState::COLON_STATE;
                        break;
                    case ';':
                        currentState = DFAState::SEMICOLON_STATE;
                        break;
                    default:
                        invalid=true;
                        break;
                }
            }
            if(currentState!=DFAState::INITIAL_STATE) startPostion=Postion();
            if(invalid){
                unGetCh();
                return Token(startPostion);
            }
            str+=currentChar;
            break;
        }
        case IDENTIFIER_STATE:{
            if(isEOF) return checkIDENTIFIER(str,startPostion);
            if(isalnum(currentChar) || currentChar=='_') str+=currentChar;
            else{
                unGetCh();
                return checkIDENTIFIER(str,startPostion);
            }
            break;
        }
        case NUMBER_STATE:{
            if(isEOF){
                if(doubleFlag){
                    return Token(str,TokenType::DOUBLE_LITERAL,startPostion);
                }
                else{
                    return Token(str,TokenType::UINT_LITERAL,startPostion);
                }
            }
            if(isdigit(currentChar)) str+=currentChar;
            else if(doubleFlag==false && currentChar=='.'){
                char preChar = getChar();
                if(isdigit(preChar)){
                    unGetCh();
                    doubleFlag=true;
                    str+=currentChar;
                }
                else{
                    //包括了isEOf的错误返回
                    return Token(startPostion);
                }
            }
            else if(doubleFlag==true && eEFlag==false && (currentChar=='e' || currentChar=='E')){
                str+=currentChar;
                char preChar = getChar();
                if(isdigit(preChar) || preChar=='+' || preChar=='-'){
                    str+=preChar;
                }
                eEFlag=true;
            }
            else{
                unGetCh();
                if(doubleFlag){
                    return Token(str,TokenType::DOUBLE_LITERAL,startPostion);
                }
                else{
                    return Token(str,TokenType::UINT_LITERAL,startPostion);
                }
            }
            break;
        }
        case STRING_LITERAL_STATE:{
            if(isEOF) return Token(startPostion);
            if(currentChar=='\\'){
                char preChar = getChar();
                if(preChar=='n') str+='\n';
                else if(preChar=='r') str+='\r';
                else if(preChar=='t') str+='\t';
                else if(preChar=='\\') str+='\\';
                else if(preChar=='\'') str+='\'';
                else if(preChar=='\"') str+='\"';
                else return Token(startPostion);
            }
            else if(currentChar!='\"') str+=currentChar;
            else return Token(str,TokenType::STRING_LITERAL,startPostion);
            break;
        }
        case CHAR_LITERAL_STATE:{
            if(isEOF) return Token(startPostion);
            if(currentChar=='\\'){
                char preChar = getChar();
                if(preChar=='n') str+='\n';
                else if(preChar=='r') str+='\r';
                else if(preChar=='t') str+='\t';
                else if(preChar=='\\') str+='\\';
                else if(preChar=='\'') str+='\'';
                else if(preChar=='\"') str+='\"';
                else return Token(startPostion);
                //必须以'结尾
                preChar = getChar();
                if(preChar=='\'') return Token(str,TokenType::CHAR_LITERAL,startPostion);
                else return Token(startPostion);
            }
            else if(currentChar!='\'') {
                str+=currentChar;
                char preChar = getChar();
                if(preChar=='\'') return Token(str,TokenType::CHAR_LITERAL,startPostion);
                else return Token(startPostion);
            }
            else return Token(startPostion);
            break;
            break;
        }
        case PLUS_STATE:{
            unGetCh();
            return Token("+",TokenType::PLUS,startPostion);
            break;
        }
        case MINUS_SIGN_STATE:{
            if(currentChar=='>') return Token("->",TokenType::ARROW,startPostion);
            else{
                unGetCh();
                return Token("-",TokenType::MINUS,startPostion);
            }
            break;
        }
        case MUL_STATE:{
            unGetCh();
            return Token("*",TokenType::MUL,startPostion);
            break;
        }
        case DIV_SIGN_STATE:{
            if(currentChar=='/'){
                char preChar=getChar();
                while(preChar!=EOF || preChar!='\n') preChar=getChar();
                if(preChar==EOF) return Token(startPostion);
                currentState = DFAState::INITIAL_STATE;
            }
            else{
                unGetCh();
                return Token("/",TokenType::DIV,startPostion);
            }
            break;
        }
        case EQUAL_SIGN_STATE:{
            if(currentChar=='=') return Token("==",TokenType::EQ,startPostion);
            else{
                unGetCh();
                return Token("=",TokenType::ASSIGN,startPostion);
            }
            break;
        }
        case LESS_SIGN_STATE:{
            if(currentChar=='=') return Token("<=",TokenType::LE,startPostion);
            else{
                unGetCh();
                return Token("<",TokenType::LT,startPostion);
            }
            break;
        }
        case GREATER_SIGN_STATE:{
            if(currentChar=='=') return Token(">=",TokenType::GE,startPostion);
            else{
                unGetCh();
                return Token(">",TokenType::GT,startPostion);
            }
            break;
        }
        case LP_STATE:{
            unGetCh();
            return Token("(",TokenType::L_PAREN,startPostion);
            break;
        }
        case RP_STATE:{
            unGetCh();
            return Token(")",TokenType::R_PAREN,startPostion);
            break;
        }
        case LB_STATE:{
            unGetCh();
            return Token("{",TokenType::L_BRACE,startPostion);
            break;
        }
        case RB_STATE:{
            unGetCh();
            return Token("}",TokenType::R_BRACE,startPostion);
            break;
        }
        case COMMA_STATE:{
            unGetCh();
            return Token(",",TokenType::COMMA,startPostion);
            break;
        }
        case COLON_STATE:{
            unGetCh();
            return Token(":",TokenType::COLON,startPostion);
            break;
        }
        case SEMICOLON_STATE:{
            unGetCh();
            return Token(";",TokenType::SEMICOLON,startPostion);
            break;
        }
        default:{
            return Token();
            break;
        }
        }
    }
}
//使用过token后要将unusedToken置为false
bool unusedToken=false;
//  处理表达式需预读2个token
Token currentToken;
Token beforeToken;
void nextToken(){
    if(!unusedToken) {
        beforeToken=currentToken;
        currentToken=NT();
        unusedToken=true;
    }
}
//ty -> IDENT    仅限int double void
bool tyIDENT(string &ty){
    nextToken();
    if(!currentToken.success){
        return false;
    }
    if(currentToken.type!=TokenType::IDENT) return false;
    else if(currentToken.value=="int" || currentToken.value=="double" || currentToken.value=="void"){
        ty=currentToken.value;
        unusedToken=false;
        return true;
    }
    else {
        unusedToken=false;
        return false;
    }
}
/*
    符号表分为3部分：
    1.全局变量常量表
        储存全局变量常量，若为常量，储存值，包括字符串字面量
    2.一个block一个局部变量常量表
        储存局部变量常量，若为常量，储存值
    3.函数表
        储存名字，返回值属性、指令集（唯一存指令的位置）、参数表
    变量常量插入时：
        全局变量：只在全局变量表中查重
        局部变量：若当前域的父域为-1（即函数第一层域），需要在参数列表和当前局部变量表中查重；
            其他情况只当前局部变量表中查重；
        参数变量：与所在函数的参数变量查重
        函数：与函数表查重
    变量常量查表时：先在局部变量表树中向上查找，再在所属函数参数表查找，最后在全局变量查找，找到即返回true，没有时为false
*/
//全局变量常量、字符串字面量交错存储，函数在二者之后
typedef struct GLOBAL{
    string name;
    string dataType;
    bool is_const;
    GLOBAL():name(),dataType(),is_const(){}
    GLOBAL(string name,string dataType,bool is_const):name(name),dataType(dataType),is_const(is_const){}
}Global;vector<Global> Gmap;
typedef struct LOCAL{
    vector<Global> vars;
    //loca要的位置TODO:设置
    vector<int> postionInFuntion;
    //所属函数
    int funtionPos;
    //父域，-1表示父域为全局变量，其余为Lmap下表
    int upRange;
    /*
    存放待填写的breaks，first是Fmap[funtionPos].instructions[insPos]，代码待填写的位置
    second是当前指令数，用于被insNum减，得到放入first位置的值
    */
    vector<pair<int,int> > breaks;
    bool is_while;
    int continueNum;//TODO:初始化
    LOCAL(int funtionPos,int upRange):vars(),postionInFuntion(),funtionPos(funtionPos),upRange(upRange),breaks(),is_while(false),continueNum(){}
}Local;vector<Local> Lmap;
typedef struct FUNTION{
    int pos;
    int localSlotNum;
    int paramSlotNum;//TODO:检查是否增加Num
    string name;
    string retType;
    //!=instructions.size()
    int insNum;
    vector<unsigned char> instructions;
    vector<Global> params;
    FUNTION(string name):pos(0),localSlotNum(0),paramSlotNum(0),name(name),retType(""),insNum(0),instructions(),params(){}
}Funtion;vector<Funtion> Fmap;
// # 表达式
/*
expr -> 
    operator_expr
    | negate_expr
    | assign_expr
    | as_expr
    | call_expr
    | literal_expr
    | ident_expr
    | group_expr      
expr=high_expr(比较运算符high_expr)*
high_expr=medium_expr(加减运算符medium_expr)*
medium_expr=low_expr(乘除运算符low_expr)*
*/
//应先预读到IDENT、'-'、'('、literal 才进入analyseExpr
/*
retType:
0代表需要将其赋值
1代表检查是否为int
2代表检查是否为double
3代表void
4代表bool
*/
bool analyseExpr(int funtionPos,int rangePos,int *retType){
    if(!analyseHighExpr(funtionPos,rangePos,retType)) return false;
    bool boolFlag=false;
    while(true){
        nextToken();
        if(!currentToken.success) return false;
        if(currentToken.type==TokenType::LT){
            if(!analyseHighExpr(funtionPos,rangePos,retType)) return false;
            //cmp.i或cmp.f
            if(*retType==1) Fmap[funtionPos].instructions.push_back(0x30);
            else Fmap[funtionPos].instructions.push_back(0x31);
            Fmap[funtionPos].insNum++;
            Fmap[funtionPos].instructions.push_back(0x39);
            Fmap[funtionPos].insNum++;
            boolFlag=true;
        }
        else if(currentToken.type==TokenType::LE){
            if(!analyseHighExpr(funtionPos,rangePos,retType)) return false;
            if(*retType==1) Fmap[funtionPos].instructions.push_back(0x30);
            else Fmap[funtionPos].instructions.push_back(0x31);
            Fmap[funtionPos].insNum++;
            Fmap[funtionPos].instructions.push_back(0x3a);
            Fmap[funtionPos].insNum++;
            Fmap[funtionPos].instructions.push_back(0x2d);
            Fmap[funtionPos].insNum++;
            boolFlag=true;
        }
        else if(currentToken.type==TokenType::GT){
            if(!analyseHighExpr(funtionPos,rangePos,retType)) return false;
            if(*retType==1) Fmap[funtionPos].instructions.push_back(0x30);
            else Fmap[funtionPos].instructions.push_back(0x31);
            Fmap[funtionPos].insNum++;
            Fmap[funtionPos].instructions.push_back(0x3a);
            Fmap[funtionPos].insNum++;
            boolFlag=true;
        }
        else if(currentToken.type==TokenType::GE){
            if(!analyseHighExpr(funtionPos,rangePos,retType)) return false;
            if(*retType==1) Fmap[funtionPos].instructions.push_back(0x30);
            else Fmap[funtionPos].instructions.push_back(0x31);
            Fmap[funtionPos].insNum++;
            Fmap[funtionPos].instructions.push_back(0x39);
            Fmap[funtionPos].insNum++;
            Fmap[funtionPos].instructions.push_back(0x2d);
            Fmap[funtionPos].insNum++;
            boolFlag=true;
        }   
        else if(currentToken.type==TokenType::EQ){
            if(!analyseHighExpr(funtionPos,rangePos,retType)) return false;
            if(*retType==1) Fmap[funtionPos].instructions.push_back(0x30);
            else Fmap[funtionPos].instructions.push_back(0x31);
            Fmap[funtionPos].insNum++;
            Fmap[funtionPos].instructions.push_back(0x2d);
            Fmap[funtionPos].insNum++;
            boolFlag=true;
        }
        else if(currentToken.type==TokenType::NEQ){
            if(!analyseHighExpr(funtionPos,rangePos,retType)) return false;
            if(*retType==1) Fmap[funtionPos].instructions.push_back(0x30);
            else Fmap[funtionPos].instructions.push_back(0x31);
            Fmap[funtionPos].insNum++;
            boolFlag=true;
        }
        else break;
        unusedToken=false;
    }
    if(boolFlag) *retType=4;
    return true;
}
bool analyseHighExpr(int funtionPos,int rangePos,int *retType){
    if(!analyseMediumExpr(funtionPos,rangePos,retType)) return false;
    while(true){
        nextToken();
        if(!currentToken.success) return false;
        if(currentToken.type==TokenType::PLUS){
            if(!analyseMediumExpr(funtionPos,rangePos,retType)) return false;
            if(*retType==1){
                Fmap[funtionPos].instructions.push_back(0x20);
                Fmap[funtionPos].insNum++;
            }
            else if(*retType==2){
                Fmap[funtionPos].instructions.push_back(0x24);
                Fmap[funtionPos].insNum++;
            }
            else return false;
        }
        else if(currentToken.type==TokenType::MINUS){
            if(!analyseMediumExpr(funtionPos,rangePos,retType)) return false;
            if(*retType==1) {
                Fmap[funtionPos].instructions.push_back(0x21);
                Fmap[funtionPos].insNum++;
            }
            else if(*retType==2){
                Fmap[funtionPos].instructions.push_back(0x25);
                Fmap[funtionPos].insNum++;
            }
            else return false;
        }
        else break;
        unusedToken=false;
    }
    return true;
}
bool analyseMediumExpr(int funtionPos,int rangePos,int *retType){
    if(!analyseLowExpr(funtionPos,rangePos,retType)) return false;
    while(true){
        nextToken();
        if(!currentToken.success) return false;
        if(currentToken.type==TokenType::MUL){
            if(!analyseLowExpr(funtionPos,rangePos,retType)) return false;
            if(*retType==1){
                Fmap[funtionPos].instructions.push_back(0x22);
                Fmap[funtionPos].insNum++;
            }
            else if(*retType==2){
                Fmap[funtionPos].instructions.push_back(0x26);
                Fmap[funtionPos].insNum++;
            }
            else return false;
        }
        else if(currentToken.type==TokenType::DIV){
            if(!analyseLowExpr(funtionPos,rangePos,retType)) return false;
            if(*retType==1) {
                Fmap[funtionPos].instructions.push_back(0x23);
                Fmap[funtionPos].insNum++;
            }
            else if(*retType==2){
                Fmap[funtionPos].instructions.push_back(0x27);
                Fmap[funtionPos].insNum++;
            }
        }
        else break;
        unusedToken=false;
    }
    return true;
}
/*
low_expr:
@预读
if 预读1==IDENT
    @预读
    if 预读2=='('
        call_expr -> IDENT '(' call_param_list? ')' call_param_list -> expr (',' expr)*
    else if 预读2=='='
        assign_expr -> l_expr '=' expr  //能够解决右结合
    else ident_expr -> IDENT
else if 预读1=='-'
    negate_expr -> '-' expr
else if 预读1=='('
    group_expr '(' expr ')'
else if 预读1==literal
    literal_expr
else wrong
@预读
if 预读1=='as'
    as_expr = expr 'as' ty
*/
//检查类型后向栈中压入参数
bool analyseCallParamList(int funtionPos,int rangePos,int callFuntionPos){
    //已保证至少有一个参数才会进入该函数
    int retType=0;
    if(!analyseExpr(funtionPos,rangePos,&retType)) return false;
    if(retType==1){
        if(Fmap[callFuntionPos].params[0].dataType!="int") return false;
    }
    else if(retType==2){
        if(Fmap[callFuntionPos].params[0].dataType!="double") return false;
    }
    else if(retType==3){
        if(Fmap[callFuntionPos].params[0].dataType!="void") return false;
    }
    else return false;
    for(int i=1;i<Fmap[callFuntionPos].params.size();i++){
        nextToken();
        if(!currentToken.success) return false;
        if(currentToken.type!=TokenType::COMMA) return false;
        unusedToken=false;
        nextToken();
        if(!currentToken.success) return false;
        retType=0;
        if(!analyseExpr(funtionPos,rangePos,&retType)) return false;
        if(retType==1){
            if(Fmap[callFuntionPos].params[i].dataType!="int") return false;
        }
        else if(retType==2){
            if(Fmap[callFuntionPos].params[i].dataType!="double") return false;
        }
        else if(retType==3){
            if(Fmap[callFuntionPos].params[i].dataType!="void") return false;
        }
        else return false;
    }
    return true;
}
bool analyseLowExpr(int funtionPos,int rangePos,int *retType){
    if(currentToken.type==TokenType::L_PAREN){
        unusedToken=false;
        nextToken();
        if(!currentToken.success) return false;
        if(currentToken.type==TokenType::L_PAREN
        ||currentToken.type==TokenType::UINT_LITERAL 
        || currentToken.type==TokenType::DOUBLE_LITERAL 
        ||currentToken.type==TokenType::STRING_LITERAL
        ||currentToken.type==TokenType::CHAR_LITERAL
        ||currentToken.type==TokenType::IDENT
        ||currentToken.type==TokenType::GETINT
        ||currentToken.type==TokenType::GETDOUBLE
        ||currentToken.type==TokenType::GETCHAR
        ||currentToken.type==TokenType::PUTINT
        ||currentToken.type==TokenType::PUTDOUBLE
        ||currentToken.type==TokenType::PUTCHAR
        ||currentToken.type==TokenType::PUTSTR
        ||currentToken.type==TokenType::PUTLN
        ||currentToken.type==TokenType::MINUS){
            //expr
            if(!analyseExpr(funtionPos,rangePos,retType)) return false;
            //')'
            nextToken();
            if(!currentToken.success) return false;
            if(currentToken.type==TokenType::R_PAREN) unusedToken=false;
            else return false;
        }
        else return false;
        unusedToken=false;
        nextToken();
    }
    else if(currentToken.type==TokenType::UINT_LITERAL){
        //要求该lowexpr的返回类型不是int
        if(*retType!=0 && *retType!=1) return false;
        //TODO:int64_t的上限？
        int64_t temp = atoi(currentToken.value.c_str());
        Fmap[funtionPos].instructions.push_back(0x01);
        pushIns(temp,Fmap[funtionPos].instructions);
        Fmap[funtionPos].insNum++;
        if(*retType==0) *retType=1;
        unusedToken=false;
        nextToken();
    }
    else if(currentToken.type==TokenType::DOUBLE_LITERAL){
        //要求该lowexpr的返回类型不是double
        if(*retType!=0 && *retType!=2) return false;
        double tempd = atof(currentToken.value.c_str());
        int64_t temp = doubleToRawBits(tempd);
        Fmap[funtionPos].instructions.push_back(0x01);
        pushIns(temp,Fmap[funtionPos].instructions);
        Fmap[funtionPos].insNum++;
        if(*retType==0) *retType=2;
        unusedToken=false;
        nextToken();
    }
    else if(currentToken.type==TokenType::STRING_LITERAL){
        //要求该lowexpr的返回类型不是int
        if(*retType!=0 && *retType!=1) return false;
        Global temp=Global(currentToken.value,"string",true);
        int64_t tempNum = Gmap.size();
        Gmap.push_back(temp);
        Fmap[0].instructions.push_back(0x01);
        pushIns(tempNum,Fmap[0].instructions);
        Fmap[0].insNum++;
        if(*retType==0) *retType=1;
        unusedToken=false;
        nextToken();
    }
    else if(currentToken.type==TokenType::CHAR_LITERAL){
        //要求该lowexpr的返回类型不是int
        if(*retType!=0 && *retType!=1) return false;
        int64_t tempNum = currentToken.value.c_str()[0];
        Fmap[0].instructions.push_back(0x01);
        pushIns(tempNum,Fmap[0].instructions);
        Fmap[0].insNum++;
        if(*retType==0) *retType=1;
        unusedToken=false;
        nextToken();
    }
    //函数调用或赋值语句或变量调用
    else if(currentToken.type==TokenType::IDENT){
        Token tempToken=currentToken;
        unusedToken=false;
        nextToken();
        if(!currentToken.success) return false;
        //函数调用TODO:函数调用自身 检查
        if(currentToken.type==TokenType::L_PAREN){
            //查找funtion，从main开始
            int callFuntionPos=0;
            for(int i=1;i<Fmap.size();i++)
                if(tempToken.value==Fmap[i].name){
                    callFuntionPos=i;
                    break;
                }
            //没找到
            if(callFuntionPos==0) return false;
            unusedToken=false;
            if(Fmap[callFuntionPos].params.size()>=1)
                if(!analyseCallParamList(funtionPos,rangePos,callFuntionPos)) return false;
            //压入_ret
            if(Fmap[callFuntionPos].retType=="void"){
                //找到的函数返回值不是要求的返回值
                if(*retType!=0 && *retType!=3) return false;
                //压入
                Fmap[funtionPos].instructions.push_back(0x1a);
                pushIns(0,Fmap[funtionPos].instructions);
                Fmap[funtionPos].insNum++;
                //返回调用者想要查看的返回值类型
                if(*retType==0) *retType=3;
            }
            else if(Fmap[callFuntionPos].retType=="int"){
                //找到的函数返回值不是要求的返回值
                if(*retType!=0 && *retType!=1) return false;
                //压入
                Fmap[funtionPos].instructions.push_back(0x1a);
                pushIns(1,Fmap[funtionPos].instructions);
                Fmap[funtionPos].insNum++;
                //返回调用者想要查看的返回值类型
                if(*retType==0) *retType=1;
            } 
            else if(Fmap[callFuntionPos].retType=="double"){
                //找到的函数返回值不是要求的返回值
                if(*retType!=0 && *retType!=2) return false;
                //压入
                Fmap[funtionPos].instructions.push_back(0x1a);
                pushIns(1,Fmap[funtionPos].instructions);
                Fmap[funtionPos].insNum++;
                //返回调用者想要查看的返回值类型
                if(*retType==0) *retType=2;
            }
            //call(callFuntionPos)
            Fmap[funtionPos].instructions.push_back(0x48);
            pushIns(callFuntionPos,Fmap[funtionPos].instructions);
            Fmap[funtionPos].insNum++;
            //右括号闭合
            nextToken();
            if(!currentToken.success) return false;
            if(currentToken.type!=TokenType::R_PAREN) return false;
            unusedToken=false;
        }
        //赋值语句
        else if(currentToken.type==TokenType::ASSIGN){
            //调用者想要的返回值不是void
            if(*retType!=0 && *retType!=3) return false;
            //查找变量
            int tempRangePos = rangePos;
            int varType=0;
            bool local=false,param=false,global=false;
            //向上域进行查找
            while(Lmap[tempRangePos].upRange!=-1){
                for(int i=0;i<Lmap[tempRangePos].vars.size();i++){
                    if(tempToken.value==Lmap[tempRangePos].vars[i].name){
                        if(Lmap[tempRangePos].vars[i].is_const) return false;
                        if(Lmap[tempRangePos].vars[i].dataType=="int") varType=1;
                        else if(Lmap[tempRangePos].vars[i].dataType=="double") varType=2;
                        else return false;
                        local=true;
                        //loca()
                        Fmap[funtionPos].instructions.push_back(0x0a);
                        pushIns(Lmap[tempRangePos].postionInFuntion[i],Fmap[funtionPos].instructions);
                        Fmap[funtionPos].insNum++;
                        break;
                    }
                }
                if(local) break;
            }
            //函数的block
            if(!local){
                for(int i=0;i<Lmap[tempRangePos].vars.size();i++){
                    if(tempToken.value==Lmap[tempRangePos].vars[i].name){
                        if(Lmap[tempRangePos].vars[i].is_const) return false;
                        if(Lmap[tempRangePos].vars[i].dataType=="int") varType=1;
                        else if(Lmap[tempRangePos].vars[i].dataType=="double") varType=2;
                        else return false;
                        local=true;
                        //loca()
                        Fmap[funtionPos].instructions.push_back(0x0a);
                        pushIns(Lmap[tempRangePos].postionInFuntion[i],Fmap[funtionPos].instructions);
                        Fmap[funtionPos].insNum++;
                        break;
                    }
                }
            }
            //函数的参数
            if(!local){
                for(int i=0;i<Fmap[funtionPos].params.size();i++){
                    if(tempToken.value==Fmap[funtionPos].params[i].name){
                        if(Fmap[funtionPos].params[i].is_const) return false;
                        if(Fmap[funtionPos].params[i].dataType=="int") varType=1;
                        else if(Fmap[funtionPos].params[i].dataType=="double") varType=2;
                        else return false;
                        param=true;
                        //arga()
                        Fmap[funtionPos].instructions.push_back(0x0b);
                        pushIns(i+1,Fmap[funtionPos].instructions);
                        Fmap[funtionPos].insNum++;
                        break;
                    }
                }
            }
            //全局变量
            if(!local && !param){
                for(int i=0;i<Gmap.size();i++){
                    if(Gmap[i].dataType!="string"){
                        if(tempToken.value==Gmap[i].name){
                            if(Gmap[i].is_const) return false;
                            if(Gmap[i].dataType=="int") varType=1;
                            else if(Gmap[i].dataType=="double") varType=2;
                            else return false;
                            global=true;
                            //globa()
                            Fmap[funtionPos].instructions.push_back(0x0c);
                            pushIns(i+1,Fmap[funtionPos].instructions);
                            Fmap[funtionPos].insNum++;
                            break;
                        }
                    }
                }
            }
            if(!local && !param && !global) return false;
            //expr
            unusedToken=false;
            nextToken();
            if(!currentToken.success) return false;
            if(!analyseExpr(funtionPos,rangePos,&varType)) return false;
            if(*retType==0) *retType=3;
            //store64
            Fmap[funtionPos].instructions.push_back(0x17);
            Fmap[funtionPos].insNum++;
            return true;
        }
        //变量调用
        else{
            //查找变量
            int tempRangePos = rangePos;
            bool local=false,param=false,global=false;
            //向上域进行查找
            while(Lmap[tempRangePos].upRange!=-1){
                for(int i=0;i<Lmap[tempRangePos].vars.size();i++){
                    if(tempToken.value==Lmap[tempRangePos].vars[i].name){
                        if(Lmap[tempRangePos].vars[i].is_const) return false;
                        if(Lmap[tempRangePos].vars[i].dataType=="int") {
                            if(*retType!=0 && *retType!=1) return false;
                            if(*retType==0) *retType=1;
                        }
                        else if(Lmap[tempRangePos].vars[i].dataType=="double") {
                            if(*retType!=0 && *retType!=2) return false;
                            if(*retType==0) *retType=2;
                        }
                        else if(Lmap[tempRangePos].vars[i].dataType=="void") {
                            if(*retType!=0 && *retType!=3) return false;
                            if(*retType==0) *retType=3;
                        }
                        else return false;
                        local=true;
                        //loca()
                        Fmap[funtionPos].instructions.push_back(0x0a);
                        pushIns(Lmap[tempRangePos].postionInFuntion[i],Fmap[funtionPos].instructions);
                        Fmap[funtionPos].insNum++;
                        break;
                    }
                }
                if(local) break;
            }
            //函数的block
            if(!local){
                for(int i=0;i<Lmap[tempRangePos].vars.size();i++){
                    if(tempToken.value==Lmap[tempRangePos].vars[i].name){
                        if(Lmap[tempRangePos].vars[i].is_const) return false;
                        if(Lmap[tempRangePos].vars[i].dataType=="int") {
                            if(*retType!=0 && *retType!=1) return false;
                            if(*retType==0) *retType=1;
                        }
                        else if(Lmap[tempRangePos].vars[i].dataType=="double") {
                            if(*retType!=0 && *retType!=2) return false;
                            if(*retType==0) *retType=2;
                        }
                        else if(Lmap[tempRangePos].vars[i].dataType=="void") {
                            if(*retType!=0 && *retType!=3) return false;
                            if(*retType==0) *retType=3;
                        }
                        else return false;
                        local=true;
                        //loca()
                        Fmap[funtionPos].instructions.push_back(0x0a);
                        pushIns(Lmap[tempRangePos].postionInFuntion[i],Fmap[funtionPos].instructions);
                        Fmap[funtionPos].insNum++;
                        break;
                    }
                }
            }
            //函数的参数
            if(!local){
                for(int i=0;i<Fmap[funtionPos].params.size();i++){
                    if(tempToken.value==Fmap[funtionPos].params[i].name){
                        if(Fmap[funtionPos].params[i].is_const) return false;
                        if(Fmap[funtionPos].params[i].dataType=="int") {
                            if(*retType!=0 && *retType!=1) return false;
                            if(*retType==0) *retType=1;
                        }
                        else if(Fmap[funtionPos].params[i].dataType=="double") {
                            if(*retType!=0 && *retType!=2) return false;
                            if(*retType==0) *retType=2;
                        }
                        else if(Fmap[funtionPos].params[i].dataType=="void") {
                            if(*retType!=0 && *retType!=3) return false;
                            if(*retType==0) *retType=3;
                        }
                        else return false;
                        param=true;
                        //arga()
                        Fmap[funtionPos].instructions.push_back(0x0b);
                        pushIns(i+1,Fmap[funtionPos].instructions);
                        Fmap[funtionPos].insNum++;
                        break;
                    }
                }
            }
            //全局变量
            if(!local && !param){
                for(int i=0;i<Gmap.size();i++){
                    if(Gmap[i].dataType!="string"){
                        if(tempToken.value==Gmap[i].name){
                            if(Gmap[i].is_const) return false;
                            if(Gmap[i].dataType=="int") {
                                if(*retType!=0 && *retType!=1) return false;
                                if(*retType==0) *retType=1;
                            }
                            else if(Gmap[i].dataType=="double") {
                                if(*retType!=0 && *retType!=2) return false;
                                if(*retType==0) *retType=2;
                            }
                            else if(Gmap[i].dataType=="void") {
                                if(*retType!=0 && *retType!=3) return false;
                                if(*retType==0) *retType=3;
                            }
                            else return false;
                            global=true;
                            //globa()
                            Fmap[funtionPos].instructions.push_back(0x0c);
                            pushIns(i+1,Fmap[funtionPos].instructions);
                            Fmap[funtionPos].insNum++;
                            break;
                        }
                    }
                }
            }
            if(!local && !param && !global) return false;
        }
        unusedToken=false;
        nextToken();
    }
    else if(currentToken.type==TokenType::GETINT){
        if(*retType!=0 && *retType!=1) return false;
        //'('
        unusedToken=false;
        nextToken();
        if(!currentToken.success) return false;
        if(currentToken.type!=TokenType::L_PAREN) return false;
        //')'
        unusedToken=false;
        nextToken();
        if(!currentToken.success) return false;
        if(currentToken.type!=TokenType::R_PAREN) return false;
        //scan.i
        Fmap[funtionPos].instructions.push_back(0x50);
        Fmap[funtionPos].insNum++;
        if(*retType==0) *retType=1;
        unusedToken=false;
        nextToken();
    }
    else if(currentToken.type==TokenType::GETDOUBLE){
        if(*retType!=0 && *retType!=2) return false;
        //'('
        unusedToken=false;
        nextToken();
        if(!currentToken.success) return false;
        if(currentToken.type!=TokenType::L_PAREN) return false;
        //')'
        unusedToken=false;
        nextToken();
        if(!currentToken.success) return false;
        if(currentToken.type!=TokenType::R_PAREN) return false;
        //scan.f
        Fmap[funtionPos].instructions.push_back(0x52);
        Fmap[funtionPos].insNum++;
        if(*retType==0) *retType=2;
        unusedToken=false;
        nextToken();
    }
    else if(currentToken.type==TokenType::GETCHAR){
        if(*retType!=0 && *retType!=1) return false;
        //'('
        unusedToken=false;
        nextToken();
        if(!currentToken.success) return false;
        if(currentToken.type!=TokenType::L_PAREN) return false;
        //')'
        unusedToken=false;
        nextToken();
        if(!currentToken.success) return false;
        if(currentToken.type!=TokenType::R_PAREN) return false;
        //scan.f
        Fmap[funtionPos].instructions.push_back(0x51);
        Fmap[funtionPos].insNum++;
        if(*retType==0) *retType=1;
        unusedToken=false;
        nextToken();
    }
    else if(currentToken.type==TokenType::PUTINT){
        if(*retType!=0 && *retType!=3) return false;
        int retT=1;
        //'('
        unusedToken=false;
        nextToken();
        if(!currentToken.success) return false;
        if(currentToken.type!=TokenType::L_PAREN) return false;
        //int
        unusedToken=false;
        if(!analyseExpr(funtionPos,rangePos,&retT)) return false;
        //')'
        nextToken();
        if(!currentToken.success) return false;
        if(currentToken.type!=TokenType::R_PAREN) return false;
        //print.i
        Fmap[funtionPos].instructions.push_back(0x54);
        Fmap[funtionPos].insNum++;
        if(*retType==0) *retType=3;
        unusedToken=false;
        nextToken();
    }
    else if(currentToken.type==TokenType::PUTDOUBLE){
        if(*retType!=0 && *retType!=3) return false;
        int retT=2;
        //'('
        unusedToken=false;
        nextToken();
        if(!currentToken.success) return false;
        if(currentToken.type!=TokenType::L_PAREN) return false;
        //double
        unusedToken=false;
        if(!analyseExpr(funtionPos,rangePos,&retT)) return false;
        //')'
        nextToken();
        if(!currentToken.success) return false;
        if(currentToken.type!=TokenType::R_PAREN) return false;
        //print.f
        Fmap[funtionPos].instructions.push_back(0x56);
        Fmap[funtionPos].insNum++;
        if(*retType==0) *retType=3;
        unusedToken=false;
        nextToken();
    }
    else if(currentToken.type==TokenType::PUTCHAR){
        if(*retType!=0 && *retType!=3) return false;
        int retT=1;
        //'('
        unusedToken=false;
        nextToken();
        if(!currentToken.success) return false;
        if(currentToken.type!=TokenType::L_PAREN) return false;
        //int
        unusedToken=false;
        if(!analyseExpr(funtionPos,rangePos,&retT)) return false;
        //')'
        nextToken();
        if(!currentToken.success) return false;
        if(currentToken.type!=TokenType::R_PAREN) return false;
        //print.c
        Fmap[funtionPos].instructions.push_back(0x55);
        Fmap[funtionPos].insNum++;
        if(*retType==0) *retType=3;
        unusedToken=false;
        nextToken();
    }
    else if(currentToken.type==TokenType::PUTSTR){
        if(*retType!=0 && *retType!=3) return false;
        int retT=2;
        //'('
        unusedToken=false;
        nextToken();
        if(!currentToken.success) return false;
        if(currentToken.type!=TokenType::L_PAREN) return false;
        //STRING
        unusedToken=false;
        nextToken();
        if(currentToken.type!=TokenType::STRING_LITERAL) return false;
        Global temp=Global(currentToken.value,"string",true);
        int64_t tempNum = Gmap.size();
        Gmap.push_back(temp);
        Fmap[0].instructions.push_back(0x01);
        pushIns(tempNum,Fmap[0].instructions);
        Fmap[0].insNum++;
        //')'
        unusedToken=false;
        nextToken();
        if(!currentToken.success) return false;
        if(currentToken.type!=TokenType::R_PAREN) return false;
        //print.s
        Fmap[funtionPos].instructions.push_back(0x57);
        Fmap[funtionPos].insNum++;
        if(*retType==0) *retType=3;
        unusedToken=false;
        nextToken();
    }
    else if(currentToken.type==TokenType::PUTLN){
        if(*retType!=0 && *retType!=3) return false;
        //'('
        unusedToken=false;
        nextToken();
        if(!currentToken.success) return false;
        if(currentToken.type!=TokenType::L_PAREN) return false;
        //')'
        unusedToken=false;
        nextToken();
        if(!currentToken.success) return false;
        if(currentToken.type!=TokenType::R_PAREN) return false;
        //println
        Fmap[funtionPos].instructions.push_back(0x58);
        Fmap[funtionPos].insNum++;
        if(*retType==0) *retType=3;
        unusedToken=false;
        nextToken();
    }
    else if(currentToken.type==TokenType::MINUS){
        unusedToken=false;
        nextToken();
        if(!currentToken.success) return false;
        int retType=0;
        if(!analyseExpr(funtionPos,rangePos,&retType)) return false;
        if(retType==1){
            Fmap[funtionPos].instructions.push_back(0x34);
            Fmap[funtionPos].insNum++;
        }
        else if(retType==2){
            Fmap[funtionPos].instructions.push_back(0x35);
            Fmap[funtionPos].insNum++;
        }
        else return false;
    }
    else return false;
    return true;
}
// # 语句
/*stmt ->
      expr_stmt
    | decl_stmt
    | if_stmt
    | while_stmt
    | break_stmt
    | continue_stmt
    | return_stmt
    | block_stmt
    | empty_stmt  */
bool analyseStmt(int funtionPos,int rangePos){
    nextToken();
    if(!currentToken.success) return false;
    if(currentToken.type==TokenType::L_PAREN
        ||currentToken.type==TokenType::UINT_LITERAL 
        || currentToken.type==TokenType::DOUBLE_LITERAL 
        ||currentToken.type==TokenType::STRING_LITERAL
        ||currentToken.type==TokenType::CHAR_LITERAL
        ||currentToken.type==TokenType::IDENT
        ||currentToken.type==TokenType::GETINT
        ||currentToken.type==TokenType::GETDOUBLE
        ||currentToken.type==TokenType::GETCHAR
        ||currentToken.type==TokenType::PUTINT
        ||currentToken.type==TokenType::PUTDOUBLE
        ||currentToken.type==TokenType::PUTCHAR
        ||currentToken.type==TokenType::PUTSTR
        ||currentToken.type==TokenType::PUTLN
        ||currentToken.type==TokenType::MINUS){
        if(!analyseExprStmt(funtionPos,rangePos)) return false;
        unusedToken=false;
    }
    else if(currentToken.type==TokenType::LET_KW || currentToken.type==TokenType::CONST_KW){
        if(!analyseDeclStmt(funtionPos,rangePos)) return false;
        unusedToken=false;
    }
    else if(currentToken.type==TokenType::IF_KW){
        if(!analyseIfStmt(funtionPos,rangePos)) return false;
        unusedToken=false;
    }
    else if(currentToken.type==TokenType::WHILE_KW){
        if(!analyseWhileStmt(funtionPos,rangePos)) return false;
        unusedToken=false;
    }
    else if(currentToken.type==TokenType::CONTINUE_KW){
        if(!analyseContinueStmt(funtionPos,rangePos)) return false;
        unusedToken=false;
    }
    else if(currentToken.type==TokenType::BREAK_KW){
        if(!analyseBreakStmt(funtionPos,rangePos)) return false;
        unusedToken=false;
    }
    else if(currentToken.type==TokenType::RETURN_KW){
        if(!analyseReturnStmt(funtionPos,rangePos)) return false;
        unusedToken=false;
    }
    else if(currentToken.type==TokenType::L_BRACE){
        if(!analyseBlockStmt(funtionPos,rangePos)) return false;
        unusedToken=false;
    }
    else if(currentToken.type==TokenType::SEMICOLON){
        if(!analyseEmptyStmt(funtionPos,rangePos)) return false;
        unusedToken=false;
    }
    else return false;
    return true;
}
/*
expr_stmt -> expr ';' 
一些表达式会在栈里留下一个数字，这时需要pop掉；有些又不会
不能使用预读的方式解决（需要至少预读2次，影响expr的实现）
所以，传flag？
或者将不会留下数字的改为留下数字，然后统一pop掉？？？
答案：根据返回值来判断就好了void就是不会留下数字，int和double都会留下数字
*/
bool analyseExprStmt(int funtionPos,int rangePos){
    //已预读判断
    int retType=0;
    if(!analyseExpr(funtionPos,rangePos,&retType)) return false;
    if(retType!=3){
        Fmap[funtionPos].instructions.push_back(0x03);
        pushIns(1,Fmap[funtionPos].instructions);
    }
    unusedToken=false;
    nextToken();
    if(!currentToken.success) return false;
    if(currentToken.type!=TokenType::SEMICOLON) return false;
    unusedToken=false;
    return true;
}
/*
let_decl_stmt -> 'let' IDENT ':' ty ('=' expr)? ';'
const_decl_stmt -> 'const' IDENT ':' ty '=' expr ';'
decl_stmt -> let_decl_stmt | const_decl_stmt
*/
bool analyseDeclStmt(int funtionPos,int rangePos){
    nextToken();
    if(!currentToken.success) return false;
    bool is_const=false;
    if(currentToken.type==TokenType::CONST_KW) is_const=true;
    unusedToken=false;
    nextToken();
    if(!currentToken.success) return false;
    if(currentToken.type!=TokenType::IDENT) return false;
    Token tempToken=currentToken;
    unusedToken=false;
    nextToken();
    if(!currentToken.success) return false;
    if(currentToken.type!=TokenType::COLON) return false;
    unusedToken=false;
    string dataType;
    nextToken();
    if(!currentToken.success) return false;
    if(currentToken.type!=TokenType::IDENT) return false;
    if(!tyIDENT(dataType)) return false;
    unusedToken=false;
    int retType=0;
    if(dataType=="int") retType=1;
    else if(dataType=="double") retType=2;
    else return false;
    Global tempVar(tempToken.value,dataType,is_const);
    int varPos=0;
    //查重后放入符号表
    if(funtionPos==0){
        //全局变量查重
        for(int i=0;i<Gmap.size();i++){
            if(Gmap[i].dataType=="string") continue;
            else if(Gmap[i].name==tempVar.name) return false;
        }
        varPos=Gmap.size();
        Gmap.push_back(tempVar);
    }
    else{
        //所在域查重
        for(int i=0;i<Lmap[rangePos].vars.size();i++){
            if(Lmap[rangePos].vars[i].dataType=="string") continue;
            else if(Lmap[rangePos].vars[i].name==tempVar.name) return false;
        }
        if(Lmap[rangePos].upRange==-1){
            //和函数参数查重
            for(int i=0;i<Fmap[funtionPos].params.size();i++){
                if(Fmap[funtionPos].params[i].dataType=="string") continue;
                else if(Fmap[funtionPos].params[i].name==tempVar.name) return false;
            }
        }
        varPos=Fmap[funtionPos].localSlotNum++;
        Lmap[rangePos].vars.push_back(tempVar);
    }
    nextToken();
    if(!currentToken.success) return false;
    if(currentToken.type==TokenType::ASSIGN){
        if(funtionPos==0){
            //globa
           Fmap[funtionPos].instructions.push_back(0x0c);
           pushIns(varPos,Fmap[funtionPos].instructions);
        }
        else{
            //loca
            Fmap[funtionPos].instructions.push_back(0x0a);
            pushIns(varPos,Fmap[funtionPos].instructions);
        }
        unusedToken=false;
        nextToken();
        if(!currentToken.success) return false;
        if(!analyseExpr(funtionPos,rangePos,&retType)) return false;
        //store64
        Fmap[funtionPos].instructions.push_back(0x17);
    }
    else{
        if(is_const) return false;
    }
    nextToken();
    if(!currentToken.success) return false;
    if(currentToken.type!=TokenType::SEMICOLON) return false;
    unusedToken=false;
    return true;
}
//if_stmt -> 'if' expr block_stmt ('else' 'if' expr block_stmt)* ('else' block_stmt)?
//if_stmt -> 'if' expr block_stmt ('else' block_stmt|if_stmt)?
//if同时也需要知道block的指令数量
bool analyseIfStmt(int funtionPos,int rangePos){
    //已预读判断if
    int retType=0;
    unusedToken=false;
    nextToken();
    if(!analyseExpr(funtionPos,rangePos,&retType)) return false;
    unusedToken=false;
    //brtrue(1)
    Fmap[funtionPos].instructions.push_back(0x43);
    pushIns(1,Fmap[funtionPos].instructions);
    Fmap[funtionPos].insNum++;
    //br(0)0等待替换;
    Fmap[funtionPos].instructions.push_back(0x41);
    int waitPos=Fmap[funtionPos].instructions.size();
    pushIns(0,Fmap[funtionPos].instructions);
    int tempNum = ++Fmap[funtionPos].insNum;
    //block
    if(!analyseBlockStmt(funtionPos,rangePos)) return false;
    unusedToken=false;
    //修改等待替换的0
    unsigned char str[5];
    memset(str,0,sizeof(str));
    intToFourBits(Fmap[funtionPos].insNum-tempNum+1,str);
    for(int i=0;i<4;i++){
        Fmap[funtionPos].instructions[waitPos+i]=str[i+1];
    }
    //预读else
    nextToken();
    if(!currentToken.success) return false;
    if(currentToken.type==TokenType::ELSE_KW){
        unusedToken=false;
        nextToken();
        if(!currentToken.success) return false;
        if(currentToken.type==TokenType::L_BRACE){
            //br(0)0等待替换;
            Fmap[funtionPos].instructions.push_back(0x41);
            int waitPos=Fmap[funtionPos].instructions.size();
            pushIns(0,Fmap[funtionPos].instructions);
            int tempNum = ++Fmap[funtionPos].insNum;
            //block
            if(!analyseBlockStmt(funtionPos,rangePos)) return false;
            unusedToken=false;
            //修改等待替换的0
            unsigned char str[5];
            memset(str,0,sizeof(str));
            intToFourBits(Fmap[funtionPos].insNum-tempNum,str);
            for(int i=0;i<4;i++){
                Fmap[funtionPos].instructions[waitPos+i]=str[i+1];
            }
        }
        else if(currentToken.type==TokenType::IF_KW){
            if(!analyseIfStmt(funtionPos,retType)) return false;
            unusedToken=false;
        }
        else return false;
    }
    //结尾添加一个br(0)
    Fmap[funtionPos].instructions.push_back(0x41);
    pushIns(0,Fmap[funtionPos].instructions);
    return true;
}
//while_stmt -> 'while' expr block_stmt
bool analyseWhileStmt(int funtionPos,int rangePos){
    //已预读while
    //br(0)
    Fmap[funtionPos].instructions.push_back(0x41);
    int whilePos=Fmap[funtionPos].instructions.size();
    pushIns(0,Fmap[funtionPos].instructions);
    int whileNum=++Fmap[funtionPos].insNum;
    //expr
    int retType=0;
    unusedToken=false;
    nextToken();
    if(!analyseExpr(funtionPos,rangePos,&retType)) return false;
    unusedToken=false;
    //brtrue(1)
    Fmap[funtionPos].instructions.push_back(0x43);
    pushIns(1,Fmap[funtionPos].instructions);
    Fmap[funtionPos].insNum++;
    //br(0)0等待替换，在符号表中添加continueNum;
    Fmap[funtionPos].instructions.push_back(0x41);
    int waitPos=Fmap[funtionPos].instructions.size();
    pushIns(0,Fmap[funtionPos].instructions);
    int tempNum = ++Fmap[funtionPos].insNum;
    Lmap[rangePos].continueNum=tempNum;
    //block
    if(!analyseBlockStmt(funtionPos,rangePos)) return false;
    unusedToken=false;
    //添加循环br(-?)
    Fmap[funtionPos].instructions.push_back(0x41);
    pushIns(whileNum-Fmap[funtionPos].insNum,Fmap[funtionPos].instructions);
    Fmap[funtionPos].insNum++;
    //修改开头等待替换的0
    unsigned char str[5];
    memset(str,0,sizeof(str));
    intToFourBits(Fmap[funtionPos].insNum-tempNum+1,str);//TODO:check
    for(int i=0;i<4;i++){
        Fmap[funtionPos].instructions[waitPos+i]=str[i+1];
    }
    //修改break等待替换的0
    for(int i=0;i<Lmap[rangePos].breaks.size();i++){
        unsigned char str[5];
        memset(str,0,sizeof(str));
        intToFourBits(Fmap[funtionPos].insNum-Lmap[rangePos].breaks[i].second,str);
        for(int i=0;i<4;i++){
            Fmap[funtionPos].instructions[Lmap[rangePos].breaks[i].first+i]=str[i+1];
        }
    }
    return true;
}
/*
执行break时，向函数的指令集中插入br（0）并且记录0的第一个字节的位置insPos和当前指令数breakNum
while执行完block后，将该函数的指令数insNum-breakNum放入insPos中
*/
//break_stmt -> 'break' ';'
bool analyseBreakStmt(int funtionPos,int rangePos){
    //已预读break
    unusedToken=false;
    nextToken();
    if(!currentToken.success) return false;
    if(currentToken.type!=TokenType::SEMICOLON) return false;
    //check inwhile
    int tempRangePos=rangePos;
    while(!Lmap[tempRangePos].is_while){
        if(Lmap[tempRangePos].upRange!=-1)
            tempRangePos=Lmap[tempRangePos].upRange;
        else return false;
    }
    //br(0)0等待替换
    Fmap[funtionPos].instructions.push_back(0x41);
    int waitPos=Fmap[funtionPos].instructions.size();
    pushIns(0,Fmap[funtionPos].instructions);
    int tempNum = ++Fmap[funtionPos].insNum;
    Lmap[tempRangePos].breaks.push_back(pair<int,int>(waitPos,tempNum));
    return true;
}
//continue_stmt -> 'continue' ';'
bool analyseContinueStmt(int funtionPos,int rangePos){
    //已预读continue
    unusedToken=false;
    nextToken();
    if(!currentToken.success) return false;
    if(currentToken.type!=TokenType::SEMICOLON) return false;
    //check inwhile
    int tempRangePos=rangePos;
    while(!Lmap[tempRangePos].is_while){
        if(Lmap[tempRangePos].upRange!=-1)
            tempRangePos=Lmap[tempRangePos].upRange;
        else return false;
    }
    //返回到开头
    Fmap[funtionPos].instructions.push_back(0x41);
    Fmap[funtionPos].insNum++;
    pushIns(Lmap[tempRangePos].continueNum-Fmap[funtionPos].insNum,Fmap[funtionPos].instructions);
    return true;
}
//return_stmt -> 'return' expr? ';'
bool analyseReturnStmt(int funtionPos,int rangePos){
    //已预读return
    unusedToken=false;
    nextToken();
    if(Fmap[funtionPos].retType=="void"){
        if(currentToken.type!=TokenType::SEMICOLON) return false;
    }
    else{
        //arga(0)
        Fmap[funtionPos].instructions.push_back(0x0b);
        pushIns(0,Fmap[funtionPos].instructions);
        Fmap[funtionPos].insNum++;
        //返回值
        int retType;
        if(Fmap[funtionPos].retType=="int") retType=1;
        else if(Fmap[funtionPos].retType=="double") retType=2;
        else return false;
        if(!analyseExpr(funtionPos,rangePos,&retType)) return false;
        //store64
        Fmap[funtionPos].instructions.push_back(0x17);
        Fmap[funtionPos].insNum++;
    }
    //ret
    Fmap[funtionPos].instructions.push_back(0x49);
    Fmap[funtionPos].insNum++;
    return true;
}
//block_stmt -> '{' stmt* '}'
bool analyseBlockStmt(int funtionPos,int upRange){
    int rangePos = Lmap.size();
    Lmap.push_back(Local(funtionPos,upRange));
    nextToken();
    if(!currentToken.success) return false;
    if(currentToken.type!=TokenType::L_BRACE) return false;
    unusedToken=false;
    while(true){
        nextToken();
        if(currentToken.type!=TokenType::R_BRACE){
            if(!analyseStmt(funtionPos,rangePos)) return false;
            unusedToken=false;
            continue;
        }
        else{
            unusedToken=false;
            break;
        }
    }
    return true;
}
//empty_stmt -> ';'
bool analyseEmptyStmt(int funtionPos,int rangePos){
    nextToken();
    if(!currentToken.success) return false;
    if(currentToken.type!=TokenType::SEMICOLON) return false;
    unusedToken=false;
    return true;
}
// # 函数
/*
域要标注所属函数，函数的域中的变量要在指令集中加入局部变量数，域只是在编译时检查规范
调用函数:
    1.调用者先向栈中压入参数,然后call id;
    2.被调用函数执行指令直到ret;
    由于调用参数和调用局部变量的指令不同，所以不能单纯的把参数直接加入函数block的域中
    读到IDENT时，首先按照域的顺序查找，再查找参数列表
*/
//function_param -> 'const'? IDENT ':' ty
bool analyseFuntionParam(Global *tempParam){
    bool is_const=false;
    //预读
    nextToken();
    if(!currentToken.success) return false;
    if(currentToken.type==TokenType::CONST_KW){
        is_const=true;
        unusedToken=false;
        nextToken();
        if(!currentToken.success) return false;
    }
    tempParam->is_const=is_const;
    if(currentToken.type!=TokenType::IDENT) return false;
    tempParam->name=currentToken.value;
    unusedToken=false;
    //读入':'
    nextToken();
    if(!currentToken.success) return false;
    if(currentToken.type!=TokenType::COLON) return false;
    unusedToken=false;
    //读入ty
    string tempTy;
    if(!tyIDENT(tempTy)) return false;
    tempParam->dataType=tempTy;
    return true;
}
//function_param_list -> function_param (',' function_param)*
bool analyseFuntionParamList(int funtionPos){
    vector<Global> tempParams;
    while(true){
        Global tempParam;
        if(!analyseFuntionParam(&tempParam)) return false;
        tempParams.push_back(tempParam);
        unusedToken=false;
        nextToken();
        if(!currentToken.success) return false;
        if(currentToken.type!=TokenType::COMMA) break;
    }
    Fmap[funtionPos].params=tempParams;
    return true;
}
//function -> 'fn' IDENT '(' function_param_list? ')' '->' ty block_stmt 
bool analyseFuntion(){
    int funtionPos=Fmap.size();
    //读入IDENT
    nextToken();
    if(!currentToken.success) return false;
    if(currentToken.type!=TokenType::IDENT) return false;
    if(currentToken.value=="main"){
        //确定位置
        funtionPos=1;
    }
    else{
        //重名检查
        for(int i=0;i<Fmap.size();i++){
            if(currentToken.value==Fmap[i].name) return false;
        }
        Fmap.push_back(Funtion(currentToken.value));
    }
    unusedToken=false;
    //读入'('
    nextToken();
    if(!currentToken.success) return false;
    if(currentToken.type!=TokenType::L_PAREN) return false;
    unusedToken=false;
    //设置参数列表
    nextToken();
    if(!currentToken.success) return false;
    if(currentToken.type==TokenType::CONST_KW || currentToken.type==TokenType::IDENT)
        //设置参数列表
        if(!analyseFuntionParamList(funtionPos)) return false;
    //读入')'
    nextToken();
    if(!currentToken.success) return false;
    if(currentToken.type!=TokenType::R_PAREN) return false;
    unusedToken=false;
    //读入'->'
    nextToken();
    if(!currentToken.success) return false;
    if(currentToken.type!=TokenType::ARROW) return false;
    unusedToken=false;
    //读入ty
    if(!tyIDENT(Fmap[funtionPos].retType)) return false;
    //函数体分析
    if(!analyseBlockStmt(funtionPos,-1)) return false;
    return true;
}
// 程序
/*item -> function | decl_stmt
program -> item* */
bool analyseProgram(){
    //默认Fmap[0]是_start
    Fmap.push_back(Funtion("_start"));
    Fmap[0].retType="void";
    //默认Fmap[1]是main
    Fmap.push_back(Funtion("main"));
    while(true){
        nextToken();
        if(!currentToken.success){
            if(isEOF) return true;
            else return false;
        }
        if(currentToken.type==TokenType::FN_KW){
            unusedToken=false;
            if(!analyseFuntion()) return false;
        }
        else if(currentToken.type==TokenType::CONST_KW || currentToken.type==TokenType::LET_KW){
            if(!analyseDeclStmt(0,0)) return false;
        }
    }
    return true;
}
vector<unsigned char> instructions;
bool init_start(){
    //stackalloc(1)
    Fmap[0].instructions.push_back(0x1a);
    Fmap[0].instructions.push_back(0x00);
    Fmap[0].instructions.push_back(0x00);
    Fmap[0].instructions.push_back(0x00);
    Fmap[0].instructions.push_back(0x01);
    Fmap[0].insNum++;
    //call(1)
    Fmap[0].instructions.push_back(0x48);
    Fmap[0].instructions.push_back(0x00);
    Fmap[0].instructions.push_back(0x00);
    Fmap[0].instructions.push_back(0x00);
    Fmap[0].instructions.push_back(0x01);
    Fmap[0].insNum++;
    //popn(1)
    Fmap[0].instructions.push_back(0x03);
    Fmap[0].instructions.push_back(0x00);
    Fmap[0].instructions.push_back(0x00);
    Fmap[0].instructions.push_back(0x00);
    Fmap[0].instructions.push_back(0x01);
    Fmap[0].insNum++;
    return true;
}
//前提：将所有符号表搞定
bool analyse(){//TODO: 写入文件
    //magic
    instructions.push_back(0x72);
    instructions.push_back(0x30);
    instructions.push_back(0x3b);
    instructions.push_back(0x3e);
    //version
    pushIns(1,instructions);
    if(!analyseProgram()) return false;
    //_start的指令集
    if(!init_start()) return false;
    //Array<GlobalDef>：全局变量常量（全设为0），字符串字面量，函数名
    //Array<GlobalDef>.count
    int globalNum=Gmap.size()+Fmap.size();
    pushIns(globalNum,instructions);
    //Array<GlobalDef>.item 全局变量部分（包括字符串字面量和标准库函数）
    for(int i=0;i<Gmap.size();i++){
        if(Gmap[i].dataType=="string"){
            //Array<GlobalDef>.item[i].is_const = 1
            pushIns(1,instructions);
            int arrayNum=Gmap[i].name.size();
            //Array<GlobalDef>.item[i].value.count
            pushIns(arrayNum,instructions);
            for(int j=0;j<arrayNum;j++){
                //Array<GlobalDef>.item[i].value.item[j]
                instructions.push_back(Gmap[i].name[j]);
            }
        }
        else if(Gmap[i].dataType=="int" || Gmap[i].dataType=="double"){
            //Array<GlobalDef>.item[i].is_const
            if(Gmap[i].is_const) pushIns(1,instructions);
            else pushIns(0,instructions);
            //Array<GlobalDef>.item[i].value.count=8
            pushIns(8,instructions);
            for(int j=0;j<8;j++){
                //Array<GlobalDef>.item[i].value.item[j]
                instructions.push_back(0x00);
            }
        }
        else return false;
    }
    //Array<GlobalDef>.item 函数部分
    for(int i=0;i<Fmap.size();i++){
        Fmap[i].pos=Gmap.size()+i;
        //Array<GlobalDef>.item[Gmap.size()+i].is_const = 1
        pushIns(1,instructions);
        int arrayNum=Fmap[i].name.size();
        //Array<GlobalDef>.item[Gmap.size()+i].value.count
        pushIns(arrayNum,instructions);
        for(int j=0;j<arrayNum;j++){
            //Array<GlobalDef>.item[Gmap.size()+i].value.item[j]
            instructions.push_back(Fmap[i].name[j]);
        }
    }
    //Array<FunctionDef>.count
    int funtionNum=Fmap.size();
    pushIns(funtionNum,instructions);
    for(int i=0;i<funtionNum;i++){
        //Array<FunctionDef>.item[i].name
        int name = Fmap[i].pos;
        pushIns(name,instructions);
        //Array<FunctionDef>.item[i].return_slots
        if(Fmap[i].retType=="int" || Fmap[i].retType=="double"){
            int return_slots = 1;
            pushIns(return_slots,instructions);
        }
        else if(Fmap[i].retType=="void"){
            int return_slots = 0;
            pushIns(return_slots,instructions);
        }
        else return false;
        //Array<FunctionDef>.item[i].param_slots
        pushIns(Fmap[i].paramSlotNum,instructions);
        //Array<FunctionDef>.item[i].loc_slots
        pushIns(Fmap[i].localSlotNum,instructions);
        //Array<FunctionDef>.item[i].Array<Instruction>.count
        pushIns(Fmap[i].insNum,instructions);
        //Array<FunctionDef>.item[i].Array<Instruction>.item
        for(int j=0;j<Fmap[i].instructions.size();j++){
            instructions.push_back(Fmap[i].instructions[j]);
        }
    }
    string str;
    for(int i=0;i<instructions.size();i++){
        cout << instructions[i] << endl;
        str+=instructions[i];
    }
    fwrite(str.c_str(),str.size(),1,outFile);
    return true;
}

int main(int argc,char** argv){
    file = fopen(argv[1],"rt");
    //file = fopen("test.txt","rt");
    outFile = fopen(argv[3],"wt");
    //outFile = fopen("out.txt","wt");
    if(!analyse()) return -1;
    fclose(file);
    fclose(outFile);
    return 0;
}