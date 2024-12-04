
class vector2{
    constructor(options){
        this.x = 0;
        this.y = 0;
        if(options.vec2 !== undefined){
            this.x = options.vec2.x;
            this.y = options.vec2.y;
        }
        else if(options.vector2 !== undefined){
            this.x = options.vector2.x;
            this.y = options.vector2.y;
        }
        if(options.x !== undefined){
            this.x = options.x;
        }
        if(options.y !== undefined){
            this.y = options.y;
        }
    }
};