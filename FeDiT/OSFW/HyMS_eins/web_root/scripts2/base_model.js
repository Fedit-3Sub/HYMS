// 포트 삼각형 도형의 가로길이
const PORT_ARROW_WIDTH = 10;
// 포트 삼각형 도형의 세로길이
const PORT_ARROW_HEIGHT = 20;
// 모델 타입들
const MODEL_TYPE = {
    UNKNOWN : 0,
    DISCRETE : 1,
    CONTINUOUS : 2,
    FUNCTION : 3,
    ATOMIC : 11,
    COUPLED : 99,
    EMBEDED_FUNC : 10000,
    EMBEDED_FUNC_PRINT : 10001,
    EMBEDED_FUNC_GRAPH_2D : 10002,
    EMBEDED_FUNC_PLUS : 10003,
    EMBEDED_FUNC_MINUS : 10004,
    EMBEDED_FUNC_MUL : 10005,
    EMBEDED_FUNC_DIV : 10006,
};
const PORT_TYPE = {
    IN_PORT : 1,
    OUT_PORT : 2,
    EX_IN_PORT : 3,
    EX_OUT_PORT : 4,        
};
// 라인 그리기시 방향
const DRAW_DIR = {
    NONE : 0,
    UP : 1,
    RIGHT : 2,
    DOWN : 3,
    LEFT : 4
};
// 기본 모델 클래스(커플, 아토믹, 연속, 함수)
class base_model{
    constructor(o){
        this.is_root_model = o.is_root_model || false;//최상위 모델여부
        this.parent = o.parent;
        this.ctx = o.ctx;//canvas context
        this.in_port = [];//입력포트
        this.out_port = [];//출력포트
        this.idx = o.idx || -1;//모델 index
        this.ext_window = null;//그래프, 콘솔창 같이 특별히 연결된 창
        this.models = [];//하위 모델들(base_model)
        this.load(o);
    }
    destructor(){
        let d = document.getElementById(this.iid + '_div');
        if(d !== undefined && d != null){//특별한 창이 있다면
            console.log(d);
            remove_resize_observer(d);//index.html에 있는 ResizeObserver 객체 삭제
            d.remove();
        }
        this.models.forEach((v) => {
            v.destructor();
        });
        this.in_port.forEach(v=>{
            v.destructor();
        });
        this.out_port.forEach(v=>{
            v.destructor();
        });
    }
    load(o){
        console.log(o);
        this.iid = o.iid || get_random_string();//instance id
        this.uid = o.uid || get_random_string();//unique id
        this.font_title = 'bold 11pt arial';//모델명 출력 font
        this.font_port = '10pt arial';//포트명 출력 font
        this.file_name = o.file_name || 'untitled.json';
        let ext = this.file_name.substring(this.file_name.lastIndexOf('.')+1).toLowerCase();//모델 파일 확장자
        if(o.model_type === undefined && ext == 'json'){
            o.model_type = MODEL_TYPE.COUPLED;
        }
        else if(o.model_type === undefined){
            if(ext == 'dll' || ext == 'py' || ext == 'm'){
                o.model_type = MODEL_TYPE.DISCRETE;
            }
        }
        this.model_type = o.model_type || MODEL_TYPE.COUPLED;
        if(this.model_type == 4)
            this.model_type = MODEL_TYPE.EMBEDED_FUNC_PRINT;
        this.file_path = o.file_path || project_name;
        if(o.ip !== undefined){
            o.ip_port = o.ip + ':' + o.port;
        }
        this.ip_port = o.ip_port || 'localhost:8865';//모델 실행을 위한 접속 주소
        this.model_name = o.model_name || 'untitled';//모델 명
        this.ctx.font = this.font_title;
        this.fontmetrics = this.ctx.measureText(this.model_name);
        this.fontHeight = this.fontmetrics.fontBoundingBoxAscent + this.fontmetrics.fontBoundingBoxDescent;//폰트 높이
        if(o.position !== undefined){
            if(o.position.x < 150)
                o.position.x = 150;
            if(o.position.y < 50)
                o.position.y = 50;
            this.position = new vector2({x:o.position.x, y:o.position.y});//모델 위치
        }
        else
            this.position = new vector2({x:150, y:50});
        this.color = o.color || get_random_color();//포트 컬러
        let port_name_max_length = 0;//최대 모델 가로 크기를 알기 위해 포트명 최대 길이를 구한다
        let port_name_max_length_str = '';
        if(o.models !== undefined){
            o.models.forEach((v)=>{
                this.add_model(v);
            });
        }
        if(o.out_port !== undefined){
            o.out_port.forEach((v, idx) => {
                if(this.is_root_model && this.model_type == MODEL_TYPE.COUPLED)
                    this.add_eout_port(v);
                else
                    this.add_out_port(v, false);
                let port_name = v.name || v;
                if(port_name_max_length < port_name.length){
                    port_name_max_length = port_name.length;
                    port_name_max_length_str = port_name;
                }
            });
        }
        else{
            this.out_port = [];
        }
        if(o.in_port !== undefined){
            o.in_port.forEach((v, idx) => {
                if(this.is_root_model && this.model_type ==  MODEL_TYPE.COUPLED)
                    this.add_ein_port(v);
                else
                    this.add_in_port(v, false);
                let port_name = v.name || v;
                if(port_name_max_length < port_name.length){
                    port_name_max_length = port_name.length;
                    port_name_max_length_str = port_name;
                }
            });
        }
        else{
            this.in_port = [];
        }
        
        if(o.in_param !== undefined){
            this.in_param = o.in_param;//모델 파라미터
        }
        else{
            this.in_param = [];
        }
        this.selected = false;//모델 선택여부
        this.selected_pos = new vector2({});
        
        this.ctx.font = this.font_port;
        var fm = this.ctx.measureText(port_name_max_length_str);
        this.width = fm.width + PORT_ARROW_WIDTH * 4;
        
        if((this.fontmetrics.width + PORT_ARROW_WIDTH * 2) > this.width)
            this.width = this.fontmetrics.width + PORT_ARROW_WIDTH * 2;
        //this.model_name_lines = wrap_text(this.model_name, this.width - 10, this.ctx);
        let min_height = this.fontHeight * 1.5;//(this.model_name_lines.length + 0.5);
        if(this.in_port.length > 0 || this.out_port.length > 0)
            min_height += ((this.in_port.length > this.out_port.length? this.in_port.length * (PORT_ARROW_HEIGHT * 2):this.out_port.length * (PORT_ARROW_HEIGHT*2) + PORT_ARROW_HEIGHT));
        this.height = min_height;
        
        if(this.width < 100)
            this.width = 100;
        if(this.height < 100)
            this.height = 100;

        if(this.model_type == MODEL_TYPE.EMBEDED_FUNC_PRINT){//콘솔출력창 모델일 경우
            add_print_window(this.iid);
        }
        else if(this.model_type == MODEL_TYPE.EMBEDED_FUNC_GRAPH_2D){//2D 그래프 출력창 모델일경우
            add_graph2d_window(this.iid);
        }

        console.log(this.model_type, o);
        // 모든 모델의 포트가 생성된 후 커플링
        // 출발포트에 라인정보(커플링)가 있으나 모델에서 라인정보를 가지고 있는것으로 수정필요
        if(o.in_port !== undefined && this.model_type == MODEL_TYPE.COUPLED){//커플모델일경우 입력포트에서 라인 출력
            o.in_port.forEach((v, idx) => {
                console.log(v.line);
                if(v.line !== undefined){
                    v.line.forEach(l=>{
                        this.set_coupling(l);
                    });
                }
                
            });
        }
        if(o.models !== undefined){
            o.models.forEach((m)=>{//하위모델들
                if(m.out_port !== undefined){//출력포트
                    m.out_port.forEach((v, idx) => {
                        if(v.line !== undefined){
                            v.line.forEach(l=>{
                                this.set_coupling(l);
                            });
                        }
                        
                    });
                }
            });
        }
        
        /*if(o.couples !== undefined && Array.isArray(o.couples)){
            o.couples.forEach((v) =>{
                this.set_coupling(v);
            });
        }*/

    }
    // project view창의 트리뷰를 위한 정보 생성
    get_tree(ret, parentid){
        let d = new Object();
        d.id = this.iid;
        d.parent = parentid || "#";
        d.text = this.model_name;
        if(this.model_type == MODEL_TYPE.COUPLED){
            d.icon = "images/cpd.png";
        }
        else if(this.model_type >= MODEL_TYPE.EMBEDED_FUNC){
            d.icon = "images/function.png";
        }
        else{
            d.icon = "images/atomic.png";
            d.text = this.file_name;
        }
        d.state = {};
        d.state.opened = true;
        ret.push(d);
        this.models.forEach(v=>{
            v.get_tree(ret, this.iid);
        });
        return ret;
    }
    // external in port
    add_ein_port(o){
        this.in_port.push(new base_port(Object.assign(o, {parent:this, ctx:this.ctx,
                port_type:PORT_TYPE.EX_IN_PORT, color:'rgba(128,128,128,0.5)', idx:this.in_port.length})));
        return this.in_port[this.in_port.length-1];
    }
    // external out port
    add_eout_port(o){
        this.out_port.push(new base_port(Object.assign(o, {parent:this, ctx:this.ctx,
            port_type:PORT_TYPE.EX_OUT_PORT, color:'rgba(128,128,128,0.5)', idx:this.out_port.length})));

            return this.out_port[this.out_port.length-1];
    }
    add_in_port(o, update_size = true){
        let p = new base_port(Object.assign(o, {parent:this,
            ctx:this.ctx, 
            port_type:PORT_TYPE.IN_PORT,
            color:this.color, idx:this.in_port.length}));
        this.in_port.push(p);
        if(update_size){//포트가 추가되어 기존 모델 크기보다 더 커져야 되는지 확인
            let port_name = o.name || o;
            this.ctx.font = this.font_port;
            var fm = this.ctx.measureText(port_name);
            if(this.width < fm.width + PORT_ARROW_WIDTH * 4)
                this.width = fm.width + PORT_ARROW_WIDTH * 4;
            let min_height = this.fontHeight * 1.5;//(this.model_name_lines.length + 0.5);
            if(this.in_port.length > 0 || this.out_port.length > 0)
                min_height += ((this.in_port.length > this.out_port.length? this.in_port.length * (PORT_ARROW_HEIGHT * 2):this.out_port.length * (PORT_ARROW_HEIGHT*2) + PORT_ARROW_HEIGHT));
            this.height = min_height;
        }
        return p;
    }
    add_out_port(o, update_size = true){
        let p = new base_port(Object.assign(o, {parent:this,
            ctx:this.ctx, 
            port_type:PORT_TYPE.OUT_PORT,
            color:this.color, idx:this.out_port.length}));
        this.out_port.push(p);
        if(update_size){//포트가 추가되어 기존 모델 크기보다 더 커져야 되는지 확인
            let port_name = o.name || o;
            this.ctx.font = this.font_port;
            var fm = this.ctx.measureText(port_name);
            if(this.width < fm.width + PORT_ARROW_WIDTH * 4)
                this.width = fm.width + PORT_ARROW_WIDTH * 4;
            let min_height = this.fontHeight * 1.5;//(this.model_name_lines.length + 0.5);
            if(this.in_port.length > 0 || this.out_port.length > 0)
                min_height += ((this.in_port.length > this.out_port.length? this.in_port.length * (PORT_ARROW_HEIGHT * 2):this.out_port.length * (PORT_ARROW_HEIGHT*2) + PORT_ARROW_HEIGHT));
            this.height = min_height;
        }
        return p;
    }
    // 자식모델 추가
    add_model(o){
        //console.log(JSON.stringify(o));
        if(Array.isArray(o.model_name)){
            let m = o.model_name;
            m.forEach(v =>{
                this.models.push(new atomic(Object.assign(o, {parent:this, ctx:this.ctx, width:100, height:100, idx:this.models.length, model_name:v})));
            });
            
        }
        else{
            this.models.push(new atomic(Object.assign(o, {parent:this, ctx:this.ctx, width:100, height:100, idx:this.models.length})));
        }
        return this.models[this.models.length-1];
    }
    // iid로 모델 찾기
    // is_search_sub_models : child 모델 하위로 계속 찾을것인지 여부
    get_model_by_id(model_id, is_search_sub_models = false){
        //console.log(model_id, is_search_sub_models, this.iid);
        if(this.iid == model_id){
            return this;
        }

        let ret = null;
        this.models.forEach(v=>{
            if(v.iid == model_id){
                ret = v;
                return;
            }
        });
        if(ret == null && is_search_sub_models){
            this.models.forEach(v=>{
                let ret2 = v.get_model_by_id(model_id, is_search_sub_models);
                if(ret2 != null){
                    ret = ret2;
                    return;
                }
            });
        }
        return ret;
    }
    // 모델명으로 모델 찾기
    // get_model_by_id()와 달리 하위 모델 1 depth에서만 찾는다
    get_model_by_name(model_name){ 
        let ret = null;
        this.models.forEach(v=>{
            if(v.model_name == model_name){
                ret = v;
                return;
            }
        });
        return ret;
    }
    // 모델 커플링
    set_coupling(o){
        console.log(o);
        let from_obj = null;
        if(o.from_obj_info.iid == 'this' || o.from_obj_info.iid == this.iid){
            this.in_port.forEach((v, idx) =>{
                if(v.name == o.from_obj_info.port_name){
                    from_obj = v;
                    return;
                }
            });
        }
        else{
            let src = this.get_model_by_id(o.from_obj_info.iid, true);
            if(src != null){
                console.log(src);
                src.out_port.forEach((v, idx) =>{
                    if(v.name == o.from_obj_info.port_name){
                        from_obj = v;
                        return;
                    }
                });
                
            }
        }
        if(from_obj == null){
            console.log('error : set_coupling::from_obj is null', o, this.iid);
            return;
        }
        let to_obj = null;
        if(o.to_obj_info.iid == 'this' || o.to_obj_info.iid == this.iid){
            this.out_port.forEach((v, idx) =>{
                if(v.name == o.to_obj_info.port_name){
                    to_obj = v;
                    return;
                }
            });
        }
        else{
            let to_obj_model = this.get_model_by_id(o.to_obj_info.iid, true);
            if(to_obj_model == null){
                console.log('error : set_coupling::to_obj_model is null', o, this.iid);
                return;    
            }
            to_obj_model.in_port.forEach((v, idx) =>{
                if(v.name == o.to_obj_info.port_name){
                    to_obj = v;
                    return;
                }
            });
        }
        if(to_obj != null){
            from_obj.load(Object.assign(o, {to_obj:to_obj}));
        }

    }
    
