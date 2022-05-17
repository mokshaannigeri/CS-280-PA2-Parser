#include "parse.h"
#include <map>

map<string, bool> defVar;
map<string, Token> SymTable;
LexItem currLex;

namespace Parser {
    bool pushed_back = false;
    LexItem	pushed_token;

    static LexItem GetNextToken(istream& in, int& line) {
        if (pushed_back) {
            pushed_back = false;
            return pushed_token;
        }
        return getNextToken(in, line);
    }

    static void PushBackToken(LexItem& t) {
        if (pushed_back) {
            abort();
        }
        pushed_back = true;
        pushed_token = t;
    }
}

static int error_count = 0;

int ErrCount()
{
    return error_count;
}

void ParseError(int line, string msg)
{
    ++error_count;
    cout << line << ": " << msg << endl;
}

bool Prog(istream& in, int& line)
{
    bool check = false;
    currLex = Parser::GetNextToken(in, line);
    if (currLex.GetToken() != PROGRAM)
    {
        ParseError(line, "Missing PROG");
        return false;
    }
    currLex = Parser::GetNextToken(in, line);
    if (currLex.GetToken() != IDENT)
    {
        ParseError(line, "Missing Program");
        return false;
    }
    currLex = Parser::GetNextToken(in, line);
    if (currLex.GetToken() != SEMICOL)
    {
        ParseError(line, "Missing SEMICOL in PROG");
        return false;
    }
    check = DeclBlock(in, line);
    if (!check)
    {
        ParseError(line, "Non-recognizable Declaration Block");
        return false;
    }
    check = ProgBody(in, line);
    if (!check)
    {
        ParseError(line, "Incorrect Program Body.");
        return false;
    }

    return true;
}

bool DeclBlock(istream& in, int& line)
{
    bool check= false;
    currLex = Parser::GetNextToken(in, line);
    if (currLex.GetToken() == VAR)
    {
        {
        currLex = Parser::GetNextToken(in, line);

        while (currLex.GetToken() != BEGIN)
        {
            Parser::PushBackToken(currLex);
            check = DeclStmt(in, line);
            if (check==true)
            {
                currLex = Parser::GetNextToken(in, line);
                if (currLex.GetToken() != SEMICOL)
                {
                    ParseError(line, "Missing SEMICOL in DeclBlock.");
                    return false;
                }
                
            }
            else
            {
               ParseError(line, "Incorrect Declaration Section.");
                return false;
                break; 
            }
            currLex = Parser::GetNextToken(in, line);
        }
        Parser::PushBackToken(currLex);
      }
    }
    else
    { 
        ParseError(line, "Mistake in Declaration Type in function.");
        return false;
    }
    
   return true;
}
bool DeclStmt(istream& in, int& line)
{
    currLex = Parser::GetNextToken(in, line);
    if (currLex.GetToken() == IDENT)
    {
        
    }
    else
    {
        ParseError(line, "Error with IDENT Token");
        return false;
    }
    if (defVar.count(currLex.GetLexeme()))
    {
        ParseError(line, "Variable Redefinition");
        return false;
    }
    else
    {
        defVar.insert({currLex.GetLexeme(), true});
    }
    currLex = Parser::GetNextToken(in, line);
    while (currLex.GetToken() != COLON)
    {
        if (currLex.GetToken() == COMMA)
        {
            currLex = Parser::GetNextToken(in, line);

            if (currLex.GetToken() != IDENT)
            {
                ParseError(line, "Error with COMMA Token");
                break;
            }
            if (!defVar.count(currLex.GetLexeme()))
            {
                defVar.insert({currLex.GetLexeme(), true});
            }
            else
            {
                ParseError(line, "Variable Redefinition");
                return false;
            }
            
        }
        else
        {
            ParseError(line, "Error with COMMA Token");
            break; 
        }
        currLex = Parser::GetNextToken(in, line);
    }
    currLex = Parser::GetNextToken(in, line);
    if (currLex.GetLexeme() != "INTEGER" && currLex.GetLexeme() != "REAL" && currLex.GetLexeme() != "STRING")
    {
        ParseError(line, "Mistake in the Declaration statement.");
        return false;
    }
    return true;
}

