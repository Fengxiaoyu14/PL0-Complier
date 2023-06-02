#include <iostream>
#include <string>
#include <vector>
#include <cctype>
#include <stack>
#include "Token.h"
#include "Node.h"

using namespace std;

class Parser
{
public:
	Parser(const vector<Token>& tokens) : tokens(tokens), index(0), nestedLevel(0), error(false)
	{
		currentToken = tokens[index];
		root = nullptr;
		answer = "";
	}

	string output()
	{
		preorder(root);
		return answer;
	}

	// 语法分析入口
	bool parse()
	{
		program();

		return !error;
	}

private:
	vector<Token> tokens;
	int index;
	Token currentToken;
	bool error;
	Node* root;
	string answer;

	int nestedLevel;											 // 嵌套层数
	static const int MAX_NESTED_LEVEL = 3;  // 最大嵌套层数

	// 遍历语法树
	void preorder(Node* node)
	{
		if (node == nullptr)
			return;

		answer += node->getLabel();
		if (node->getChildCnt() >= 1)
		{
			answer += "(";
		}

		auto& child = node->getChildren();
		for (int i = 0; i < child.size(); i++)
		{
			preorder(child[i]);
			if (i < child.size() - 1)
				answer += ",";
		}

		if (node->getChildCnt() >= 1)
		{
			answer += ")";
		}
	}

	// 获取下一个 Token
	void nextToken()
	{
		if (index < tokens.size())
		{
			currentToken = tokens[++index];
		}
	}

	// 匹配当前 Token 的类型
	bool match(TokenType expectedType)
	{
		return currentToken.type == expectedType;
	}

	// 报告错误信息
	void reportError(const string& message)
	{
		cerr << "错误: " << message << endl;
		error = true;
		return;
	}

	// <程序>→<分程序>.
	Node* program()
	{
		Node* programNode = new Node("PROGRAM");

		programNode->addChild(subprogram());
		programNode->addChild(period());

		if (currentToken.type != EndOfFile)
			error = true;

		root = programNode;
		return programNode;
	}

	Node* period()
	{
		if (!match(Period))
			reportError("缺少句点 '.'");

		if (!error)
			nextToken();
		return new Node(".");
	}

	// <分程序>→ [<常量说明部分>][<变量说明部分>][<过程说明部分>]<语句>
	Node* subprogram()
	{
		Node* subprogramNode = new Node("SUBPROG");

		if (match(CONST)) {
			subprogramNode->addChild(constantDeclaration());
		}
		if (match(VAR)) {
			subprogramNode->addChild(variableDeclaration());
		}
		if (match(PROCEDURE)) {
			subprogramNode->addChild(procedureDeclaration());
		}

		subprogramNode->addChild(statement());

		return subprogramNode;
	}

	// <常量说明部分> → CONST<常量定义>{ ,<常量定义>};
	Node* constantDeclaration()
	{
		Node* constantDeclarationNode = new Node("CONSTANTDECLARE");

		constantDeclarationNode->addChild(Const());
		constantDeclarationNode->addChild(constantDefine());

		while (match(Comma))
		{
			constantDeclarationNode->addChild(comma());
			constantDeclarationNode->addChild(constantDefine());
		}

		constantDeclarationNode->addChild(semicolon());

		return constantDeclarationNode;
	}

	Node* comma()
	{
		if (!match(Comma))
			reportError("缺少逗号','");

		if (!error)
			nextToken();
		return new Node("COMMA");
	}

	Node* semicolon()
	{
		if (!match(Semicolon))
			reportError("缺少分号 ';'");

		if (!error)
			nextToken();
		return new Node(";");
	}

	Node* Const()
	{
		if (!match(CONST))
			reportError("缺少关键字 'CONST'");

		if (!error)
			nextToken();
		return new Node("CONST");
	}

	// <常量定义> → <标识符>=<无符号整数>
	Node* constantDefine()
	{
		Node* constantDefineNode = new Node("CONSTANTDEFINE");

		constantDefineNode->addChild(identifier());
		constantDefineNode->addChild(equal());
		constantDefineNode->addChild(number());

		return constantDefineNode;
	}

