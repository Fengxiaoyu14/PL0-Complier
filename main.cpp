#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include "Lxical_analyzer.h"
#include "Token.h"
#include "Parser.h"

using namespace std;

int main() 
{
#if _DEBUG
	freopen("in.txt", "r", stdin);
	freopen("out.txt", "w", stdout);
#endif

	string str, source;
	while (getline(cin, str))
	{
		source += str;
		source += ' ';
	}

	Lexer lexer(source);

	vector<Token> tokenStream;
	Token token;
	do {
		token = lexer.getNextToken();
		if (token.type == LexicalError)
		{
			cout << "Lexical Error" << endl;
			return 0;
		}
		tokenStream.push_back(token);
	} while (token.type != EndOfFile);

	// LAB1
	//for (auto& t : tokenStream)
	//{
	//	if (t.type == Identifier)
	//		cout << "IDENTIFIER ";
	//	else if (t.type == Number)
	//		cout << "NUMBER ";
	//	else if (t.type == EndOfFile)
	//		continue;
	//	cout << t.lexeme << endl;
	//}

	// LAB2
	Parser parser(tokenStream);
 	if (!parser.parse())
	{
		cout << "Syntax Error" << endl;
		return 0;
	}

	string exp = parser.output();
	cout << exp << endl;

	return 0;
}
