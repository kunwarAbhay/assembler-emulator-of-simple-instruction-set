/*****************************************************************************																	
AUTHOR: Kunwar Abhay Rai 
*****************************************************************************/

#include <bits/stdc++.h>

using namespace std;

struct symbol{
    string name;
    int address;
    bool set;
    int set_value;
};

vector<symbol> symbol_table;

// remove all leading or trailing white spaces from the string
string trim(string str) {
    str.erase(find_if(str.rbegin(), str.rend(), not1(ptr_fun<int, int>(isspace))).base(), str.end());
    str.erase(str.begin(), find_if(str.begin(), str.end(), not1(ptr_fun<int, int>(isspace))));

    return str;
}

// check whether given string is a number
bool is_number(string s){
    if(s.empty()) return false;

    if (s[0] == '-' || s[0] == '+') return is_number(s.substr(1));
    if(s.find("0x") == 0) return is_number(s.substr(2));

    for(int c : s){
        if(!isdigit(c)){
            return false;
        }
    }

    return true;
}

// convert to a given string to its corresponding numeral
int to_num(string s) {
    s = trim(s);

    if(s.find("0x") == 0) return stoul(s, nullptr, 16);
    if(s.find("0") == 0) return stoul(s, nullptr, 8);
    if(s.find("-") == 0) return -stoul(s.substr(1, s.size()), nullptr, 10);
    if(s.find("+") == 0) return stoul(s.substr(1, s.size()), nullptr, 10);

    return stoul(s, nullptr, 10);
}

// check whether elements exist in symbol table
bool contains(string name){
   for(auto symbol : symbol_table){
      if(symbol.name == name){
        return true;
      }
   }

   return false;
}

// convert integer to hexadecimal format
string int_to_hex(int num){
  stringstream stream;
  stream << setfill ('0') << setw(8) << hex << num;
  return stream.str();
}

// check whether given label is valid
int is_valid_label(string label) {
    if(isdigit(label[0])) return false;

    for(char l : label) {
        l = tolower(l);
        
        if(!(isdigit(l) || (l >= 'a' && l <= 'z') || l == '_')) {
            return false;
        }
    }

    return true;
}

// MOT table
unordered_map<string, string> mot;
void mot_init(){

	mot["ldc"] = string("00");
	mot["adc"] = string("01");
	mot["ldl"] = string("02");
	mot["stl"] = string("03");
	mot["ldnl"] = string("04");
	mot["stnl"] = string("05");
	mot["add"] = string("06");
	mot["sub"] = string("07");
	mot["shl"] = string("08");
	mot["shr"] = string("09");
	mot["adj"] = string("0a");
	mot["a2sp"] = string("0b");
	mot["sp2a"] = string("0c");
	mot["call"] = string("0d");
	mot["return"] = string("0e");
	mot["brz"] = string("0f");
	mot["brlz"] = string("10");
	mot["br"] = string("11");
	mot["HALT"] = string("12");
	mot["data"] = string("13");
	mot["SET"] = string("14");
}

// Reading instructions and derive literal and symbol table
string inst_to_table(string instr, int* pc, int line){
    int loc = *pc;
    string errors = "";

    // Identify label and variables
    if(instr.find(':') != string::npos){
        int colon = instr.find(":", 0);
        string label = instr.substr(0, colon);

        if(contains(label)){
            string error = "ERROR: Duplicate Label at line " + to_string(line) + "\n";
        	cout << error;
        	errors += error;
        }

        if(!is_valid_label(label)){
            string error = "WARNING: Incorrect label format at line " + to_string(line) + "\n";
        	cout << error;
        	errors += error;
        }

        // fetch instruction after colon
        if(colon != instr.size() - 1){
        	string subs = instr.substr(colon + 1, instr.size());
        	subs = trim(subs);
        	inst_to_table(subs, &loc, line);
        	int space = subs.find(" ", 0);
        	string sub_op = subs.substr(0, space);
        	string sub_val = subs.substr(space + 1, subs.size());
        	sub_op = trim(sub_op);
        	sub_val = trim(sub_val);

        	// Dealing with set instructions
        	if(sub_op == "SET"){
        		symbol_table.push_back({label, loc, 1, stoi(sub_val)});
        	}
        	else{
        		symbol_table.push_back({label, loc, 0, -1});
        	}
        }
        else{
        	*pc = *pc - 1;
        	symbol_table.push_back({label, loc, 0, -1});
        }
    }
    
    return errors;
}

void analyse(string file, ofstream& log_file){
    string line;

    int loc = 0;
    
    int line_count = 1;

    // Read the input file
    ifstream asm_file(file);

    while (getline (asm_file, line)) {
        // fetch instruction from each line
        string instr;
        instr = line.substr(0, line.find(";"));
        instr = trim(instr);

        // skip this line
        if(instr == ""){
            line_count++;
            continue;
        }

        log_file << inst_to_table(instr, &loc, line_count++);
        loc++;
    }

    asm_file.close();
}

