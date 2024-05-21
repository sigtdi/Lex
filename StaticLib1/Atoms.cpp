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
	auto name = parsLexem(node.name);
	size_t pos = 0;
	std::string token;
	std::vector<Lexem> lineVec = {};
	while ((pos = terms.find(" ")) != std::string::npos) {
		token = terms.substr(0, pos);
		lineVec.push_back(parsLexem(token));
		terms.erase(0, pos + 1);
	}

	switch (state) {
	case 0://Рассматриваем StmtList или Stmt узел
		if (moveFrom0.find(name.first) != moveFrom0.end()) {
			state = moveFrom0[name.first];
		}
		break;
	case 1: //рассматриваем DeclareStmt узел, определяем тип переменной или функции, которую создаем
		if (name.first == "kwint") {
			current_type = "int";
			state = 2;
		}
		else if (name.first == "kwchar")
		{
			current_type = "char";
			state = 2;
		}
		break;
	case 2: //Определяем имя объявляемой переменной или функции
		if (!lineVec.empty() && name.first == "StmtList") {
			current_type = "";
			state = 0;
			nested += 1;
		}
		else if (name.first == "StmtList") {
			current_type = "";
			state = 0;
		}
		else if (lineVec.empty()) {
			break;
		}
		else if (lineVec[0].first == "id") {
			stack_val.push_back(lineVec[0]); // в стек переменных записываем название
			state = 3;
		}
		else if (lineVec[0].first == "comma") {
			stack_val.push_back(lineVec[1]); //перед id может быть терминал comma 
			state = 3;
		}
		else if (name.first == "semicolon") {
			current_type = "";
			state = 0;
		} 
		break;
	case 3: //определяем, объявляется функция или переменная
		if (name.first == "ParamList") { //объявляем фунцкию
			if (scope != -1) {
				state = -1;
				//вывести в файл ошибку создание функции внутри функции ---------------------------------------------------------------------------
			}

			scope = table.addFunc(stack_val[stack_val.size() - 1].second, current_type); //меняем область видимости
			if (scope == -1) {
				state = -1; //вывести в файл ошибку множественной инициализации --------------------------------------------------------
				break;
			}
			stack_val.pop_back();
			current_type = "";
			state = 4;
		}
		else { // объявляем переменную
			if (name.first == "epsilon") { //переменная объявляется, но не инициализируется
				int check = table.addVar(stack_val[stack_val.size() - 1].second, scope, current_type);
				if (check == -1) {
					state = -1;
					break;
				}
				stack_val.pop_back();
				state = 2;
			}
			else if (lineVec.empty()) {
				break;
			}
			else if (lineVec[0].first == "num" || lineVec[0].first == "char") {
				int check = table.addVar(stack_val[stack_val.size() - 1].second, scope, current_type, lineVec[0].second);
				if (check == -1) {
					state = -1; //вывести в файл ошибку множественной инициализации --------------------------------------------------------
					break;
				}
				stack_val.pop_back();
				state = 2;
			}
			else if (lineVec[0].first == "opassign") {
				int check = table.addVar(stack_val[stack_val.size() - 1].second, scope, current_type, lineVec[1].second);
				if (check == -1) {
					state = -1; //вывести в файл ошибку множественной инициализации --------------------------------------------------------
					break;
				}
				stack_val.pop_back();
				state = 2;
			}
		}
		break;
	case 4://считываем тип параметра функции
		if (name.first == "kwint") {
			current_type = "int";
			state = 5;
		}
		else if (name.first == "kwchar")
		{
			current_type = "char";
			state = 5;
		}
		else if (name.first == "epsilon") {
			state = 2;
		}
		break;
	case 5: //считываем имя параметра
		if (lineVec.empty()) {
			break;
		}
		else if (lineVec[0].first == "id") {
			int check = table.addVar(lineVec[0].second, scope, current_type);
			if (check == -1) {
				state = -1; //вывести в файл ошибку множественной инициализации --------------------------------------------------------
				break;
			}
			state = 4;
		}
	case 6: //AssignOrCallOp узел, проверяем есть ли id в таблице символов
		if (lineVec.empty()) {
			break;
		}
		else if (lineVec[0].first == "id") { // имя переменной, которой присваеваем значение
			if (!table.checkVar(lineVec[0].second, scope)) { //переменной нет в таблице символов
				state = 7; 
				break;
			}
			stack_val.push_back(lineVec[0]);
			ExpPlace = "AssignOrCallOp";
			state = 8;
		}
		break;
	case 7://проверяем, идет ли после id ArgList
		if (name.first != "ArgList") {//id нет в таблице символов и это не функция
			state = -1; //вывести ошибку неизвестная переменная ---------------------------------------------------------------------------
			break;
		}
		state = 10;
		break;
	case 8: //Разбираем выражение E
		if (name.first == "num") {
			stack_val.push_back(name);
			state = 9;
		}
		else if (name.first == "id") {
			if (!table.checkVar(lineVec[0].second, scope)) { //переменной нет в таблице символов
				state = -1;//вывести ошибку неизвестная переменная ---------------------------------------------------------------------------
				break;
			}
			stack_val.push_back(name);
			state = 9;
		}
		else if (lineVec.empty()) {
			break;
		}
		else if (lineVec[0].first == "opne") {
			stack_ops.push_back("opne");
			state = 9;
		}
		else if (lineVec[0].first == "opinc") {
			stack_ops.push_back("opinc");
			state = 9;
		}

	case 9:
		break;
	case 10:
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
std::string SymbTable::getTypeByName(std::string name, int scope) { return ""; }
int SymbTable::getScopeByID(std::string) { return 0; }
std::string SymbTable::getInitByName(std::string name, int scope) { 
	for (auto elem : table) {
		if (elem.name == name && elem.scope == scope) {
			return elem.init;
		}
	}
	return "";
}
int SymbTable::addVar(std::string name, int scope, std::string type, std::string init) { 
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
bool SymbTable::checkVar(std::string name, int scope){
	if (getIDByName(name, scope) != -1 || getIDByName(name, -1) != -1) {
		return true;
	}
	return false; 
}
bool SymbTable::checkFunc(std::string name, int len){
	if (getIDByName(name, -1) == -1) {
		return false;
	}
	if (std::stoi(getInitByName(name, -1)) != len) {
		return false;
	}
	return true;
}
void SymbTable::setInit(std::string, int len){}