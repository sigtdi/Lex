#include "pch.h"
#include "Atoms.h"
#include <string>
#include <vector>
#include <map>

AtomGen::AtomGen(Tree& tree) {
	Node root = tree.getRoot();
	NLR(root);
}

AtomGen::~AtomGen(){
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
		main_state = -2;
	}

	switch (main_state) {
	case 0: //StmtList узел
		if (stack_states[stack_states.size() - 1] == 2) { //находимс€ внутри функции
			table.setInit(scope, func_params); //устанавливаем количество параметров объ€вленной функции
			func_params = 0;
		}
		break;
	case 1:
		break;
	case 2: //—морим DeclareStmt
		stack_states.push_back(1); //объ€вл€ем переменную
		break;
	case 3: //AssignOrCallOp узел
		stack_states.push_back(3); //находимс€ в ветке присваивани€ переменной или вызова функции
		break;
	case 4:
		break;
	case 5:
		break;
	case 6:
		break;
	case 7:
		break;
	case 8:
		break;
	case 9:
		break;
	case 10:
		break;
	case 11: //—мотрим Type узел
		break;
	case 12: //—мотрим DeclareStmtl
		val = lineVec[0]; //в стек переменных добавл€ем им€ переменной 
		break;
	case 13: //ParamList узел
		stack_states.pop_back();
		stack_states.push_back(2); //объ€вл€ем функцию
		scope = table.addFunc(val.second, current_type);
		current_type = "";
		val = LEX_EMPTY;
		break;
	case 14: //—мотрим DeclVarListl
		if (lineVec.size() > 1) {
			int temp = stringToint(lineVec[1].second);
			table.addVar(val.second, scope, current_type, temp);
			val = LEX_EMPTY;
		}
		break;
	case 15: //—мотрим InitVar
		val = lineVec[1]; //в стек переменных добавл€ем им€ переменной 
		break;
	case 16: //ParamListl узел
		table.addVar(val.second, scope, current_type);
		current_type = "";
		func_params++;
		val = LEX_EMPTY;
		break;
	case 17:
		break;
	case 18: //AssignOrCalll узел
		value = table.checkVar(lineVec[0].second, scope);
		if (value == -1) {
			wait_func = true;
			val = lineVec[0];
		}
		else stack_val.push_back(value);
		break;
	case 19: //ArgList узел
		wait_func = false;
		stack_states.push_back(4); //передаем параметры функции
		break;
	case 20:
		break;
	case 21:
		break;
	case 22:
		break;
	case 23:
		break;
	case 24:
		break;
	case 25:
		break;
	case 26:
		break;
	case 27:
		break;
	case 28: //E узел
		if (stack_states[stack_states.size() - 1] == 3 && wait_func) {
			main_state = -1;
			break; //неизвестна€ переменна€ id-------------------------------------------------------------------------------
		}
		else if (lineVec[0].first == "kwreturn") {
			stack_states.push_back(5); //находимс€ в узле return E
		}
		exp.push_back(0); //считаем выражение
		break;
	case 29: //E7
		break;
	case 30: //E6
		if (lineVec[0].first == "opor") {

		}
		break;
	case 31: //E5
		break;
	case 32: //E4
		break;
	case 33: //E3
		break;
	case 34: //E2
		break;
	case 35: //E1
		break;
	case 36: //ArgListL узел
		func_params++;
		break;
	case 37: //rpar узел
		if (stack_states[stack_states.size() - 1] == 4) {//происходит вызов функции
			int id = table.checkFunc(val.second, func_params);
			if (id == -1) {
				main_state = -1;
				break; //функци€ не объ€влена --------------------------------------------------------------------------------------------
			}
			func_params = 0;
			val = LEX_EMPTY;
			int r = table.alloc(scope);
			result.push_back(Atom("CALL", id, -1, r)); //атом вызова функции
			stack_val.push_back(r);
		}
		break;
	case 38:
		break;
	case 39:
		break;
	case 40: // —мотрим epsilon узел
		if (stack_states[stack_states.size() - 1] == 1 && val != LEX_EMPTY) {
			table.addVar(val.second, scope, current_type);
			val = LEX_EMPTY;
		}
		break;
	case 41: //rbrace узел
		if (stack_states[stack_states.size() - 1] == 2) {
			stack_states.pop_back(); //вышли из функции
			scope = -1;
		}
		break;
	case 42: //semicolon узел
		if (stack_states[stack_states.size() - 1] == 1) {
			stack_states.pop_back(); //вышли из ветки объ€влени€ переменной
			current_type = "";
			result.push_back(Atom("RET", -1, -1, -1));
		}
		else if (stack_states[stack_states.size() - 1] == 3) {
			stack_states.pop_back(); //вышли из ветки присваивани€ или вызова
			result.push_back(Atom("MOV", stack_val[0], -1, stack_val[1]));
			stack_val.clear();
		}
		break;
	case 43: //kwchar узел
		current_type = "char";
		break;
	case 44: //kwint узел
		current_type = "int";
		break;
	case 45:
		break;
	case 46:
		break;
	case -1:
		////конец всего, состо€ние ошибки
		break;
	default:
		break;
	}
	
}

void AtomGen::NLR(Node& node) {
	genNewAtoms(node);
	for (Node n : node.sons) {
		NLR(n);
	}
}

int stringToint(std::string elem) {
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
int SymbTable::getInitByName(std::string name, int scope) { 
	for (auto elem : table) {
		if (elem.name == name && elem.scope == scope) {
			return elem.init;
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
		if (getTypeByName(name, scope) == "var")
			return id;
	}
	id = getIDByName(name, -1);
	if (id != -1) {
		if (getTypeByName(name, -1) == "var")
			return id;
	}
	return -1; 
}
int SymbTable::checkFunc(std::string name, int len){
	int id = getIDByName(name, -1);
	if (id == -1) {
		return -1;
	}
	if (getTypeByName(name, -1) != "func") {
		return -1;
	}
	if ((getInitByName(name, -1)) != len) {
		return -1;
	}
	return id;
}
void SymbTable::setInit(int id, int len){
	table[0].init = len;
}