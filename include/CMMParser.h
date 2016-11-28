#ifndef CMMPARSER_H
#define CMMPARSER_H

#include "CMMLexer.h"
#include <memory>
#include <list>
#include <map>
#include <iostream>

///code.h
namespace cvm {
enum BasicType { BoolType, IntType, DoubleType, StringType, VoidType };
}
///

namespace cmm {
class DerivedType {
protected:
  enum DerivedKind {
    FunctionDerive
  };
private:
  DerivedKind Kind;
public:
  DerivedType(DerivedKind Kind) : Kind(Kind) {}
  bool isFunction() { return Kind == FunctionDerive; }
};

class TypeSpecifier {
  cvm::BasicType BasicType;
  DerivedType *Derived;
public:
  TypeSpecifier(cvm::BasicType BasicType)
    : BasicType(BasicType), Derived(nullptr) {}

  std::string toString() const {
    std::string Res;
    switch (BasicType) {
    default:                Res = "UnknownT"; break;
    case cvm::BoolType:     Res = "BoolT"; break;
    case cvm::IntType:      Res = "IntT"; break;
    case cvm::DoubleType:   Res = "DoubleT"; break;
    case cvm::StringType:   Res = "StringT"; break;
    case cvm::VoidType:     Res = "VoidT";  break;
    }
    return Res;
  }
};

class Parameter {
  std::string Name;
  std::unique_ptr<TypeSpecifier> Type;
  SourceMgr::LocTy Loc;
public:
  Parameter(const std::string &Name, cvm::BasicType Type, SourceMgr::LocTy Loc)
    : Name(Name), Type(new TypeSpecifier(Type)), Loc(Loc) {}

