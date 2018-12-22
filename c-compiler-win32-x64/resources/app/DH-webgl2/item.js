const createGL = function(canvas){
    
    try{
        var ctx = canvas.getContext("webgl2");
    }catch(error){
        console.log("FUCK");
    }

    return ctx;
}

const unbindArrayBuffer = function(gl){
    gl.bindBuffer(gl.ARRAY_BUFFER, null);
    gl.bufferData(gl.ELEMENT_ARRAY_BUFFER, null);
}

const initABO = function(gl, typeArr, type){

    var buffer = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, buffer);
    gl.bufferData(gl.ARRAY_BUFFER, typeArr, gl.STATIC_DRAW);

    unbindArrayBuffer(gl);

    return {
        buffer : buffer,
        type : type,
        num : num
    };
}

const unbindFrameBuffer = function(gl){
    gl.bindFramebuffer(gl.FRAMEBUFFER, null);
    gl.bindTexture(gl.TEXTURE_2D, null);
    gl.bindRenderbuffer(gl.RENDERBUFFER, null);
}

const initFBO = function(gl, OFFSCREEN_WIDTH, OFFSCREEN_HEIGHT, texture, depthBuffer){
    
    var frameBuffer = gl.createFramebuffer();

    gl.bindFramebuffer(gl.FRAMEBUFFER, frameBuffer);

    if(texture === null){
        texture = gl.createTexture();  
        gl.bindTexture(gl.TEXTURE_2D, texture);
        gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, OFFSCREEN_WIDTH, OFFSCREEN_HEIGHT, 0, gl.RGBA, gl.UNSIGNED_BYTE, null);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
    }
    
    gl.framebufferTexture2D(gl.FRAMEBUFFER, gl.COLOR_ATTACHMENT0, gl.TEXTURE_2D, texture, 0);

    if(depthBuffer === null){
        depthBuffer = gl.createRenderbuffer();
        gl.bindRenderbuffer(gl.RENDERBUFFER, depthBuffer);
        gl.renderbufferStorage(gl.RENDERBUFFER, gl.DEPTH_COMPONENT16, OFFSCREEN_WIDTH, OFFSCREEN_HEIGHT);
    }

    gl.framebufferRenderbuffer(gl.FRAMEBUFFER, gl.DEPTH_ATTACHMENT, gl.RENDERBUFFER, depthBuffer);


    unbindFrameBuffer(gl);

    return {
        frameBuffer : frameBuffer,
        texture : texture,
        depthBuffer : depthBuffer,
        width : OFFSCREEN_WIDTH,
        height : OFFSCREEN_HEIGHT
    };
}

const loadShader = function(gl, shaderStr, type){
    var shader = gl.createShader(type);
    gl.shaderSource(shader, shaderStr);
    gl.compileShader(shader);

    if(!gl.getShaderParameter(shader, gl.COMPILE_STATUS)){
        console.log('Failed to compile shader: ' + gl.getShaderInfoLog(shader));
    }

    return shader;
}

const createProgram = function(gl, VSS, FSS, feedback){
    var vShader = loadShader(gl, VSS, gl.VERTEX_SHADER),
    fShader = loadShader(gl, FSS, gl.FRAGMENT_SHADER),
    program = gl.createProgram();

    gl.attachShader(program, vShader);
    gl.attachShader(program, fShader);

    if(feedback){
        gl.transformFeedbackVaryings(program, feedback, gl.SEPARATE_ATTRIBS);
    }

    gl.linkProgram(program);


    if(!gl.getProgramParameter(program, gl.LINK_STATUS)){

        console.log('Failed to link program: ' + gl.getProgramInfoLog(program));
        gl.deleteProgram(program);
        gl.deleteShader(vShader);
        gl.deleteShader(fShader);

        return null;
    }

    return program;
}

const createDataTexture = function(gl, arr, width, height, type){
    if(type === gl.UNSIGNED_BYTE){
        var tex = gl.createTexture();
        gl.bindTexture(gl.TEXTURE_2D, tex);
        gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, width, height, 0, gl.RGBA, gl.UNSIGNED_BYTE, new Uint8Array(arr));
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
        
        return tex;
    }
}

const createPlaneArr = function(width, height, numWidth, numHeight){

    let arr = [],
    wp = width/numWidth,
    hp = height/numHeight,
    x0 = -width/2,
    y0 = -height/2,
    isEven = true;

    //
    arr[0] = x0;
    arr[1] = 2;
    for(let i = 0; i < numHeight; i++){
        if(isEven){

            // 每行 numWidth*2+2个点, 
            //arr[i*]
            for(let j = 0; j < numWidth; j++){

                //arr[]
            }

            isEven = false;
        }else{
            isEven = true;
        }
    }
}

class FeedbackABO{
    constructor(gl, arrA, arrB, loc, interval, index, isFinal){

        this.gl = gl;

        gl.enableVertexAttribArray(loc);

        this.bufferA = gl.createBuffer();
        this.bufferB = gl.createBuffer();

        gl.bindBuffer(this.gl.ARRAY_BUFFER, this.bufferA);
        gl.bufferData(this.gl.ARRAY_BUFFER, arrA, this.gl.DYNAMIC_COPY);

        gl.bindBuffer(this.gl.ARRAY_BUFFER, this.bufferB);
        gl.bufferData(this.gl.ARRAY_BUFFER, arrB, this.gl.DYNAMIC_COPY);

        this.feedback = gl.createTransformFeedback();
        gl.bindTransformFeedback(gl.TRANSFORM_FEEDBACK, this.feedback);
        //gl.bindTransformFeedback(null);

        this.loc = loc;
        this.interval = interval || 4;
        this.index = index || 0;
        
        this.isFinal = (isFinal === false)?false : true;
    }

    begin(arg){

        let gl = this.gl;
        gl.bindBuffer(gl.ARRAY_BUFFER, this.bufferA);
        gl.vertexAttribPointer(this.loc, this.interval, gl.FLOAT, false, 0, 0);
        gl.bindBufferBase(gl.TRANSFORM_FEEDBACK_BUFFER, this.index, this.bufferB);

        if(this.isFinal){
            gl.beginTransformFeedback(arg);
        }
        
    }
    end(){

        if(this.isFinal){
            this.gl.endTransformFeedback();
           
            for(let i = 0; i <= this.index; i++){
                this.gl.bindBufferBase(this.gl.TRANSFORM_FEEDBACK_BUFFER, i, null);
            }
        }
        
        
        

        let t = this.bufferA;
        this.bufferA = this.bufferB;
        this.bufferB = t;
    }
}

class akABO{
    constructor(gl, typeArr, interval, loc){

        this.gl = gl;
        var buffer = gl.createBuffer();
        gl.bindBuffer(gl.ARRAY_BUFFER, buffer);
        gl.bufferData(gl.ARRAY_BUFFER, typeArr, gl.STATIC_DRAW);

        this.buffer = buffer;
        this.interval = interval;
        this.loc = loc || null;

        if(loc){
            gl.enableVertexAttribArray(loc);
        }
        

        //unbindArrayBuffer(gl);

    }
    enable(){

        //loc = loc || this.loc;
        let gl = this.gl;

        gl.bindBuffer(gl.ARRAY_BUFFER, this.buffer);
        gl.vertexAttribPointer(this.loc, this.interval, gl.FLOAT, false, 0, 0);
    }
}

module.exports = {createGL, createProgram, initFBO, initABO, createDataTexture, FeedbackABO, akABO};