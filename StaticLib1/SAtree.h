#pragma once

struct Node
{
	std::string name = "";
	std::vector<Node> sons = {};
	std::string elem1 = "";
	
	int lenght = 0;
};

class Tree 
{
public:
	virtual ~Tree();
	void addNodes(Rule);
	void MakeTree();
	void printTree();

protected:

private:
	void printNode(Node& node, std::vector<std::pair<std::string, int>> is_need_print, std::ofstream& fout);
	std::vector<Node> MakeNodesSon(Rule);
	std::vector<Rule> queue;
	std::vector<Node> Nodes;
};

void printError();