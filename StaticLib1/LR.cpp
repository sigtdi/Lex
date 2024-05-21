#include "pch.h"
#include "LR.h"
#include <string>
#include <vector>
#include <sstream>
#include <map>
#include "lexer.h"
#include "grammar.h"
#include "Atoms.h"
#include "SAtree.h"
#include <fstream>
#include <set>

LR::LR()
{
	ReadTables();
}
LR::~LR()
{
	//dtor
}
void LR::MakeTables() {
	C_Items = {};
	action_table = {};
	goto_graph = {};

	MakeItems();

	for (int i = 0; i < C_Items.size(); ++i) {
		auto I = C_Items[i];
		for (auto& item : I) {
			ACTION(item, i);
		}
	}

	DumpLRdata();
}

void LR::DumpLRdata()
{
	std::ofstream fout;
	fout.open("citems.txt");
	for (int i = 0; i < C_Items.size(); ++i) {
		auto items = C_Items[i];
		fout << i << std::endl;
		for (auto item : items) {
			fout << item.gramm_symbol << " " << item.position << " " << item.rule.getLeft();
			for (auto r : item.rule.getRight()) {
				fout << " " << r;
			}
			fout << std::endl;
		}
	}
	fout.close();

	fout.open("action.txt");
	for (auto pairAct : action_table) {
		fout << pairAct.first.first << " " << pairAct.first.second << " " << pairAct.second.act << " " << pairAct.second.shift << " " << pairAct.second.reduce.getLeft();
		for (auto r : pairAct.second.reduce.getRight()) {
			fout << " " << r;
		}
		fout << std::endl;
	}
	fout.close();

	fout.open("goto.txt");
	for (auto move : goto_graph) {
		fout << move.first.first << " " << move.first.second << " " << move.second << std::endl;
	}
	fout.close();
}

void LR::MakeItems() {
	Item start{ rules[0], 0, "end" };
	C_Items.push_back(closure({ {start} }));
	std::vector<int> queue = { 0 };
	while (!queue.empty()) {
		auto setofitem = C_Items[queue[0]];
		queue.erase(queue.begin());
		for (auto& gramm_elem : gramm_elems) {
			auto temp_SetOfItems = GOTO(setofitem, gramm_elem);
			std::vector<SetOfItems>::iterator itrTemp = std::find(C_Items.begin(), C_Items.end(), temp_SetOfItems);
			std::vector<SetOfItems>::iterator itrSet1 = std::find(C_Items.begin(), C_Items.end(), setofitem);
			if (!temp_SetOfItems.empty() && itrTemp == C_Items.end()) {
				goto_graph.insert({ {std::distance(C_Items.begin(), itrSet1), gramm_elem}, int(C_Items.size()) });
				C_Items.push_back(temp_SetOfItems);
				queue.push_back(int(C_Items.size()) - 1);
			}
			else if (!temp_SetOfItems.empty()) {
				goto_graph.insert({ {std::distance(C_Items.begin(), itrSet1), gramm_elem}, int(std::distance(C_Items.begin(), itrTemp)) });
			}
		}
	}
	for (auto move : goto_graph) {
		if (move.first.second == "epsilon") {
			C_Items[move.first.first].insert(C_Items[move.first.first].end(), C_Items[move.second].begin(), C_Items[move.second].end());
		}
	}
}