	// <无符号整数> → <数字>{<数字>}
	Node* number()
	{
		if (!match(Number))
			reportError("缺少无符号整数");

		Node* numberNode = new Node(currentToken.lexeme);
		nextToken();
		return numberNode;
	}

	Node* equal()
	{
		if (!match(Equal))
			reportError("缺少等号 '='");

		if (!error)
			nextToken();
		return new Node("=");
	}

	// <变量说明部分> → VAR<标识符>{, <标识符>};
	Node* variableDeclaration()
	{
		Node* variableDeclarationNode = new Node("VARIABLEDECLARE");

		variableDeclarationNode->addChild(var());
		variableDeclarationNode->addChild(identifier());

		while (match(Comma))
		{
			variableDeclarationNode->addChild(comma());
			variableDeclarationNode->addChild(identifier());
		}

		variableDeclarationNode->addChild(semicolon());

		return variableDeclarationNode;
	}

	Node* var()
	{
		if (!match(VAR))
			reportError("缺少关键字 'VAR'");

		if (!error)
			nextToken();
		return new Node("VAR");
	}

	// <标识符> → <字母>{<字母> | <数字>}
	Node* identifier()
	{
		if (!match(Identifier))
			reportError("缺少标识符");

		Node* indentifierNode = new Node(currentToken.lexeme);
		nextToken();
		return indentifierNode;
	}

	// <过程说明部分> → <过程首部><分程序>;{<过程说明部分>}
	Node* procedureDeclaration()
	{
		// 检查嵌套层数是否超过最大层数
		nestedLevel++;
		if (nestedLevel > MAX_NESTED_LEVEL)
		{
			reportError("嵌套定义函数超过最大层数");
		}

		Node* procedureDeclarationNode = new Node("PROCEDUREDECLARE");

		procedureDeclarationNode->addChild(procedureHead());
		procedureDeclarationNode->addChild(subprogram());
		procedureDeclarationNode->addChild(semicolon());

		nestedLevel--;
		while (match(PROCEDURE))
		{
			procedureDeclarationNode->addChild(procedureDeclaration());
		}

		return procedureDeclarationNode;
	}

	// <过程首部> → PROCEDURE <标识符>;
	Node* procedureHead()
	{
		Node* procedureHeadNode = new Node("PROCEDUREHEAD");

		procedureHeadNode->addChild(procedure());
		procedureHeadNode->addChild(identifier());
		procedureHeadNode->addChild(semicolon());

		return procedureHeadNode;
	}

	Node* procedure()
	{
		if (!match(PROCEDURE))
			reportError("缺少关键字 'PROCEDURE'");

		if (!error)
			nextToken();
		return new Node("PROCEDURE");
	}

	// <语句> → <赋值语句>|<条件语句>|<当型循环语句>|<过程调用语句>|<读语句>|<写语句>|<复合语句>|<空语句>
	Node* statement()
	{
		Node* statementNode = new Node("SENTENCE");

		if (match(Identifier)) {
			statementNode->addChild(assignmentStatement());
		}
		else if (match(IF)) {
			statementNode->addChild(conditionalStatement());
		}
		else if (match(WHILE)) {
			statementNode->addChild(whileStatement());
		}
		else if (match(CALL)) {
			statementNode->addChild(procedureCallStatement());
		}
		else if (match(READ)) {
			statementNode->addChild(readStatement());
		}
		else if (match(WRITE)) {
			statementNode->addChild(writeStatement());
		}
		else if (match(BEGIN)) {
			statementNode->addChild(compoundStatement());
		}
		else {
			statementNode->addChild(emptyStatement());
		}

		return statementNode;
	}

	// <赋值语句> → <标识符>:=<表达式>
	Node* assignmentStatement()
	{
		Node* assignmentStatementNode = new Node("ASSIGNMENT");

		assignmentStatementNode->addChild(identifier());
		assignmentStatementNode->addChild(assign());
		assignmentStatementNode->addChild(expression());

		return assignmentStatementNode;
	}

