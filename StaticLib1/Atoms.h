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
	int init = 0;
	
};

class SymbTable {
public:
	SymbTable();
	virtual ~SymbTable();
	int getIDByName(std::string, int scope);
	std::string getTypeByName(std::string name, int scope);
	int getScopeByID(std::string);
	int getInitByName(std::string, int scope);
	int addVar(std::string name, int scope, std::string type, int init = 0); //���������� ID ����� ���������� � �������, ���� ����� ���������� ��� ����, ���������� -1 (������������� �������������)
	int alloc(int scope); //������� ������ � ������� ��� ��������� ����������
	int addFunc(std::string name, std::string type); //��������� � �������� �������, ���������� ID ��� -1, ���� ������
	int checkVar(std::string, int scope);
	int checkFunc(std::string, int len);
	void setInit(int, int len); //������������� �������� ���������� ��� ���������� ���������� �������
	int tempVarNum = 0;

private:
	int nextID = 2;
	std::vector<SymbTableRow> table = {SymbTableRow(0, "", "const", -1, "int", 0), SymbTableRow(1, "", "const", -1, "int", 1) };
protected:
};

class AtomGen {
public:
	AtomGen(Tree&);
	virtual ~AtomGen();

private:
	SymbTable table;
	std::vector<Atom> result = {};
	void genNewAtoms(Node&);
	void NLR(Node&);
	std::pair<std::string, std::string> parsLexem(std::string);
	int scope = -1;
	int main_state = 0;
	int func_params = 0;
	bool wait_func = false;
	int value = -1;
	std::vector<int> stack_states = {};
	std::vector<std::string> stack_ops = { 0 };
	std::vector<Atom> stack_atoms = {};
	std::vector<int> exp = {};
	Lexem val = LEX_EMPTY;
	std::vector<int> stack_val = {};
	std::map<std::string, int> stateDict = { {"StmtList", 0}, {"Stmt", 1},  {"DeclareStmt", 2}, {"AssignOrCallOp", 3}, {"WhileOp", 4}, {"ForOp", 5}, 
											{"IfOp", 6},  {"SwitchOp", 7}, {"IOp", 8}, {"OOp", 9}, {"kwbreak", 10}, {"Type", 11}, {"DeclareStmtl", 12}, 
											{"ParamList", 13}, {"DeclVarListl", 14},  {"InitVar", 15}, {"ParamListl", 16}, {"AssignOrCall", 17}, {"AssignOrCalll", 18},
											{"ArgList", 19},  {"ForInit", 20}, {"ForExp", 21}, {"ForLoop", 22}, {"ElsePart", 23}, {"Cases", 24}, {"ACase", 25}, 
											{"Casesl", 26}, {"OOpl", 27},  {"E", 28}, {"E7", 29}, {"E6", 30}, {"E5", 31}, {"E4", 32}, {"E3", 33}, {"E2", 34},
											{"E1", 35},  {"ArgListl", 36}, {"rpar", 37}, {"num", 38}, {"id", 39}, {"epsilon", 40}, {"rbrace", 41},
											{"semicolon", 42}, {"kwchar", 43},  {"kwint", 44}, {"char", 45}, {"opassign", 46} };
	std::string current_type = "";
	std::string newLable();
	int stringToint(std::string);

protected:
};



