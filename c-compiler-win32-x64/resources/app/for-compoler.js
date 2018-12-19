const shell = require('electron').shell
const os = require('os')
const {dialog} = require('electron').remote
const fs = require('fs')
const cp = require('child_process')

const LexState = require('./lex').LexState
const Lex = require('./lex').Lex

const codeViewList = [
    document.getElementById("source-code-view"),
    document.getElementById("token-code-view"),
    document.getElementById("ir-code-view"),
    document.getElementById("better-ir-code-view"),
    document.getElementById("asm-code-view")
];

const lcvc = document.getElementById("left-code-view-container");
const rcvc = document.getElementById("right-code-view-container");

var curP = null;
var compilerFilePath = null;
var curErrorFunc = "";
var spimPath = 'D:/item/spim/QtSpim.exe';
const iniPath = 'option.json';

spimPath = JSON.parse(fs.readFileSync(iniPath)).spimPath;

const setSpimPath = function(str){
    spimPath = str;
    fs.writeFileSync(iniPath, JSON.stringify({
        spimPath : str
    }));
}

const addSourceCodeSpan = function(style, str){
    let span = document.createElement("span");
    span.classList.add(style);
    span.innerHTML = str;
    curP.appendChild(span);
}

let lex = new Lex(function(str, token){
    if(token >= LexState.LCB && token <= LexState.assign){
        addSourceCodeSpan("highlight-symbol" ,str);
    }else if(token >= LexState.int_k && token <= LexState.void_k){
        addSourceCodeSpan("highlight-key-type" ,str);
    }else if(token >= LexState.return_k && token <= LexState.else_k){
        addSourceCodeSpan("highlight-key-s" ,str);
    }else if(token >= LexState.int_const && token <= LexState.char_const){
        addSourceCodeSpan("highlight-const" ,str);
    }else if(token == LexState.identifier){
        addSourceCodeSpan("highlight-id" ,str);
    }
});


const openFileDialog = function(dp, callback){
    dialog.showOpenDialog({
        defaultPath : dp,
        properties:['openFile'],
    },function(res){
        if(res != null){
            callback(res);
        }
        
    });
}

const showCompilerOut = function(json){
    showIRCode(json);
    showTokenCode(json.Token);
    showASMCode(json.ASM);
    if(json["BIR"] != null){
        showBIRCode(json.BIR);
    }
}

const showSourceCode = function(str){
    let codeArr = str.split("\n");
    for(v of codeViewList){
        v.innerHTML = "";
    }
    for(let i = 0; i < codeArr.length; i++){
        handleSourcePerLine(codeArr[i], i+1);
    }
}

const handleSourcePerLine = function(str, line){
    curP = document.createElement("p");
    let lineSpan = document.createElement("div");
    let lineSpace = document.createElement("div");
    lineSpan.classList.add("code-view-line-num");
    lineSpace.classList.add("code-view-line-num-space");
    lineSpan.innerHTML = line;
    curP.appendChild(lineSpan);
    curP.appendChild(lineSpace);
    //console.log("+++++++++++++++++++++++++++++++++++++++++++++++++");
    lex.reset();
    for(let i = 0; i < str.length; i++){
        let ch = str[i];
        if(ch === '\n'){
            ch = ' ';
        }
        lex.lex(ch);
        if(str[i] === ' '){
            let space = document.createElement("div");
            space.innerHTML = " ";
            space.classList.add("code-space");
            curP.appendChild(space);
        }else if(str[i] === '\t'){
            let space = document.createElement("div");
            space.innerHTML = " ";
            space.classList.add("code-t");
            curP.appendChild(space);
        }
    }
   
    document.getElementById("source-code-view").appendChild(curP);
}

const showTokenCode = function(tokenStr){
    let tokens = tokenStr.split(">");
    for(let i = 0; i < tokens.length - 1; i++){
        let code = tokens[i];
        //console.log("fucl" + code);
        curP = document.createElement("p");
       
        let lineSpan = document.createElement("div");
        //let lineSpace = document.createElement("div");
        lineSpan.classList.add("code-view-line-num");
        //lineSpace.classList.add("code-view-line-num-space");
        lineSpan.innerHTML = i+1;
        curP.appendChild(lineSpan);
        //curP.appendChild(lineSpace);
    
        /*let labelSpan = document.createElement("div");
        labelSpan.classList.add("code-label");
        curP.appendChild(labelSpan);

        if(code.hasLabel === true){
            labelSpan.innerHTML = "L" + code.label;
        }*/

        let span = document.createElement("span");
        span.classList.add("highlight-key-type");
        span.innerText= "\<" + code.substr(1) + "\>";
        curP.appendChild(span);

        document.getElementById("token-code-view").appendChild(curP);
    }
}