	Node* assign()
	{
		if (!match(Assign))
			reportError("缺少赋值符号 ':='");

		if (!error)
			nextToken();
		return new Node(":=");
	}

	// <复合语句> → BEGIN<语句>{ ;<语句>} END
	Node* compoundStatement()
	{
		Node* compoundStatementNode = new Node("COMBINED");

		compoundStatementNode->addChild(begin());
		compoundStatementNode->addChild(statement());

		while (match(Semicolon))
		{
			compoundStatementNode->addChild(semicolon());
			compoundStatementNode->addChild(statement());
		}

		compoundStatementNode->addChild(end());

		return compoundStatementNode;
	}

	Node* begin()
	{
		if (!match(BEGIN))
			reportError("缺少关键字 'BEGIN'");

		if (!error)
			nextToken();
		return new Node("BEGIN");
	}

	Node* end()
	{
		if (!match(END))
			reportError("缺少关键字 'END'");

		if (!error)
			nextToken();
		return new Node("END");
	}

	//<条件> → <表达式><关系运算符><表达式>|ODD<表达式>
	Node* condition()
	{
		Node* conditionNode = new Node("CONDITION");

		if (match(ODD))
		{
			conditionNode->addChild(odd());
			conditionNode->addChild(expression());
		}
		else
		{
			conditionNode->addChild(expression());
			conditionNode->addChild(relationalOperator());
			conditionNode->addChild(expression());
		}

		return conditionNode;
	}

	Node* odd()
	{
		if (!match(ODD))
			reportError("缺少关键字'ODD'");

		if (!error)
			nextToken();
		return new Node("ODD");
	}

	// <表达式> → [+|-]<项>{<加减运算符><项>}
	Node* expression()
	{
		Node* expressionNode = new Node("EXPRESSION");

		if (match(Plus))
			expressionNode->addChild(plus());
		else if (match(Minus))
			expressionNode->addChild(minus());

		expressionNode->addChild(term());

		while (match(Plus) || match(Minus))
		{
			if (match(Plus))
				expressionNode->addChild(plus());
			else if (match(Minus))
				expressionNode->addChild(minus());

			expressionNode->addChild(term());
		}

		return expressionNode;
	}

	// <项> → <因子>{<乘除运算符><因子>}
	Node* term()
	{
		Node* termNode = new Node("ITEM");

		termNode->addChild(factor());

		while (match(Times) || match(Slash))
		{
			if (match(Times))
				termNode->addChild(times());
			else if (match(Slash))
				termNode->addChild(slash());

			termNode->addChild(factor());
		}

		return termNode;
	}

	Node* times()
	{
		if (!match(Times))
			reportError("缺少乘号'*'");

		if (!error)
			nextToken();
		return new Node("*");
	}

	Node* slash()
	{
		if (!match(Slash))
			reportError("缺少除号'/'");

		if (!error)
			nextToken();
		return new Node("/");
	}

	// <因子> → <标识符>|<无符号整数>|(<表达式>)
	Node* factor()
	{
		Node* factorNode = new Node("FACTOR");

		if (match(Identifier))
			factorNode->addChild(identifier());
		else if (match(Number))
			factorNode->addChild(number());
		else if (match(LParen))
		{
			factorNode->addChild(lparen());
			factorNode->addChild(expression());
			factorNode->addChild(rparen());
		}
		else
		{
			reportError("因子错误");
		}

		return factorNode;
	}

	Node* lparen()
	{
		if (!match(LParen))
			reportError("缺少左括号'('");

		if (!error)
			nextToken();
		return new Node("LP");
	}

	Node* rparen()
	{
		if (!match(RParen))
			reportError("缺少右括号')'");

		if (!error)
			nextToken();
		return new Node("RP");
	}

