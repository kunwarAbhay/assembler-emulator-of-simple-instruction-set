/*****************************************************************************																	AUTHOR: Kunwar Abhay Rai. 
*****************************************************************************/

#include <bits/stdc++.h>
using namespace std;

int32_t A, B, PC, SP; // program registers
int32_t memory[10000]; // memory array

// MOT table
unordered_map<int, string> mot;
void mot_init(){
	mot[0] = string("ldc");
	mot[1] = string("adc");
	mot[2] = string("ldl");
	mot[3] = string("stl");
	mot[4] = string("ldnl");
	mot[5] = string("stnl");
	mot[6] = string("add");
	mot[7] = string("sub");
	mot[8] = string("shl");
	mot[9] = string("shr");
	mot[10] = string("adj");
	mot[11] = string("a2sp");
	mot[12] = string("sp2a");
	mot[13] = string("call");
	mot[14] = string("return");
	mot[15] = string("brz");
	mot[16] = string("brlz");
	mot[17] = string("br");
	mot[18] = string("HALT");
	mot[19] = string("data");
	mot[20] = string("SET");
}

// show usage instructions
void usage_intruction() {
    cout << "Usage: ./emu.exe [option] file.o"             << endl;
    cout << "Options:"                                     << endl;
    cout << "  -trace  show instruction trace"             << endl;
    cout << "  -before  show memory dump before execution" << endl;
    cout << "  -after  show memory dump after execution"   << endl;
}


// convert integer to hexadecimal format
string int_to_hex(int num){
  stringstream stream;
  stream << setfill ('0') << setw(8) << hex << num;
  return stream.str();
}

// parse operand & opcode from machine code
tuple<int32_t, int32_t> inst_to_operand_opcode(int32_t instr){
	int32_t opcode = instr & 0xff; // masking first 6 bits
	int32_t operand = instr & 0xffffff00; // masking last 2 bits
    operand >>= 8; // Arithematic shift by 2
	
    return(make_tuple(operand, opcode));
}

// show memory dump 
void mem_dump(int poc, ofstream& trace_file){
	cout << "Dumping from memory";
	trace_file << "Dumping from memory";

	for (int k = 0; k < poc; k++){
		if(!(k % 4)){
			cout << "\n\n" << int_to_hex(k) << "\t" << int_to_hex(memory[k]) << " ";
			trace_file << "\n\n" << int_to_hex(k) << "\t" << int_to_hex(memory[k]) << " ";
		}
		else{
			cout << int_to_hex(memory[k]) << " ";
			trace_file << int_to_hex(memory[k]) << " ";
        }
	}
	cout << endl;
}

void execute_instruction(int32_t operand, int32_t operation, int count){
    switch(operation) {
        case 0:
            B = A;
            A = operand;
            break;
        case 1:
            A += operand;
            break;
        case 2:
            B = A;
            A = memory[SP+operand];
            break;
        case 3:
            memory[SP+operand] = A;
            A = B;
            break;
        case 4:
            A = memory[A+operand];
            break;
        case 5:
            memory[A+operand] = B;
            break;
        case 6:
            A += B;
            break;
        case 7:
            A = B - A;
            break;
        case 8:
            A = B << A;
            break;
        case 9:
            A = B >> A;
            break;
        case 10:
            SP += operand;
            break;
        case 11:
            SP = A;
            A = B;
            break;
        case 12:
            B = A;
            A = SP;
            break;
        case 13:
            B = A;
            A = PC;
            PC += operand;
            break;
        case 14:
            PC = A;
            A = B;
            break;
        case 15:
            if(A == 0) 
            	PC += operand;
            break;
        case 16:
            if(A < 0) 
            	PC += operand;
            break;
        case 17:
            PC += operand;
            break;
        case 18:
        	cout << count << " instructions executed" << endl << endl;
    }
}

// Function to trace individual instructions
int trace(int PC, int poc, ofstream& trace_file){
	// Number of executed instructions
	int count = 0;

	// Loop until HALT
	while(true){
		// tracker for infinite loop
		int old_pc = PC;

		int32_t operand, operation;

		int32_t instr = memory[PC];

		tie(operand, operation) = inst_to_operand_opcode(instr);

		// Print statement for tracing
		cout << "PC: " << int_to_hex(PC) << "\tSP: " << int_to_hex(SP) << "\tA: " 
		<< int_to_hex(A) << "\tB: " << int_to_hex(B) << "\t" << mot[operation] 
		<< " " <<operand << "\n\n";

		trace_file << "PC: " << int_to_hex(PC) << "\tSP: " << int_to_hex(SP) << "\tA: " 
		<< int_to_hex(A) << "\tB: " << int_to_hex(B) << "\t" << mot[operation] 
		<< " " <<operand << "\n\n";

        execute_instruction(operand, operation, count);		

        PC++;

        // Check for infinite loop anamoly
		if(old_pc == PC){
			cout << "Infinite loop detected" << endl;
			break;
		}

		count++;
	}

	cout << count << " instructions executed" << endl;
}

int main(int argc, char* argv[]){
    // Invalid number of commands
	if(argc != 3){
		// show how to use this emulator
		usage_intruction();
		return 0;
	}
    
    string option = argv[1];
	string file = argv[2];

	// Initialize the machine operand table
	mot_init();

	ifstream inp_file(file);
	ofstream trace_file(file.substr(0, file.find(".")) + ".trace");

	int32_t instr_code;

	// count number of instructions
	int poc = 0;

	// load machine code into main memory
	while (inp_file.read((char*) &instr_code, sizeof(instr_code))){
		int32_t operand, operation;
		tie(operand, operation) = inst_to_operand_opcode(instr_code);
		
		if(operation == 19 || operation == 20){
		    // store operand in memory in case of DATA || SET
			memory[poc++] = operand;
		}
		else if(operation >= 0 && operation < 20){
		    // store instruction in memory
			memory[poc++] = instr_code;
		}
		else{
		    // Identify invalid instruction
			cout << "Invalid instruction\n";
			return 0;
		}
	}

	A = 0;
	B = 0;
	PC = 0;
	SP = sizeof(memory)/sizeof(memory[0])-1;

	// Memory dump before execution
	if(option == "-before"){
		mem_dump(poc, trace_file);
    }

	// Tracing for each executed instruction
	if(option == "-trace"){
		trace(0, poc, trace_file);
    }

	// Memory dump after execution
	if(option == "-after"){
		trace(0, poc, trace_file);
		mem_dump(poc, trace_file);
	}

	// Close file
	inp_file.close();

    return 0;
}