const showIRCode = function(json){
    document.getElementById("ir-code-view").innerHTML = "";
    //console.log(json);
    let irArr = json.IR;
    for(let i = 0; i < irArr.length; i++){
        let code = irArr[i];

        curP = document.createElement("p");
        let lineSpan = document.createElement("div");
        let lineSpace = document.createElement("div");
        lineSpan.classList.add("code-view-line-num");
        lineSpace.classList.add("code-view-line-num-space");
        lineSpan.innerHTML = i+1;
        curP.appendChild(lineSpan);
        curP.appendChild(lineSpace);
    
        let labelSpan = document.createElement("div");
        labelSpan.classList.add("code-label");
        curP.appendChild(labelSpan);

        if(code.hasLabel === true){
            labelSpan.innerHTML = "L" + code.label;
        }

        let span = document.createElement("span");
        span.classList.add("highlight-key-type");
        span.innerHTML = code.code;
        curP.appendChild(span);

        document.getElementById("ir-code-view").appendChild(curP);
    }
    document.getElementById("error-view").innerHTML = "";
    if(json.hasError){
        lcvc.classList.add("code-view-container-f");
        rcvc.classList.add("code-view-container-f");
        document.getElementById("error-view-container").classList.add("error-view-container-e");
        let eArr = json.ERROR;
        for(let i = 0; i < eArr.length; i++){
            let e = eArr[i];
            if(e.func != 'null' && e.func != curErrorFunc){
                curErrorFunc = e.func;
                curP = document.createElement("p");

                let lineSpan = document.createElement("div");
                lineSpan.classList.add("e-line-span");
                curP.appendChild(lineSpan);
                let contentSpan = document.createElement("span");
                contentSpan.classList.add("e-func-span")
                curP.appendChild(contentSpan);

                contentSpan.innerHTML = "in function \'" + e.func + "\'";
                document.getElementById("error-view").appendChild(curP);
            }
            curP = document.createElement("p");

            let lineSpan = document.createElement("div");
            lineSpan.classList.add("e-line-span");
            if(e["line"] != null){
                lineSpan.innerHTML = e.line + 1;
            }
            curP.appendChild(lineSpan);

            let contentSpan = document.createElement("span");
            contentSpan.classList.add("e-content-span")
            curP.appendChild(contentSpan);
            contentSpan.innerHTML = e.content;

            document.getElementById("error-view").appendChild(curP);
        }
    }
}

const showBIRCode = function(irArr){
    for(let i = 0; i < irArr.length; i++){
        let code = irArr[i];

        curP = document.createElement("p");
        let lineSpan = document.createElement("div");
        let lineSpace = document.createElement("div");
        lineSpan.classList.add("code-view-line-num");
        lineSpace.classList.add("code-view-line-num-space");
        lineSpan.innerHTML = i+1;
        curP.appendChild(lineSpan);
        curP.appendChild(lineSpace);
    
        let labelSpan = document.createElement("div");
        labelSpan.classList.add("code-label");
        curP.appendChild(labelSpan);

        if(code.hasLabel === true){
            labelSpan.innerHTML = "L" + code.label;
        }

        let span = document.createElement("span");
        span.classList.add("highlight-key-type");
        span.innerHTML = code.code;
        curP.appendChild(span);

        document.getElementById("better-ir-code-view").appendChild(curP);
    }
}

