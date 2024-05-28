#include "pch.h"
#include "Atoms.h"
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <algorithm>

AtomGen::AtomGen(Tree& tree) {
	Node root = tree.getRoot();
	NLR(root);
	printRes();
}

AtomGen::~AtomGen(){
}

std::string AtomGen::newLable() {
	std::string l = "L" + std::to_string(lableNum);
	++lableNum;
	return l;
}

int cantorNum(int x, int y) {
	switch (x)
	{
	case - 1:
		return -1;
	case 28:
	case 29:
	case 30:
	case 31:
	case 32:
	case 33:
	case 34:
	case 35:
		y = 0;
	default:
		return ((x + y) * (x + y) + 3 * x + y) / 2;
	}
}

void AtomGen::genNewAtoms(Node& node) {
	auto terms = node.elem1;
	size_t pos = 0;
	std::string token;
	std::vector<Lexem> lineVec = {};
	while ((pos = terms.find(" ")) != std::string::npos) {
		token = terms.substr(0, pos);
		lineVec.push_back(parsLexem(token));
		terms.erase(0, pos + 1);
	}
	lineVec.push_back(parsLexem(node.name));
	if (stateDict.find(lineVec[lineVec.size() - 1].first) != stateDict.end()) {
		if (main_state != -1)
			main_state = stateDict[lineVec[lineVec.size() - 1].first];
	}
	else {
		main_state = 0;
	}


	switch (cantorNum(main_state, stack_states[stack_states.size() - 1]))
	{
	case 0: //StmtList вне функции 
		break;
	case 3: //ѕервый StmtList в функции
	{
		table.setLen(scope, func_params[func_params.size() - 1]); //устанавливаем количество параметров объ€вленной функции
		stack_states.pop_back();
		stack_states.push_back(5);
		func_params.pop_back();
		break;
	}
	case 30: //DeclareStmt внутри функции
	case 38: //DeclareStmt внутри цикла
	case 47: //DeclareStmt внутри if
	case 5: //DeclareStmt вне функции
	{
		stack_states.push_back(1);
		break;
	}
	case 39: //AssignOrCallOp внутри функции
	case 48: //AssignOrCallOp внутри цикла
	case 58: //AssignOrCallOp внутри if
	{
		stack_states.push_back(3); //находимс€ в ветке присваивани€ переменной или вызова функции
		break;
	}
	case 103: //DeclareStmtl
	{
		val = lineVec[0]; //в стек переменных добавл€ем им€ переменной 
		break;
	}
	case 134: //DeclVarList
	{
		if (lineVec.size() > 1) { //перед нетерминалом есть значение объ€вл€емой переменной
			int temp = stringToint(lineVec[1].second);
			table.addVar(val.second, scope, current_type, temp);
			val = LEX_EMPTY;
		}
		break;
	}
	case 118: //ParamList
	{
		stack_states.pop_back();
		stack_states.push_back(2); //объ€вл€ем функцию
		scope = table.addFunc(val.second, current_type); //проверить есть ли функци€
		current_type = "";
		func_params.push_back(0);
		val = LEX_EMPTY;
		break;
	}
	case 151: //InitVar
	{
		val = lineVec[1]; //запоминаем им€ переменной 
		break;
	}
	case 187: //ParamListl
	{
		int id = table.addVar(lineVec[0].second, scope, current_type);
		if (id == -1) {
			main_state = -1; //множественна€ инициализаци€ ------------------------------------------------------------------------------------
			error = "Error: multiple initialization";
		}
		current_type = "";
		func_params[func_params.size() - 1]++;
		break;
	}
	case 249: //AssignOrCalll
	{
		if (table.checkVar(lineVec[0].second, scope) == -1) {
			wait_func = true;
			val = lineVec[0];
		}
		else stack_val.push_back(table.checkVar(lineVec[0].second, scope));
		break;
	}
	case 272: //ArgList в AssignOrCall
	{
		wait_func = false;
		func_params.push_back(0);
		stack_states.push_back(4); //передаем параметры функции
		break;
	}
	case 547: //ArgList в выражении
	{
		stack_func_exp.push_back(lineVec[0]);
		func_params.push_back(0);
		stack_states.push_back(4); //передаем параметры функции
		break;
	} 
	case 856://ArgListl
	{
		func_params[func_params.size() - 1]++;
		result.push_back(Atom("PARAM", -1, -1, stack_val[stack_val.size() - 1]));
		stack_val.pop_back();
		break;
	}
	case 898: //rpar, вызов функции
	{
		int id = table.checkFunc(stack_func_exp[stack_func_exp.size() - 1].second, func_params[func_params.size() - 1]);
		if (id == -1) {
			main_state = -1;
			error = "Error: function is not found";
			break; //функци€ не объ€влена --------------------------------------------------------------------------------------------
		}
		func_params.pop_back();
		stack_func_exp.pop_back();
		int r = table.alloc(scope);
		stack_states.pop_back();
		result.push_back(Atom("CALL", id, -1, r)); //атом вызова функции
		stack_val.push_back(r);
		break;
	}
	case 901: //epsilon, объ€вление переменной
			table.addVar(val.second, scope, current_type);
			val = LEX_EMPTY;
		break;
	case 1122: //rbrace, в конце функции
		stack_states.pop_back(); //вышли из функции
		result.push_back(Atom("RET", -1, -1, 0));
		big_result.insert({ table.getNameById(scope, -1), result });
		scope = -1;
		result.clear();
		break;
	case 1317: //semicolon после return
	{
		stack_states.pop_back();
		current_type = "";
		result.push_back(Atom("RET", -1, -1, stack_val[0]));
		break;
	}
	case 1077: //semicolon после присваивани€
	{
		stack_states.pop_back(); //вышли из ветки присваивани€
		result.push_back(Atom("MOV", stack_val[1], -1, stack_val[0]));
		stack_val.clear();
		break;
	}
	case 1033: //kwchar в объ€влении переменной
	case 1078: //kwchar в объ€влении параметров функции
	{
		current_type = "char";
		break;
	}
	case 1079: //kwint в объ€влении переменной
	case 1125: //kwint в объ€влении параметров функции
	{
		current_type = "int";
		break;
	}
	case 434: //E  
	{
		if (wait_func) {
			main_state = -1;
			error = "Error: variable is not declared";
			break; //неизвестна€ переменна€ id-------------------------------------------------------------------------------
		}
		else if (lineVec[0].first == "kwreturn") {
			stack_states.push_back(8); //находимс€ в узле return E
		}
		if (stack_states[stack_states.size() - 1] == 13) {
			break;
		}
		stack_states.push_back(13); //разбираем выражение
		exp = true;; //считаем выражение
		break;
	}
	case 464: //E7
		break;
	case 495: //E6
	{
		if (lineVec[0].first == "opor") {
			int id1 = stack_val[stack_val.size() - 1];
			stack_val.pop_back();
			int id2 = stack_val[stack_val.size() - 1];
			stack_val.pop_back();
			int r = table.alloc(scope);
			result.push_back(Atom("OR", id2, id1, r));
			stack_val.push_back(r);
		}
		break;
	}
	case 527: //E5
	{
		if (lineVec[0].first == "opand") {
			int id1 = stack_val[stack_val.size() - 1];
			stack_val.pop_back();
			int id2 = stack_val[stack_val.size() - 1];
			stack_val.pop_back();
			int r = table.alloc(scope);
			result.push_back(Atom("AND", id2, id1, r));
			stack_val.push_back(r);
		}
		break;
	}
	case 560: //E4
	{
		if (lineVec[0].first == "opeq") {
			int r = table.alloc(scope);
			result.push_back(Atom("MOV", 1, -1, r));
			int id1 = stack_val[stack_val.size() - 1];
			stack_val.pop_back();
			int id2 = stack_val[stack_val.size() - 1];
			stack_val.pop_back();
			std::string l = newLable();
			result.push_back(Atom("EQ", id2, id1, -1, l));
			result.push_back(Atom("MOV", 0, -1, r));
			result.push_back(Atom("LBL", -1, -1, -1, l));
			stack_val.push_back(r);
		}
		else if (lineVec[0].first == "opne") {
			int r = table.alloc(scope);
			result.push_back(Atom("MOV", 1, -1, r));
			int id1 = stack_val[stack_val.size() - 1];
			stack_val.pop_back();
			int id2 = stack_val[stack_val.size() - 1];
			stack_val.pop_back();
			std::string l = newLable();
			result.push_back(Atom("NE", id2, id1, -1, l));
			result.push_back(Atom("MOV", 0, -1, r));
			result.push_back(Atom("LBL", -1, -1, -1, l));
			stack_val.push_back(r);
		}
		else if (lineVec[0].first == "opgt") {
			int r = table.alloc(scope);
			result.push_back(Atom("MOV", 1, -1, r));
			int id1 = stack_val[stack_val.size() - 1];
			stack_val.pop_back();
			int id2 = stack_val[stack_val.size() - 1];
			stack_val.pop_back();
			std::string l = newLable();
			result.push_back(Atom("GT", id2, id1, -1, l));
			result.push_back(Atom("MOV", 0, -1, r));
			result.push_back(Atom("LBL", -1, -1, -1, l));
			stack_val.push_back(r);
		}
		else if (lineVec[0].first == "oplt") {
			int r = table.alloc(scope);
			result.push_back(Atom("MOV", 1, -1, r));
			int id1 = stack_val[stack_val.size() - 1];
			stack_val.pop_back();
			int id2 = stack_val[stack_val.size() - 1];
			stack_val.pop_back();
			std::string l = newLable();
			result.push_back(Atom("LT", id2, id1, -1, l));
			result.push_back(Atom("MOV", 0, -1, r));
			result.push_back(Atom("LBL", -1, -1, -1, l));
			stack_val.push_back(r);
		}
		else if (lineVec[0].first == "ople") {
			int r = table.alloc(scope);
			result.push_back(Atom("MOV", 1, -1, r));
			int id1 = stack_val[stack_val.size() - 1];
			stack_val.pop_back();
			int id2 = stack_val[stack_val.size() - 1];
			stack_val.pop_back();
			std::string l = newLable();
			result.push_back(Atom("LE", id2, id1, -1, l));
			result.push_back(Atom("MOV", 0, -1, r));
			result.push_back(Atom("LBL", -1, -1, -1, l));
			stack_val.push_back(r);
		}
		break;
	}
	case 594: //E3
	{
		if (lineVec[0].first == "opplus") {
			int r = table.alloc(scope);
			int id1 = stack_val[stack_val.size() - 1];
			stack_val.pop_back();
			int id2 = stack_val[stack_val.size() - 1];
			stack_val.pop_back();
			result.push_back(Atom("ADD", id2, id1, r));
			stack_val.push_back(r);
		}
		else if (lineVec[0].first == "opminus") {
			int r = table.alloc(scope);
			int id1 = stack_val[stack_val.size() - 1];
			stack_val.pop_back();
			int id2 = stack_val[stack_val.size() - 1];
			stack_val.pop_back();
			result.push_back(Atom("SUB", id2, id1, r));
			stack_val.push_back(r);
		}
		break;
	}
	case 629: //E2
	{
		if (lineVec.size() == 1) {
			break;
		}
		int r = table.alloc(scope);
		int id1 = stack_val[stack_val.size() - 1];
		stack_val.pop_back();
		int id2 = stack_val[stack_val.size() - 1];
		stack_val.pop_back();
		result.push_back(Atom("MUL", id2, id1, r));
		stack_val.push_back(r);
		break;
	}
	case 665: //E1
	{
		arg = false;
		if (stack_states[stack_states.size() - 1] == 14) {
			stack_states.pop_back();
		}
		if (lineVec.size() == 1) {
			break;
		}
		int r = table.alloc(scope);
		int id = stack_val[stack_val.size() - 1];
		stack_val.pop_back();
		result.push_back(Atom("NOT", id, -1, r));
		stack_val.push_back(r);
		break;
	}
	case 1417: //id в выражении
	{
		stack_states.push_back(14);
		int id = table.checkVar(lineVec[0].second, scope);
		if (id == -1) {
			main_state = -1; //неизвестна€ переменна€ id-------------------------------------------------------------------------------
			error = "Error: variable is not declared";
			break;
		}
		stack_val.push_back(id);
		break;
	}
	case 1470: //id после opinc
	{
		stack_states.pop_back();
		int id = table.checkVar(lineVec[0].second, scope);
		if (id == -1) {
			main_state = -1; //неизвестна€ переменна€ id-------------------------------------------------------------------------------
			error = "Error: variable is not declared";
			break;
		}
		result.push_back(Atom("ADD", id, 1, id));
		stack_val.push_back(id);
		break;
	}
	case 1364: //num в выражении
	{
		int id = table.addConst(scope, "int", stringToint(lineVec[0].second));
		stack_val.push_back(id);
		break;
	}
	case 1877: //opinc перед id в выражении
	{
		stack_states.push_back(14);
		break;
	}
	case 1938: //opinc после id в выражении
	{
		int r = table.alloc(scope);
		int id = stack_val[stack_val.size() - 1];
		stack_val.pop_back();
		result.push_back(Atom("MOV", id, -1, r));
		result.push_back(Atom("ADD", id, 1, id));
		stack_val.push_back(r);
		break;
	}
	case -1:
	{
		if (error.empty()) {
			error = "Error";
		}
		big_result.insert({ table.getNameById(scope, -1), result });
		printRes();
		break;
	}
	default:
		break;
	}
	
}

