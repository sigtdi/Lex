#include "pch.h"
#include "LR.h"
#include "SAtree.h"
#include "grammar.h"
#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <set>
#include <algorithm>


void Tree::printNode(Node& node, std::vector<std::pair<std::string, int>> is_need_print, std::ofstream& fout) {
	for (int i = 0; i < is_need_print.size(); ++i) {
		auto p = is_need_print[i];
		fout << p.first;
		if (p.first == " ") {
			for (int _ = 0; _ < p.second; ++_) {
				fout << " "; //пробелы перед первой вертикальной линией
			}
		}
		if (p.first == "\xE2\x94\x94") {
			for (int _ = 0; _ < p.second; ++_) {
				fout << "\xe2\x80\x94"; //горизонтальные линии после вертикальной уголком 
			}
			is_need_print.pop_back();
			is_need_print[i - 1].second += p.second + 1;
		}
		else if (p.first == "\xE2\x94\x9C") {
			for (int _ = 0; _ < p.second; ++_) {
				fout << "\xe2\x80\x94"; //горизонтальные линии после вертикальной в виде "T" 
			}
			is_need_print.pop_back();
			is_need_print.push_back({ "\xE2\x94\x82", p.second });
		}
		else if (p.first == "\xE2\x94\x82") {
			for (int _ = 0; _ < p.second; ++_) {
				fout << " "; //пробелы после вертикальной 
			}
		}
	}

	fout << node.elem1;
	fout << node.name;
	fout << std::endl;
	node.lenght = node.elem1.size() + node.name.size();
	is_need_print.push_back({ "\xE2\x94\x9C" , node.lenght });

	for (int i = 0; i < node.sons.size(); ++i) {
		if (i == node.sons.size() - 1) {
			is_need_print.pop_back();
			is_need_print.push_back({ "\xE2\x94\x94" , node.lenght });
			printNode(node.sons[i], is_need_print, fout);
		}
		else {
			printNode(node.sons[i], is_need_print, fout);
		}
	}
}

void Tree::printTree() {
	std::ofstream fout;
	fout.open("tree.txt");
	Node root = Nodes[0];
	fout << "Start" << std::endl;
	root.lenght = root.elem1.size() + root.name.size();
	std::vector<std::pair<std::string, int>> is_need_print = { {" ", 0}, {"\xE2\x94\x94", root.lenght} };

	printNode(root.sons[0], is_need_print, fout);
}

void printError() {
	std::ofstream fout;
	fout.open("tree.txt");
	fout << "Error" << std::endl;
}

Tree::~Tree() {
	
};

void Tree::addNodes(Rule rule) {
	queue.push_back(rule);
}

void Tree::MakeTree() {
	for (auto rule : queue) {
		auto n = MakeNodesSon(rule);
		Node node; 
		std::reverse(n.begin(), n.end());
		node.name = rule.getLeft();
		node.sons = n;
		Nodes.push_back(node);
	}
}

std::vector<Node> Tree::MakeNodesSon(Rule rule) {
	std::vector<Node> nodess = {};
	bool all_right_is_term = true;
	std::string elem = "";
	std::vector<std::string> terms = {};
	for (auto term : rule.getRight()) {
		all_right_is_term *= isTerminal(term);
	}
	if (all_right_is_term) {
		for (auto term : rule.getRight()) {
			Node node{ term };
			nodess.push_back(node);
		}
		std::reverse(nodess.begin(), nodess.end());
	}
	else {
		auto tempRight = rule.getRight();
		std::vector<Node> TempNodes = {};
		bool flag = true;
		for (int i = tempRight.size() - 1; i > -1 ; --i) {
			auto term = tempRight[i];
			if (isTerminal(term)) {
				elem = term + " " + elem;
				if (flag) {
					Node node;
					node.name = elem;
					elem = "";
					nodess.push_back(node);
				}
				else if (i == 0) {
					nodess[nodess.size() - 1].elem1 = elem;
					elem = "";
				}
			}
			else {
				flag = false;
				if (!elem.empty()) {
					nodess[nodess.size() - 1].elem1 = elem;
					elem = "";
				}
				nodess.push_back(Nodes[Nodes.size() - 1]);
				Nodes.pop_back();
			}
		}
	}
	return nodess;
}