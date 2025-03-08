#include <iostream>
#include <fstream>
#include <string>
#include <vector>

struct variable { std::string name; std::string type; };
struct record { std::string name; std::vector<variable> vars; };
struct function { std::string name; std::string type; std::vector<variable> params; bool is_public = false; };
struct unit { std::string name; std::vector<function> procedures; std::vector<std::string> uses; };
struct insertion { size_t loc; std::string str; };

void sanitise(std::string& in) {
	// i wonder which one is faster
	while (true) {
		const char* t_loc_p = strchr(in.c_str(), '\t');
		if (t_loc_p != NULL) {
			size_t t_loc = t_loc_p - in.c_str();
			in.erase(t_loc, 1);
		} else break;
	} return;
	//for (size_t i = 0; i < in.length(); i++) {
	//	if (in[i] == '\t') {
	//		in.erase(i, 1); i--;
	//	}
	//}
}

std::string sanitise2(std::string in) {
	// i wonder which one is faster
	while (true) {
		const char* t_loc_p = strchr(in.c_str(), '\n');
		if (t_loc_p != NULL) {
			size_t t_loc = t_loc_p - in.c_str();
			in.erase(t_loc, 1);
		}
		else break;
	} return in;
	//for (size_t i = 0; i < in.length(); i++) {
	//	if (in[i] == '\n') {
	//		in.erase(i, 1); i--;
	//	}
	//} return in;
}

size_t scan_c(std::string str, char start_token, char end_token, size_t* st_loc_ = NULL) {
	size_t ret = 0; // return abs position of matched end_token
	const char* st_loc_p = strchr(str.c_str(), start_token);
	if (st_loc_p == NULL) return 0;
	size_t st_loc = st_loc_p - str.c_str(); 
	if (st_loc_ != NULL) *st_loc_ = st_loc;
	size_t ct = 1;
	for (size_t i = st_loc+1; i < str.size(); i++) {
		if (str[i] == start_token) ct++;
		if (str[i] == end_token) ct--;
		if (ct == 0) { ret = i; break; }
	} return ret;
}

std::string peekFront(std::string& str, char delim, bool w_delim = false, size_t* seeked = NULL) {
	size_t loc = 0; loc = str.find(delim);
	if (loc != std::string::npos) {
		if (seeked != NULL) *seeked = loc;
		if (w_delim) return str.substr(0, loc + 1); else return str.substr(0, loc);
	} else return "";
}

std::string getFront(std::string& str, char delim, bool w_delim = false) {
	size_t loc = 0; loc = str.find(delim);
	if (loc != std::string::npos) {
		std::string front = str.substr(0, loc + 1);
		str.erase(0, front.length());
		if (!w_delim) front.erase(front.size()-1);
		return front;
	} else return "";
}

std::string peekEnd(std::string str, char delim) {
	size_t loc = 0; loc = str.find(delim);
	if (delim != std::string::npos) return str.substr(loc+1);
	else return "";
}

std::string getline_igws(std::fstream& fp, std::string& out, char delim) {
	std::string ret = ""; 
	while (true) {
		std::getline(fp, ret, delim);
		if (ret.length() == 0) { if (fp.eof()) return ""; else continue; }
		else out = ret; return ret;
	}
}

std::string tokenize(std::string in) {
	std::string ret = in;
	for (size_t i = 0; i < ret.size(); i++) {
		if (ret[i] == ' ' || ret[i] == ':' || ret[i] == '=' || ret[i] == '\n')
			{ ret.erase(i, 1); i--; }
	} in = ret; return ret;
}