void AtomGen::NLR(Node& node) {
	if (!error.empty()) return;
	if (node.name == "ArgList" || node.name == "ArgListl") {
		arg = true;
	}
	if (node.name == "E7") {
		arg = false;
	}
	if (arg) {
		genNewAtoms(node);
		for (Node n : node.sons) {
			NLR(n);
		}
		if (node.name == "E") {
			stack_states.pop_back();
		}
	}
	else if (exp) {
		for (Node n : node.sons) {
			NLR(n);
		}
		genNewAtoms(node);
	}
	else {
		genNewAtoms(node);
		for (Node n : node.sons) {
			NLR(n);
		}
		if (node.name == "E") { 
			exp = false;
			stack_states.pop_back();
		}
	}
}

int AtomGen::stringToint(std::string elem) {
	int res = 0;
	bool flag = true;
	for (auto ch : elem) {
		if (std::isdigit(ch)) {
			continue;
		}
		flag = false;
		break;
	}

	if (flag) {
		return std::stoi(elem);
	}
	else {
		return int(elem[0]);
	}
}

std::pair<std::string, std::string> AtomGen::parsLexem(std::string str) {
	Lexem l = {str, ""};
	size_t pos = 0;
	std::string token;
	bool flag = false;
	while ((pos = str.find("(")) != std::string::npos) {
		token = str.substr(0, pos);
		l.first = token;
		str.erase(0, pos + 1);
		flag = true;
	}
	str.pop_back();
	if (flag) l.second = str;
	return l;
}