  std::string toString() const {
    return Name + ":" + Type->toString();
  }
};

class FunctionType : public DerivedType {
  std::unique_ptr<TypeSpecifier> Type;
  std::list<Parameter> ParameterList;
};

class AST {
public:
  virtual ~AST() {};
  virtual void dump(const std::string &prefix = "") const {};
};

class ExpressionAST : public AST {
protected:
  enum ExpressionKind {
    IntExpression,
    DoubleExpression,
    BoolExpression,
    StringExpression,
    IdentifierExpression,
    FunctionCallExpression,
    BinaryOperatorExpression,
    UnaryOperatorExpression
  };
private:
  ExpressionKind Kind;
public:
  ExpressionAST(ExpressionKind Kind) : Kind(Kind) {}
  bool isInt() { return Kind == IntExpression; }
  bool isDouble() { return Kind == DoubleExpression; }
  bool isBool() { return Kind == BoolExpression; }
  bool isString() { return Kind == StringExpression; }
  bool isNumeric() { return isInt() || isDouble(); }
};
class StatementAST : public AST {
protected:
  enum StatementKind {
    ExprStatement,
    BlockStatement,
    IfStatement,
    WhileStatement,
    ForStatement,
    ReturnStatement,
    ContinueStatement,
    BreakStatement
  };
private:
  StatementKind Kind;
public:
  StatementAST(StatementKind Kind) : Kind(Kind) {}
  bool isBlock() { return Kind == BlockStatement; }
  bool isIfStatement() { return Kind == IfStatement; }
  bool isWhileStatement() { return Kind == WhileStatement; }
};

class IntAST : public ExpressionAST {
  int Value;
public:
  IntAST(int Value) : ExpressionAST(IntExpression), Value(Value) {}
  void dump(const std::string &prefix = "") const override {
    std::cout << "(int)" << Value << std::endl;
  }
};

class DoubleAST : public ExpressionAST {
  double Value;
public:
  DoubleAST(double Value) : ExpressionAST(DoubleExpression), Value(Value) {}
  void dump(const std::string &prefix = "") const override {
    std::cout << "(double)" << Value << std::endl;
  }
};

class BoolAST : public ExpressionAST {
  bool Value;
public:
  BoolAST(bool Value) : ExpressionAST(BoolExpression), Value(Value) {}
  void dump(const std::string &prefix = "") const override {
    std::cout << "(bool)" << (Value ? "true" : "false") << std::endl;
  }
};

class StringAST : public ExpressionAST {
  std::string Value;
public:
  StringAST(const std::string &Value)
    : ExpressionAST(StringExpression), Value(Value) {}
  void dump(const std::string &prefix = "") const override {
    std::cout << "(str)" << Value << std::endl;
  }
};

class IdentifierAST : public ExpressionAST {
  std::string Name;
public:
  IdentifierAST(const std::string &Name)
    : ExpressionAST(IdentifierExpression), Name(Name) {}
  void dump(const std::string &prefix = "") const override {
    std::cout << "(id)" << Name << std::endl;
  }
};

class FunctionCallAST : public ExpressionAST {
  std::string Callee;
  std::vector<std::unique_ptr<ExpressionAST>> Arguments;
public:
  FunctionCallAST(const std::string &Callee,
                  std::vector<std::unique_ptr<ExpressionAST>> Arguments)
    : ExpressionAST(FunctionCallExpression), Callee(Callee)
    , Arguments(std::move(Arguments)) {}
};

// Unary operator
//class CastAST : public ExpressionAST {};

class BinaryOperatorAST : public ExpressionAST {
public:
  enum OperatorKind {
    Add, Minus, Multiply, Division, Modulo,
    LogicalAnd, LogicalOr,
    Less, LessEqual, Equal, Greater, GreaterEqual,
    BitwiseAnd, BitwiseOr, BitwiseXor, LeftShift, RightShift,
    Assign /**, Comma**/
  };
private:
  OperatorKind Kind;
  std::unique_ptr<ExpressionAST> LHS, RHS;
public:
  BinaryOperatorAST(OperatorKind Kind,
                    std::unique_ptr<ExpressionAST> LHS,
                    std::unique_ptr<ExpressionAST> RHS)
    : ExpressionAST(BinaryOperatorExpression)
    , Kind(Kind), LHS(std::move(LHS)), RHS(std::move(RHS)) {}
  static std::unique_ptr<ExpressionAST> create(Token::TokenKind TokenKind,
    std::unique_ptr<ExpressionAST> LHS, std::unique_ptr<ExpressionAST> RHS);


  void dump(const std::string &prefix = "") const override {
    std::string OperatorSymbol;
    switch (Kind) {
    default: break;
    case Add:           OperatorSymbol = "Add"; break;
    case Minus:         OperatorSymbol = "Sub"; break;
    case Multiply:      OperatorSymbol = "Mul"; break;
    case Division:      OperatorSymbol = "Div"; break;
    case Modulo:        OperatorSymbol = "Mod"; break;
    case LogicalAnd:    OperatorSymbol = "And"; break;
    case LogicalOr:     OperatorSymbol = "Or"; break;
    case Less:          OperatorSymbol = "Less"; break;
    case LessEqual:     OperatorSymbol = "LessEq"; break;
    case Equal:         OperatorSymbol = "Equal"; break;
    case Greater:       OperatorSymbol = "Greater"; break;
    case BitwiseAnd:    OperatorSymbol = "BitAnd"; break;
    case BitwiseOr:     OperatorSymbol = "BitOr"; break;
    case BitwiseXor:    OperatorSymbol = "Xor"; break;
    case LeftShift:     OperatorSymbol = "LShift"; break;
    case RightShift:    OperatorSymbol = "RShift"; break;
    case Assign:        OperatorSymbol = "Assign"; break;
    case GreaterEqual:  OperatorSymbol = "GreaterEq"; break;
    }
    std::cout << OperatorSymbol << std::endl;
    std::cout << prefix << "|---";
    LHS->dump(prefix + "|   ");
    std::cout << prefix << "`---";
    RHS->dump(prefix + "    ");
  }
};

class UnaryOperatorAST : public ExpressionAST {
public:
  enum OperatorKind { Plus, Minus, LogicalNot, BitwiseNot };
private:
  OperatorKind Kind;
  std::unique_ptr<ExpressionAST> Operand;
public:
  UnaryOperatorAST(OperatorKind Kind, std::unique_ptr<ExpressionAST> Operand)
    : ExpressionAST(UnaryOperatorExpression)
    , Kind(Kind), Operand(std::move(Operand)) {}

