const LexState = {
    Start : 0, 
	ID : 1, 
	digit_1 : 2, digit_2 : 3, 
	int_1 : 4, int_2 : 4 , int_3 : 4,
	if_2 : 7,
	char_1 : 8, char_2 : 4, char_3 : 4, char_4 : 4,
	float_1 : 12, float_2 : 4, float_3 : 4, float_4 : 4, float_5 : 4,
	for_2 : 17, for_3 : 4,
	void_1 : 19, void_2 : 4, void_3 : 4, void_4 : 4,
	return_1 : 23, return_2 : 4, return_3 : 4, return_4 : 4, return_5 : 4, return_6 : 4,
	while_1 : 29, while_2 : 4, while_3 : 4, while_4 : 4, while_5 : 4,
	bool_1 : 34, bool_2 : 4, bool_3 : 4, bool_4 : 4,
	else_1 : 38, else_2 : 4, else_3 : 4, else_4 : 4,
	char_const_1 : 42, char_const_2 : 4, char_const_3 : 4,
	equal_1 : 44,
	greater_1 : 4,
	less_1 : 4,
	logic_or_1 : 4,
	logic_and_1 : 4,

	// fin
	int_const : 100, float_const : 4, char_const : 4, 
	int_k : 4, float_k : 4, char_k : 4, bool_k : 4, void_k : 4,
	return_k : 4, for_k : 4, while_k : 4, if_k : 4, else_k : 4,
	identifier : 4,
	
	//120 { } ( ) . ; , 
	LCB : 120, RCB : 4, LB : 4, RB : 4, LSB : 4, RSB : 4, dot : 4, semicolon : 4, comma : 4,  
	// * / + -
	multiply : 129, divide : 4, add : 4, substract : 4,
	logic_or : 4, logic_and : 4,
	// . >: <: > < :
	double_equal : 135, unequal : 4,greater_equal : 4, less_equal : 4, greater : 4, less : 4, assign : 4,

	end : 200
	

};
let n = 0;
for( s in LexState){
	//console.log(n);
	LexState[s] = n++;
	
}
Object.freeze(LexState);

class Lex{