SymbTable::SymbTable() {}
SymbTable::~SymbTable() {}
int SymbTable::getIDByName(std::string name, int scope) { 
	for (auto elem : table) {
		if (elem.name == name && elem.scope == scope) {
			return elem.ID;
		}
	}
	return -1; 
}
std::string SymbTable::getTypeByName(std::string name, int scope) { 
	for (auto elem : table) {
		if (elem.name == name && elem.scope == scope) {
			return elem.type;
		}
	}
	return "";
}

std::string SymbTable::getNameById(int id, int scope) {
	return table[id].name;
}
int SymbTable::getLenByName(std::string name, int scope) { 
	for (auto elem : table) {
		if (elem.name == name && elem.scope == scope) {
			return elem.len;
		}
	}
	return 0;
}
int SymbTable::addVar(std::string name, int scope, std::string type, int init) { 
	int ID = -1;
	if (getIDByName(name, scope) != -1) {
		return -1;
	}

	ID = nextID;
	nextID++;
	SymbTableRow row(ID, name, type, scope, "var", init);
	table.push_back(row);
	return ID; 
}
int SymbTable::alloc(int scope) { 
	int id = addVar(std::to_string(tempVarNum) + "t", scope, "int");
	tempVarNum++;
	return id;
}
int SymbTable::addFunc(std::string name, std::string type){ 
	int ID = -1;
	if (getIDByName(name, -1) != -1) {
		return -1;
	}
	ID = nextID;
	nextID++;
	SymbTableRow row(ID, name, type, -1, "func");
	table.push_back(row);
	return ID;
}
int SymbTable::checkVar(std::string name, int scope){
	int id = getIDByName(name, scope);
	if (id != -1) {
		if (table[id].kind == "var")
			return id;
	}
	id = getIDByName(name, -1);
	if (id != -1) {
		if (table[id].kind == "var")
			return id;
	}
	return -1; 
}
int SymbTable::checkFunc(std::string name, int len){
	int id = getIDByName(name, -1);
	if (id == -1) {
		return -1;
	}
	if (table[id].kind != "func") {
		return -1;
	}
	if ((getLenByName(name, -1)) != len) {
		return -1;
	}
	return id;
}
void SymbTable::setLen(int id, int len){
	table[id].len = len;
}

