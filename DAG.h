#include "FrontEndInterface.h"

string ops[] = {
	"add", "sub", "mult", "div", "assign"
};

//DAG���
struct DNode {
	bool isRoot; //�Ƿ��Ǹ����
	bool isuseful;	//�Ƿ�����
	IRType ir;	//�����
	int M;	//����ǵ��±�
	vector<IRArg> A;	//���(���������),������
	int oprs[2];  //��̽���±�
	vector<int> pioneer; //ǰ������±�
	DNode(IRType ir) :
		ir(ir){
			oprs[0] = oprs[1] = -1;
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
		for (int i = 1; i >= 0; --i) {
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

//������
struct Eblock{
	int start, end; //�����鿪ʼ�����±�
	vector<DNode> nodes;	//DAG���
	map<IRArg , int> mIR;	//���ڼ�¼ĳ�������������±�
	vector<IRArg> reserve_temp;	//������������ʱ����
	Eblock(int start, int end, vector<IRArg> reserve_temp):
		start(start),end(end),reserve_temp(std::move(reserve_temp)){}
	void open(int i);	//��ĳ�����
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
	void gene_blocks(); //���ɻ�����
	void optimize();	//�Ż�
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
		IRNode* curIR = FEI->ir_list[i]; //��ǰɨ�赽����Ԫʽ

		if (curIR->type == IRType::ret || curIR->type == IRType::func_param_in || curIR->type == IRType::print) {
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
			cout << "shit" << endl;
			curIR->print();
		}
		else if(curIR->label != NULL  ) {
			if (start < i) {
				if (i > 0) {
					end = i - 1;
					blocks.push_back(Eblock(start, end, reserve_temp));
					reserve_temp.clear();
				}
			}
			start = i;
			cout << "fuck" << endl;
			curIR->print();
		}
	}
	if (end < start) {
		cout << "fuck" << endl;
	}
	cout << "fuckyou" << endl;
	for (auto it : blocks) {
		cout << it.start << ' ' << it.end << endl;
		for (auto jt : it.reserve_temp) {
			jt.print();
		}
		cout << endl;
	}
}

inline void DAG::optimize(){
	//ɨ��ÿ��������
	for (auto block : blocks) {

		//����DAGͼ
		for (int i = block.start; i <= block .end; ++i) {
			IRNode* curIR = FEI->ir_list[i]; //��ǰɨ�赽����Ԫʽ
			if (curIR->type == IRType::assign) {
				//����Ǹ�ֵ��Ԫʽ
				auto index = block.mIR.find(curIR->args[0]);	//DAG���Ƿ��Ѵ�����ֵ
				if(index == block.mIR.end()){
					int _size = block.nodes.size();
					//DAG��û��

					//����map
					block.mIR[curIR->args[0]] = _size;
					block.mIR[curIR->args[1]] = _size;
					
					
					//����nodes
					block.nodes.push_back(DNode(IRType::assign)); //����һ�����
					DNode& curNode = block.nodes[_size]; //���õ�ǰ���


					curNode.A.push_back(curIR->args[0]);//������
					curNode.A.push_back(curIR->args[1]);
					
					
					//ȷ�������
					if (curIR->args[1].type == IRAType::var && curIR->args[0].type == IRAType::temp) {
						//�û����� = ��ʱ����ʱ
						curNode.M = curNode.A.size() - 1; //��args[1]��Ϊ�����
					}
					else {
						//Ĭ�����
						curNode.M = curNode.A.size() - 2; //��args[0]��Ϊ�����
					}
					
					//��Ϊ���ڵ�
					curNode.isRoot = true;
				}
				
				else {
					//DAG����
					DNode& curNode = block.nodes[index->second]; //���õ�ǰ���;
					
					//����map
					block.mIR[curIR->args[1]] = index->second;


					int index_arg1 = -1;
					//Ѱ�ұ�����Ƿ���arg1
					int cursize = curNode.A.size();
					for (int j = 0; j < cursize;++j) {
						if (curNode.A[j] == curIR->args[1]) {
							index_arg1 = j;
							break;
						}
					}


					if (index_arg1 == -1) {
						//û���ҵ�,push
						index_arg1 = curNode.A.size();
						curNode.A.push_back(curIR->args[1]);
					}
					
					//���������(if necessary)
					if (curIR->args[1].type == IRAType::var && curIR->args[0].type == IRAType::temp ) {
						//�û����� = ��ʱ����ʱ
						curNode.M = index_arg1; //��args[1]��Ϊ�����
					}
				}
			}
			else if (curIR->type == IRType::add || curIR->type == IRType::sub || curIR->type == IRType::mult || curIR->type == IRType::div) {
				//�����������Ԫʽ
				if (curIR->args[0].type == IRAType::int_imm && curIR->args[1].type == IRAType::int_imm) {
					//��ʽ�ұ�ȫΪ������ֱ�Ӽ��㣬���ո�ֵ��䴦��

					//��Ԫʽ����
					int rst;	//���
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
					curIR->type = IRType::assign;	//����Ϊ��ֵ������
					curIR->setArg(1, &(curIR->args[2])); //��ֵ����λ��
					curIR->setArg(0, rst);	//��ֵ

					//DAG��㴦��(����ɨ��)
					i--;
					continue;
				}
				else {
					//��������������

					auto find0 = block.mIR.find(curIR->args[0]);
					int index0;
					if (find0 == block.mIR.end()) {
						//��㲻����,�½����
						int _size = block.nodes.size();

						//����map
						block.mIR[curIR->args[0]] = _size;

						//����nodes
						block.nodes.push_back(DNode(IRType::assign)); //����һ�����
						DNode& curNode = block.nodes[_size]; //���õ�ǰ���

						curNode.A.push_back(curIR->args[0]);//������
						curNode.M = 0;	//�����

						index0 = _size;	//����indexֵ
					}
					else {
						index0 = find0->second;
					}

					
					auto find1 = block.mIR.find(curIR->args[1]);
					int index1;
					if (find1 == block.mIR.end()) {
						//��㲻����,�½����
						int _size = block.nodes.size();

						//����map
						block.mIR[curIR->args[1]] = _size;

						//����nodes
						block.nodes.push_back(DNode(IRType::assign)); //����һ�����
						DNode& curNode = block.nodes[_size]; //���õ�ǰ���

						curNode.A.push_back(curIR->args[1]);//������
						curNode.M = 0;	//�����

						index1 = _size;	//����indexֵ
					}
					else {
						index1 = find1->second;
					}

					//���ý��
					DNode& curNode0 = block.nodes[index0];


					int index = -1;
					//�����Ƿ������ͬ���
					int cursizej = curNode0.pioneer.size();
					for (int j = 0; j < cursizej; ++j) {
						if (block.nodes[curNode0.pioneer[j]].ir == curIR->type) {
							//����ҵ��������ͬ��;
							for (int q = 0; q < 2; ++q) {
								if (block.nodes[curNode0.pioneer[j]].oprs[q] == index1) {
									//�ҵ�
									index = curNode0.pioneer[j];
									break;
								}
							}
						}
					}

					if(index == -1){
						//��㲻����,�½����
						int _size = block.nodes.size();

						//����map
						block.mIR[curIR->args[2]] = _size;

						//����nodes
						block.nodes.push_back(DNode(curIR->type)); //����һ�����
						DNode& curNode = block.nodes[_size]; //���õ�ǰ���


						curNode.A.push_back(curIR->args[2]);//������
						curNode.M = 0;	//�����

						//���ǰ�����

						//���
						curNode.oprs[0] = index0;
						curNode.oprs[1] = index1;

						//�����������
						DNode& curNode0 = block.nodes[index0];
						DNode& curNode1 = block.nodes[index1];

						//ǰ��
						curNode0.pioneer.push_back(_size);
						curNode1.pioneer.push_back(_size);

						//��Ϊ�����
						curNode.isRoot = true;
						curNode0.isRoot = false;
						curNode1.isRoot = false;
					}
					else {
						//���У����
						DNode& curNode = block.nodes[index]; //���õ�ǰ���

						int index_arg2 = -1;
						//Ѱ�ұ�����Ƿ���arg2
						int cursize = curNode.A.size();
						for (int j = 0; j < cursize; ++j) {
							if (curNode.A[j] == curIR->args[2]) {
								index_arg2 = j;
								break;
							}
						}

						if (index_arg2 == -1) {
							//û���ҵ�,push
							index_arg2 = curNode.A.size();
							curNode.A.push_back(curIR->args[2]);

							//����map
							block.mIR[curIR->args[2]] = index;
						}



						//���������(if necessary)
						if (curIR->args[2].type == IRAType::var && curNode.A[curNode.M].type == IRAType::temp) {
							//�����Ϊ��ʱ����, �����Ϊ�û�����ʱ
							curNode.M = index_arg2; //��args[2]��Ϊ�����
						}

					}
				}
			}
		}
		
		block.print();
		
		//����DAG�����Ż���Ԫʽ
		int length = block.start; //��Ԫʽ��������ʼ��Ϊ�����鿪ʼ
		while (length <= block.end) {
			//�ƹ�������Ԫʽ
			IRNode* curIR = FEI->ir_list[length]; //��ǰɨ�赽����Ԫʽ
			if (curIR->type == IRType::assign || curIR->type == IRType::add || curIR->type == IRType::sub || curIR->type == IRType::mult || curIR->type == IRType::div) {
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

		//ɨ����Щ�������
		for (int i = 0; i < block.nodes.size(); ++i) {
			DNode& curNode = block.nodes[i];	//��ǰ���
			if (curNode.isRoot == true) {
				if (curNode.A[curNode.M].type == IRAType::int_imm)
					curNode.isuseful = true;
				else if (curNode.A[curNode.M].type == IRAType::var||block.isreserve_temp(curNode.A[curNode.M])) {
					int ok = 0;
					for (int j = 0; j < curNode.A.size(); j++) {
						if (block.mIR[curNode.A[j]] == i) {
							if (curNode.A[j].type == IRAType::var||block.isreserve_temp(curNode.A[curNode.M])) {
								ok = 1;
								break;
							}
						}
					}
					if (ok) {
						curNode.isuseful = true;
						if (curNode.oprs[0] != -1) {
							block.open(curNode.oprs[0]);
							block.open(curNode.oprs[1]);
						}
					}
				}
			}
		}

		for (int i = 0; i < block.nodes.size(); ++i) {
			//��С�������ɨ����

			DNode& curNode = block.nodes[i];	//��ǰ���

			if (curNode.isuseful) {
				//����������
				if (curNode.oprs[0] != -1) {

					//�滻������
					FEI->ir_list[length]->type = curNode.ir;

					//�滻������
					FEI->ir_list[length]->args[2] = curNode.A[curNode.M];
					FEI->ir_list[length]->args[1] = block.nodes[curNode.oprs[1]].A[block.nodes[curNode.oprs[1]].M];
					FEI->ir_list[length]->args[0] = block.nodes[curNode.oprs[0]].A[block.nodes[curNode.oprs[0]].M];

					//�ƶ�����һ��
					length++;
				}

				//Ȼ��ֵ
				for (int j = 0; j < curNode.A.size(); ++j) {
					//ɨ����
					if (j == curNode.M) continue; //���������
					if (curNode.A[j].type == IRAType::var|| block.isreserve_temp(curNode.A[curNode.M])) {
						//������û�������������ʱ����
						if (block.mIR[curNode.A[j]] != i) break;	//������
						else {
							//���������,�����Ԫʽ
							FEI->ir_list[length]->type = IRType::assign;

							//��ǰ��� = �����
							FEI->ir_list[length]->args[1] = curNode.A[j];
							FEI->ir_list[length]->args[0] = curNode.A[curNode.M];

							//�ƶ�����һ����Ԫʽ
							length++;
						}
					}
				}
			}

		}

		//ɾ��������Ԫʽ
		while (length <= block.end) {
			IRNode* curIR = FEI->ir_list[length]; //��ǰɨ�赽����Ԫʽ
			if (curIR->type == IRType::assign || curIR->type == IRType::add || curIR->type == IRType::sub || curIR->type == IRType::mult || curIR->type == IRType::div) {
				curIR->type = IRType::null;
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
	}

	//ɾ��ȫ������Ԫʽ
	vector<IRNode*>::iterator it2;
	for (vector<IRNode*>::iterator it = FEI->ir_list.begin(); it != FEI->ir_list.end();) {
		if ((*it)->type == IRType::null) {
			it = FEI->ir_list.erase(it);
		}
		else it++;
	}
	
}

inline void Eblock::open(int i) {
	nodes[i].isuseful = true;
	if (nodes[i].oprs[0] != -1) {
		open(nodes[i].oprs[0]);
		open(nodes[i].oprs[1]);
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
