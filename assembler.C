/*	SOURCE CODE
 * 	written by: Shashikant Kadam, Vassant Shirodkar
 * 	This is the source code of "ONE PASS SIC/XE" assembler
 * 	This was made as project for subject "SYSTEM PROGRAMMING"
 * 	We assure you that we have written this code from scratch and did not do any kind of malpractice such as copying from friend/internet
 * 	We assure you that we tried our best to make our program bug free 
 * 	For compilation guide and input restrictions, see README.txt
 */
#include<iostream>						//for console input/output
#include<cstdlib>						//for atoi() function
#include<iomanip>						//for setw() and setfill()
#include<fstream>						//for file input/output
#include<climits>						//for INT_MIN
#include<sstream>						//for stringstream class (Standard Template Library)
#include<unordered_map>					//for unordered hashmap for symtab and optab (Standart Template Library)
#include<algorithm>						//for count() function
#define REGULAR_PC 0
#define INDEXED_PC 1
#define INDIRECT_PC 2
#define IMMEDIATE_PC 3
#define IMMEDIATE_CONST_3 4
#define REGULAR 5
#define INDEXED 6
#define INDIRECT 7
#define IMMEDIATE 8
#define IMMEDIATE_CONST_4 9
using namespace std;
string bin(string hexn,int n)
{
	string b;
	int i;
	for(i=0;i<n-hexn.length()*4;i++)
		b+='0';
	for(i=0;i<hexn.length();i++)
	{
		switch(hexn[i])
		{
			case '0':
			b+="0000";
			break;
			case '1':
			b+="0001";
			break;
			case '2':
			b+="0010";
			break;
			case '3':
			b+="0011";
			break;
			case '4':
			b+="0100";
			break;
			case '5':
			b+="0101";
			break;
			case '6':
			b+="0110";
			break;
			case '7':
			b+="0111";
			break;
			case '8':
			b+="1000";
			break;
			case '9':
			b+="1001";
			break;
			case 'A':
			case 'a':
			b+="1010";
			break;
			case 'B':
			case 'b':
			b+="1011";
			break;
			case 'C':
			case 'c':
			b+="1100";
			break;
			case 'D':
			case 'd':
			b+="1101";
			break;
			case 'E':
			case 'e':
			b+="1110";
			break;
			case 'F':
			case 'f':
			b+="1111";
			break;
			default:
			b+="0000";
		}
	}
	return b;
}
string hexa(string binn)
{
	string h,s;
	int i;
	for(i=0;i<binn.length();i+=4)
	{
		s=binn.substr(i,4);
		if(s=="0000")
			h+="0";
		else if(s=="0001")
			h+="1";
		else if(s=="0010")
			h+="2";
		else if(s=="0011")
			h+="3";
		else if(s=="0100")
			h+="4";
		else if(s=="0101")
			h+="5";
		else if(s=="0110")
			h+="6";
		else if(s=="0111")
			h+="7";
		else if(s=="1000")
			h+="8";
		else if(s=="1001")
			h+="9";
		else if(s=="1010")
			h+="A";
		else if(s=="1011")
			h+="B";
		else if(s=="1100")
			h+="C";
		else if(s=="1101")
			h+="D";
		else if(s=="1110")
			h+="E";
		else if(s=="1111")
			h+="F";
	}
	return h;
}
class ASSEMBLER
{
	ifstream assembly_code;
	ofstream listing, object_code;
	string object_code_name, assembly_code_name, program_name, starting_address;
	bool BASE;
	int LOCCTR,program_length;
	struct OPCODE
	{
		public:
		string opcode;
		int format;
	};
	unordered_map<string,OPCODE>OPTAB;
	struct SYMBOL
	{
		int address;
		bool defined;
		struct node
		{
			int address,format,base_value;
			node *link;
		}*linked_list;
	};
	unordered_map<string,SYMBOL>SYMTAB;
	public:
	ASSEMBLER(string, string);
	void pass();
	void read(string&, string&, string&);
	~ASSEMBLER();
};
int main(int argc, char* argv[])
{
	string output, input;
	if(argc==1)
	{
		cout<<"PLEASE ENTER INPUT FILE NAME\n";
		cin>>input;
		cout<<"PLEASE ENTER OUPUT FILE NAME\n";
		cin>>output;
	}
	else if(argc==2)
	{
		input=argv[1];
		cout<<"PLEASE ENTER OUTPUT FILE NAME\n";
		cin>>output;
	}
	else
	{
		input=argv[1];
		output=argv[2];
	}
	ASSEMBLER A(input,output);
	A.pass();
	return 0;
}
ASSEMBLER::ASSEMBLER(string input, string output)
{
	assembly_code_name=input;
	object_code_name=output;
	assembly_code.open(input);
	if(!assembly_code)
	{
		cout<<"ERROR: INPUT FILE DOES NOT EXIST\n";
		exit(0);
	}
	object_code.open(output);
	listing.open("assembly_listing.txt");
	ifstream opcodes("optab.txt");
	if(!opcodes)
	{
		cout<<"ERROR: OPCODES FILE IS MISSING\n";
		exit(0);
	}
	string name, code, form;
	while(opcodes)
	{
		OPCODE op;
		opcodes>>name>>code>>form;
		op.opcode=code;
		op.format=atoi(form.c_str());
		OPTAB[name]=op;
	}
	opcodes.close();
	ifstream registers("register.txt");
	if(!registers)
	{
		cout<<"ERROR: REGISTERS FILE IS MISSING\n";
		exit(0);
	}
	string address;
	while(registers)
	{
		SYMBOL sym;
		registers>>name>>address;
		sym.address=atoi(address.c_str());
		sym.defined=true;
		sym.linked_list=NULL;
		SYMTAB[name]=sym;
	}
	registers.close();
}
void ASSEMBLER::pass()
{
	bool skip;
	BASE=false;
	string label, opcode, operand ,actual_opcode,actual_operand;
	stringstream init_text_record(""), text_record(""), machine_code(""), modification("");
	int text_record_length, starting_address_int, current_address_int, addressing_mode, displacement, base_value;
	read(label,opcode,operand);
	if(opcode=="START")
	{
		program_name=label;
		starting_address="0";
		stringstream LOCCTR_init(starting_address);
		LOCCTR_init>>hex>>LOCCTR;
		starting_address_int=LOCCTR;
		listing<<setw(4)<<setfill('0')<<uppercase<<hex<<LOCCTR<<"\t"<<label<<"\t"<<opcode<<"\t"<<"0"<<"\n";
		read(label,opcode,operand);
	}
	else
	{
		starting_address="0";
		LOCCTR=0;
	}
	object_code<<"H^"<<program_name;
	for(int i=0;i<6-program_name.length();i++)
		object_code<<" ";
	object_code<<"^"<<setw(6)<<setfill('0')<<starting_address<<"^";
	long program_length_position=object_code.tellp();
	object_code<<"000000\n";
	init_text_record<<"T^"<<setw(6)<<setfill('0')<<uppercase<<hex<<LOCCTR<<"^";
	text_record.str("");
	while(opcode!="END")
	{
		actual_opcode=opcode;
		actual_operand=operand;
		current_address_int=LOCCTR;
		machine_code.str("");
		if(label[0]!='.')
		{
			if(opcode=="BASE")
			{
				BASE=true;
			}
			if(opcode=="NOBASE")
			{
				BASE=false;
			}
			if(opcode[0]=='+' && operand[0]=='#' && isdigit(operand[1]))
			{
				actual_opcode=opcode.substr(1,opcode.length());
				actual_operand=operand.substr(1,operand.length());
				addressing_mode=IMMEDIATE_CONST_4;
			}
			else if(opcode[0]=='+' && operand[0]=='#')
			{
				actual_opcode=opcode.substr(1,opcode.length());
				actual_operand=operand.substr(1,operand.length());
				addressing_mode=IMMEDIATE;
			}
			else if(opcode[0]=='+' && operand[0]=='@')
			{
				actual_opcode=opcode.substr(1,opcode.length());
				actual_operand=operand.substr(1,operand.length());
				addressing_mode=INDIRECT;
			}
			else if(opcode[0]=='+' && operand.length()>=2 && operand.substr(operand.length()-2,2)==",X")
			{
				actual_opcode=opcode.substr(1,opcode.length());
				actual_operand=operand.substr(0,operand.length()-2);
				addressing_mode=INDEXED;
			}
			else if(opcode[0]=='+')
			{
				actual_opcode=opcode.substr(1,opcode.length());
				actual_operand=operand;
				addressing_mode=REGULAR;
			}
			else if(operand[0]=='#' && isdigit(operand[1]))
			{
				actual_opcode=opcode;
				actual_operand=operand.substr(1,operand.length());
				addressing_mode=IMMEDIATE_CONST_3;
			}
			else if(operand[0]=='#')
			{
				actual_opcode=opcode;
				actual_operand=operand.substr(1,operand.length());
				addressing_mode=IMMEDIATE_PC;
			}
			else if(operand[0]=='@')
			{
				actual_opcode=opcode;
				actual_operand=operand.substr(1,operand.length());
				addressing_mode=INDIRECT_PC;
			}
			else if(operand.length()>=2 && operand.substr(operand.length()-2,2)==",X")
			{
				actual_opcode=opcode;
				actual_operand=operand.substr(0,operand.length()-2);
				addressing_mode=INDEXED_PC;
			}
			else
			{
				actual_opcode=opcode;
				actual_operand=operand;
				addressing_mode=REGULAR_PC;
			}
			if(!label.empty() && SYMTAB.find(label)!=SYMTAB.end() &&skip==false)
			{
				if(SYMTAB[label].defined==false)
				{
					string s=text_record.str();
			        text_record_length=machine_code.str().length()+(text_record.str().length()-count(s.begin(),s.end(),'^'))/2;			
					if(text_record_length>0)
					{
						object_code<<init_text_record.str()<<setw(2)<<setfill('0')<<uppercase<<hex<<text_record_length<<text_record.str()<<"\n";
						init_text_record.str("");
						text_record.str("");
					}
					SYMBOL::node *r,*q;
					for(r=SYMTAB[label].linked_list;r!=NULL;r=r->link)
					{
						if(r->format==4)
						{
							object_code<<"T^"<<setw(6)<<setfill('0')<<uppercase<<hex<<r->address<<"^03^"<<setw(6)<<setfill('0')<<hex<<LOCCTR<<"\n";
						}
						else if(r->format==3)
						{
							displacement=LOCCTR-(r->address);
							if(displacement>=-2048 && displacement<=2047)
							{
								object_code<<"T^"<<setw(6)<<setfill('0')<<uppercase<<hex<<r->address<<"^02^"<<hexa("0010")<<setw(3)<<setfill('0')<<hex<<displacement<<"\n";
							}
							else if(r->base_value!=INT_MIN)
							{
								displacement=LOCCTR-(r->base_value);
								if(displacement>=0 && displacement<=4095)
								{
									object_code<<"T^"<<setw(6)<<setfill('0')<<uppercase<<hex<<r->address<<"^02^"<<hexa("0100")<<setw(3)<<setfill('0')<<hex<<displacement<<"\n";
								}
								else
								{
									cout<<"ERROR: DISPLACEMENT DOES NOT FIT: "<<displacement<<"\n@"<<label<<"\t"<<opcode<<"\t"<<operand<<"\n";
									return;
								}
							}
						}
						else
						{
							cout<<"ERROR: DISPLACEMENT DOES NOT FIT: "<<displacement<<"\n@"<<label<<"\t"<<opcode<<"\t"<<operand<<"\n";
							return;
						}
					}
					init_text_record.str("");
					init_text_record<<"T^"<<setw(6)<<setfill('0')<<uppercase<<hex<<LOCCTR<<"^";
					text_record.str("");
					for(r=SYMTAB[label].linked_list;r!=NULL;r=q)
					{
						q=r->link;
						delete r;
					}
					SYMTAB[label].address=LOCCTR;
					SYMTAB[label].defined=true;
					SYMTAB[label].linked_list=NULL;
				}
				else
				{
					cout<<"ERROR: SYMBOL REDEFINED: "<<label<<"\n@"<<label<<"\t"<<opcode<<"\t"<<operand<<"\n";
					return;
				}
			}
			else
			{
				SYMBOL sym;
				sym.address=LOCCTR;
				sym.defined=true;
				sym.linked_list=NULL;
				SYMTAB[label]=sym;
			}
			skip=false;
			if(opcode=="RSUB")
			{
				machine_code.str("010011110000000000000000");
			}
			else if(OPTAB.find(actual_opcode)!=OPTAB.end())
			{
				machine_code<<bin(OPTAB[actual_opcode].opcode,8);
				machine_code.str(machine_code.str().substr(0,6));
				stringstream ss("");
				if(actual_opcode=="LDB" && BASE==true)
				{
					if(SYMTAB.find(actual_operand)!=SYMTAB.end())
					{
						base_value=SYMTAB[actual_operand].address;
					}
				}
				if(OPTAB[actual_opcode].format==1)
				{
					machine_code.str(machine_code.str()+"00");
					LOCCTR++;
				}
				else if(OPTAB[actual_opcode].format==2)
				{
					machine_code.str(machine_code.str()+"00");
					LOCCTR+=2;
					if(actual_operand.length()==1)
					{
						ss.str("");
						ss<<hex<<SYMTAB[operand].address;
						machine_code.str(machine_code.str()+bin(ss.str(),4)+"0000");
					}
					else
					{
						ss.str("");
						if(SYMTAB.find(operand.substr(0,1))!=SYMTAB.end())
						{
							ss<<hex<<SYMTAB[operand.substr(0,1)].address;
							machine_code.str(machine_code.str()+bin(ss.str(),4));
						}
						if(SYMTAB.find(operand.substr(2,1))!=SYMTAB.end())
						{
							ss.str("");
							ss<<hex<<SYMTAB[operand.substr(2,1)].address;
							machine_code.str(machine_code.str()+bin(ss.str(),4));
						}
					}
				}
				else if(OPTAB[actual_opcode].format==3)
				{
					switch(addressing_mode)
					{
						case REGULAR_PC:
						LOCCTR+=3;
						machine_code.str(machine_code.str()+"110010");
						if(SYMTAB.find(actual_operand)!=SYMTAB.end())
						{
							if(SYMTAB[actual_operand].defined==true)
							{
								displacement=SYMTAB[actual_operand].address-(LOCCTR);
								if(displacement>=-2048 && displacement<=2047)
								{
									ss<<setw(3)<<setfill('0')<<uppercase<<hex<<displacement;
									machine_code.str(machine_code.str()+bin(ss.str().substr(ss.str().length()-3,3),12));
									ss.str("");
								}
								else if(BASE==true)
								{
									displacement=SYMTAB[actual_operand].address-base_value;
									if(displacement>=0 && displacement<=4095)
									{
										ss<<setw(3)<<setfill('0')<<uppercase<<hex<<displacement;
										machine_code.str(machine_code.str()+bin(ss.str().substr(ss.str().length()-3,3),12));
										ss.str("");
									}
									else
									{
										cout<<"ERROR: DISPLACEMENT DOES NOT FIT:"<<displacement<<"\n@"<<label<<"\t"<<opcode<<"\t"<<operand<<"\n";
										return;
									}
								}
								else
								{
									cout<<"ERROR: DISPLACEMENT DOES NOT FIT:"<<displacement<<"\n@"<<label<<"\t"<<opcode<<"\t"<<operand<<"\n";
									return;
								}
							}
							else
							{
								SYMBOL::node *temp, *r;
								temp=new SYMBOL::node;
								temp->address=LOCCTR-2;
								temp->format=3;
								temp->base_value=base_value;
								temp->link=NULL;
								for(r=SYMTAB[actual_operand].linked_list;r->link!=NULL;r=r->link);
								r->link=temp;
								machine_code.str(machine_code.str()+"000000000000");
							}
						}
						else
						{
							SYMBOL sym;
							sym.address=0;
							sym.defined=false;
							sym.linked_list=new SYMBOL::node;
							sym.linked_list->address=LOCCTR-2;
							sym.linked_list->format=3;
							sym.linked_list->base_value=base_value;
							sym.linked_list->link=NULL;
							SYMTAB[actual_operand]=sym;
							machine_code.str(machine_code.str()+"000000000000");
						}
						break;
						case INDEXED_PC:
						LOCCTR+=3;
						machine_code.str(machine_code.str()+"111010");
						if(SYMTAB.find(actual_operand)!=SYMTAB.end())
						{
							if(SYMTAB[actual_operand].defined==true)
							{
								displacement=SYMTAB[actual_operand].address-(LOCCTR);
								if(displacement>=-2048 && displacement<=2047)
								{
									ss<<setw(3)<<setfill('0')<<uppercase<<hex<<displacement;
									machine_code.str(machine_code.str()+bin(ss.str().substr(ss.str().length	()-3,3),12));
									ss.str("");
								}
								else if(BASE==true)
								{
									displacement=SYMTAB[actual_operand].address-base_value;
									if(displacement>=0 && displacement<=4095)
									{
										ss<<setw(3)<<setfill('0')<<uppercase<<hex<<displacement;
										machine_code.str(machine_code.str()+bin(ss.str().substr(ss.str().length()-3,3),12));
										ss.str("");
									}
									else
									{
										cout<<"ERROR: DISPLACEMENT DOES NOT FIT:"<<displacement<<"\n@"<<label<<"\t"<<opcode<<"\t"<<operand<<"\n";
										return;
									}
								}
								else
								{
									cout<<"ERROR: DISPLACEMENT DOES NOT FIT:"<<displacement<<"\n@"<<label<<"\t"<<opcode<<"\t"<<operand<<"\n";
									return;
								}
							}
							else
							{
								SYMBOL::node *temp, *r;
								temp=new SYMBOL::node;
								temp->address=LOCCTR-2;
								temp->format=3;
								temp->base_value=base_value;
								temp->link=NULL;
								for(r=SYMTAB[actual_operand].linked_list;r->link!=NULL;r=r->link);
								r->link=temp;
								machine_code.str(machine_code.str()+"000000000000");
							}
						}
						else
						{
							SYMBOL sym;
							sym.address=0;
							sym.defined=false;
							sym.linked_list=new SYMBOL::node;
							sym.linked_list->address=LOCCTR-2;
							sym.linked_list->format=3;
							sym.linked_list->base_value=base_value;
							sym.linked_list->link=NULL;
							SYMTAB[actual_operand]=sym;
							machine_code.str(machine_code.str()+"000000000000");
						}
						break;
						case INDIRECT_PC:
						LOCCTR+=3;
						machine_code.str(machine_code.str()+"100010");
						if(SYMTAB.find(actual_operand)!=SYMTAB.end())
						{
							if(SYMTAB[actual_operand].defined==true)
							{
								displacement=SYMTAB[actual_operand].address-(LOCCTR);
								if(displacement>=-2048 && displacement<=2047)
								{
									ss<<setw(3)<<setfill('0')<<uppercase<<hex<<displacement;
									machine_code.str(machine_code.str()+bin(ss.str().substr(ss.str().length	()-3,3),12));
									ss.str("");
								}
								else if(BASE==true)
								{
										displacement=SYMTAB[actual_operand].address-base_value;
									if(displacement>=0 && displacement<=4095)
									{
										ss<<setw(3)<<setfill('0')<<uppercase<<hex<<displacement;
										machine_code.str(machine_code.str()+bin(ss.str().substr(ss.str	().length()-3,3),12));
										ss.str("");
									}
									else
									{
										cout<<"ERROR: DISPLACEMENT DOES NOT FIT:"<<displacement<<"\n@"<<label<<"\t"<<opcode<<"\t"<<operand<<"\n";
										return;
									}
								}
								else
								{
									cout<<"ERROR: DISPLACEMENT DOES NOT FIT:"<<displacement<<"\n@"<<label<<"\t"<<opcode<<"\t"<<operand<<"\n";
									return;
								}
							}
							else
							{
								SYMBOL::node *temp, *r;
								temp=new SYMBOL::node;
								temp->address=LOCCTR-2;
								temp->format=3;
								temp->base_value=base_value;
								temp->link=NULL;
								for(r=SYMTAB[actual_operand].linked_list;r->link!=NULL;r=r->link);
								r->link=temp;
								machine_code.str(machine_code.str()+"000000000000");
							}
						}
						else
						{
							SYMBOL sym;
							sym.address=0;
							sym.defined=false;
							sym.linked_list=new SYMBOL::node;
							sym.linked_list->address=LOCCTR-2;
							sym.linked_list->format=3;
							sym.linked_list->base_value=base_value;
							sym.linked_list->link=NULL;
							SYMTAB[actual_operand]=sym;
							machine_code.str(machine_code.str()+"000000000000");
						}
						break;
						case IMMEDIATE_PC:
						LOCCTR+=3;
						machine_code.str(machine_code.str()+"010010");
						if(SYMTAB.find(actual_operand)!=SYMTAB.end())
						{
							if(SYMTAB[actual_operand].defined==true)
							{
								displacement=SYMTAB[actual_operand].address-(LOCCTR);
								if(displacement>=-2048 && displacement<=2047)
								{
									ss<<setw(3)<<setfill('0')<<uppercase<<hex<<displacement;
									machine_code.str(machine_code.str()+bin(ss.str().substr(ss.str().length	()-3,3),12));
									ss.str("");
								}
								else if(BASE==true)
								{
									displacement=SYMTAB[actual_operand].address-base_value;
									if(displacement>=0 && displacement<=4095)
									{
										ss<<setw(3)<<setfill('0')<<uppercase<<hex<<displacement;
										machine_code.str(machine_code.str()+bin(ss.str().substr(ss.str	().length()-3,3),12));
										ss.str("");
									}
									else
									{
										cout<<"ERROR: DISPLACEMENT DOES NOT FIT:"<<displacement<<"\n@"<<label<<"\t"<<opcode<<"\t"<<operand<<"\n";
										return;
									}
								}
								else
								{
									cout<<"ERROR: DISPLACEMENT DOES NOT FIT:"<<displacement<<"\n@"<<label<<"\t"<<opcode<<"\t"<<operand<<"\n";
									return;
								}
							}
							else
							{
								SYMBOL::node *temp, *r;
								temp=new SYMBOL::node;
								temp->address=LOCCTR-2;
								temp->format=3;
								temp->base_value=base_value;
								temp->link=NULL;
								for(r=SYMTAB[actual_operand].linked_list;r->link!=NULL;r=r->link);
								r->link=temp;
								machine_code.str(machine_code.str()+"000000000000");
							}
						}
						else
						{
							SYMBOL sym;
							sym.address=0;
							sym.defined=false;
							sym.linked_list=new SYMBOL::node;
							sym.linked_list->address=LOCCTR-2;
							sym.linked_list->format=3;
							sym.linked_list->base_value=base_value;
							sym.linked_list->link=NULL;
							SYMTAB[actual_operand]=sym;
							machine_code.str(machine_code.str()+"000000000000");
						}
						break;
						case IMMEDIATE_CONST_3:
						LOCCTR+=3;
						machine_code.str(machine_code.str()+"010000"+bin(actual_operand,12));
						break;
						case REGULAR:
						LOCCTR+=4;
						modification<<"M^"<<setw(6)<<setfill('0')<<uppercase<<hex<<LOCCTR-3<<"^05\n";
						machine_code.str(machine_code.str()+"110001");
						if(SYMTAB.find(actual_operand)!=SYMTAB.end())
						{
							if(SYMTAB[actual_operand].defined==false)
							{
								SYMBOL::node *temp, *r;
								temp=new SYMBOL::node;
								temp->address=LOCCTR-3;
								temp->format=4;
								temp->link=NULL;
								for(r=SYMTAB[actual_operand].linked_list;r->link!=NULL;r=r->link);
								r->link=temp;
								machine_code.str(machine_code.str()+"00000000000000000000");
							}
							else
							{
								ss<<setw(5)<<setfill('0')<<uppercase<<hex<<SYMTAB[actual_operand].address;
								machine_code.str(machine_code.str()+bin(ss.str().substr(ss.str().length()-5,5),	20));
								ss.str("");
							}
						}
						else
						{
							SYMBOL sym;
							sym.address=0;
							sym.defined=false;
							sym.linked_list=new SYMBOL::node;
							sym.linked_list->address=LOCCTR-3;
							sym.linked_list->format=4;
							sym.linked_list->link=NULL;
							SYMTAB[actual_operand]=sym;
							machine_code.str(machine_code.str()+"00000000000000000000");
						}
						break;
						case INDEXED:
						LOCCTR+=4;
						modification<<"M^"<<setw(6)<<setfill('0')<<uppercase<<hex<<LOCCTR-3<<"^05\n";
						machine_code.str(machine_code.str()+"111001");
						if(SYMTAB.find(actual_operand)!=SYMTAB.end())
						{
							if(SYMTAB[actual_operand].defined==false)
							{
								SYMBOL::node *temp, *r;
								temp=new SYMBOL::node;
								temp->address=LOCCTR+1;
								temp->format=4;
								temp->link=NULL;
								for(r=SYMTAB[actual_operand].linked_list;r->link!=NULL;r=r->link);
								r->link=temp;
								machine_code.str(machine_code.str()+"00000000000000000000");
							}
							else
							{
								ss<<setw(5)<<setfill('0')<<uppercase<<hex<<SYMTAB[actual_operand].address;
								machine_code.str(machine_code.str()+bin(ss.str().substr(ss.str().length()-5,5),	20));
								ss.str("");
							}
						}
						else
						{
							SYMBOL sym;
							sym.address=0;
							sym.defined=false;
							sym.linked_list=new SYMBOL::node;
							sym.linked_list->address=LOCCTR-3;
							sym.linked_list->format=4;
							sym.linked_list->link=NULL;
							SYMTAB[actual_operand]=sym;
							machine_code.str(machine_code.str()+"00000000000000000000");
						}
						case INDIRECT:
						LOCCTR+=4;
						modification<<"M^"<<setw(6)<<setfill('0')<<uppercase<<hex<<LOCCTR-3<<"^05\n";
						machine_code.str(machine_code.str()+"100001");
						if(SYMTAB.find(actual_operand)!=SYMTAB.end())
						{
							if(SYMTAB[actual_operand].defined==false)
							{
								SYMBOL::node *temp, *r;
								temp=new SYMBOL::node;
								temp->address=LOCCTR-3;
								temp->format=4;
								temp->link=NULL;
								for(r=SYMTAB[actual_operand].linked_list;r->link!=NULL;r=r->link);
								r->link=temp;
								machine_code.str(machine_code.str()+"00000000000000000000");
							}
							else
							{
								ss<<setw(5)<<setfill('0')<<uppercase<<hex<<SYMTAB[actual_operand].address;
								machine_code.str(machine_code.str()+bin(ss.str().substr(ss.str().length()-5,5),	20));
								ss.str("");
							}
						}
						else
						{
							SYMBOL sym;
							sym.address=0;
							sym.defined=false;
							sym.linked_list=new SYMBOL::node;
							sym.linked_list->address=LOCCTR-3;
							sym.linked_list->format=4;
							sym.linked_list->link=NULL;
							SYMTAB[actual_operand]=sym;
							machine_code.str(machine_code.str()+"00000000000000000000");
						}
						break;
						case IMMEDIATE:
						LOCCTR+=4;
						modification<<"M^"<<setw(6)<<setfill('0')<<uppercase<<hex<<LOCCTR-3<<"^05\n";
						machine_code.str(machine_code.str()+"010001");
						if(SYMTAB.find(actual_operand)!=SYMTAB.end())
						{
							if(SYMTAB[actual_operand].defined==false)
							{
								SYMBOL::node *temp, *r;
								temp=new SYMBOL::node;
								temp->address=LOCCTR-3;
								temp->format=4;
								temp->link=NULL;
								for(r=SYMTAB[actual_operand].linked_list;r->link!=NULL;r=r->link);
								r->link=temp;
								machine_code.str(machine_code.str()+"00000000000000000000");
							}
							else
							{
								ss<<setw(5)<<setfill('0')<<uppercase<<hex<<SYMTAB[actual_operand].address;
								machine_code.str(machine_code.str()+bin(ss.str().substr(ss.str().length()-5,5),	20));
								ss.str("");
							}
						}
						else
						{
							SYMBOL sym;
							sym.address=0;
							sym.defined=false;
							sym.linked_list=new SYMBOL::node;
							sym.linked_list->address=LOCCTR-3;
							sym.linked_list->format=4;
							sym.linked_list->link=NULL;
							SYMTAB[actual_operand]=sym;
							machine_code.str(machine_code.str()+"00000000000000000000");
						}
						break;
						case IMMEDIATE_CONST_4:
						LOCCTR+=4;
						ss.str("");
						ss<<setw(5)<<setfill('0')<<hex<<atoi(actual_operand.c_str());
						machine_code.str(machine_code.str()+"010001"+bin(ss.str(),20));
						break;
					}
				}
			}
			else if(opcode=="WORD")
			{
				machine_code<<setw(6)<<setfill('0')<<uppercase<<hex<<atoi(operand.c_str());
				machine_code.str(bin(machine_code.str(),24));
				LOCCTR+=3;
			}
			else if(opcode=="BYTE")
			{
				
				if(operand[0]=='C')
				{
					for(int i=2;i<operand.length()-1;i++)
						machine_code<<uppercase<<hex<<(int)operand[i];
					machine_code.str(bin(machine_code.str(),24));
					LOCCTR+=operand.length()-3;
				}
				else
				{
					machine_code.str(operand.substr(2,operand.length()-3));
					machine_code.str(bin(machine_code.str(),(operand.length()-3)*4));
					LOCCTR+=(operand.length()-3)/2;
				}
			}
			else if(opcode=="RESW" || opcode=="RESB" || opcode=="BASE" || opcode=="NOBASE");
			else
			{
				cout<<"ERROR: OPCODE NOT FOUND: "<<opcode<<"\n@"<<label<<"\t"<<opcode<<"\t"<<operand<<"\n";
				return;
			}
			string s=text_record.str();
			text_record_length=(text_record.str().length()-count(s.begin(),s.end(),'^'))/2;
			if((opcode=="RESW") || (opcode=="RESB") || (text_record_length+machine_code.str().length()/8>30))
			{
				if(text_record_length>0)
				{
					object_code<<init_text_record.str()<<setw(2)<<setfill('0')<<uppercase<<hex<<text_record_length<<text_record.str()<<"\n";
					init_text_record.str("");
					text_record.str("");
				}
				do
				{
					stringstream operand_string(operand);
					int operand_int;
					operand_string>>dec>>operand_int;
					if(opcode=="RESW")
						LOCCTR+=3*operand_int;
					else if(opcode=="RESB")
						LOCCTR+=operand_int;
					listing<<setw(4)<<setfill('0')<<uppercase<<hex<<current_address_int<<"\t"<<label<<"\t"<<opcode<<"\t"<<operand<<"\t"<<hexa(machine_code.str())<<"\n";
					current_address_int=LOCCTR;
					read(label,opcode,operand);
					if(SYMTAB.find(label)==SYMTAB.end())
					{
						SYMBOL sym;
						sym.address=LOCCTR;
						sym.defined=true;
						sym.linked_list=NULL;
						SYMTAB[label]=sym;
					}
					skip=true;
				}while(opcode=="RESW" || opcode=="RESB");
				init_text_record.str("");
				init_text_record<<"T^"<<setw(6)<<setfill('0')<<uppercase<<hex<<LOCCTR<<"^";
				text_record.str("");
			}
			if(machine_code.str().length()>0)
			{
				text_record<<"^"<<hexa(machine_code.str());
			}
		}
		if(!skip)
		{
			listing<<setw(4)<<setfill('0')<<uppercase<<hex<<current_address_int<<"\t"<<label<<"\t"<<opcode<<"\t"<<operand<<"\t"<<hexa(machine_code.str())<<"\n";
			read(label,opcode,operand);
		}
	}
	text_record_length+=machine_code.str().length()/8;
	object_code<<init_text_record.str()<<setw(2)<<setfill('0')<<uppercase<<hex<<text_record_length<<text_record.str()<<"\n";
	object_code<<modification.str();
	object_code<<"E^"<<setw(6)<<setfill('0')<<uppercase<<hex<<SYMTAB[operand].address;
	object_code.seekp(program_length_position);
	object_code<<setw(6)<<setfill('0')<<uppercase<<hex<<LOCCTR-starting_address_int<<"\n";
	listing<<"\t"<<label<<"\t"<<opcode<<"\t"<<operand<<"\n";
}
void ASSEMBLER::read(string &label,string &opcode,string &operand)
{
	here:
	getline(assembly_code,label,'\t');
	if(label[0]=='.')
	{
		getline(assembly_code,label,'\n');
		goto here;
	}
	else
	{
		getline(assembly_code,opcode,'\t');
		getline(assembly_code,operand,'\n');
	}	
}
ASSEMBLER::~ASSEMBLER()
{
	assembly_code.close();
	listing.close();
	object_code.close();
	ifstream fin;
	string buffer;
	fin.open(assembly_code_name);
	getline(fin,buffer,'\0');
	cout<<"------   INPUT FILE   ------\n";
	cout<<buffer;
	cout<<"\n----------------------------\n\n";
	fin.close();
	fin.open("assembly_listing.txt");
	getline(fin,buffer,'\0');
	cout<<"----------------ASSEMBLY LISTING----------------\n";
	cout<<buffer;
	cout<<"\n------------------------------------------------\n\n";
	fin.close();
	fin.open(object_code_name);
	getline(fin,buffer,'\0');
	cout<<"--------------------------   OUTPUT FILE   --------------------------\n";
	cout<<buffer;
	cout<<"\n---------------------------------------------------------------------\n\n";
	fin.close();
}
