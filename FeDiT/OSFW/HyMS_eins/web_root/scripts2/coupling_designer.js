// 포트 연결을 위해 드래그 중인 라인
let _enabled_line_obj = null;
// 마우스 클릭으로 선택된 라인
let _selected_line_obj = null;
// 마지막으로 클릭된 모델
let _last_selected_obj = null;
// 마지막으로 클릭된 포트
let _last_selected_port = null;
// 모델 디자이너에서 수정하는 최상위 모델
let _root_model = null;

class coupling_designer{
    constructor(o){
        this.ctx = o.ctx;//canvas context
        this.activated_model = new base_model({parent:this,ctx:o.ctx,is_root_model:true});//최상위 루트 모델
        _root_model = this.activated_model;
        this.scale_ratio = {x:1.0, y:1.0};
        this.canvas_offset = new vector2({});
        this.hist_mouse_pos = new vector2({});
        this.cpd_pane_size = new vector2({x:screenRect.width-3, y:screenRect.height-3});
        this.position = new vector2({});
        this.on_model_changed_proc = o.on_model_changed_proc || null;
    }
    on_model_changed(obj){
        if(this.on_model_changed_proc != null){
            this.on_model_changed_proc(obj);
        }
    }
    save(filename){
        if(_root_model == null){
            return {};
        }
        let json_data = _root_model.save();
        
        if(filename !== undefined && filename != ''){
            let json = JSON.stringify(json_data, null, '\t');
            make_text_file(json, filename);
        }
            
        console.log('save', json_data);
        return json_data;
    }
    
    // 초기화
    clear_designer(){
        init_graph();
        
        _root_model.destructor();
        this.activated_model = new base_model({parent:this,ctx:this.ctx,is_root_model:true});//최상위 모델 더미 생성
        _root_model = this.activated_model;
        _selected_line_obj = null;// 선택된 라인 객체
        _enabled_line_obj = null;// 시작 포트클릭후 라인이 그려지고 있는 상태의 라인 객체, 목적포트가 미연결된 상태
        _last_selected_obj = null;// 마지막 선택되었던 객체
        _last_selected_port = null;// 마지막 선택되었던 포트
        this.scale_ratio = {x:1.0, y:1.0};// 확대축소 비율
        this.canvas_offset = new vector2({});//켄버스 이동량
        this.cpd_pane_size = new vector2({x:screenRect.width-3, y:screenRect.height-3});
        update_tree();
        unload_model_on_server();
    }
    set_scale(r){
        scale_ratio = r;
    }
    delete_model(m){
        m.delete_obj();
        let midx = this.activated_model.models.indexOf(m);
        if(midx > -1){
            this.activated_model.models.splice(midx, 1);
        }
    }
    delete_port(p){
        console.log(p.port_type);
        p.delete_obj();
        if(p.port_type == PORT_TYPE.EX_IN_PORT || p.port_type == PORT_TYPE.IN_PORT){
            let midx = this.activated_model.in_port.indexOf(p);
            if(midx > -1){
                this.activated_model.in_port.splice(midx, 1);
            }
        }
        else if(p.port_type == PORT_TYPE.EX_OUT_PORT || p.port_type == PORT_TYPE.OUT_PORT){
            let midx = this.activated_model.out_port.indexOf(p);
            if(midx > -1){
                this.activated_model.out_port.splice(midx, 1);
            }
        }
    }
    