const showASMCode = function(asm){
    let line = 1;
    for(let code of asm){
        curP = document.createElement("p");
        let lineSpan = document.createElement("div");
        let lineSpace = document.createElement("div");
        lineSpan.classList.add("code-view-line-num");
        lineSpace.classList.add("code-view-line-num-space");
        lineSpan.innerHTML = line++;
        curP.appendChild(lineSpan);
        curP.appendChild(lineSpace);
    
        /*let labelSpan = document.createElement("div");
        labelSpan.classList.add("code-label");
        curP.appendChild(labelSpan);*/

        if(code.arg != null){
            let op = code.op;
            let leftSpace = document.createElement("span");
            leftSpace.classList.add("asm-left-space");
            curP.appendChild(leftSpace);

            let span = document.createElement("span");
                span.classList.add("highlight-asm-op");
                span.innerHTML = op;
                //span.style.width = "40px";
                curP.appendChild(span);
            let space = document.createElement("div");
            space.classList.add("code-space");
            curP.appendChild(space);

            

            let args = code.arg;
            for(let i = 0; i < args.length; i++){
                if(i !== 0){
                    span = document.createElement("span");
                    span.classList.add("highlight-key-type");
                    span.innerHTML = ', ';
                    curP.appendChild(span);

                    space = document.createElement("div");
                    space.classList.add("code-space");
                    curP.appendChild(space);
                }
                span = document.createElement("span");
                if(i !== args.length - 1){
                    span.classList.add("highlight-asm-arg");
                }else{
                    span.classList.add("highlight-asm-fin-arg");
                }
                
                span.innerHTML = args[i];
                curP.appendChild(span);
            }
        }else{
            let str = code.op;
            if(str[0] === '.'){
                let span = document.createElement("span");
                span.classList.add("highlight-key-type");
                span.innerHTML = str;
                curP.appendChild(span);
            }else if(str[str.length-1] === ':'){
                let span = document.createElement("span");
                span.classList.add("highlight-asm-label");
                span.innerHTML = str;
                curP.appendChild(span);
            }else if(str === 'nop'){
                let leftSpace = document.createElement("span");
                leftSpace.classList.add("asm-left-space");
                curP.appendChild(leftSpace);
                let span = document.createElement("span");
                span.classList.add("highlight-asm-op");
                span.innerHTML = str;
                curP.appendChild(span);
            }else if(str === 'syscall'){
                let leftSpace = document.createElement("span");
                leftSpace.classList.add("asm-left-space");
                curP.appendChild(leftSpace);
                let span = document.createElement("span");
                span.classList.add("highlight-asm-op");
                span.innerHTML = str;
                curP.appendChild(span);
            }else{
                let leftSpace = document.createElement("span");
                leftSpace.classList.add("asm-left-space");
                curP.appendChild(leftSpace);
                let span = document.createElement("span");
                //span.classList.add("highlight-asm-op");
                span.innerHTML = str;
                curP.appendChild(span);
            }
        }
        

        

        document.getElementById("asm-code-view").appendChild(curP);
    }
}

const selectEventHandler = function(e){
    let buttons = this.getElementsByTagName("button");
    //console.log(buttons);
    for(let i = 0; i < 5; i++){
        if(e.target === buttons[i]){
            let codeView = codeViewList[i];
            let father = codeView.parentElement;
            //console.log(father);
            //console.log(codeView);
            father.removeChild(codeView);
            //father.remove(codeView);
            this.appendChild(codeView);
            /*if(father === lcvc){
                rcvc.appendChild(codeView);
            }
            else{
                lcvc.appendChild(codeView);
            }*/

        }
    }
}

let filePath = "";
let fileButton = document.getElementById("file");

fileButton.addEventListener("click", function(){
  openFileDialog("",function(res){
      filePath = res[0];
      compilerFilePath = res[0];
      fs.readFile(filePath, function(err, data){
        if(err){
            return console.error(err);
        }
        console.log(data.toString());
        showSourceCode(data.toString());
        
    });
    
    });
});

document.getElementById("s-button").addEventListener("click", function(){
    openFileDialog("",function(res){
        setSpimPath(res[0]);
        //spimPath = res[0];      
      });
  });

document.getElementById("c-button").addEventListener("click",function(){
    if(compilerFilePath != null){
        if(lcvc.classList.contains("code-view-container-f")){
            lcvc.classList.remove("code-view-container-f");
            rcvc.classList.remove("code-view-container-f");

            document.getElementById("error-view-container").classList.remove("error-view-container-e");
        }
        cp.execFile('CT4.exe', [compilerFilePath, 'output/asm.s', 'true'], function(err, stdout, stderr){
            if(err) console.error(err);
            console.log('stdout',stdout);
            showCompilerOut(JSON.parse(stdout));
            cp.execFile(spimPath, ['output/asm.s'], function(err, stdout, stderr){
                if(err) console.error(err);
                console.log('stdout',stdout);
            });
        });
    }
});

document.getElementById("left-code-view-container").addEventListener("click", selectEventHandler);
document.getElementById("right-code-view-container").addEventListener("click", selectEventHandler);