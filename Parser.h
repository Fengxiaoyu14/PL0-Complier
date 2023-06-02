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

	// �﷨�������
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

	int nestedLevel;											 // Ƕ�ײ���
	static const int MAX_NESTED_LEVEL = 3;  // ���Ƕ�ײ���

	// �����﷨��
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

	// ��ȡ��һ�� Token
	void nextToken()
	{
		if (index < tokens.size())
		{
			currentToken = tokens[++index];
		}
	}

	// ƥ�䵱ǰ Token ������
	bool match(TokenType expectedType)
	{
		return currentToken.type == expectedType;
	}

	// ���������Ϣ
	void reportError(const string& message)
	{
		cerr << "����: " << message << endl;
		error = true;
		return;
	}

	// <����>��<�ֳ���>.
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
			reportError("ȱ�پ�� '.'");

		if (!error)
			nextToken();
		return new Node(".");
	}

	// <�ֳ���>�� [<����˵������>][<����˵������>][<����˵������>]<���>
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

	// <����˵������> �� CONST<��������>{ ,<��������>};
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
			reportError("ȱ�ٶ���','");

		if (!error)
			nextToken();
		return new Node("COMMA");
	}

	Node* semicolon()
	{
		if (!match(Semicolon))
			reportError("ȱ�ٷֺ� ';'");

		if (!error)
			nextToken();
		return new Node(";");
	}

	Node* Const()
	{
		if (!match(CONST))
			reportError("ȱ�ٹؼ��� 'CONST'");

		if (!error)
			nextToken();
		return new Node("CONST");
	}

	// <��������> �� <��ʶ��>=<�޷�������>
	Node* constantDefine()
	{
		Node* constantDefineNode = new Node("CONSTANTDEFINE");

		constantDefineNode->addChild(identifier());
		constantDefineNode->addChild(equal());
		constantDefineNode->addChild(number());

		return constantDefineNode;
	}

	// <�޷�������> �� <����>{<����>}
	Node* number()
	{
		if (!match(Number))
			reportError("ȱ���޷�������");

		Node* numberNode = new Node(currentToken.lexeme);
		nextToken();
		return numberNode;
	}

	Node* equal()
	{
		if (!match(Equal))
			reportError("ȱ�ٵȺ� '='");

		if (!error)
			nextToken();
		return new Node("=");
	}

	// <����˵������> �� VAR<��ʶ��>{, <��ʶ��>};
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
			reportError("ȱ�ٹؼ��� 'VAR'");

		if (!error)
			nextToken();
		return new Node("VAR");
	}

	// <��ʶ��> �� <��ĸ>{<��ĸ> | <����>}
	Node* identifier()
	{
		if (!match(Identifier))
			reportError("ȱ�ٱ�ʶ��");

		Node* indentifierNode = new Node(currentToken.lexeme);
		nextToken();
		return indentifierNode;
	}

	// <����˵������> �� <�����ײ�><�ֳ���>;{<����˵������>}
	Node* procedureDeclaration()
	{
		// ���Ƕ�ײ����Ƿ񳬹�������
		nestedLevel++;
		if (nestedLevel > MAX_NESTED_LEVEL)
		{
			reportError("Ƕ�׶��庯������������");
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

	// <�����ײ�> �� PROCEDURE <��ʶ��>;
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
			reportError("ȱ�ٹؼ��� 'PROCEDURE'");

		if (!error)
			nextToken();
		return new Node("PROCEDURE");
	}

	// <���> �� <��ֵ���>|<�������>|<����ѭ�����>|<���̵������>|<�����>|<д���>|<�������>|<�����>
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

	// <��ֵ���> �� <��ʶ��>:=<���ʽ>
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
			reportError("ȱ�ٸ�ֵ���� ':='");

		if (!error)
			nextToken();
		return new Node(":=");
	}

	// <�������> �� BEGIN<���>{ ;<���>} END
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
			reportError("ȱ�ٹؼ��� 'BEGIN'");

		if (!error)
			nextToken();
		return new Node("BEGIN");
	}

	Node* end()
	{
		if (!match(END))
			reportError("ȱ�ٹؼ��� 'END'");

		if (!error)
			nextToken();
		return new Node("END");
	}

	//<����> �� <���ʽ><��ϵ�����><���ʽ>|ODD<���ʽ>
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
			reportError("ȱ�ٹؼ���'ODD'");

		if (!error)
			nextToken();
		return new Node("ODD");
	}

	// <���ʽ> �� [+|-]<��>{<�Ӽ������><��>}
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

	// <��> �� <����>{<�˳������><����>}
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
			reportError("ȱ�ٳ˺�'*'");

		if (!error)
			nextToken();
		return new Node("*");
	}

	Node* slash()
	{
		if (!match(Slash))
			reportError("ȱ�ٳ���'/'");

		if (!error)
			nextToken();
		return new Node("/");
	}

	// <����> �� <��ʶ��>|<�޷�������>|(<���ʽ>)
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
			reportError("���Ӵ���");
		}

		return factorNode;
	}

	Node* lparen()
	{
		if (!match(LParen))
			reportError("ȱ��������'('");

		if (!error)
			nextToken();
		return new Node("LP");
	}

	Node* rparen()
	{
		if (!match(RParen))
			reportError("ȱ��������')'");

		if (!error)
			nextToken();
		return new Node("RP");
	}

	// <��ϵ�����> �� = | # | <|<=|>|>=
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
		else reportError("��ϵ���������");

		nextToken();
		return ret;
	}

	// <�������> �� IF<����>THEN<���>
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
			reportError("ȱ�ٹؼ��� 'IF'");

		nextToken();
		return new Node("IF");
	}

	Node* then()
	{
		if (!match(THEN))
			reportError("ȱ�ٹؼ��� 'THEN'");

		nextToken();
		return new Node("THEN");
	}

	// <���̵������> �� CALL<��ʶ��>
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
			reportError("ȱ�ٹؼ��� 'CALL'");
		nextToken();
		return new Node("CALL");
	}

	// <����ѭ�����> �� WHILE<����>DO<���>
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
			reportError("ȱ�ٹؼ��� 'WHILE'");
		nextToken();
		return new Node("WHILE");
	}

	Node* Do()
	{
		if (!match(DO))
			reportError("ȱ�ٹؼ��� 'DO'");
		nextToken();
		return new Node("DO");
	}

	// <�����> �� READ(<��ʶ��>{ ,<��ʶ��>})
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
			reportError("ȱ�ٹؼ��� 'READ'");
		nextToken();
		return new Node("READ");
	}

	// <д���> �� WRITE(<��ʶ��>{,<��ʶ��>})
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
			reportError("ȱ�ٹؼ��� 'WRITE'");
		nextToken();
		return new Node("WRITE");
	}

	// <�����> �� epsilon
	Node* emptyStatement()
	{
		Node* emptyNode = new Node("EMPTY");

		return emptyNode;
	}

	Node* plus()
	{
		if (!match(Plus))
			reportError("ȱ�ټӺ�'+'");

		nextToken();
		return new Node("+");
	}

	Node* minus()
	{
		if (!match(Minus))
			reportError("ȱ�ټ���'-'");

		nextToken();
		return new Node("-");
	}

};