    on_keydown(e){
        if(e.key == 'Delete'){
            if(_selected_line_obj != null){//선택된 라인이 있다면
                _selected_line_obj.from_obj.delete_line(_selected_line_obj);
                _selected_line_obj.to_obj.delete_line(_selected_line_obj);
                _selected_line_obj = null;
            }
            else if(_last_selected_obj != null){//선택된 모델이 있다면
                if(confirm('[' + _last_selected_obj.model_name+'] 을 정말 삭제하시겠습니까?')){
                    this.delete_model(_last_selected_obj);
                    _last_selected_obj = null;
                    update_tree();
                }
            }
            else if(_last_selected_port != null){//선택된 포트가 있다면
                if(confirm('[' + _last_selected_port.name+'] 을 정말 삭제하시겠습니까?')){
                    this.delete_port(_last_selected_port);
                    _last_selected_port = null;
                }
            }
        }
        
    }
    on_mousewheel(e){
        var x = e.pageX - screenRect.x;
        var y = e.pageY - screenRect.y;
        var x2 = (x - this.canvas_offset.x) * this.scale_ratio.x;
        var y2 = (y - this.canvas_offset.y) * this.scale_ratio.y;
        
        var zoom = e.wheelDelta>0?0.05:-0.05;
        this.scale_ratio.x += zoom;
        this.scale_ratio.y += zoom;
        if(this.scale_ratio.x < 0.25)
            this.scale_ratio.x = 0.25;
        else if(this.scale_ratio.x > 1.5)
            this.scale_ratio.x = 1.5;
        if(this.scale_ratio.y < 0.25)
            this.scale_ratio.y = 0.25;
        else if(this.scale_ratio.y > 1.5)
            this.scale_ratio.y = 1.5;
        var x3 = (x - this.canvas_offset.x) * this.scale_ratio.x;
        var y3 = (y - this.canvas_offset.y) * this.scale_ratio.y;
        this.canvas_offset.x -= (x3 - x2) / this.scale_ratio.x;
        this.canvas_offset.y -= (y3 - y2) / this.scale_ratio.y;
    }
    on_mousedbclick(e){
        var x = e.pageX - screenRect.x;
        var y = e.pageY - screenRect.y;
        var ret = false;
        x -= this.canvas_offset.x;
        y -= this.canvas_offset.y;
        x /= this.scale_ratio.x;
        y /= this.scale_ratio.y;

        for(var i=0;i<this.activated_model.models.length;i++){
            if(this.activated_model.models[i].on_mousedbclick(x, y) == 1){
                ret = true;
                break;
            }
        }
    }
    on_mousedown(e){
        var x = e.pageX - screenRect.x;
        var y = e.pageY - screenRect.y;
        this.hist_mouse_pos.x = x;
        this.hist_mouse_pos.y = y;
        var ret = false;
        
        x -= this.canvas_offset.x;
        y -= this.canvas_offset.y;
        x /= this.scale_ratio.x;
        y /= this.scale_ratio.y;
        
        if(e.buttons == 1 || e.button == 2){
            // 마우스 클릭으로 선택된 라인
            if(_selected_line_obj != null){
                if(_selected_line_obj.on_mousedown(x, y) == 1){
                    ret = true;
                    return true;
                }
            }
            // 마지막으로 클릭된 모델
            if(_last_selected_obj != null){
                if(_last_selected_obj.on_mousedown(x, y) == 1){
                    ret = true;
                    return true;
                }
            }

            for(var i=0;i<this.activated_model.models.length;i++){
                if(this.activated_model.models[i].on_mousedown(x, y) == 1){
                    ret = true;
                    set_param_to_form(this.activated_model.models[i]);
                    break;
                }
            }
            for(var i=0;i<this.activated_model.in_port.length;i++){
                if(this.activated_model.in_port[i].on_mousedown(x, y) == 1){
                    ret = true;
                    _last_selected_port = this.activated_model.in_port[i];
                    break;
                }
            }
            for(var i=0;i<this.activated_model.out_port.length;i++){
                if(this.activated_model.out_port[i].on_mousedown(x, y) == 1){
                    ret = true;
                    _last_selected_port = this.activated_model.out_port[i];
                    break;
                }
            }
            if(!ret){// 마우스 클릭시 아무것도 선택된 객체가 없다면
                set_param_to_form(null);
                _enabled_line_obj = null;
                _selected_line_obj = null;
                _last_selected_obj = null;
                _last_selected_port = null;
            }
        }
        return ret;
    }
    on_mousemove(e){
        var x = e.pageX - screenRect.x;
        var y = e.pageY - screenRect.y;
        var dx = x - this.hist_mouse_pos.x;
        var dy = y - this.hist_mouse_pos.y;
        this.hist_mouse_pos.x = x;
        this.hist_mouse_pos.y = y;
        
        if(e.buttons == 0 || e.buttons == 1){
            x -= this.canvas_offset.x;
            y -= this.canvas_offset.y;
            x /= this.scale_ratio.x;
            y /= this.scale_ratio.y;
            this.activated_model.models.forEach(m => {
                m.on_mousemove(x, y, dx, dy);
            });
            this.activated_model.in_port.forEach(v => v.on_mousemove(x, y, dx, dy));
            this.activated_model.out_port.forEach(v => v.on_mousemove(x, y, dx, dy));

        }
        else if(e.buttons == 4){//scroll
            this.canvas_offset.x += dx;
            this.canvas_offset.y += dy;
        }
    }
    on_mouseup(e){
        var x = e.pageX - screenRect.x;
        var y = e.pageY - screenRect.y;
        
        
        x -= this.canvas_offset.x;
        y -= this.canvas_offset.y;
        x /= this.scale_ratio.x;
        y /= this.scale_ratio.y;
        this.activated_model.models.forEach(m => {
            m.on_mouseup(x, y);
        });
        this.activated_model.in_port.forEach(v => v.on_mouseup(x, y));
        this.activated_model.out_port.forEach(v => v.on_mouseup(x, y));
    }
    on_mouseout(e){
        this.activated_model.models.forEach(m => {
            m.on_mouseup(0, 0);
        });
        this.activated_model.in_port.forEach(v => v.on_mouseup(0, 0));
        this.activated_model.out_port.forEach(v => v.on_mouseup(0, 0));
    }
    draw(){
        this.ctx.save();
        this.ctx.translate(this.canvas_offset.x, this.canvas_offset.y);
        this.ctx.scale(this.scale_ratio.x, this.scale_ratio.y);
        let port_y = this.draw_bg();
        this.activated_model.models.forEach(m => {
            m.draw();
        });
        this.ctx.font = '10pt arial';
        this.ctx.textAlign='left';
        this.ctx.textBaseline = 'middle';
        for(var i=0;i<this.activated_model.in_port.length;i++){
            this.activated_model.in_port[i].draw(0, port_y + PORT_ARROW_HEIGHT + i * PORT_ARROW_HEIGHT * 2);
        }

        for(var i=0;i<this.activated_model.out_port.length;i++){
            this.activated_model.out_port[i].draw(this.cpd_pane_size.x - PORT_ARROW_WIDTH, port_y + PORT_ARROW_HEIGHT + i * PORT_ARROW_HEIGHT * 2);
        }
        this.ctx.restore();
    }
    draw_bg(){
        this.ctx.fillStyle = 'rgba(255,255,255,1)';
        this.ctx.strokeStyle = 'black';
        this.ctx.fillRect(0, 0, this.cpd_pane_size.x, this.cpd_pane_size.y);
        this.ctx.beginPath();
        this.ctx.rect(0, 0, this.cpd_pane_size.x, this.cpd_pane_size.y);
        this.ctx.stroke();
        return 10;
    }
    set_model(m){
        this.activated_model.is_root_model = false;
        this.activated_model = m;
        this.activated_model.is_root_model = true;
        console.log('set_model', m);
        cpd_designer.adjust_pane(true);
        _last_selected_obj = null;
        _enabled_line_obj = null;
        _selected_line_obj = null;
        _last_selected_port = null;
    }
    
