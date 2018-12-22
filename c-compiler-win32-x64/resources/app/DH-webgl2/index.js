const {createGL, createProgram, initFBO, initABO, createDataTexture, FeedbackABO, akABO} = require('./item')
const Matrix4 = require('./cuon-matrix').Matrix4

class DarkHole{
    constructor({
        canvas = document.createElement("canvas"),
        canvasWidth = window.innerWidth,
        canvasHeight = window.innerHeight,
        radius = window.innerHeight*0.25,
        num = 40000,
        life1 = 15,
        life2 = 35,
        hasFog = true
    } = {}){

        this.domElement = canvas;
        this.domElement.width = canvasWidth;
        this.domElement.height = canvasHeight;

        this.pNum = num;

        this.animID = null;
        

        let gl = createGL(canvas);
        gl.clearColor(0.0, 0.0, 0.0, 1);
        gl.enable(gl.BLEND);
        gl.blendFunc(gl.SRC_ALPHA, gl.ONE_MINUS_SRC_ALPHA);
        this.gl = gl;

        // 黑洞粒子效果
        this.dhpProgram = createProgram(this.gl, document.getElementById("dhpVS2").innerHTML, document.getElementById("dhpFS").innerHTML, ["oPos", "oVelocity"]);
        gl.useProgram(this.dhpProgram);

        let arr = [],
        deg = 2*Math.PI/num;

        for(let i = 0; i < num*4; i+=4){ // x y health 
            arr[i] = radius * Math.sin(i/4*deg);
            arr[i+1] = radius * Math.cos(i/4*deg);
            arr[i+2] = Math.random()*50;
            arr[i+3] = i/4*deg;
        }
        // x,y,restTime,deg
        this.fbABO = new FeedbackABO(gl, new Float32Array(arr), num*16, gl.getAttribLocation(this.dhpProgram, "aPos"), 4, 0, false);

        // vx,vy
        this.vfbABO = new FeedbackABO(gl, num*8, num*8, gl.getAttribLocation(this.dhpProgram, "velocity"), 2, 1, true);

        this.uniforms = [
            {
                loc : gl.getUniformLocation(this.dhpProgram, "time"),
                type : "f",
                val : 0
            },{
                loc : gl.getUniformLocation(this.dhpProgram, "resolution"),
                type : "vec2",
                val : [canvasWidth, canvasHeight]
            },{
                loc : gl.getUniformLocation(this.dhpProgram, "lift1"),
                type : "f",
                val : 25
            },{
                loc : gl.getUniformLocation(this.dhpProgram, "lift2"),
                type : "f",
                val : 25
            },{
                loc : gl.getUniformLocation(this.dhpProgram, "radius"),
                type : "f",
                val : radius
            }

        ];

        // 雾效果
        if(this.hasFog){
            this.fogProgram = createProgram(this.gl, document.getElementById("fogVS").innerHTML, document.getElementById("fogFS").innerHTML);
            let arr2 = [-1,1, -1,-1, 1,1, 1,-1];
            this.fogABO = new akABO(this.gl, new Float32Array(arr2), 2, gl.getAttribLocation(this.fogProgram, "pos"));
            this.fogUniforms = [
                {
                    loc : gl.getUniformLocation(this.fogProgram, "time"),
                    type : "f",
                    val : 0
                },{
                    loc : gl.getUniformLocation(this.fogProgram, "resolution"),
                    type : "vec2",
                    val : [canvasWidth, canvasHeight]
                },{
                    loc : gl.getUniformLocation(this.fogProgram, "radius"),
                    type : "f",
                    val : radius
                }
            ];
        }
        

        // 背景粒子效果

        this.initUniform();
    }

    initUniform(){

        let gl = this.gl;
        gl.useProgram(this.dhpProgram);
        for(let uniform of this.uniforms){
            switch (uniform.type){
                case "f": gl.uniform1f(uniform.loc, uniform.val);break;
                case "vec2": gl.uniform2f(uniform.loc, uniform.val[0], uniform.val[1]);break;
                default: break;
            }
        }
        if(this.hasFog){
            gl.useProgram(this.fogProgram);
            for(let uniform of this.fogUniforms){
                switch (uniform.type){
                    case "f": gl.uniform1f(uniform.loc, uniform.val);break;
                    case "vec2": gl.uniform2f(uniform.loc, uniform.val[0], uniform.val[1]);break;
                    default: break;
                }
            }
        }
        
    }

    update(time){
        time = time/1000.0;
        let gl = this.gl;
        gl.useProgram(this.dhpProgram);
        gl.uniform1f(this.uniforms[0].loc, time);

        if(this.hasFog){
            gl.useProgram(this.fogProgram);
            gl.uniform1f(this.fogUniforms[0].loc, time);
        }
        
    }

    render(time){

        let gl = this.gl;

        this.update(time);

        gl.clear(gl.COLOR_BUFFER_BIT);

        gl.useProgram(this.dhpProgram);
        this.fbABO.begin(gl.POINTS);
        this.vfbABO.begin(gl.POINTS);
        gl.drawArrays(gl.POINTS, 0, this.pNum);
        this.fbABO.end();
        this.vfbABO.end();

        if(this.hasFog){
            gl.useProgram(this.fogProgram);
            this.fogABO.enable();
            gl.drawArrays(gl.TRIANGLE_STRIP, 0, 4);
        }
        

    }

    DOM(){
        return this.domElement;
    }

    play(){
        var that = this;
        function loop(time){
            that.render(time);
            requestAnimationFrame(loop);
        }
        that.animID = requestAnimationFrame(loop);
    }

    pause(){
        if(this.animID){
            cancelAnimationFrame(this.animID);
        }
    }
}
/*
var anim = new DarkHole();
document.body.appendChild(anim.DOM());
anim.play();*/
exports.DarkHole = DarkHole;