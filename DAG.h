#include "FrontEndInterface.h"

string ops[] = {
	"add", "sub", "mult", "div", "assign","ret","func_call","input","print","array_assign", "array_use"
};

//DAG结点
struct DNode {
	bool isRoot; //是否是根结点
	bool isuseful;	//是否有用
	IRType ir;	//运算符
	int M;	//主标记的下标
	vector<IRArg> A;	//标记(包含主标记)
	vector<int> oprs;  //后继结点下标
	vector<int> pioneer; //前驱结点下标
	DNode(IRType ir) :
		ir(ir){
			oprs.push_back(-1);
			oprs.push_back(-1);
			isuseful = false;
		}

	void print() {
		cout << "op: " << ops[(int)ir] << '|';
		A[M].print();
		cout << " : ";
		for (auto it : A) {
			it.print();
			cout << ' ';
		}
		cout << '|';
		for (int i = oprs.size() - 1; i >= 0; --i) {
			if (oprs[i] >= 0) {
				cout << oprs[i] << ' ';
			}
		}
		cout << '|';
		for (auto it : pioneer) {
			cout << it << ' ';
		}
		cout << endl;
	}
};

//基本块
struct Eblock{
	int start, end; //基本块开始结束下标
	vector<DNode> nodes;	//DAG结点
	map<IRArg , int> mIR;	//用于记录某操作数的最新下标
	vector<IRArg> reserve_temp;	//保留下来的临时变量
	Eblock(int start, int end, vector<IRArg> reserve_temp):
		start(start),end(end),reserve_temp(std::move(reserve_temp)){}
	void open(int i);	//打开某个结点
	void print() {
		for (int i = 0; i < nodes.size();++i ) {
			cout << i << ' ';
			nodes[i].print();
		}
	}
	bool isreserve_temp(IRArg a);
};

class DAG{
private:
	FrontEndInterface* FEI;
	vector<Eblock> blocks;
public:
	DAG(FrontEndInterface* f);
	~DAG();
	void print();
	void gene_blocks(); //生成基本块
	void optimize();	//优化
};


DAG::DAG(FrontEndInterface* f):
		FEI(f){}


DAG::~DAG(){}

inline void DAG::print()
{

	for (int i = 0; i < FEI->ir_list.size();++i) {
		cout << i << ' ';
		FEI->ir_list[i]->print();
	}
}

inline void DAG::gene_blocks()
{
	int start, end;
	start = end = -1;
	int _size = FEI->ir_list.size();
	vector<IRArg> reserve_temp;
	for (int i = 0; i < _size; ++i) {
		IRNode* curIR = FEI->ir_list[i]; //当前扫描到的四元式

		if (curIR->type == IRType::ret ) {
			reserve_temp.push_back(curIR->args[0]);
		}

		if (curIR->type == IRType::func) {
			start = i;
		}
		else if (curIR->type == IRType::ret || curIR->type == IRType::jump || curIR->type == IRType::equal_jump || curIR->type == IRType::unequal_jump || curIR->type == IRType::ge_jump || curIR->type == IRType::le_jump || curIR->type == IRType::greater_jump || curIR->type == IRType::less_jump) {
			if (i > 0) {
				end = i;
				blocks.push_back(Eblock(start, end, reserve_temp));
				reserve_temp.clear();
			}
			start = i + 1;
		}
		else if (curIR->label != NULL) {
			if (start < i) {
				if (i > 0) {
					end = i - 1;
					blocks.push_back(Eblock(start, end, reserve_temp));
					reserve_temp.clear();
				}
			}
			start = i;
		}
	}
	//cout << "fuckyou" << endl;
	/*
	for (auto it : blocks) {
		cout << it.start << ' ' << it.end << endl;
		for (auto jt : it.reserve_temp) {
			jt.print();
		}
		cout << endl;
	}
	*/
}

