// 모델간 커플라인 
class base_line{
    constructor(o){
        this.ctx = o.ctx;//canvas context
        this.from_obj = o.parent;//출발지 포트
        this.selected = true;//라인 선택여부, 처음 라인이 생성시 무조건 선택된 상태로 시작
        this.to_obj = null;//도착지 포트
        this.to_position = new vector2({vector2:o.parent.position});//라인이 드래그 될때 마우스 위치
        this.offset = [0, 0];//포트 순서에 따라 라인 그리기 시 이동
        this.user_offset = [0,0,0,0];//사용자에 의해 라인추가 이동
        this.lines = [];//라인의 좌표들
        this.line_path = new Path2D();//라인을 그리기 위한 path
        this.line_over = false;//라인 마우스오버 유무
        this.handler = [];//라인 클릭시 user_offset을 수정하기위한 핸들러
        this.handler_selected_index = -1;//핸들러 선택시 index
    }
    destructor(){
    }
    save(){
        let d = new Object();
        let from_id = this.from_obj.parent.iid;
        if(this.from_obj.port_type == PORT_TYPE.EX_IN_PORT)
            from_id = "this";
        let to_id = this.to_obj.parent.iid;
        if(this.to_obj.port_type == PORT_TYPE.EX_OUT_PORT)
            to_id = "this";
        d.from_obj_info = {iid : from_id, port_name : this.from_obj.name};
        d.to_obj_info = {iid : to_id, port_name : this.to_obj.name};
        d.user_offset = this.user_offset;
        return d;
    }
    on_mousedown(x, y){
        if(this.handler.length > 0){
            this.ctx.save();
            this.ctx.lineWidth = 12;
            for(var i=0;i<this.handler.length;i++){
                if(this.ctx.isPointInStroke(this.handler[i], x, y)){
                    this.handler_selected_index = i;
                    console.log('click pt', i);
                    this.ctx.restore();
                    return 1;
                }
            }
            this.handler_selected_index = -1;
            this.ctx.restore();
        }
        if(_last_selected_obj == null
            || (this.from_obj.parent !== undefined && this.from_obj.parent == _last_selected_obj)
            || (this.to_obj.parent !== undefined && this.to_obj.parent == _last_selected_obj)){
            if(_enabled_line_obj == null){//연결중인 라인이 아니면
                this.ctx.save();
                this.ctx.lineWidth = 8;
                if(this.ctx.isPointInStroke(this.line_path, x, y)){
                    _selected_line_obj = this;//마우스로 라인을 클릭하였다
                    this.ctx.restore();
                    return 1;
                }
                this.ctx.restore();
            }
        }
        return 0;
    }
    on_mousemove(x, y, dx, dy){
        if(this.selected){
            this.to_position.x = x;
            this.to_position.y = y;
        }
        else if(this.handler_selected_index > -1){
            if(this.handler_selected_index % 2 == 0){
                this.user_offset[this.handler_selected_index] += dx;
            }
            else{
                this.user_offset[this.handler_selected_index] += dy;
            }
            if(this.from_obj.port_type == PORT_TYPE.EX_IN_PORT){
                cpd_designer.on_model_changed(this);
            }
            else{
                cpd_designer.on_model_changed(this);
            }
        }
        else if(_selected_line_obj == null){
            this.ctx.save();
            this.ctx.lineWidth = 8;
            if(_enabled_line_obj == null && this.ctx.isPointInStroke(this.line_path, x, y)){//연결중인 라인이 아니고, 마우스커서가 라인위에 있다면
                this.line_over = true;//라인 위에 마우스커서가 있다
            }
            else{
                this.line_over = false;
            }
            this.ctx.restore();
        }
    }
    on_mouseup(x, y){
        if(this.selected){
            this.selected = false;
        }
        else if(this.handler_selected_index > -1){
            this.handler_selected_index = -1;
        }
        return 0;
    }
    // 라인 그리기
    // dir : 시작방향(NONE=0, UP=1,R=2,D=3,L=4)
    // x,y : 시작좌표
    // x2, y2 : 종료좌표
    // end_dir : 종료방향
    draw_sub(dir, x, y, x2, y2, end_dir){
        if((end_dir == DRAW_DIR.NONE || dir == end_dir || dir == DRAW_DIR.LEFT) && (
            (dir == DRAW_DIR.UP && y == y2) ||
            (dir == DRAW_DIR.RIGHT && x == x2) ||
            (dir == DRAW_DIR.DOWN && y == y2) ||
            (dir == DRAW_DIR.LEFT && x == x2)
        )){
            if((x != x2 || y != y2) && dir != end_dir)
                this.lines.push({x:x2, y:y2});
            return dir;
        }
        else{
            if(dir == DRAW_DIR.RIGHT){
                if(y2 > y){
                    dir = DRAW_DIR.DOWN;
                    if(end_dir != DRAW_DIR.NONE && x > x2){
                        y += this.from_obj.parent.height - (this.from_obj.position.y - this.from_obj.parent.position.y) + PORT_ARROW_HEIGHT * (this.from_obj.index + 0.5);
                        if(this.to_obj != null && (this.to_obj.parent.position.y - PORT_ARROW_HEIGHT) < y && x > x2){//타겟이 적당히 밑에 있다면
                            y += (this.to_obj.parent.position.y + this.to_obj.parent.height - y) + (this.to_obj.index + 1) * PORT_ARROW_HEIGHT;
                            if((this.to_obj.parent.position.x + this.to_obj.parent.width + PORT_ARROW_WIDTH * 2) > x){
                                if(this.offset[0] == 0){
                                    this.offset[0] = ((this.to_obj.parent.position.x + this.to_obj.parent.width + PORT_ARROW_WIDTH * (2 + this.from_obj.index)) - x);
                                    this.lines[this.lines.length-1].x += this.offset[0];    
                                }
                                else{
                                    this.offset[0] = ((this.to_obj.parent.position.x + this.to_obj.parent.width + PORT_ARROW_WIDTH * (2 + this.from_obj.index)) - x);
                                }
                                x += this.offset[0];
                            }
                            else{
                                this.lines[this.lines.length-1].x -= this.offset[0];
                                this.offset[0] = 0;
                            }
                        }
                        else{
                            this.lines[this.lines.length-1].x -= this.offset[0];
                            this.offset[0] = 0;
                        }
                        
                    }
                    else{
                        y = y2;
                        this.lines[this.lines.length-1].x -= this.offset[0];
                        this.offset[0] = 0;
                    }
                }
                else{
                    dir = DRAW_DIR.UP;
                    if(end_dir != DRAW_DIR.NONE && x > x2){
                        y -= ((this.from_obj.position.y - this.from_obj.parent.position.y) + PORT_ARROW_HEIGHT * (this.from_obj.index + 1) + 5);
                        if(this.to_obj != null && (this.to_obj.parent.position.y + this.to_obj.parent.height + PORT_ARROW_HEIGHT) > y && x > x2){//타겟이 적당히 왼쪽 위에 있다면
                            let dy = (y - this.to_obj.parent.position.y) + (this.to_obj.index + 1) * PORT_ARROW_HEIGHT;
                            if(dy > 0)
                                y -= dy;
                            if((this.to_obj.parent.position.x + this.to_obj.parent.width + PORT_ARROW_WIDTH * 2) > x){
                                if(this.offset[0] == 0){
                                    this.offset[0] = ((this.to_obj.parent.position.x + this.to_obj.parent.width + PORT_ARROW_WIDTH * (2 + this.from_obj.index)) - x);
                                    this.lines[this.lines.length-1].x += this.offset[0];    
                                }
                                else{
                                    this.offset[0] = ((this.to_obj.parent.position.x + this.to_obj.parent.width + PORT_ARROW_WIDTH * (2 + this.from_obj.index)) - x);
                                }
                                x += this.offset[0];
                            }
                            else{
                                this.lines[this.lines.length-1].x -= this.offset[0];
                                this.offset[0] = 0;
                            }
                        }
                        else{
                            this.lines[this.lines.length-1].x -= this.offset[0];
                            this.offset[0] = 0;
                        }
                        
                    }
                    else{
                        y = y2;
                        this.lines[this.lines.length-1].x -= this.offset[0];
                        this.offset[0] = 0;
                    }
                }
                this.lines.push({x, y});
                return this.draw_sub(dir, x, y, x2, y2, end_dir);
            }
            else if(dir == DRAW_DIR.UP){
                
                if(x2 > x)
                    return this.draw_sub(DRAW_DIR.RIGHT, x2, y, x2, y2, end_dir);
                else{
                    this.lines.push({x:x2, y});
                    return this.draw_sub(DRAW_DIR.LEFT, x2, y, x2, y2, end_dir);
                }
            }
            else if(dir == DRAW_DIR.DOWN){
                
                if(x2 > x)
                    return this.draw_sub(DRAW_DIR.RIGHT, x2, y, x2, y2, end_dir);
                else{
                    this.lines.push({x:x2, y});
                    return this.draw_sub(DRAW_DIR.LEFT, x2, y, x2, y2, end_dir);
                }
            }
            else if(dir == DRAW_DIR.LEFT){
                console.log("LEFT");
            }
        }
    }
    // 라인그리기
    draw(){
        let draw_dir = DRAW_DIR.RIGHT;//라인의 시작은 항상 오른쪽으로 나가기에
        this.handler = [];
        this.ctx.save();
        this.ctx.strokeStyle = this.from_obj.color;
        let x = this.from_obj.position.x + PORT_ARROW_WIDTH;
        let y = this.from_obj.position.y + PORT_ARROW_HEIGHT * 0.5;
        delete this.lines;
        this.lines = [];
        this.lines.push({x,y});
        if(this.from_obj.port_type == PORT_TYPE.EX_IN_PORT){
            x += PORT_ARROW_WIDTH * (this.from_obj.index + 2);
        }
        else{
            x += PORT_ARROW_WIDTH * (this.from_obj.index + 1);
        }
        let x2 = x + this.offset[0];
        this.lines.push({x:x2, y});
        //x += this.offset[0];
        if(this.to_obj == null){// 목적지 포트가 없이 아직 마우스로 드래그 중이면
            this.draw_sub(draw_dir, x, y, this.to_position.x, this.to_position.y, DRAW_DIR.NONE);
        }
        else{
            this.draw_sub(draw_dir, x, y, this.to_obj.position.x - PORT_ARROW_WIDTH * (this.to_obj.index + 2), this.to_obj.position.y + PORT_ARROW_HEIGHT * 0.5, DRAW_DIR.RIGHT);
            x = this.to_obj.position.x;
            y = this.to_obj.position.y + PORT_ARROW_HEIGHT * 0.5;
            this.lines.push({x,y});
        }
        delete this.line_path;
        this.line_path = new Path2D();
        this.lines.forEach((v, idx) =>{//라인의 좌표점으로 path를 생성
            if(idx == 0)
                this.line_path.moveTo(v.x, v.y);
            else{
                if(idx > 0 && idx < 4 && idx < this.lines.length-1){
                    if(idx % 2 == 1){
                        v.x += this.user_offset[idx-1];
                        this.lines[idx+1].x += this.user_offset[idx-1];
                    }
                    else{
                        v.y += this.user_offset[idx-1];
                        this.lines[idx+1].y += this.user_offset[idx-1];
                    }
                }
                if(_selected_line_obj == this && idx > 1 && idx < this.lines.length-1 ){//마우스로 클릭된 라인이라면 라인 위치 이동을 위한 가이드를 그리자
                    var p = new Path2D();
                    p.arc((v.x + this.lines[idx-1].x) * 0.5, (v.y + this.lines[idx-1].y) * 0.5, 8, 0, 2*Math.PI);
                    this.handler.push(p);
                    //this.handler.push(new Path{x:(v.x + this.lines[idx-1].x) * 0.5, y:(v.y + this.lines[idx-1].y) * 0.5});
                }
                
                this.line_path.lineTo(v.x, v.y);
            }
        });
        this.ctx.lineWidth = 2;

        if(this.line_over || _selected_line_obj == this){//라인에 마우스 오버이거나 라인이 선택되었다면 
            this.ctx.lineWidth = 5;//라인 굵게
            this.ctx.setLineDash([1,10]);//라인 모양을 점선으로
            this.ctx.lineCap='round';
        }
        
        if(_last_selected_obj == null
            || (this.from_obj.parent !== undefined && this.from_obj.parent == _last_selected_obj)
            || (this.to_obj !== null && this.to_obj.parent !== undefined && this.to_obj.parent == _last_selected_obj)){
            this.ctx.shadowColor='rgba(80,80,80,0.5)';
            this.ctx.shadowOffsetX = 1;
            this.ctx.shadowOffsetY = 1;
            this.ctx.shadowBlur = 1;
            this.ctx.stroke(this.line_path);
        }
        else{
            this.ctx.lineWidth = 1;
            this.ctx.strokeStyle='rgba(0,0,0,0.3)';
            this.ctx.stroke(this.line_path);
        }
        
        /*
        this.ctx.fillStyle = 'red';
        this.lines.forEach(v=>{
            this.ctx.beginPath();
            this.ctx.arc(v.x, v.y, 5, 0, 2*Math.PI);
            this.ctx.fill();
        });
        */

        this.ctx.fillStyle = this.ctx.strokeStyle;
        this.handler.forEach(v=>{
            this.ctx.beginPath();
            //this.ctx.arc(v.x, v.y, 5, 0, 2*Math.PI);
            this.ctx.fill(v);
        });
        
        this.ctx.restore();
    }
}// end of class base_line