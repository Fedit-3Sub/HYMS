// 포트 클래스
class base_port{
    constructor(o){
        this.parent = o.parent;
        this.ctx = o.ctx;//canvas context
        this.id = o.id || get_random_string();//id == iid , 통합 필요
        this.index = o.idx;
        this.width = PORT_ARROW_WIDTH;
        this.height = PORT_ARROW_HEIGHT;
        
        this.selected = false;
        this.name = o.name;
        this.position = new vector2({});
        this.is_mouseon = 0;  
        this.port_type = o.port_type; 
        this.line = [];
        //if(this.port_type == PORT_TYPE.EX_IN_PORT || this.port_type == PORT_TYPE.OUT_PORT)
            this.color = get_random_color();//o.color || 'rgba(255,128,128,1)';
        //else
            //this.color = 'white';
        console.log(this);
    }
    destructor(){
        this.line.forEach(v=>{
            v.destructor();
        });
    }
    save(){
        let d = new Object();
        d.index = this.index;
        d.name = this.name;
        d.line = [];
        if(this.port_type == PORT_TYPE.EX_IN_PORT || this.port_type == PORT_TYPE.OUT_PORT
            || (this.parent.model_type == MODEL_TYPE.COUPLED
                && this.port_type == PORT_TYPE.IN_PORT)){
            this.line.forEach(v =>{
                d.line.push(v.save());
            });
        }
        return d;
    }
    load(o){
        var line = new base_line({parent:this, ctx:this.ctx});
        if(o.user_offset !== undefined)
            line.user_offset = o.user_offset;
        line.to_obj = o.to_obj;
        this.line.push(line);
        line.to_obj.line.push(line);
    }
    chk_mousepos(x, y){
        if ((x >= (this.position.x)) && 
            (x <= (this.position.x + this.width)) && 
            (y >= (this.position.y)) && 
            (y <= (this.position.y + this.height))) {
            return true;
        }

        return false;
    }
    // 마우스 좌 다운
    on_mousedown(x, y){
        var ret = 0;
        
        if(this.chk_mousepos(x, y)){//포트에서 마우스 다운 상태
            this.selected = true;
            _enabled_line_obj = new base_line({parent:this, ctx:this.ctx});//새로운 커플링을 위한 라인을 생성하자
            this.line.push(_enabled_line_obj);
            ret = 1;
        }
        else if(this.line.length != 0){
            this.line.forEach((v)=>{
                if(v.on_mousedown(x, y) == 1){
                    ret = 1;
                    return;
                }
            });
        }
        return ret;
    }
    // 마우스 이동
    on_mousemove(x, y, dx, dy){
        if(this.selected){
            _enabled_line_obj.on_mousemove(x, y, dx, dy);
        }
        else if(this.chk_mousepos(x, y)){// 마우스 커서가 포트위에 있다
            if(_enabled_line_obj != null){// 커플링을 연결중이면
                if((this.parent.is_root_model == false && this.port_type == PORT_TYPE.OUT_PORT)
                    || this.port_type == PORT_TYPE.EX_IN_PORT){//도착지로 연결이 불가능한 포트이면
                    this.is_mouseon = 2;
                }
                else{
                    _enabled_line_obj.to_obj = this;//커플링 중인 라인의 도착지를 현재 포트로 설정
                    this.is_mouseon = 1;
                }
            }
            else if(this.port_type == PORT_TYPE.OUT_PORT || this.port_type == PORT_TYPE.EX_IN_PORT){
                this.is_mouseon = 1;
            }
        }
        else{// 마우스 커서가 포트위에 없다
            if(_enabled_line_obj != null && _enabled_line_obj.to_obj == this)// 커플링 중인 라인이 현재 포트이면 
                _enabled_line_obj.to_obj = null;// 커플링 중인 도착포트를 삭제
            this.is_mouseon = 0;
            if(this.line.length != 0 && (this.port_type == PORT_TYPE.OUT_PORT || this.port_type == PORT_TYPE.EX_IN_PORT))
                this.line.forEach(v => v.on_mousemove(x, y, dx, dy));
        }
    }
    // 마우스 좌 업
    on_mouseup(x, y){
        if(this.selected){
            if(_enabled_line_obj.on_mouseup(x, y) == 0){
                if(_enabled_line_obj.to_obj == null){// 커플링 중인 라인의 도착포트가 없다면
                    delete this.line[this.line.length-1];
                    this.line.pop();
                }
                else{
                    cpd_designer.on_model_changed(this);
                    let is_dup = false;
                    _enabled_line_obj.to_obj.line.forEach(v=>{// 중복으로 연결되었는지 확인
                        // 중복검사
                        if(v.from_obj == this){
                            is_dup = true;
                            console.log('중복');
                            return;
                        }
                    });
                    if(!is_dup){//중복이 아니면
                        _enabled_line_obj.to_obj.line.push(_enabled_line_obj);
                        if(this.port_type == PORT_TYPE.EX_IN_PORT){
                            this.name = _enabled_line_obj.to_obj.name;
                        }
                        else if((_enabled_line_obj.to_obj.parent.is_root_model && _enabled_line_obj.to_obj.port_type == PORT_TYPE.OUT_PORT) || _enabled_line_obj.to_obj.port_type == PORT_TYPE.EX_OUT_PORT){
                            _enabled_line_obj.to_obj.name = this.name;
                        }
                    }
                    else{//중복이면 
                        delete this.line[this.line.length-1];
                        this.line.pop();
                    }
                }
                _enabled_line_obj = null;//커플링 중인 라인 객체 해제
            }
            this.selected = false;
        }
        if(this.port_type == PORT_TYPE.OUT_PORT || this.port_type == PORT_TYPE.EX_IN_PORT){
            this.line.forEach(v=>v.on_mouseup(x, y));//포트가 출발지이면 연결된 라인에게도 이벤트 활성화
        }
    }
    delete_obj(){
        this.line.forEach(v=>{//연결된 라인들 모두 삭제
            if(this.port_type == PORT_TYPE.OUT_PORT || this.port_type == PORT_TYPE.EX_IN_PORT){
                v.to_obj.delete_line(v);
            }
            else{
                v.from_obj.delete_line(v);
            }
        });
    }
    // 연결된 라인 삭제
    delete_line(line){
        let idx = this.line.indexOf(line);
        if(idx > -1){
            this.line.splice(idx, 1);
            if(this.parent.parent != null){
                cpd_designer.on_model_changed(this);
            }
        }
    }
    // 화면에 그리기
    draw(x, y){
        switch(this.port_type){
            case PORT_TYPE.IN_PORT:
                if(this.parent.is_root_model)
                    this.draw_exin(x, y);
                else
                    this.draw_in(x, y);
                break;
            case PORT_TYPE.OUT_PORT:
                if(this.parent.is_root_model)
                    this.draw_exout(x, y);
                else
                    this.draw_out(x, y);
                break;
            case PORT_TYPE.EX_IN_PORT:
                this.draw_exin(x, y); break;
            case PORT_TYPE.EX_OUT_PORT:
                this.draw_exout(x, y); break;
        }
    }
    // external input port 그리기
    draw_exin(x, y){
        let _this = this;
        if(this.line.length != 0){
            this.line.forEach(v => {
                if(v.from_obj.id == _this.id){
                    v.draw();
                }
            });
        }
        this.ctx.save();
        this.position.x = x;
        this.position.y = y;
        this.ctx.translate(x, y);
        if(this.is_mouseon == 1 || this.selected || this.line.length > 0){
            if(this.line.length > 0){
                this.ctx.fillStyle = this.line[0].from_obj.color;
            }
            else
                this.ctx.fillStyle = this.color;
        }
        else if(this.is_mouseon == 2){
            this.ctx.fillStyle='rgba(255,0,0,1)';
        }
        else
            this.ctx.fillStyle='white';

        this.ctx.fill(arrow_obj);
        this.ctx.stroke(arrow_obj);
        this.ctx.translate(0, -PORT_ARROW_HEIGHT);
        this.ctx.fillStyle = 'rgba(47, 155, 71, 1)';
        this.ctx.fillText(this.name, 5, PORT_ARROW_HEIGHT * 0.5);
        this.ctx.restore();
        
    }
    // input port 그리기
    draw_in(x, y){
        //console.log(this.line, this.id);
        
        this.ctx.save();
        this.position.x = x;
        this.position.y = y;
        this.ctx.translate(x, y);
        if(this.is_mouseon == 1 || this.selected || (this.line !== undefined && this.line.length > 0)){
            if(this.line.length > 0){
                this.ctx.fillStyle = this.line[0].from_obj.color;
            }
            else
                this.ctx.fillStyle = this.color;
        }
        else if(this.is_mouseon == 2){
            this.ctx.fillStyle='rgba(255,0,0,1)';
        }
        else
            this.ctx.fillStyle='white';

        this.ctx.fill(arrow_obj);
        this.ctx.stroke(arrow_obj);
        this.ctx.fillStyle = 'rgba(47, 155, 71, 1)';
        this.ctx.fillText(this.name, PORT_ARROW_WIDTH + 5, PORT_ARROW_HEIGHT * 0.5);
        this.ctx.restore();
    }
    // external out port 그리기
    draw_exout(x, y){
        //console.log(this.line, this.id);
        this.ctx.save();
        this.position.x = x;
        this.position.y = y;
        this.ctx.translate(x, y);
        this.ctx.textAlign='right';
        if(this.is_mouseon == 1 || this.selected || this.line.length > 0){
            if(this.line.length > 0){
                this.ctx.fillStyle = this.line[0].from_obj.color;
            }
            else
                this.ctx.fillStyle = this.color;
        }
        else if(this.is_mouseon == 2){
            this.ctx.fillStyle='rgba(255,0,0,1)';
        }
        else
            this.ctx.fillStyle='white';
        this.ctx.fill(arrow_obj);
        this.ctx.stroke(arrow_obj);
        this.ctx.translate(0, -PORT_ARROW_HEIGHT);
        this.ctx.fillStyle = 'rgba(65,55,147,1)';
        this.ctx.fillText(this.name, 5, PORT_ARROW_HEIGHT * 0.5);
        this.ctx.restore();
        
    }
    // out port 그리기
    draw_out(x, y){
        this.ctx.save();
        this.position.x = x;
        this.position.y = y;
        this.ctx.translate(x, y);
        this.ctx.textAlign='right';
        if(this.is_mouseon == 1 || this.selected || (this.line !== undefined && this.line.length != 0)){
            this.ctx.fillStyle = this.color;
        }
        else if(this.is_mouseon == 2){
            this.ctx.fillStyle='rgba(255,0,0,1)';
        }
        else{
            this.ctx.fillStyle = 'white';
        }
        this.ctx.fill(arrow_obj);
        this.ctx.stroke(arrow_obj);
        this.ctx.fillStyle = 'rgba(65,55,147,1)';
        this.ctx.fillText(this.name, -5, PORT_ARROW_HEIGHT * 0.5);
        this.ctx.restore();
       // console.log(this.line, this.id);
        if(!this.parent.is_root_model && this.line !== undefined && this.line.length != 0){
            this.line.forEach(v =>{
                
                if(v.from_obj.id == this.id)
                    v.draw();
            });
        }
        
    }
} // end of class base_port