	// <关系运算符> → = | # | <|<=|>|>=
	Node* relationalOperator()
	{
		Node* ret = nullptr;
		if (match(Equal))
			ret = new Node("=");
		else if (match(Hash))
			ret = new Node("#");
		else if (match(LessThan))
			ret = new Node("<");
		else if (match(LessThanOrEqual))
			ret = new Node("<=");
		else if (match(MoreThan))
			ret = new Node(">");
		else if (match(MoreThanOrEqual))
			ret = new Node(">=");
		else reportError("关系运算符错误");

		nextToken();
		return ret;
	}

	// <条件语句> → IF<条件>THEN<语句>
	Node* conditionalStatement()
	{
		Node* conditionalStatementNode = new Node("IFSENTENCE");

		conditionalStatementNode->addChild(If());
		conditionalStatementNode->addChild(condition());
		conditionalStatementNode->addChild(then());
		conditionalStatementNode->addChild(statement());

		return conditionalStatementNode;
	}

	Node* If()
	{
		if (!match(IF))
			reportError("缺少关键字 'IF'");

		nextToken();
		return new Node("IF");
	}

	Node* then()
	{
		if (!match(THEN))
			reportError("缺少关键字 'THEN'");

		nextToken();
		return new Node("THEN");
	}

	// <过程调用语句> → CALL<标识符>
	Node* procedureCallStatement()
	{
		Node* procedureCallStatementNode = new Node("CALLSENTENCE");

		procedureCallStatementNode->addChild(call());
		procedureCallStatementNode->addChild(identifier());

		return procedureCallStatementNode;
	}

	Node* call()
	{
		if (!match(CALL))
			reportError("缺少关键字 'CALL'");
		nextToken();
		return new Node("CALL");
	}

	// <当型循环语句> → WHILE<条件>DO<语句>
	Node* whileStatement()
	{
		Node* whileStatementNode = new Node("WHILESENTENCE");

		whileStatementNode->addChild(While());
		whileStatementNode->addChild(condition());
		whileStatementNode->addChild(Do());
		whileStatementNode->addChild(statement());

		return whileStatementNode;
	}

	Node* While()
	{
		if (!match(WHILE))
			reportError("缺少关键字 'WHILE'");
		nextToken();
		return new Node("WHILE");
	}

	Node* Do()
	{
		if (!match(DO))
			reportError("缺少关键字 'DO'");
		nextToken();
		return new Node("DO");
	}

	// <读语句> → READ(<标识符>{ ,<标识符>})
	Node* readStatement()
	{
		Node* readStatementNode = new Node("READSENTENCE");

		readStatementNode->addChild(read());
		readStatementNode->addChild(lparen());
		readStatementNode->addChild(identifier());

		while (match(Comma))
		{
			readStatementNode->addChild(comma());
			readStatementNode->addChild(identifier());
		}

		readStatementNode->addChild(rparen());

		return readStatementNode;
	}

	Node* read()
	{
		if (!match(READ))
			reportError("缺少关键字 'READ'");
		nextToken();
		return new Node("READ");
	}

	// <写语句> → WRITE(<标识符>{,<标识符>})
	Node* writeStatement()
	{
		Node* writeStatementNode = new Node("WRITESENTENCE");

		writeStatementNode->addChild(write());
		writeStatementNode->addChild(lparen());
		writeStatementNode->addChild(identifier());
		while (match(Comma))
		{
			writeStatementNode->addChild(comma());
			writeStatementNode->addChild(identifier());
		}
		writeStatementNode->addChild(rparen());

		return writeStatementNode;
	}

	Node* write()
	{
		if (!match(WRITE))
			reportError("缺少关键字 'WRITE'");
		nextToken();
		return new Node("WRITE");
	}

	// <空语句> → epsilon
	Node* emptyStatement()
	{
		Node* emptyNode = new Node("EMPTY");

		return emptyNode;
	}

	Node* plus()
	{
		if (!match(Plus))
			reportError("缺少加号'+'");

		nextToken();
		return new Node("+");
	}

	Node* minus()
	{
		if (!match(Minus))
			reportError("缺少减号'-'");

		nextToken();
		return new Node("-");
	}

};
