#pragma once
#include <string>

enum TokenType 
{
	Identifier,
	Equal,
	Assign,
	Number,
	Plus,
	Minus,
	Times,
	Slash,
	Hash,
	LessThan,
	LessThanOrEqual,
	MoreThan,
	MoreThanOrEqual,
	LParen,
	RParen,
	Semicolon,
	Comma,
	Period,
	EndOfFile,
	LexicalError,

	CONST,
	VAR,
	PROCEDURE,
	BEGIN,
	END,
	ODD,
	IF,
	THEN,
	CALL,
	WHILE,
	DO,
	READ,
	WRITE
};

struct Token {
	TokenType type;
	std::string lexeme;
};