    adjust_pane(auto_size=false){
        let r = 1;
        let b = 1;
        if(this.activated_model === undefined){
            return;
        }
        console.log(this.activated_model);
        this.activated_model.models.forEach(v => {
            if(v.position.x + v.width + 150 > r){
                r = v.position.x + v.width + 150;
            }
            if(v.position.y + v.height + 50 > b){
                b = v.position.y + v.height + 50;
            }
        });
        screenRect = canvas.getBoundingClientRect();
        canvas.width = screenRect.width;
        canvas.height = screenRect.height;
        if(this.activated_model.models !== undefined && this.activated_model.models.length == 0){
            r = canvas.width;
            b = canvas.height;
        }
        cpd_designer.cpd_pane_size.x = r;
        cpd_designer.cpd_pane_size.y = b;
        if(auto_size){
            cpd_designer.canvas_offset.x = 10;
            cpd_designer.canvas_offset.y = 10;
            let new_ratio = {x:(screenRect.width-20) / cpd_designer.cpd_pane_size.x, y:(screenRect.height-20) / cpd_designer.cpd_pane_size.y};
            if(new_ratio.x < new_ratio.y)
                new_ratio.y = new_ratio.x;
            else
                new_ratio.x = new_ratio.y;
            cpd_designer.scale_ratio = new_ratio;
        }
    }
}//end of class coupling_designer