  void dump(const std::string &prefix = "") const override {
    std::string OperatorSymbol;
    switch (Kind) {
    default: break;
    case Plus:       OperatorSymbol = "Positive"; break;
    case Minus:      OperatorSymbol = "Negative"; break;
    case LogicalNot: OperatorSymbol = "Not"; break;
    case BitwiseNot: OperatorSymbol = "BitNot"; break;
    }
    std::cout << "(" << OperatorSymbol << ")" << std::endl;
    std::cout << prefix << "`---";
    Operand->dump(prefix + "    ");
  }
};

class DeclarationAST : public StatementAST {
  std::string Name;
  std::unique_ptr<TypeSpecifier> Type;
  std::unique_ptr<ExpressionAST> Initializer;
  int VariableIndex;
  bool IsLocalVariable;
public:
  bool isLocal() { return IsLocalVariable; }
};

class BlockAST : public StatementAST {
public:
  enum BlockKind { UndefinedBlock, FunctionBlock, NormalBlock };
private:
  BlockAST *OuterBlock;
  std::list<std::unique_ptr<StatementAST>> StatementList;
  std::list<std::unique_ptr<DeclarationAST>> DeclarationList;
public:
  BlockAST(BlockAST *OuterBlock = nullptr)
    : StatementAST(BlockStatement), OuterBlock(OuterBlock) {}
  void addStatement(std::unique_ptr<StatementAST> Statement) {
    StatementList.push_back(std::move(Statement));
  }
  void addDeclaration(std::unique_ptr<DeclarationAST> Declaration) {
    DeclarationList.push_back(std::move(Declaration));
  }
  const decltype(StatementList) &getStatementList() const {
    return StatementList;
  }
  const decltype(DeclarationList) &getDeclarationList() const {
    return DeclarationList;
  }
  BlockAST *getOuterBlock() const { return OuterBlock; }

  void dump(const std::string &prefix = "") const override {
    std::cout << "(Block)\n";
    for (const auto &Statement : StatementList) {
      if (Statement != StatementList.back()) {
        std::cout << prefix << "|---";
        Statement->dump(prefix + "|   ");
      } else {
        std::cout << prefix << "`---";
        Statement->dump(prefix + "    ");
      }
    }
  }
};

class ExprStatementAST : public StatementAST {
  std::unique_ptr<ExpressionAST> Expression;
public:
  ExprStatementAST(std::unique_ptr<ExpressionAST> Expression)
    : StatementAST(ExprStatement), Expression(std::move(Expression)) {}

  void dump(const std::string &prefix) const override {
    std::cout << "(ExprStmt)" << std::endl;
    std::cout << prefix << "`---";
    Expression->dump(prefix + "    ");
  }
};

// class StatementBlockAST : public BlockAST {
//   // TODO? Statement   *statement;
//   size_t ContinueLabel;
//   size_t BreakLabel;
// };

class FunctionDefinitionAST /*: public StatementAST*/ {
  std::string Name;
  std::unique_ptr<TypeSpecifier> Type;
  std::list<Parameter> ParameterList;
  std::unique_ptr<StatementAST> Statement;
  std::list<std::unique_ptr<DeclarationAST>> LocalVariableList;
  // int Index;
public:
  FunctionDefinitionAST() = default;
  FunctionDefinitionAST(const std::string &Name,
                        std::unique_ptr<TypeSpecifier> Type,
                        std::list<Parameter> &&ParameterList,
                        std::unique_ptr<StatementAST> Statement)
    : Name(Name), Type(std::move(Type)), ParameterList(std::move(ParameterList))
    , Statement(std::move(Statement)) {}