	constructor(df){
        this.curState = LexState.Start;
        this.table = [];
        this.funcTable = [];
		this.defaultFunc = df;
		this.buf = "";
		this.initTable();
		this.initFunc(df);
	}
	initTable(){
		for(let i = LexState.Start; i <= LexState.assign; i++){
			this.table[i] = [];
		}

		this._meetDigit(LexState.Start, LexState.digit_1);
		this._meetLetter(LexState.Start, LexState.ID);
		this.table[LexState.Start]['i'] = LexState.int_1;
		this.table[0]['c'] = LexState.char_1;
		this.table[0]['f'] = LexState.float_1;
		this.table[0]['r'] = LexState.return_1;
		this.table[0]['b'] = LexState.bool_1;
		this.table[0]['w'] = LexState.while_1;
		this.table[0]['v'] = LexState.void_1;
		this.table[0]['e'] = LexState.else_1;

		this.table[0]['\''] = LexState.char_const_1;

		this.table[0][' '] = LexState.Start;

		this.table[0]['{'] = LexState.LCB;
		this.table[0]['}'] = LexState.RCB;
		this.table[0]['('] = LexState.LB;
		this.table[0][')'] = LexState.RB;
		this.table[0]['['] = LexState.LSB;
		this.table[0][']'] = LexState.RSB;
		this.table[0]['.'] = LexState.dot;
		this.table[0][';'] = LexState.semicolon;
		this.table[0][','] = LexState.comma;
		this.table[0]['='] = LexState.equal_1;

		this.table[0]['>'] = LexState.greater_1;
		this.table[0]['<'] = LexState.less_1;

		this.table[0]['&'] = LexState.logic_and_1;
		this.table[0]['|'] = LexState.logic_or_1;

		this.table[0]['*'] = LexState.multiply;
		this.table[0]['/'] = LexState.divide;
		this.table[0]['+'] = LexState.add;
		this.table[0]['-'] = LexState.substract;


		// key
		for(let j = LexState.int_1; j <= LexState.else_4; j++){
			for(let i = 'a'.charCodeAt(); i <= 'z'.charCodeAt(); i++){
				this.table[j][String.fromCharCode(i)] = LexState.ID;
			}
			for(let i = 'A'.charCodeAt(); i <= 'Z'.charCodeAt(); i++){
				this.table[j][String.fromCharCode(i)] = LexState.ID;
			}
			for(let i = '0'.charCodeAt(); i <= '9'.charCodeAt(); i++){
				this.table[j][String.fromCharCode(i)] = LexState.ID;
			}
			for(let i = ' '.charCodeAt(); i <= '/'.charCodeAt(); i++){
				this.table[j][String.fromCharCode(i)] = LexState.identifier;
			}
			for(let i = ':'.charCodeAt(); i <= '@'.charCodeAt(); i++){
				this.table[j][String.fromCharCode(i)] = LexState.identifier;
			}
			for(let i = '['.charCodeAt(); i <= '`'.charCodeAt(); i++){
				this.table[j][String.fromCharCode(i)] = LexState.identifier;
			}
			for(let i = '{'.charCodeAt(); i <= '~'.charCodeAt(); i++){
				this.table[j][String.fromCharCode(i)] = LexState.identifier;
			}
		}

		// int_k

		this._setK(LexState.int_1, "nt", LexState.int_k);

		// float_k

		this._setK(LexState.float_1, "loat", LexState.float_k);

		// char_k
		this._setK(LexState.char_1, "har", LexState.char_k);
		// bool_k
		this._setK(LexState.bool_1, "ool", LexState.bool_k);

		this._setK(LexState.return_1, "eturn", LexState.return_k);

		this._setK(LexState.while_1, "hile", LexState.while_k);

		this._setK(LexState.void_1, "oid", LexState.void_k);

		this._setK(LexState.else_1, "lse", LexState.else_k);

		this.table[LexState.int_1]['f'] = LexState.if_2;
		this._setKTI(LexState.if_2, LexState.if_k);
		this.table[LexState.float_1]['o'] = LexState.for_2;
		this.table[LexState.for_2]['r'] = LexState.for_3;
		this._setKTI(LexState.for_3, LexState.for_k);

		//
		this._setKTI(LexState.ID, LexState.identifier);
		this._meetLetter(LexState.ID, LexState.ID);
		this._meetDigit(LexState.ID, LexState.ID);
		//

		this._meetDigit(LexState.digit_1, LexState.digit_1);
		this._meetDigit(LexState.digit_2, LexState.digit_2);
		this._meetLetter(LexState.digit_1, LexState.error);
		this._meetLetter(LexState.digit_2, LexState.error);
		this._setKTI(LexState.digit_1, LexState.int_const);
		this.table[LexState.digit_1]['.'] = LexState.digit_2;
		this._setKTI(LexState.digit_2, LexState.float_const);

		// char_const
		for (let i = 0; i < 127; i++) {
			this.table[LexState.char_const_1][String.fromCharCode(i)] = LexState.char_const_3;
			this.table[LexState.char_const_2][String.fromCharCode(i)] = LexState.char_const_3;
		}
		this.table[LexState.char_const_1]['\\'] = LexState.char_const_2;
		this.table[LexState.char_const_3]['\''] = LexState.char_const;
		// =
		
		for (let i = ' '.charCodeAt(); i < '~'.charCodeAt(); i++) {
			this.table[LexState.equal_1][String.fromCharCode(i)] = LexState.assign;
			this.table[LexState.less_1][String.fromCharCode(i)] = LexState.less;
			this.table[LexState.greater_1][String.fromCharCode(i)] = LexState.greater;
		}
		
		this.table[LexState.equal_1]['='] = LexState.double_equal;
		this.table[LexState.less_1]['='] = LexState.less_equal;
		this.table[LexState.greater_1]['='] = LexState.greater_equal;

		this.table[LexState.logic_and_1]['&'] = LexState.logic_and;
		this.table[LexState.logic_or_1]['|'] = LexState.logic_or;

	}
	initFunc(df){
		for(let i = LexState.int_const; i <= LexState.assign; i++){
			this.table[i] = df;
		}
	}
	setDeaultFunc(func){
		/*for(i of LexState){
			
		}*/
	}
	setFunc(s, func){
		this.funcTable[s] = func;
	}
	_meetDigit(s, f){
        for(let i = '0'.charCodeAt(); i <= '9'.charCodeAt(); i++){
            this.table[s][String.fromCharCode(i)] = f;
        }
	}
	_meetLetter(s, f){
        for (let i = 'a'.charCodeAt(); i <= 'z'.charCodeAt(); i++)
			this.table[s][String.fromCharCode(i)] = f;
	    for (let i = 'A'.charCodeAt(); i <= 'Z'.charCodeAt(); i++)
			this.table[s][String.fromCharCode(i)] = f;
	}
	_setKTI(s, f){
        for (let i = ' '.charCodeAt(); i <= '/'.charCodeAt(); i++)
			this.table[s][String.fromCharCode(i)] = f;
        for (let i = ':'.charCodeAt(); i <= '@'.charCodeAt(); i++)
			this.table[s][String.fromCharCode(i)] = f;
        for (let i = '['.charCodeAt(); i <= '`'.charCodeAt(); i++)
			this.table[s][String.fromCharCode(i)] = f;
        for (let i = '{'.charCodeAt(); i <= '~'.charCodeAt(); i++)
			this.table[s][String.fromCharCode(i)] = f;
            
	}
	_setK(k_1, str, k){
        let len = str.length;
        for(let i = 0; i < len; i++){
            this.table[k_1 + i][str[i]] = k_1 + i + 1;
        }
        this._setKTI(k_1 + len, k);
	}
	reset(){
		this.buf = "";
		this.curState = LexState.Start;
	}
	lex(ch){
		/*if(this.table[this.curState] === undefined){
			console.log(this.curState);
		}*/
		if(ch === '\t')ch = ' ';
		/*console.log(this.curState);
		console.log("+");
		console.log(ch);*/
		//console.log("!!!!!!!!!!!!! " + ch + " !!!!!!!");
		let res = this.table[this.curState][ch];
		if(res >= LexState.int_const && res <= LexState.end){
			if ((res < LexState.LCB && res !== LexState.char_const) || res >= LexState.greater) {
				
				this.curState = LexState.Start;
				this.defaultFunc(this.buf, res);
				//console.log("-----------------------------------");
				//console.log(this.buf + " " + ch);
				this.buf = "";

				this.lex(ch);
			}
			else {
				this.buf += ch;
				
				this.curState = LexState.Start;
				this.defaultFunc(this.buf, res);
				//console.log(this.buf + " " + ch);
				this.buf = "";
				
			}
			/*console.log("this.curState");
			console.log(this.curState);*/
			/*let f = this.funcTable[res];
			if(f != null){
				f(res);
			}*/
			//console.log(this.buf);
			
		}else{
			/*if(ch != ' '){
				this.buf += ch;
			}*/
			this.buf += ch;
			this.curState = res;
		}
		/*console.log("=");
		console.log(this.curState);
		console.log("-------------------------------------------");*/

	}
}



exports.LexState = LexState;
exports.Lex = Lex;