int SymbTable::addConst(int scope, std::string type, int init) {
	int ID = nextID;
	nextID++;
	SymbTableRow row(ID, std::to_string(tempVarNum) + "c", type, scope, "const", init);
	tempVarNum++;
	table.push_back(row);
	return ID;
}

void AtomGen::printRes() {
	std::ofstream fout;
	fout.open("atoms.txt");
	for (auto results: big_result) {
		fout << results.first << ": " << std::endl;
		for (auto res : results.second) {
			fout << "(" << res.operation << ", ";
			if (res.op1 == -1) {
				fout << ", ";
			}
			else {
				fout << res.op1 << ", ";
			}
			if (res.op2 == -1) {
				fout << ", ";
			}
			else {
				fout << res.op2 << ", ";
			}
			if (res.res == -1) {
				if (res.lable == "") {
					fout << ")";
				}
				else {
					fout << res.lable << ")";
				}
			}
			else {
				fout << res.res << ")";
			}
			fout << std::endl;
		}
	}
	fout << error << std::endl;
	table.print(fout);
}

void SymbTable::print(std::ostream& fout) {
	int len_name = 6;
	int len_id = 4;
	int len_kind = 6;
	int len_type = 6;
	int len_len = 8;
	int len_init = 9;
	int len_scope = 7;
	for (auto row : table) {
		len_name = std::max(len_name, int(row.name.size() + 2));
		len_id = std::max(len_id, int(std::to_string(row.ID).size() + 2));
		len_kind = std::max(len_kind, int(row.kind.size() + 2));
		len_type = std::max(len_type, int(row.type.size() + 2));
		len_len = std::max(len_len, int(std::to_string(row.len).size() + 2));
		len_init = std::max(len_init, int(std::to_string(row.init).size() + 2));
		len_scope = std::max(len_scope, int(std::to_string(row.scope).size() + 2));
	}
	fout << std::endl << std::endl;

	fout << "Name";
	for (int _ = 0; _ < len_name - 4; ++_) {
		fout << " ";
	}
	fout << "ID";
	for (int _ = 0; _ < len_id - 2; ++_) {
		fout << " ";
	}
	fout << "Kind";
	for (int _ = 0; _ < len_kind - 4; ++_) {
		fout << " ";
	}
	fout << "Type";
	for (int _ = 0; _ < len_type - 4; ++_) {
		fout << " ";
	}
	fout << "Lenght";
	for (int _ = 0; _ < len_len - 6; ++_) {
		fout << " ";
	}
	fout << "Default";
	for (int _ = 0; _ < len_init - 7; ++_) {
		fout << " ";
	}
	fout << "Scope";
	for (int _ = 0; _ < len_scope - 5; ++_) {
		fout << " ";
	}
	fout << std::endl;

	for (auto row : table) {
		fout << row.name;
		for (int _ = 0; _ < len_name - row.name.size(); ++_) {
			fout << " ";
		}
		fout << row.ID;
		for (int _ = 0; _ < len_id - std::to_string(row.ID).size(); ++_) {
			fout << " ";
		}
		fout << row.kind;
		for (int _ = 0; _ < len_kind - row.kind.size(); ++_) {
			fout << " ";
		}
		fout << row.type;
		for (int _ = 0; _ < len_type - row.type.size(); ++_) {
			fout << " ";
		}
		if (row.len != -1) {
			fout << row.len;
			for (int _ = 0; _ < len_len - std::to_string(row.len).size(); ++_) {
				fout << " ";
			}
		}
		else {
			fout << "none";
			for (int _ = 0; _ < len_len - 4; ++_) {
				fout << " ";
			}
		}
		if (row.kind != "func") {
			fout << row.init;
			for (int _ = 0; _ < len_init - std::to_string(row.init).size(); ++_) {
				fout << " ";
			}
		}
		else {
			fout << "none";
			for (int _ = 0; _ < len_init - 4; ++_) {
				fout << " ";
			}
		}
		fout << row.scope;
		for (int _ = 0; _ < len_scope - std::to_string(row.scope).size(); ++_) {
			fout << " ";
		}
		fout << std::endl;
	}
}