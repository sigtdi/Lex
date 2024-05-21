#pragma once
#include "SAtree.h"

struct Atom
{
	std::string operation = "";
	int op1 = -1;
	int op2 = -1;
	int res = -1;
};

struct SymbTableRow {
	int ID = -1;
	std::string name = "";
	std::string type = "";
	int scope = -1;
	std::string kind = "";
	std::string init = "";
	
};

class SymbTable {
public:
	SymbTable();
	virtual ~SymbTable();
	int getIDByName(std::string, int scope);
	std::string getTypeByName(std::string name, int scope);
	int getScopeByID(std::string);
	std::string getInitByName(std::string, int scope);
	int addVar(std::string name, int scope, std::string type, std::string init = ""); //возвращает ID новой переменной в таблице, если такая переменная уже есть, возвращает -1 (множественная инициализация)
	int alloc(int scope); //создает строку в таблице для временной переменной
	int addFunc(std::string name, std::string type); //добавляет в тацблицу функцию, возвращает ID или -1, если ошибка
	bool checkVar(std::string, int scope);
	bool checkFunc(std::string, int len);
	void setInit(std::string, int len); //устанавливает значение переменной или количество параметров функции
	int tempVarNum = 0;

private:
	int nextID = 0;
	std::vector<SymbTableRow> table;
protected:
};

class AtomGen {
public:
	AtomGen(Tree&);
	virtual ~AtomGen();

private:
	SymbTable table;
	std::vector<Atom> result = {};
	//std::map<std::string, void(*)(std::string)> dictOfLambdas;
	void genNewAtoms(Node&);
	void NLR(Node&);
	std::pair<std::string, std::string> parsLexem(std::string);
	//void initDict();
	void fsm();
	int scope = -1;
	int state = 0;
	int func_params = 0;
	int nested = 0;
	std::string ExpPlace = "";
	std::vector<std::string> stack_ops = { 0 };
	std::vector<Atom> stack_atoms = {};
	std::vector<Lexem> stack_val = {};
	std::map<std::string, int> moveFrom0 = { {"DeclareStmt", 1}, {"AssignOrCallOp", 6} };
	std::string current_type = "";
	std::string newLable();

protected:
};