inline void DAG::optimize(){
	//扫描每个基本块
	for (auto block : blocks) {
		//生成DAG图
		for (int i = block.start; i <= block .end; ++i) {
			IRNode* curIR = FEI->ir_list[i]; //当前扫描到的四元式
			if (curIR->type == IRType::assign) {
				//如果是赋值四元式
				auto index = block.mIR.find(curIR->args[0]);	//DAG中是否已存在右值
				if(index == block.mIR.end()){
					int _size = block.nodes.size();
					//DAG中没有

					//更新map
					block.mIR[curIR->args[0]] = _size;
					block.mIR[curIR->args[1]] = _size;
					
					
					//更新nodes
					block.nodes.push_back(DNode(IRType::assign)); //创建一个结点
					DNode& curNode = block.nodes[_size]; //引用当前结点


					curNode.A.push_back(curIR->args[0]);//加入标记
					curNode.A.push_back(curIR->args[1]);
					
					
					//确定主标记
					if (curIR->args[1].type == IRAType::var && curIR->args[0].type == IRAType::temp) {
						//用户变量 = 临时变量时
						curNode.M = curNode.A.size() - 1; //将args[1]作为主标记
					}
					else {
						//默认情况
						curNode.M = curNode.A.size() - 2; //将args[0]作为主标记
					}
					
					//设为根节点
					curNode.isRoot = true;
				}
				
				else {
					//DAG中有
					DNode& curNode = block.nodes[index->second]; //引用当前结点;
					
					//更新map
					block.mIR[curIR->args[1]] = index->second;


					int index_arg1 = -1;
					//寻找标记中是否有arg1
					int cursize = curNode.A.size();
					for (int j = 0; j < cursize;++j) {
						if (curNode.A[j] == curIR->args[1]) {
							index_arg1 = j;
							break;
						}
					}


					if (index_arg1 == -1) {
						//没有找到,push
						index_arg1 = curNode.A.size();
						curNode.A.push_back(curIR->args[1]);
					}
					
					//调换主标记(if necessary)
					if (curIR->args[1].type == IRAType::var && curNode.A[curNode.M].type == IRAType::temp ) {
						//用户变量 = 临时变量时
						curNode.M = index_arg1; //将args[1]作为主标记
					}
				}
			}
			else if (curIR->type == IRType::add || curIR->type == IRType::sub || curIR->type == IRType::mult || curIR->type == IRType::div) {
				//如果是运算四元式
				if (curIR->args[0].type == IRAType::int_imm && curIR->args[1].type == IRAType::int_imm) {
					//等式右边全为常数，直接计算，按照赋值语句处理

					//四元式整理
					int rst;	//结果
					switch(curIR->type) {
					case IRType::add:
						rst = curIR->args[1].int_imm + curIR->args[0].int_imm;
						break;
					case IRType::sub:
						rst = curIR->args[1].int_imm - curIR->args[0].int_imm;
						break;
					case IRType::mult:
						rst = curIR->args[1].int_imm * curIR->args[0].int_imm;
						break;
					case IRType::div:
						rst = curIR->args[1].int_imm / curIR->args[0].int_imm;
						break;
					}
					curIR->type = IRType::assign;	//设置为赋值操作符
					curIR->setArg(1, &(curIR->args[2])); //左值交换位置
					curIR->setArg(0, rst);	//右值

					//DAG结点处理(重新扫描)
					i--;
					continue;
				}
				else {
					//搜索两个操作数

					auto find0 = block.mIR.find(curIR->args[0]);
					int index0;
					if (find0 == block.mIR.end()) {
						//结点不存在,新建结点
						int _size = block.nodes.size();

						//更新map
						block.mIR[curIR->args[0]] = _size;

						//更新nodes
						block.nodes.push_back(DNode(IRType::assign)); //创建一个结点
						DNode& curNode = block.nodes[_size]; //引用当前结点

						curNode.A.push_back(curIR->args[0]);//加入标记
						curNode.M = 0;	//主标记

						index0 = _size;	//更新index值
					}
					else {
						index0 = find0->second;
					}

					
					auto find1 = block.mIR.find(curIR->args[1]);
					int index1;
					if (find1 == block.mIR.end()) {
						//结点不存在,新建结点
						int _size = block.nodes.size();

						//更新map
						block.mIR[curIR->args[1]] = _size;

						//更新nodes
						block.nodes.push_back(DNode(IRType::assign)); //创建一个结点
						DNode& curNode = block.nodes[_size]; //引用当前结点

						curNode.A.push_back(curIR->args[1]);//加入标记
						curNode.M = 0;	//主标记

						index1 = _size;	//更新index值
					}
					else {
						index1 = find1->second;
					}

					if (block.nodes[index0].A[block.nodes[index0].M].type == IRAType::int_imm && block.nodes[index1].A[block.nodes[index1].M].type == IRAType::int_imm) {
						//等式右边全为常数，直接计算，按照赋值语句处理

						//四元式整理
						int rst;	//结果
						switch (curIR->type) {
						case IRType::add:
							rst = block.nodes[index1].A[block.nodes[index1].M].int_imm + block.nodes[index0].A[block.nodes[index0].M].int_imm;
							break;
						case IRType::sub:
							rst = block.nodes[index1].A[block.nodes[index1].M].int_imm - block.nodes[index0].A[block.nodes[index0].M].int_imm;
							break;
						case IRType::mult:
							rst = block.nodes[index1].A[block.nodes[index1].M].int_imm * block.nodes[index0].A[block.nodes[index0].M].int_imm;
							break;
						case IRType::div:
							rst = block.nodes[index1].A[block.nodes[index1].M].int_imm / block.nodes[index0].A[block.nodes[index0].M].int_imm;
							break;
						}
						curIR->type = IRType::assign;	//设置为赋值操作符
						curIR->args[1] = curIR->args[2]; //左值交换位置
						curIR->setArg(0, rst);	//右值

						//DAG结点处理(重新扫描)
						i--;
						continue;
					}

					//引用结点
					DNode& curNode0 = block.nodes[index0];


					int index = -1;
					//搜索是否存在相同结点
					int cursizej = curNode0.pioneer.size();
					for (int j = 0; j < cursizej; ++j) {
						if (block.nodes[curNode0.pioneer[j]].ir == curIR->type) {
							//如果找到运算符相同的;
							for (int q = 0; q < 2; ++q) {
								if (block.nodes[curNode0.pioneer[j]].oprs[q] == index1) {
									//找到
									index = curNode0.pioneer[j];
									break;
								}
							}
						}
					}

					if(index == -1){
						//结点不存在,新建结点
						int _size = block.nodes.size();

						//更新map
						block.mIR[curIR->args[2]] = _size;

						//更新nodes
						block.nodes.push_back(DNode(curIR->type)); //创建一个结点
						DNode& curNode = block.nodes[_size]; //引用当前结点


						curNode.A.push_back(curIR->args[2]);//加入标记
						curNode.M = 0;	//主标记

						//添加前驱后继

						//后继
						curNode.oprs[0] = index0;
						curNode.oprs[1] = index1;

						//引用两个结点
						DNode& curNode0 = block.nodes[index0];
						DNode& curNode1 = block.nodes[index1];

						//前驱
						curNode0.pioneer.push_back(_size);
						curNode1.pioneer.push_back(_size);

						//设为根结点
						curNode.isRoot = true;
						curNode0.isRoot = false;
						curNode1.isRoot = false;
					}
					else {
						//已有，添加
						DNode& curNode = block.nodes[index]; //引用当前结点

						int index_arg2 = -1;
						//寻找标记中是否有arg2
						int cursize = curNode.A.size();
						for (int j = 0; j < cursize; ++j) {
							if (curNode.A[j] == curIR->args[2]) {
								index_arg2 = j;
								break;
							}
						}

						if (index_arg2 == -1) {
							//没有找到,push
							index_arg2 = curNode.A.size();
							curNode.A.push_back(curIR->args[2]);

							//更新map
							block.mIR[curIR->args[2]] = index;
						}



						//调换主标记(if necessary)
						if (curIR->args[2].type == IRAType::var && curNode.A[curNode.M].type == IRAType::temp) {
							//主标记为临时变量, 引入的为用户变量时
							curNode.M = index_arg2; //将args[2]作为主标记
						}

					}
					
				}
				
			}
			else if (curIR->type == IRType::func_call) {
				//函数调用

				vector<int> params;	//参数列表
				//搜索参数
				int t;
				for (t = i + 1; t <= block.end; ++t) {
					if (FEI->ir_list[t]->type != IRType::func_param_in) break;
					else {
						auto find = block.mIR.find(FEI->ir_list[t]->args[0]);
						int index;
						if (find == block.mIR.end()) {
							//结点不存在，新建结点

							int _size = block.nodes.size();

							//更新map
							block.mIR[FEI->ir_list[t]->args[0]] = _size;

							//更新nodes
							block.nodes.push_back(DNode(IRType::assign)); //创建一个结点
							DNode& curNode = block.nodes[_size]; //引用当前结点

							curNode.A.push_back(FEI->ir_list[t]->args[0]);//加入标记
							curNode.M = 0;	//主标记

							index = _size;	//更新index值
						}
						else {
							index = find->second;
						}
						params.push_back(index);
					}
				}
				i = t - 1;

				//搜索函数指针
				auto find0 = block.mIR.find(curIR->args[0]);
				int index0;
				if (find0 == block.mIR.end()) {
					//结点不存在,新建结点
					int _size = block.nodes.size();

					//更新map
					block.mIR[curIR->args[0]] = _size;

					//更新nodes
					block.nodes.push_back(DNode(IRType::assign)); //创建一个结点
					DNode& curNode = block.nodes[_size]; //引用当前结点

					curNode.A.push_back(curIR->args[0]);//加入标记
					curNode.M = 0;	//主标记

					index0 = _size;	//更新index值
				}
				else {
					index0 = find0->second;
				}


				//新建结点
				int _size = block.nodes.size();

				//更新map
				block.mIR[curIR->args[1]] = _size;

				//更新nodes
				block.nodes.push_back(DNode(curIR->type)); //创建一个结点
				DNode& curNode = block.nodes[_size]; //引用当前结点


				curNode.A.push_back(curIR->args[1]);//加入标记
				curNode.M = 0;	//主标记

				//添加前驱后继

				//后继
				curNode.oprs[0] = index0;
				for (int j = 0; j < params.size(); ++j) {
					if (j + 1 >= curNode.oprs.size()) {
						curNode.oprs.push_back(params[j]);
					}
					else {
						curNode.oprs[j + 1] = params[j];
					}
				}

				//引用结点
				DNode& curNode0 = block.nodes[index0];

				//前驱
				curNode0.pioneer.push_back(_size);

				//设为根结点
				curNode.isRoot = true;
				curNode0.isRoot = false;

				for (int j = 0; j < params.size(); ++j) {

					DNode& curNodej = block.nodes[params[j]];

					curNodej.pioneer.push_back(_size);
					curNodej.isRoot = false;
				}

			}
			else if (curIR->type == IRType::print ) {
				//更新nodes
				int _size = block.nodes.size();

				block.nodes.push_back(DNode(curIR->type)); //创建一个结点
				DNode& curNode = block.nodes[_size]; //引用当前结点


				curNode.A.push_back(curIR->args[0]);//加入标记
				curNode.M = 0;

				curNode.isRoot = true;

				if (curIR->args[0].type == IRAType::var) {

					//寻找该结点
					auto find = block.mIR.find(curIR->args[0]);
					int index;
					if (find == block.mIR.end()) {
						//结点不存在，新建结点

						int _size0 = block.nodes.size();

						//更新map
						block.mIR[curIR->args[0]] = _size0;

						//更新nodes
						block.nodes.push_back(DNode(IRType::assign)); //创建一个结点
						DNode& curNode = block.nodes[_size0]; //引用当前结点

						curNode.A.push_back(curIR->args[0]);//加入标记
						curNode.M = 0;	//主标记

						index = _size0;	//更新index值
					}
					else {
						index = find->second;
						curNode.A[0] = block.nodes[find->second].A[block.nodes[find->second].M];//更换
					}

					DNode& curNode = block.nodes[_size]; //引用当前结点

					//作为根结点
					curNode.oprs[0] = index;
				}

			}
			else if (curIR->type == IRType::input) {
				//更新nodes
				int _size = block.nodes.size();

				//更新map
				block.mIR[curIR->args[0]] = _size;

				block.nodes.push_back(DNode(curIR->type)); //创建一个结点
				DNode& curNode = block.nodes[_size]; //引用当前结点


				curNode.A.push_back(curIR->args[0]);//加入标记
				curNode.M = 0;

				curNode.isRoot = true;
			}
			else if (curIR->type == IRType::array_use) {

				//搜索两个操作数

				auto find0 = block.mIR.find(curIR->args[0]);
				int index0;
				if (find0 == block.mIR.end()) {
					//结点不存在,新建结点
					int _size = block.nodes.size();

					//更新map
					block.mIR[curIR->args[0]] = _size;

					//更新nodes
					block.nodes.push_back(DNode(IRType::assign)); //创建一个结点
					DNode& curNode = block.nodes[_size]; //引用当前结点

					curNode.A.push_back(curIR->args[0]);//加入标记
					curNode.M = 0;	//主标记

					index0 = _size;	//更新index值
				}
				else {
					index0 = find0->second;
				}


				auto find1 = block.mIR.find(curIR->args[1]);
				int index1;
				if (find1 == block.mIR.end()) {
					//结点不存在,新建结点
					int _size = block.nodes.size();

					//更新map
					block.mIR[curIR->args[1]] = _size;

					//更新nodes
					block.nodes.push_back(DNode(IRType::assign)); //创建一个结点
					DNode& curNode = block.nodes[_size]; //引用当前结点

					curNode.A.push_back(curIR->args[1]);//加入标记
					curNode.M = 0;	//主标记

					index1 = _size;	//更新index值
				}
				else {
					index1 = find1->second;
				}

				//引用结点
				DNode& curNode0 = block.nodes[index0];


				int index = -1;
				//搜索是否存在赋值结点
				int cursizej = curNode0.pioneer.size();
				for (int j = 0; j < cursizej; ++j) {
					if (block.nodes[curNode0.pioneer[j]].ir == IRType::array_assign) {
						//如果找到运算符为数组赋值的;
						if (block.nodes[curNode0.pioneer[j]].oprs[1] == index1) {
							//找到
							index = curNode0.pioneer[j];
							break;
						}
					}
				}

				if (index == -1) {
					//没有被赋值过，新建结点
					int _size = block.nodes.size();

					//更新map
					block.mIR[curIR->args[2]] = _size;

					//更新nodes
					block.nodes.push_back(DNode(curIR->type)); //创建一个结点
					DNode& curNode = block.nodes[_size]; //引用当前结点

					curNode.A.push_back(curIR->args[2]);//加入标记
					curNode.M = 0;	//主标记

					//引用结点
					DNode& curNode0 = block.nodes[index0];
					DNode& curNode1 = block.nodes[index1];

					//添加前驱后继

					//后继
					curNode.oprs[0] = index0;
					curNode.oprs[1] = index1;

					//前驱
					curNode0.pioneer.push_back(_size);
					curNode1.pioneer.push_back(_size);

					//设为根结点
					curNode.isRoot = true;
					curNode0.isRoot = false;
					curNode1.isRoot = false;
				}
				else {
					//cout << "fuck" << endl;
					//如果被赋值过，变为简单赋值结点
					curIR->type = IRType::assign;
					curIR->args[1] = curIR->args[2];
					curIR->args[0] = block.nodes[index].A[block.nodes[index].M];

					//重新扫描
					i--;
				}

			}
			else if (curIR->type == IRType::array_assign) {
				//搜索两个操作数

				auto find0 = block.mIR.find(curIR->args[1]);
				int index0;
				if (find0 == block.mIR.end()) {
					//结点不存在,新建结点
					int _size = block.nodes.size();

					//更新map
					block.mIR[curIR->args[1]] = _size;

					//更新nodes
					block.nodes.push_back(DNode(IRType::assign)); //创建一个结点
					DNode& curNode = block.nodes[_size]; //引用当前结点

					curNode.A.push_back(curIR->args[1]);//加入标记
					curNode.M = 0;	//主标记

					index0 = _size;	//更新index值
				}
				else {
					index0 = find0->second;
				}


				auto find1 = block.mIR.find(curIR->args[2]);
				int index1;
				if (find1 == block.mIR.end()) {
					//结点不存在,新建结点
					int _size = block.nodes.size();

					//更新map
					block.mIR[curIR->args[2]] = _size;

					//更新nodes
					block.nodes.push_back(DNode(IRType::assign)); //创建一个结点
					DNode& curNode = block.nodes[_size]; //引用当前结点

					curNode.A.push_back(curIR->args[2]);//加入标记
					curNode.M = 0;	//主标记

					index1 = _size;	//更新index值
				}
				else {
					index1 = find1->second;
				}


				//引用结点
				DNode& curNode0 = block.nodes[index0];


				int index = -1;
				//搜索是否存在赋值结点
				int cursizej = curNode0.pioneer.size();
				for (int j = 0; j < cursizej; ++j) {
					if (block.nodes[curNode0.pioneer[j]].ir == IRType::array_assign) {
						//如果找到运算符为数组赋值的;
						if (block.nodes[curNode0.pioneer[j]].oprs[1] == index1) {
							//找到
							index = curNode0.pioneer[j];
							break;
						}
					}
				}

				if (index == -1) {
					//新建结点

					auto find = block.mIR.find(curIR->args[0]);
					if (find != block.mIR.end()) {
						curIR->args[0] = block.nodes[find->second].A[block.nodes[find->second].M];
					}

					int _size = block.nodes.size();

					//更新nodes
					block.nodes.push_back(DNode(curIR->type)); //创建一个结点
					DNode& curNode = block.nodes[_size]; //引用当前结点

					curNode.A.push_back(curIR->args[0]);//加入标记
					curNode.M = 0;	//主标记

					//引用结点
					DNode& curNode0 = block.nodes[index0];
					DNode& curNode1 = block.nodes[index1];

					//添加前驱后继

					//后继
					curNode.oprs[0] = index0;
					curNode.oprs[1] = index1;

					//前驱
					curNode0.pioneer.push_back(_size);
					curNode1.pioneer.push_back(_size);

					//设为根结点
					curNode.isRoot = true;
					curNode0.isRoot = false;
					curNode1.isRoot = false;
				}
				else {
					//已有，添加
					auto find = block.mIR.find(curIR->args[0]);
					if (find != block.mIR.end()) {
						curIR->args[0] = block.nodes[find->second].A[block.nodes[find->second].M];
					}
					DNode& curNode = block.nodes[index]; //引用当前结点

					curNode.A[0] = curIR->args[0];
				}
			}
		}
		
		//block.print();
		
		//根据DAG生成优化四元式
		int length = block.start; //四元式总数，初始化为基本块开始
		while (length <= block.end) {
			//绕过其他四元式
			IRNode* curIR = FEI->ir_list[length]; //当前扫描到的四元式
			if (curIR->type == IRType::assign || curIR->type == IRType::add || curIR->type == IRType::sub || curIR->type == IRType::mult || curIR->type == IRType::div|| curIR->type == IRType::func_call|| curIR->type == IRType::print|| curIR->type == IRType::input|| curIR->type == IRType::array_assign|| curIR->type == IRType::array_use) {
				break;
			}
			else {
				for (int i = 0; i < 3; ++i) {
					if (block.mIR.find(curIR->args[i]) != block.mIR.end()) {
						auto index = block.mIR.find(curIR->args[i]);
						curIR->setArg(i, &block.nodes[index->second].A[block.nodes[index->second].M]);
					}
				}
				length++;
			}
		}

		//扫描哪些结点有用
		for (int i = 0; i < block.nodes.size(); ++i) {
			DNode& curNode = block.nodes[i];	//当前结点
			if (curNode.isRoot == true) {
				if (curNode.A[curNode.M].type == IRAType::int_imm|| curNode.ir == IRType::input||curNode.ir == IRType::array_assign)
					curNode.isuseful = true;
				else if (curNode.A[curNode.M].type == IRAType::var|| curNode.A[curNode.M].type == IRAType::temp) {
					int ok = 0;
					for (int j = 0; j < curNode.A.size(); j++) {
						if (block.mIR[curNode.A[j]] == i) {
							if (curNode.A[j].type == IRAType::var|| curNode.A[curNode.M].type == IRAType::temp) {
								ok = 1;
								break;
							}
						}
					}
					if (curNode.ir == IRType::func_call|| curNode.ir == IRType::print) ok = 1;
					if (ok) {
						curNode.isuseful = true;
						if (curNode.oprs[1] != -1) {
							for (int j = 0; j < curNode.oprs.size(); ++j) {
								block.open(curNode.oprs[j]);
							}
						}
						else if (curNode.oprs[0] != -1) {
							block.open(curNode.oprs[0]);
						}
					}
				}
				else if (curNode.ir == IRType::func_call) {
					curNode.isuseful = true;
					if (curNode.oprs[1] != -1) {
						for (int j = 0; j < curNode.oprs.size(); ++j) {
							block.open(curNode.oprs[j]);
						}
					}
					else if (curNode.oprs[0] != -1) {
						block.open(curNode.oprs[0]);
					}
				}
				else if (curNode.ir == IRType::print) {
					curNode.isuseful = true;
				}
			}
		}

		for (int i = 0; i < block.nodes.size(); ++i) {
			//由小到大逐个扫描结点
			DNode& curNode = block.nodes[i];	//当前结点

			if (curNode.isuseful) {
				//首先做运算
				if (curNode.oprs[0] != -1 && curNode.ir != IRType::print) {
					if (curNode.ir == IRType::array_assign) {
						//替换操作符
						FEI->ir_list[length]->type = curNode.ir;

						//替换操作数
						FEI->ir_list[length]->args[2] = block.nodes[curNode.oprs[1]].A[block.nodes[curNode.oprs[1]].M];
						FEI->ir_list[length]->args[1] = block.nodes[curNode.oprs[0]].A[block.nodes[curNode.oprs[0]].M];
						FEI->ir_list[length]->args[0] = curNode.A[curNode.M];
						
					}
					else if (curNode.ir != IRType::func_call) {
						//替换操作符
						FEI->ir_list[length]->type = curNode.ir;

						//替换操作数
						FEI->ir_list[length]->args[2] = curNode.A[curNode.M];
						FEI->ir_list[length]->args[1] = block.nodes[curNode.oprs[1]].A[block.nodes[curNode.oprs[1]].M];
						FEI->ir_list[length]->args[0] = block.nodes[curNode.oprs[0]].A[block.nodes[curNode.oprs[0]].M];
					}
					else {
						//替换操作符
						FEI->ir_list[length]->type = curNode.ir;

						//替换操作数
						FEI->ir_list[length]->args[1] = curNode.A[curNode.M];
						FEI->ir_list[length]->args[0] = block.nodes[curNode.oprs[0]].A[block.nodes[curNode.oprs[0]].M];

						//以下生成参数调用四元式
						for (int j = 1; j < curNode.oprs.size(); ++j) {
							if (curNode.oprs[j] != -1) {
								FEI->ir_list[++length]->type = IRType::func_param_in;

								FEI->ir_list[length]->args[0] = block.nodes[curNode.oprs[j]].A[block.nodes[curNode.oprs[j]].M];
							}
						}
					}

					//移动到下一个
					length++;
				}
				//然后赋值
				for (int j = 0; j < curNode.A.size(); ++j) {
					//扫描标记
					if (j == curNode.M) continue; //跳过主标记
					if (curNode.A[j].type == IRAType::var || block.isreserve_temp(curNode.A[curNode.M])) {
						//如果有用户变量或保留的临时变量
						if (block.mIR[curNode.A[j]] != i) {
							continue;	//非最新
						}
						else {
							//如果是最新,添加四元式
							FEI->ir_list[length]->type = IRType::assign;

							//当前标记 = 主标记
							FEI->ir_list[length]->args[1] = curNode.A[j];
							FEI->ir_list[length]->args[0] = curNode.A[curNode.M];

							//移动到下一个四元式
							length++;
						}
					}
				}
				//特殊处理
				if (curNode.ir == IRType::input) {
					FEI->ir_list[length]->type = IRType::input;
					FEI->ir_list[length]->args[0] = curNode.A[curNode.M];

					//移动到下一个四元式
					length++;
				}
				else if (curNode.ir == IRType::print) {

					FEI->ir_list[length]->type = IRType::print;

					auto find = block.mIR.find(curNode.A[curNode.M]);
					if (find != block.mIR.end()) {
						FEI->ir_list[length]->args[0] = block.nodes[find->second].A[block.nodes[find->second].M];
					}
					else {
						FEI->ir_list[length]->args[0] = curNode.A[curNode.M];
					}

					//移动到下一个四元式
					length++;
				}
			}

		}

		//删除无用四元式
		while (length <= block.end) {
			IRNode* curIR = FEI->ir_list[length]; //当前扫描到的四元式
			if (curIR->type == IRType::assign || curIR->type == IRType::add || curIR->type == IRType::sub || curIR->type == IRType::mult || curIR->type == IRType::div|| curIR->type == IRType::func_call|| curIR->type == IRType::func_param_in|| curIR->type == IRType::print|| curIR->type == IRType::input|| curIR->type == IRType::array_assign|| curIR->type == IRType::array_use) {
				curIR->type = IRType::null;
			}
			else {
				if (curIR->type == IRType::equal_jump|| curIR->type == IRType::unequal_jump || curIR->type == IRType::ge_jump || curIR->type == IRType::le_jump || curIR->type == IRType::greater_jump|| curIR->type == IRType::less_jump) {
					for (int i = 0; i < 2; ++i) {
						if (block.mIR.find(curIR->args[i]) != block.mIR.end()) {
							auto index = block.mIR.find(curIR->args[i]);
							if (block.nodes[index->second].A[block.nodes[index->second].M].type == IRAType::int_imm && curIR->args[i].type == IRAType::var) {
								continue;
							}
							else curIR->setArg(i, &block.nodes[index->second].A[block.nodes[index->second].M]);
						}
					}
				}
				else {
					for (int i = 0; i < 3; ++i) {
						if (block.mIR.find(curIR->args[i]) != block.mIR.end()) {
							auto index = block.mIR.find(curIR->args[i]);
							curIR->setArg(i, &block.nodes[index->second].A[block.nodes[index->second].M]);
						}
					}
				}
				length++;
			}
			
		}
	}


	//删除全部空四元式
	for (vector<IRNode*>::iterator it = FEI->ir_list.begin(); it != FEI->ir_list.end();) {
		if ((*it)->type == IRType::null) {
			it = FEI->ir_list.erase(it);
		}
		else it++;
	}
	
}

inline void Eblock::open(int i) {
	nodes[i].isuseful = true;
	if (nodes[i].oprs[1] != -1) {
		for (int j = 0; j < nodes[i].oprs.size(); ++j) {
			open(nodes[i].oprs[j]);
		}
	}
	else if (nodes[i].oprs[0] != -1) {
		open(nodes[i].oprs[0]);
	}
}

inline bool Eblock::isreserve_temp(IRArg a)
{
	for (auto it : reserve_temp) {
		if (it == a) {
			return true;
		}
	}
	return false;
}