int main() {
	std::cout << "pascal_parser.\n";
	std::fstream fp("source.pp");
	if (fp.bad()) { fp.close(); std::cout << "unable to open file.\n"; return 0; }

	std::string buf = ""; size_t buf_lnc = 0; size_t ln_read = 0;
	while (!fp.eof()) {
		std::string buff = "";
		std::getline(fp, buff); buf_lnc++;
		buf.append(buff); buf.push_back('\n'); ln_read++;
	} fp.close(); std::cout << "read " << ln_read << " lines.\n";

	std::string out_buf = buf; // save clean buf for later
	sanitise(buf);

	std::cout << "positions are now inaccurate to file (tab characters removed from parsing buffer).\n";

	unit the_unit; int cur_part = 0; size_t file_len = buf.length();
	while (true) {
		std::string word = peekFront(buf, ' ');
		if (word.length() == 0) {
			if (buf.empty()) { std::cout << "end of " << the_unit.name << "(source.pp).\n\n"; break; }
			else buf.erase(0, 1); continue; 
		}
		const char* nl_loc = strchr(word.c_str(), '\n');
		if (nl_loc != NULL) {
			size_t off = nl_loc - word.c_str();
			if (off == 0) { buf.erase(0, 1); continue; }
			else buf.erase(0, nl_loc - word.c_str() + 1);
			word.erase(off);
		} else buf.erase(0, word.size()+1);

		size_t where = file_len - buf.length() - word.length(); std::cout << "current loc: " << where << '\n';

		if (word == "interface") { std::cout << "begin interface block.\n\n"; cur_part = 1; continue; }
		if (word == "implementation") { std::cout << "begin implementation block.\n\n"; cur_part = 2; continue; }

		if (cur_part == 1) {
			if (word == "type") {
				variable var;
				std::string type_name = getFront(buf, ' ');

				size_t h = 0;
				std::string type_type = tokenize(peekFront(buf, '\n', false, &h));
				if (type_type == "record") {
					buf.erase(0, h);
					record rec; rec.name = type_name;
					while (true) {
						std::string member_str = getFront(buf, ';');
						if (sanitise2(member_str) == "end") break;
						variable var;
						var.name = tokenize(getFront(member_str, ':'));
						var.type = tokenize(peekEnd(member_str, ':'));
						rec.vars.push_back(var);
					}
					std::cout << "record, called " << type_name << '\n';
					for (size_t i = 0; i < rec.vars.size(); i++)
						std::cout << "     " << rec.vars[i].name << " : " << rec.vars[i].type << '\n';
				} else {
					type_type.erase(type_type.size());
					std::cout << "type, called " << type_name << ", of type " << type_type << '\n';
				}

				std::cout << '\n';

				var.name = type_name; var.type = type_type;
			} // end type

			if (word == "function" || word == "procedure") {
				function func; func.is_public = true;
				std::string fn_name = tokenize(peekFront(buf, '(')); buf.erase(0, fn_name.size());
				std::string param_list = getFront(buf, ')', true); // or catch to ':' ?
				if (param_list != "()") {
					param_list = param_list.substr(1, param_list.size() - 2);
					while (true) {
						if (param_list.length() == 0) break;
						std::string param_str = getFront(param_list, ';');
						if (param_str == "") { param_str = param_list; param_list.clear(); }// single parameter
						std::string param_name = tokenize(peekFront(param_str, ':'));
						std::string param_type = tokenize(peekEnd(param_str, ':'));
						variable param; param.name = param_name; param.type = param_type;
						func.params.push_back(param);
						// param_list.erase(0, param_str.size()+1);
					} 
				}
				std::string fn_type = "";
				if (word == "procedure") fn_type = "void";
				else fn_type = tokenize(getFront(buf, ';'));

				std::cout << word << ' ' << fn_name << ", of type " << fn_type << '\n';
				for (size_t i = 0; i < func.params.size(); i++)
					std::cout << "     parameter " << i << ": " << func.params[i].name << ", of type " << func.params[i].type << '\n';
				std::cout << '\n';
				func.name = fn_name;
				func.type = fn_type;

				the_unit.procedures.push_back(func);
			} // end function

			if (word == "var" || word == "const") {
				variable var; std::string var_type = "";
				std::string var_name = getFront(buf, ':');
				std::string var_str = peekFront(buf, ';');
				const char* eq_loc = strchr(var_str.c_str(), '=');
				if (eq_loc != NULL) { // variable w/ record initialise 
					var_type = buf.substr(0, eq_loc+1 - var_str.c_str());
					buf.erase(0, var_type.length());
					var_type = tokenize(var_type);

					size_t var_str_stl = 0;
					size_t var_str_len = scan_c(buf, '(', ')', &var_str_stl);
					if (var_str_len == 0) { std::cout << "could not match open pt to close pt.\n"; }
					// to bail: erase current expression to '\n' or ';' ?
					if (var_str_stl > 0) buf.erase(0, var_str_stl);
					var_str = buf.substr(0, var_str_len); buf.erase(0, var_str_len+1);
					// above string should be the record init values, not used
				} else {
					buf.erase(0, var_str.length());
					var_type = peekEnd(var_str, ':');
				}
				std::cout << "variable " << var_name << ", of type ";
				if (word == "const") std::cout << "const ";
				std::cout << var_type << "\n\n";
				var.name = var_name;
				var.type = var_type;
			} // end var

			continue;
		}

		if (word == "unit") {
			the_unit.name =  getFront(buf, ';');
			std::cout << "file source.pp, is the unit " << the_unit.name << "\n\n";
			continue;
		}

		if (word == "end.") { std::cout << "end of " << the_unit.name << "(source.pp).\n\n"; break; }
	}

	// gen fn comment 
	for (size_t i = 0; i < the_unit.procedures.size(); i++) {
		std::cout << '\n';
		std::string fn_top_comment = "";
		fn_top_comment.append("(*\n");
		fn_top_comment.append(" * @name " + the_unit.procedures[i].name + '\n');
		fn_top_comment.append(" * @description comment1 comment2\n");
		for (size_t j = 0; j < the_unit.procedures[i].params.size(); j++)
			fn_top_comment.append(" * @param " + the_unit.procedures[i].params[j].type + ' ' + the_unit.procedures[i].params[j].name + '\n');
		fn_top_comment.append(" *)\n");

		std::cout << fn_top_comment << '\n';
	}

	// to insert code comment above function: must keep track of \n consumed ? and \t consumed
	// insert from rear onwards to preserve absolute insertion positions

	// fp.close();

	return 0;
}