    save(){
        let d = new Object();
        d.model_name = this.model_name;
        d.iid = this.iid;
        d.uid = this.uid;
        d.idx = this.idx;
        d.model_type = this.model_type;
        d.file_name = this.file_name;
        d.file_path = this.file_path;
        d.ip_port = this.ip_port;
        d.position = this.position;
        d.color = this.color;
        d.models = [];
        this.models.forEach((v) => {
            d.models.push(v.save());
        });
        d.in_port=[];
        this.in_port.forEach(v=>{
            d.in_port.push(v.save());
        });
        d.out_port=[];
        this.out_port.forEach(v=>{
            d.out_port.push(v.save());
        });
        d.in_param=this.in_param;
        
        console.log(d);

        return d;
    }
    // 시뮬레이션 서버에서 init을 요청한 경우(함수모델, console, graph같은...)
    on_init_from_sim_server(json_msg){
        if(this.model_type < MODEL_TYPE.EMBEDED_FUNC){
            return;
        }
        if(this.model_type == MODEL_TYPE.EMBEDED_FUNC_PRINT){
            if(this.ext_window == null){
                this.ext_window = document.getElementById(this.iid + '_div_content');
            }
            this.ext_window.innerHTML = '';
            this.ext_window.scrollTop = this.ext_window.scrollHeight;
        }
        else if(this.model_type == MODEL_TYPE.EMBEDED_FUNC_GRAPH_2D){
            graph_clear(this.iid);
        }
    }
    // 시뮬레이션 서버에서 msg를 보낸경우
    on_message_from_sim_server(json_msg){
        if(this.model_type < MODEL_TYPE.EMBEDED_FUNC){
            return;
        }
        if(this.model_type == MODEL_TYPE.EMBEDED_FUNC_PRINT){
            if(this.ext_window == null){
                this.ext_window = document.getElementById(this.iid + '_div_content');
            }
            this.ext_window.innerHTML += json_msg.sim_time.toFixed(2) + '][' + json_msg.src_port_name
                + '] ' + json_msg.value + '<br>';
            this.ext_window.scrollTop = this.ext_window.scrollHeight;
        }
        else if(this.model_type == MODEL_TYPE.EMBEDED_FUNC_GRAPH_2D){
            graph_add_data(this.iid, json_msg.src_port_name,
                 Math.round(json_msg.sim_time*1e3)/1e3, json_msg.value);
        }
    }
    // 입력 좌표가 모델 범위 안에 있는지
    chk_mousepos(x, y){
        if ((x >= (this.position.x)) && 
            (x <= (this.position.x + this.width)) && 
            (y >= (this.position.y)) && 
            (y <= (this.position.y + this.height))) {
            return true;
        }

        return false;
    }
    // 마우스 더블클릭시
    on_mousedbclick(x, y){
        if(this.chk_mousepos(x, y)){
            if(this.model_type == MODEL_TYPE.COUPLED){//커플모델일경우 모델 자세히 보기
                console.log('cpd', this.model_name, this.file_name);
                //save_to_stack();
                //setTimeout(load_from_api_server, 100, project_name, this.file_name, this.uid);
                cpd_designer.set_model(this);
                
            }
            else{
                console.log(this.model_name, this.file_name);
                if(this.model_type == MODEL_TYPE.EMBEDED_FUNC_PRINT){//콘솔창 모델일 경우 창 활성화
                    let d = document.getElementById(this.iid + '_div');
                    d.style.display='';
                    d.style.left = event.clientX;
                    d.style.top = event.clientY;
                    if(this.ext_window == null){
                        this.ext_window = document.getElementById(this.iid + '_div_content');
                    }
                    this.ext_window.scrollTop = this.ext_window.scrollHeight;
                }
                else if(this.model_type == MODEL_TYPE.EMBEDED_FUNC_GRAPH_2D){//그래프 모델일 경우 창 활성화
                    let d = document.getElementById(this.iid + '_div');
                    d.style.display='';
                    d.style.left = event.clientX;
                    d.style.top = event.clientY;
                    line_graph[this.iid+'_div_graph'].update();
                }
            }
            return 1;
        }
        return 0;
    }
    // 마우스 클릭시
    on_mousedown(x, y){
        for(var i=0;i<this.out_port.length;i++){
            if(this.out_port[i].on_mousedown(x, y) == 1){//출력포트 클릭시
                return 1;
            }
        }

        if(this.chk_mousepos(x, y)){// 모델 클릭시
            this.selected = true;
            this.selected_pos.x = x - this.position.x;
            this.selected_pos.y = y - this.position.y;
            _last_selected_obj = this;
            
            return 1;
        }
        return 0;
    }
    on_mousemove(x, y, dx, dy){
        if(this.selected){// 모델이 선택되어 있다면
            var new_x = x - this.selected_pos.x;
            var new_y = y - this.selected_pos.y;
            if(new_x >= 150)
                this.position.x = new_x;
            if(new_y >= 50)
                this.position.y = new_y;
            cpd_designer.on_model_changed(this);// 모델이 변경되었기에 SAVE버튼 활성화
        }
        else{
            this.in_port.forEach(v => v.on_mousemove(x, y, dx, dy));
            this.out_port.forEach(v => v.on_mousemove(x, y, dx, dy));
        }
    }
    on_mouseup(x, y){
        if(this.selected){
            this.selected = false;
            cpd_designer.adjust_pane();//모델 위치가 변경되었수도 있어 창크기 조절
        }
        else{
            this.in_port.forEach(v => v.on_mouseup(x, y));
            this.out_port.forEach(v => v.on_mouseup(x, y));
        }
    }
    // 모델 삭제시
    delete_obj(){
        // 하위모델은?
        this.in_port.forEach(v=>{
            v.delete_obj();
        });
        this.out_port.forEach(v=>{
            v.delete_obj();
        });
    }
    // 백그라운드 그리기
    draw_bg(){
        this.ctx.save();
        this.ctx.font
        this.ctx.font = this.font_title;
        this.ctx.fillStyle = 'rgba(255,255,255,0.8)';
        this.ctx.strokeStyle = 'black';
        this.ctx.beginPath();
        this.ctx.roundRect(this.position.x, this.position.y, this.width, this.height, [5, 5, 0, 0]);//잴 외곽 상단 두곳만 동그랗게
        this.ctx.fill();//패스 채우기
        this.ctx.stroke();//라인그리기
        this.ctx.beginPath();
        // 위쪽 타이틀 부분 그리기
        this.ctx.roundRect(this.position.x, this.position.y, this.width, this.fontHeight * 1.5, [5, 5, 0, 0]);//(this.model_name_lines.length + 1));//제목 외곽
        if(this.model_type >= MODEL_TYPE.EMBEDED_FUNC){//모델에 따라 배경색 다르게, 이것도 case별로 지정하는걸로 바꿔야...
            this.ctx.fillStyle = 'rgba(0,0,255,0.2)';
        }
        else if(this.in_param.length > 0){
            this.ctx.fillStyle = 'rgba(255,0,0,0.2)';
        }
        else{
            this.ctx.fillStyle = 'rgba(0,255,0,0.2)';
        }
        
        this.ctx.fill();
        this.ctx.textBaseline = 'top';
        this.ctx.textAlign = 'center';
        this.ctx.fillStyle = 'rgba(0,0,0,1)';
        this.ctx.fillText(this.model_name, this.position.x + this.width * 0.5, this.position.y + this.fontHeight * 0.25);
        this.ctx.restore();
        return this.fontHeight * 1.5;//(this.model_name_lines.length + 1);
    }
    // 모델 그리기
    draw(){
        this.ctx.save();
        var port_y = this.draw_bg();//배경 그리기
        this.ctx.font = this.font_port;
        this.ctx.textAlign='left';
        this.ctx.textBaseline = 'middle';
        for(var i=0;i<this.in_port.length;i++){//입력포트 그리기
            this.in_port[i].draw(this.position.x, port_y + this.position.y + i * PORT_ARROW_HEIGHT * 2);
        }
        for(var i=0;i<this.out_port.length;i++){//출력포트 그리기
            this.out_port[i].draw(this.position.x + this.width, port_y + PORT_ARROW_HEIGHT + this.position.y + i * PORT_ARROW_HEIGHT * 2);
        }
        this.ctx.restore();
    }

} //end of class base_model