bool ProgBody(istream& in, int& line)
{
    bool check=true;
    currLex = Parser::GetNextToken(in, line);
    if (currLex.GetToken() == BEGIN)
    {
        currLex = Parser::GetNextToken(in, line);
        while (currLex.GetToken() != END)
        {
            Parser::PushBackToken(currLex);
            check = Stmt(in, line);
            if (!check)
            {
                ParseError(line, "Mistake in STMT Block here.");
                return false;
            }
            else
            {
                currLex = Parser::GetNextToken(in, line);


                if (currLex.GetToken() != SEMICOL)
                {
                    ParseError(line - 1, "Missing semicolon in Statement.");
                    return false;
                }
            }
            currLex = Parser::GetNextToken(in, line);
        }
        
    }
    else
    {
        ParseError(line, "No BEGIN Token here.");
        return false;
    }
    if (currLex.GetToken() == END)
    {
        return true;
    }
  return false;
}

//Stmt is either a WriteLnStmt, ForepeatStmt, IfStmt, or AssigStmt
//Stmt = AssigStmt | IfStmt | WriteStmt | ForStmt
bool Stmt(istream& in, int& line) {
    bool check;
    //cout << "in ContrlStmt" << endl;
    LexItem t = Parser::GetNextToken(in, line);

    switch (t.GetToken()) {

    case WRITELN:
        check = WriteLnStmt(in, line);
        //cout << "After WriteStmet status: " << (status? true:false) <<endl;
        break;

    case IF:
        Parser::PushBackToken(t);
        check = IfStmt(in, line);
        break;

    case IDENT:
        Parser::PushBackToken(t);
        check = AssignStmt(in, line);

        break;

    case FOR:
        Parser::PushBackToken(t);
        check = ForStmt(in, line);

        break;


    default:
        Parser::PushBackToken(t);
        return false;
    }

    return check;
}//End of Stmt

//WriteStmt:= wi, ExpreList
bool WriteLnStmt(istream& in, int& line) {
    LexItem currLex2;
    //cout << "in WriteStmt" << endl;

    currLex2 = Parser::GetNextToken(in, line);
    if (currLex2  != LPAREN) {

        ParseError(line, "Missing Left Parenthesis");
        return false;
    }

    bool ex = ExprList(in, line);

    if (!ex) {
        ParseError(line, "Missing expression after WriteLn");
        return false;
    }

    currLex2  = Parser::GetNextToken(in, line);
    if (currLex2 != RPAREN) {

        ParseError(line, "Missing Right Parenthesis");
        return false;
    }
    //Evaluate: print out the list of expressions values

    return ex;
}
bool IfStmt(istream& in, int& line)
{
    bool check=true;
    currLex = Parser::GetNextToken(in, line);
    if (currLex.GetToken() == IF)
    {
        currLex = Parser::GetNextToken(in, line);
        if (currLex.GetToken() == LPAREN)
        {
              check = LogicExpr(in, line);
            if (!check)
            {
                ParseError(line, "If-Stmt Syntax Error");
                return false;
            }
            else
            {
                currLex = Parser::GetNextToken(in, line);

                if (currLex.GetToken() != RPAREN)
                {
                    ParseError(line, "If-Stmt Syntax Error");
                    return false;
                }
                else
                {
                    currLex = Parser::GetNextToken(in, line);

                    if (currLex.GetToken() != THEN)
                    {
                        ParseError(line, "If-Stmt Syntax Error");
                        return false;
                    }
                    else
                    {
                        check = Stmt(in, line);

                        if (!check)
                        {
                            ParseError(line, "Error in the Stmt Block here");
                            return false;
                        }
                        else
                        {
                            currLex = Parser::GetNextToken(in, line);


                            if (currLex.GetToken() != ELSE)
                            {
                                Parser::PushBackToken(currLex);
                            }
                            else
                            {
                                check = Stmt(in, line);

                                if (check == false)
                                {
                                    ParseError(line, "Error in Statement Block here");
                                    return false;
                                }
                               
                            }
                        }
                    }
                }
            }
        
        }
        else
        {       
            ParseError(line, "If-Stmt Syntax Error");
            return false; 
        } 
    }
    else
    {
       ParseError(line, "Mistake in IF token");
        return false; 
    }
     return true;
}