  void dump() const {
    std::cout << "Func: " << Name << "\nPara: ";
    for (auto &P : ParameterList)
      std::cout << P.toString() << ", ";
    std::cout << "\n";
    Statement->dump();
  }
};

class FunctionBlock : public BlockAST {
  std::unique_ptr<FunctionDefinitionAST> Function;
  size_t EndLabel;
};

class IfStatementAST : public StatementAST {
  std::unique_ptr<ExpressionAST> Condition;
  std::unique_ptr<StatementAST> StatementThen;
  std::unique_ptr<StatementAST> StatementElse;
public:
  IfStatementAST(std::unique_ptr<ExpressionAST> Condition,
                 std::unique_ptr<StatementAST> StatementThen,
                 std::unique_ptr<StatementAST> StatementElse)
    : StatementAST(IfStatement)
    , Condition(std::move(Condition))
    , StatementThen(std::move(StatementThen))
    , StatementElse(std::move(StatementElse)) {}

  void dump(const std::string &prefix = "") const override {
    std::cout << "if" << std::endl;
    std::cout << prefix << "|---";
    Condition->dump(prefix + "|   ");
    if (StatementElse) {
      std::cout << prefix << "|---";
      StatementThen->dump(prefix + "|   ");
      std::cout << prefix << "`---";
      StatementElse->dump(prefix + "    ");
    } else {
      std::cout << prefix << "`---";
      StatementThen->dump(prefix + "    ");
    }
  }
};

class WhileStatementAST : public StatementAST {
  // char *label? TODO
  std::unique_ptr<ExpressionAST> Condition;
  std::unique_ptr<StatementAST> Statement;
public:
  WhileStatementAST(std::unique_ptr<ExpressionAST> Condition,
                    std::unique_ptr<StatementAST> Statement)
    : StatementAST(WhileStatement)
    , Condition(std::move(Condition))
    , Statement(std::move(Statement)) {}


  void dump(const std::string &prefix) const override {
    std::cout << "while" << std::endl;
    std::cout << prefix << "|---";
    Condition->dump(prefix + "|   ");
    std::cout << prefix << "`---";
    Statement->dump(prefix + "    ");
  }
};

class ForStatementAST : public StatementAST {
  // char *label? TODO
  std::unique_ptr<ExpressionAST> Init;
  std::unique_ptr<ExpressionAST> Condition;
  std::unique_ptr<ExpressionAST> Post;
  std::unique_ptr<StatementAST> Statement;
public:
  ForStatementAST(std::unique_ptr<ExpressionAST> Init,
                  std::unique_ptr<ExpressionAST> Condition,
                  std::unique_ptr<ExpressionAST> Post,
                  std::unique_ptr<StatementAST> Statement)
    : StatementAST(ForStatement)
    , Init(std::move(Init)), Condition(std::move(Condition))
    , Post(std::move(Post)), Statement(std::move(Statement)) {}

  void dump(const std::string &prefix) const override {
    std::cout << "for" << std::endl;
    std::cout << prefix << "|---"; Init->dump(prefix + "|   ");
    std::cout << prefix << "|---"; Condition->dump(prefix + "|   ");
    std::cout << prefix << "|---"; Post->dump(prefix + "|   ");
    std::cout << prefix << "`---"; Statement->dump(prefix + "    ");
  }
};

class ReturnStatementAST : public StatementAST {
  std::unique_ptr<ExpressionAST> ReturnValue;
public:
  ReturnStatementAST(std::unique_ptr<ExpressionAST> ReturnValue)
    : StatementAST(ReturnStatement), ReturnValue(std::move(ReturnValue)) {}

