#include <fstream>
#include <map>
#include <string>
#include <vector>
#include <stack>

using namespace std;

class expression {
protected:
	int answer;
public:
	expression() : answer(-1) {}

	virtual int get_answer() {
		return answer;
	}
};

class simple_expression : public expression {
public:
	simple_expression(int _answer) {
		answer = _answer;
	}
};

enum class token_type { EXP, OR, AND };

struct token {
	token_type op;
	expression* exp;
};

class compound_expression : public expression {
	string str_expression;
	vector<token> rpn; // reversed polish notation
	vector<compound_expression*> dependencies; // expressions that depend on the current expression
public:
	compound_expression(string str_expression) : str_expression(str_expression) {}

	void parse(const map<string, expression*> &all_expressions) {
		string current_token = "";
		stack<token> operators;
		str_expression += ' ';
		for (char c : str_expression) {
			if (c == ' ') {
				if (current_token == "|") {
					if (!operators.empty() && operators.top().op == token_type::AND) {
						rpn.push_back(operators.top());
						operators.pop();
					}
					operators.push({ token_type::OR, nullptr });
				}
				else if (current_token == "&") {
					operators.push({ token_type::AND, nullptr });
				}
				else {
					if (all_expressions.find(current_token) == all_expressions.end())
						rpn.push_back({ token_type::EXP, nullptr });
					else {
						expression* exp = all_expressions.find(current_token)->second;
						compound_expression* c_exp = dynamic_cast<compound_expression*>(exp);
						if (c_exp) {
							c_exp->dependencies.push_back(this);
						}
						rpn.push_back({ token_type::EXP, all_expressions.find(current_token)->second });
					}
				}
				current_token = "";
			}
			else {
				current_token += c;
			}
		}
		str_expression = "";
		while (!operators.empty()) {
			rpn.push_back(operators.top());
			operators.pop();
		}
	}

	int calculate() {
		stack<int> values;
		for (token& current_token : rpn){
			if (current_token.op == token_type::AND) {
				int left = values.top();
				values.pop();
				int right = values.top();
				values.pop();
				if (!left || !right) values.push(0);
				else if (left == -1 || right == -1) values.push(-1);
				else values.push(1);
			}
			else if (current_token.op == token_type::OR) {
				int left = values.top();
				values.pop();
				int right = values.top();
				values.pop();
				if (left == 1 || right == 1) values.push(1);
				else if (left == -1 || right == -1) values.push(-1);
				else values.push(0);
			}
			else {
				if (current_token.exp == nullptr) {
					values.push(-1);
				}
				else {
					values.push(current_token.exp->get_answer());
				}
			}
		}
		answer = values.top();
		if (answer != -1) {
			for (compound_expression*& exp : dependencies) {
				if (exp->get_answer() == -1) {
					exp->calculate();
				}
			}
		}
		return answer;
	}
};

int main(int argc, char** args)
{
	ifstream in = ifstream(args[1]);
	ofstream out = ofstream(args[2]);

	int N, M;
	map<string, expression*> all_expressions;
	vector<compound_expression*> compound_expressions;

	in >> N >> M;
	for (int i = 0; i < N; i++) {
		string name;
		int answer;
		in >> name >> answer;
		all_expressions[name] = new simple_expression(answer);
	}

	string inp;
	getline(in, inp);

	for (int i = 0; i < M; i++) {
		getline(in, inp);	
		string name = inp.substr(0, inp.find(' '));
		string str_expression = inp.substr(inp.find('=') + 2, inp.length());
		compound_expression* exp = new compound_expression(str_expression);
		compound_expressions.push_back(exp);
		all_expressions[name] = exp;
	}

	for (auto exp : compound_expressions) {
		exp->parse(all_expressions);
	}

	for (auto exp : compound_expressions) {
		if (exp->get_answer() == -1)
			exp->calculate();
	}

	for (auto exp : compound_expressions) {
		int answer = exp->get_answer();
		if (answer == -1) out << "?\n";
		else out << answer << "\n";
	}
	return 0;
}