bool ForStmt(istream& in, int& line)
{
    bool check=true;
    currLex = Parser::GetNextToken(in, line);
    if (currLex.GetToken() == FOR)
    {
        
        check = Var(in, line);

        if (check == false)
        {
            ParseError(line, "Error in Var");
            return false;
        }
        else
        {
            currLex = Parser::GetNextToken(in, line);

            if (currLex.GetToken() == ASSOP)
            {
                
                 currLex = Parser::GetNextToken(in, line);

                if (currLex.GetToken() != ICONST)
                {
                    ParseError(line, "Missing Initialization Value in For Statement.");
                    return false;
                }
                else
                {
                    currLex = Parser::GetNextToken(in, line);

                    if (currLex.GetToken() != TO && currLex.GetToken() != DOWNTO)
                    {
                        ParseError(line, "For Statement Syntax Error");
                        return false;
                    }
                    else
                    {
                        currLex = Parser::GetNextToken(in, line);


                        if (currLex.GetToken() != ICONST)
                        {
                            ParseError(line, "Missing Termination Value in For Statement.");
                            return false;
                        }
                        else
                        {
                            currLex = Parser::GetNextToken(in, line);

                            if (currLex.GetToken() != DO)
                            {
                                ParseError(line, "DO Token");
                                return false;
                            }
                            else
                            {
                                check = Stmt(in, line);

                                if (check == false)
                                {
                                    ParseError(line, "Incorrect Stmt Block here");
                                    return false;
                                }
                                
                            }
                        }
                    }
                }               
            }
            else
            {
                ParseError(line, "Error with ASSOP");
                return false;
            }
        }
        
    }
    else
    {
        ParseError(line, "Error in the FOR Token");
        return false; 
    }
    return true;
}

bool AssignStmt(istream& in, int& line)
{
    bool check=true;

    check = Var(in, line);
    if (check)
    {
        currLex = Parser::GetNextToken(in, line);

        if (currLex.GetToken() == ASSOP)
        {
            check = Expr(in, line);

            if (!check)
            {
                ParseError(line, "Error Expr Stmt here");
                return false;
            }
            
        }
        else
        {
            ParseError(line, "Missing Assignment Operator");
            return false;
        }   
    }
    else
    {
        ParseError(line, "Error in Var Block here");
        return false;
    }
    return true;
}

bool Var(istream& in, int& line)
{
    currLex = Parser::GetNextToken(in, line);
    if (currLex.GetToken() == IDENT)
    {
       if (defVar.count(currLex.GetLexeme()))
        {
            return true;
        }  
        ParseError(line, "Undefined Variable");
        return false;
    }
    else 
    {
        ParseError(line, "Error with IDENT token");
        return false;
    }
    
   return true;
}

bool ExprList(istream& in, int& line) {
    bool check = false;
    //cout << "in ExprList and before calling Expr" << endl;
    check = Expr(in, line);
    if (!check) {
        ParseError(line, "Missing Expression");
        return false;
    }

    LexItem currLex3 = Parser::GetNextToken(in, line);

    if (currLex3 == COMMA) {
        //cout << "before calling ExprList" << endl;
        check = ExprList(in, line);
        //cout << "after calling ExprList" << endl;
    }
    else if (currLex3.GetToken() == ERR) {
        ParseError(line, "Unrecognized Input Pattern");
        cout << "(" << currLex3.GetLexeme() << ")" << endl;
        return false;
    }
    else {
        Parser::PushBackToken(currLex3);
        return true;
    }
    return check;
}