void LR::ReadTables() {
	std::ifstream finc("citems.txt");
	std::string line;
	SetOfItems tempSet = {};
	std::string delimiter = " ";
	if (finc.is_open()) {
		while (std::getline(finc, line)) {
			size_t pos = 0;
			std::string token;
			std::vector<std::string> lineVec = {};
			while ((pos = line.find(delimiter)) != std::string::npos) {
				token = line.substr(0, pos);
				lineVec.push_back(token);
				line.erase(0, pos + delimiter.length());
			}
			lineVec.push_back(line);
			if (lineVec.size() != 1 && lineVec.size() != 0) {
				Item item;
				item.gramm_symbol = lineVec[0];
				item.position = std::stoi(lineVec[1]);
				std::vector<std::string> tempRight = {};
				tempRight.insert(tempRight.end(), lineVec.begin() + 3, lineVec.end());
				Rule rule(lineVec[2], tempRight);
				item.rule = rule;
				tempSet.push_back(item);
			}
			else {
				if (!tempSet.empty()) {
					C_Items.push_back(tempSet);
				}
				tempSet = {};
			}
		}
		if (!tempSet.empty()) {
			C_Items.push_back(tempSet);
		}
		tempSet = {};
	}
	else {
		std::cout << "citems.txt is not found" << std::endl;
	}
	finc.close();

	std::ifstream fing("goto.txt");
	if (fing.is_open()) {
		while (std::getline(fing, line)) {
			size_t pos = 0;
			std::string token;
			std::vector<std::string> lineVec = {};
			while ((pos = line.find(delimiter)) != std::string::npos) {
				token = line.substr(0, pos);
				lineVec.push_back(token);
				line.erase(0, pos + delimiter.length());
			}
			lineVec.push_back(line);
			if (!lineVec.empty()) {
				goto_graph.insert({ {std::stoi(lineVec[0]), lineVec[1]}, std::stoi(lineVec[2]) });
			}
		}
	}
	else {
		std::cout << "goto.txt is not found" << std::endl;
	}
	fing.close();

	std::ifstream fina("action.txt");
	if (fina.is_open()) {
		while (std::getline(fina, line)) {
			size_t pos = 0;
			std::string token;
			std::vector<std::string> lineVec = {};
			while ((pos = line.find(delimiter)) != std::string::npos) {
				token = line.substr(0, pos);
				lineVec.push_back(token);
				line.erase(0, pos + delimiter.length());
			}
			lineVec.push_back(line);
			if (!lineVec.empty()) {
				Action action;
				action.act = lineVec[2];
				action.shift = std::stoi(lineVec[3]);
				if (action.act == "Shift" || action.act == "Error") {
					action.reduce = ZeroRule;
				}
				else if (action.act == "Reduce" || action.act == "Accept") {
					std::vector<std::string> tempRight = {};
					tempRight.insert(tempRight.end(), lineVec.begin() + 5, lineVec.end());
					Rule rule(lineVec[4], tempRight);
					action.reduce = rule;
				}
				action_table.insert({ {std::stoi(lineVec[0]), lineVec[1]}, action });
			}
		}
	}
	else {
		std::cout << "action.txt is not found" << std::endl;
	}
	fina.close();
}

SetOfItems LR::closure(SetOfItems setofitems) {
	for (int i = 0; i < setofitems.size(); i++) {
		auto item = setofitems[i];
		std::string term = getTermByItem(item.rule, item.position); //(не)терминал справа от пункта
		if (isTerminal(term)) {
			//пукт перед теминалом, значит по нему нельзя сделать замыкание
			continue;
		}

		auto listOfTerms = getTermsAfter(item);
		listOfTerms.push_back(item.gramm_symbol);

		//если нетерминал - ищем все правила, которые он задает 
		for (auto&& rule : rules) {
			if (rule.getLeft() != term) {
				// правило не подходит, так как начинается не с нужного нетерминала
				continue;
			}
			
			auto temp_firstTerms = first(listOfTerms);
			for (auto& term : temp_firstTerms) {
				Item new_item{ rule, 0, term }; //новый пункт с точкой в начале
				if (std::find(setofitems.begin(), setofitems.end(), new_item) == setofitems.end()) {
					setofitems.push_back(new_item);
				}
			}
		}
	}
	return setofitems;
}

SetOfItems LR::GOTO(SetOfItems& items, std::string gramm_elem) {
	SetOfItems new_set = {};
	for (auto& item : items) {
		if (getTermByItem(item.rule, item.position) == gramm_elem) {
			Item new_item{ item.rule, item.position + 1, item.gramm_symbol };
			new_set.push_back(new_item);
		}
	}
	return closure(new_set);
}