  void dump(const std::string &prefix = "") const override {
    std::cout << "return" << std::endl;
    if (!ReturnValue)
      return;
    std::cout << prefix << "`---";
    ReturnValue->dump(prefix + "    ");
  }
};

class BreakStatementAST : public StatementAST {
public:
  BreakStatementAST() : StatementAST(BreakStatement) {}

  void dump(const std::string &prefix = "") const override {
    std::cout << "break" << std::endl;
  }
};

class ContinueStatementAST : public StatementAST {
public:
  ContinueStatementAST() : StatementAST(ContinueStatement) {}

  void dump(const std::string &prefix = "") const override {
    std::cout << "continue" << std::endl;
  }
};

/************************** Parser class ****************************/
class CMMParser {
public:
  using LocTy = CMMLexer::LocTy;
private:
  SourceMgr &SrcMgr;
  CMMLexer Lexer;
  BlockAST TopLevelBlock;
  BlockAST *CurrentBlock;

  std::map<std::string, int8_t> BinOpPrecedence;
  std::map<std::string, FunctionDefinitionAST> FunctionDefinition;
private:
  Token::TokenKind getKind() { return Lexer.getKind(); }
  Token Lex() { return Lexer.Lex(); }

  bool Error(LocTy Loc, const std::string &Msg) { return Lexer.Error(Loc, Msg);}
  bool Error(const std::string &Msg) { return Lexer.Error(Msg); }
  void Warning(LocTy Loc, const std::string &Msg) { Lexer.Warning(Loc, Msg); }
  void Warning(const std::string &Msg) { Lexer.Warning(Msg); }

  int8_t getBinOpPrecedence();

  bool parseToplevel(); //TODO
  bool parseFunctionDefinition();
  bool parseFunctionDefinition(cvm::BasicType Type, const std::string &Name);
  bool parseStatement(std::unique_ptr<StatementAST> &Res);
  bool parseBlock(std::unique_ptr<StatementAST> &Res);  //TODO
  bool parseTypeSpecifier(cvm::BasicType &Type); //?
  bool parseParameterList(std::list<Parameter> &ParameterList);
  bool parseArgumentList(); //TODO
  bool parseExprStatement(std::unique_ptr<StatementAST> &Res);
  bool parseIfStatement(std::unique_ptr<StatementAST> &Res);
  bool parseWhileStatement(std::unique_ptr<StatementAST> &Res);
  bool parseForStatement(std::unique_ptr<StatementAST> &Res);
  bool parseReturnStatement(std::unique_ptr<StatementAST> &Res);
  bool parseBreakStatement(std::unique_ptr<StatementAST> &Res);
  bool parseContinueStatement(std::unique_ptr<StatementAST> &Res);
  bool parseDeclarationStatement(std::unique_ptr<StatementAST> &Res); //TODO
  bool parseDeclarationStatement(cvm::BasicType Type,
                                 std::unique_ptr<StatementAST> &Res); //TODO
  // First: LParen,Id,Int,Double,Str,Bool,Plus,Minus,Tilde,Exclaim
  bool parseExpression(std::unique_ptr<ExpressionAST> &Res);
  bool parsePrimaryExpression(std::unique_ptr<ExpressionAST> &Res);
  bool parseBinOpRHS(int8_t ExprPrec, std::unique_ptr<ExpressionAST> &Res);
  bool parseParenExpression(std::unique_ptr<ExpressionAST> &Res);
  bool parseIdentifierExpression(std::unique_ptr<ExpressionAST> &Res); //TODO
  bool parseConstantExpression(std::unique_ptr<ExpressionAST> &Res);
public:
  CMMParser(SourceMgr &SrcMgr)
    : SrcMgr(SrcMgr), Lexer(SrcMgr), CurrentBlock(&TopLevelBlock) {
  }
  bool Parse();
};
}

#endif // !CMMPARSER_H