bool LogicExpr(istream& in, int& line)
{
    bool check=true;
    check = Expr(in, line);
    if (check)
    {
        currLex = Parser::GetNextToken(in, line);
        if ((currLex.GetToken() != LTHAN) && (currLex.GetToken() != EQUAL) &&  (currLex.GetToken() != GTHAN))
        {
            ParseError(line, "Missing EQUAL or LTHAN or GTHAN Token");
            return false;
        }
        else
        {
            check = Expr(in, line);

            if (!check)
            {
                ParseError(line, "Incorrect Mistake Error Expr Block");
                return false;
            }
        }
    }
    else
    {
        ParseError(line, "Incorrect Mistake Error Expr Block");
        return false;
    }
    return true;
}

bool Expr(istream& in, int& line)
{
    bool t1 = Term(in, line);
    LexItem tok;
    if (!t1) {
        return false;
    }
    tok = Parser::GetNextToken(in, line);
    if (tok.GetToken() == ERR) {
        ParseError(line, "Unrecognized Input Pattern");
        return false;
    }
    while (tok == PLUS || tok == MINUS) {
        t1 = Term(in, line);
        if (!t1) {
            ParseError(line, "Missing operand after operator");
            return false;
        }
        tok = Parser::GetNextToken(in, line);
        if (tok.GetToken() == ERR) {
            ParseError(line, "Unrecognized Input Pattern");
            cout << "(" << tok.GetLexeme() << ")" << endl;
            return false;
        }
    }
    Parser::PushBackToken(tok);
    return true;
}

bool Term(istream& in, int& line)
{
    bool t1 = SFactor(in, line);
    LexItem currLex;
    if (!t1) {
        return false;
    }
    currLex = Parser::GetNextToken(in, line);
    if (currLex.GetToken() == ERR) {
        ParseError(line, "Unrecognized Input Pattern");
        return false;
    }
    while (currLex.GetToken() == MULT || currLex.GetToken() == DIV) {
        t1 = SFactor(in, line);
        if (!t1) {
            ParseError(line, "Missing operand after operator");
            return false;
        }
        currLex = Parser::GetNextToken(in, line);
        if (currLex.GetToken() == ERR) {
            ParseError(line, "Unrecognized Input Pattern");
            cout << "(" << currLex.GetLexeme() << ")" << endl;
            return false;
        }
    }
    Parser::PushBackToken(currLex);
    return true;
}

bool SFactor(istream& in, int& line)
{
    bool check=false;
    currLex = Parser::GetNextToken(in, line);
    if ((defVar.count(currLex.GetLexeme())) || currLex.GetToken() != IDENT)
    {
        
    } 
    else 
    {
        ParseError(line, "Using Undefined Variable");
        return false;
    }
    if ( currLex.GetToken() != MINUS && currLex.GetToken() != PLUS)
    {
          Parser::PushBackToken(currLex);
    }
    check = Factor(in, line, 1);
    if (!check)
    {
        ParseError(line, "Error in Factor Stmt");
        return false;
    }
    else
    {
        return true;
    }
}

bool Factor(istream& in, int& line, int sign)
{
    bool check=true;
    currLex = Parser::GetNextToken(in, line);
    if ((currLex.GetToken() != ICONST) && (currLex.GetToken() != IDENT)  && (currLex.GetToken() != SCONST) && (currLex.GetToken() != RCONST))
    {
        if (currLex.GetToken() == LPAREN)
        {
            check = Expr(in, line);

            if (!check)
            {
                ParseError(line, "Error Expr Block Stmt");
                return false;
            }
            else
            {
                currLex = Parser::GetNextToken(in, line);
                if (currLex.GetToken() != RPAREN)
                {
                    ParseError(line, "Missing ) after expression");
                    return false;
                }
            }
        }
        else
        {
            ParseError(line, "Missing expression after relational operator");
            return false; 
        }
    }
    else
    {
        return true;
    }
   return true;
}