tuple<string, string, string> inst_to_code(string instr, int* pc, int line){

	int loc = *pc;

	string encoding = int_to_hex(loc) + " ";

    string encoding_ = "";
	string errors = "";
	string machine_code = "";

    // Identify label and variables
    if(instr.find(':') != string::npos){
        int colon = instr.find(":", 0);

        // fetch instruction after the colon
        if(colon != instr.size() - 1){
        	string subs = instr.substr(colon + 1, instr.size());
        	subs = trim(subs);
        	tie(encoding_, errors, machine_code) = inst_to_code(subs, &loc, line);
        	string temp = encoding_;
        	temp = temp.substr(9, 9);
        	encoding += temp;
        }
        // If no instruction after colon, PC shouldn't change
        else{
        	encoding += "         ";
        	*pc = *pc - 1;
        }

        encoding += instr + "\n";
    }
    // fetch mnemonics & operands
    else{
        int space = instr.find(" ", 0);

        string hex_string;

        string operation = instr.substr(0, space);
        string operand = instr.substr(space + 1, instr.size());

        operand = trim(operand);
        operation = trim(operation);

        // Check for invalid mnemonics
        if(mot[operation] == ""){
            string error = "ERROR: Mnemonic not defined at line " + to_string(line) + "\n";
        	cout << error;
        	errors += error;
        }

        // Check for No operand instructions
        else if(operation == "add" || operation == "sub"
        		|| operation == "shl"|| operation == "shr"
        		|| operation == "a2sp"|| operation == "sp2a"
        		|| operation == "return"|| operation == "HALT"){

        	encoding += "000000" + mot[operation] + " "; 
        	machine_code += "000000" + mot[operation] + " "; 
        	
            if(operation.size() != instr.size()){
                string error = "ERROR: Operand not expected at line " + to_string(line) + "\n";
        	    cout << error;
        	    errors += error;
			}
        }
        // Check whether operand is number
        else if(is_number(operand)){
        	hex_string = int_to_hex(to_num(operand));
        	encoding += hex_string.substr(hex_string.size() - 6) + mot[operation] + " "; 
        	machine_code += hex_string.substr(hex_string.size() - 6) + mot[operation] + " "; 
        }
        // Checking for variable operand to encode address
        else{
        	int defined = 0;
        	for(int i=0; i < symbol_table.size(); i++){
        		if(symbol_table[i].name.compare(operand) == 0){
        			defined = 1;
        			// SET variables considered numeral
        			if(symbol_table[i].set){
        				hex_string = int_to_hex(symbol_table[i].set_value);
        			}
        			// Operands which need offset from PC
        			else if(operation == "call" || operation == "brz"
        				|| operation == "brlz"|| operation == "br"){
        				hex_string = int_to_hex(symbol_table[i].address - loc - 1);
        			}
        			// Normal case, encode address
        			else{
        				hex_string = int_to_hex(symbol_table[i].address);
        			}
                    encoding += hex_string.substr(hex_string.size() - 6) + mot[operation] + " "; 
        			machine_code += hex_string.substr(hex_string.size() - 6) + mot[operation] + " "; 
        			break;
        		}
        	}
        	if(operation.size() == instr.size()){
        		string error = "ERROR: Operand expected at line " + to_string(line) + "\n";
        		cout << error;
                errors += error;   
        	}
        	else if(!defined){
        		string error = "ERROR: Unknown Symbol as operand at line " + to_string(line) + "\n";
                cout << error;
                errors += error;
        	}
        }

        encoding += instr + "\n";
    }

    return make_tuple(encoding, errors, machine_code);
}

void synthesize(string file, ofstream& out_file, ofstream& log_file, ofstream& obj_file){
    string line;

    // Program Counter
    int loc = 0;

    // Line counter
    int line_count = 1;

    // Reading the input file
    ifstream asm_file(file);

    // Read individual lines
    while (getline (asm_file, line)) {
        // Pre-process the line, trim extra spaces
        string instr;
        instr = line.substr(0, line.find(";", 0));
        instr = trim(instr);

        // Skip empty lines
        if(instr == ""){
            line_count++;
            continue;
        }

        // Write Encoded instruction into listing file
        string encoding, errors, machine_code;
        tie(encoding, errors, machine_code) = inst_to_code(instr, &loc, line_count++);
        
        out_file << encoding;

        if(machine_code != ""){
        	uint32_t temp = stoul("0x" + trim(machine_code), nullptr, 16);
        	obj_file.write((char *)&temp, sizeof(temp));
        }

        log_file << errors;

        loc++;
    }

    asm_file.close();
}

// driver code
int main(int argc, char* argv[]) {
	mot_init(); // Initalize Machine operation table

	string in_file = argv[1]; // input file

    string filename = in_file.substr(0, in_file.find("."));

    ofstream out_file(filename + ".L");
    ofstream log_file(filename + ".log");
    ofstream obj_file(filename + ".o");

    // First Pass
    analyse(in_file, log_file);

    // Second Pass
    synthesize(in_file, out_file, log_file, obj_file);

    // Close files
    out_file.close();
    log_file.close();
    obj_file.close();
}