void LR::ACTION(Item item, int numOfSet) {
	auto termByItem = getTermByItem(item.rule, item.position);
	if (item.rule.getLeft() != "Start" && isEndOfRule(item.rule, item.position)) {
		Action result{ "Reduce", -1, item.rule };
		if (action_table.find({ numOfSet, item.gramm_symbol }) == action_table.end()) {
			action_table.insert({ {numOfSet, item.gramm_symbol}, result });
		}
		else if (action_table[{numOfSet, item.gramm_symbol}].act != result.act) {
			action_table.insert({ {numOfSet, item.gramm_symbol}, result });
		}
	}
	else if (goto_graph.find({ numOfSet, termByItem }) != goto_graph.end() && isTerminal(termByItem)) {
		Action result{ "Shift", goto_graph[{numOfSet, termByItem}], ZeroRule };
		if (action_table.find({ numOfSet, termByItem }) == action_table.end()) {
			action_table.insert({ {numOfSet, termByItem}, result });
		}
		else if (action_table[{numOfSet, termByItem}].act != result.act) {
			std::cout << "Error S" << std::endl;
		}
	}
	else if (item.rule.getLeft() == "Start" && isEndOfRule(item.rule, item.position)) {
		Action result{ "Accept", -1, item.rule };
		if (action_table.find({ numOfSet, "end" }) == action_table.end()) {
			action_table.insert({ {numOfSet, "end"}, result });
		}
		else if (action_table[{numOfSet, termByItem}].act != result.act) {
			std::cout << "Error" << std::endl;
		}
	}
}

bool LR::Analysis(std::istream& stream) {
	Tree tree;
	std::cout << "Input: " << std::endl;
	Lexer lex(stream);
	Lexem l = lex.getNextLexem();
	std::vector<int> states = { 0 };
	std::vector<std::string> elem_stack = {};
	std::vector<Lexem> lexList = {};
	if (l.second != "") {
		lexList.push_back(l);
	}

	while (true) {
		auto action = action_table[{states[states.size() - 1], l.first}];
		if (action.act == "Shift") {
			states.push_back(action.shift);
			elem_stack.push_back(l.first);
			l = lex.getNextLexem();
			if (l.second != "") {
				lexList.push_back(l);
			}
		}
		else if (action.act == "Reduce") {
			if (action.reduce.getRight()[0] != "epsilon") {
				for (int i = 0; i < action.reduce.getRightSize(); ++i) {
					states.pop_back();
					elem_stack.pop_back();
				}
			}
			elem_stack.push_back(action.reduce.getLeft());
			states.push_back(goto_graph[{states[states.size() - 1], action.reduce.getLeft()}]);
			tree.addNodes(addLexemToActionReduce(lexList, action));
		}
		else if (action.act == "Accept") {
			tree.addNodes(action.reduce);
			tree.MakeTree();
			tree.printTree();
			AtomGen atoms(tree);
			return true;
		}
		else {
			printError();
			return false;
		}
	}
}

std::set<std::string> LR::first(std::vector<std::string> terms) {
	std::set<std::string> new_set = { "epsilon" };
	int i = 0;
	
	while (new_set.contains("epsilon")) {
		new_set.erase("epsilon");
		auto term = terms[i];
		++i;

		if (isTerminal(term)) {
			new_set.insert(term);
			break;
		}

		for (auto& rule : rules) {
			if (rule.getLeft() != term) {
				continue;
			}
			auto tempFirst = first(rule.getRight());
			new_set.insert(tempFirst.begin(), tempFirst.end());
		}
	}
	return new_set;
}

std::vector<std::string> LR::getTermsAfter(Item item) {
	std::vector<std::string> res = {};
	int i = 1;
	while (!isEndOfRule(item.rule, item.position + i)) {
		res.push_back(getTermByItem(item.rule, item.position + i));
		++i;
	}
	return res;
}

Rule LR::addLexemToActionReduce(std::vector<Lexem>& lexList, Action& action) {
	std::vector<std::string> resRight = {};
	for (int i = 0; i < action.reduce.getRight().size(); ++i) {
		std::string temp = action.reduce.getRight()[i];
		if (action.reduce.getRight()[i] == "id" || action.reduce.getRight()[i] == "str" || action.reduce.getRight()[i] == "num" || action.reduce.getRight()[i] == "char") {
			temp += "(" + lexList[lexList.size() - 1].second + ")";
			lexList.pop_back();
		}
		resRight.push_back(temp);
	}
	Rule resRule(action.reduce.getLeft(), resRight);
	return resRule;
}