let canvas;
let ctx;
let screenRect;
let inport=['in_water_height','in_signal','in_3'];
let outport=['out_gen','out_water_height','out_signal'];
let outport2=['in_water_height','in_signal','inport3'];
let inport2=['out_gen','out_water_height','out_signal'];
let animate_state = 1;
let json_dict = {};
let cpd_designer = null;
let arrow_obj = null;
let project_name = null;
let api_server_url = 'http://220.124.222.89:8081';
//let api_server_url = 'http://localhost:8081';
let selected_model_data = null;
let add_model_tmp = null;//드래그로 임시 추가중인 모델
let _window_z_index = 1000;//print, graph2d 등 서브창의 z-index
function init_designer() {
    console.log("init()");
    
    
    canvas = document.getElementById('canvas');
    ctx = canvas.getContext('2d');

    screenRect = canvas.getBoundingClientRect();
    arrow_obj = new Path2D();
    arrow_obj.moveTo(0,0);
    arrow_obj.lineTo(PORT_ARROW_WIDTH, PORT_ARROW_HEIGHT * 0.5);
    arrow_obj.lineTo(0, PORT_ARROW_HEIGHT);
    arrow_obj.closePath();
    cpd_designer = new coupling_designer({ctx:ctx, on_model_changed_proc:on_model_changed});
    cpd_designer.adjust_pane(true);

    /*cpd_designer.add_model({model_name:'한글 모델명', in_port:inport, out_port:outport,
        position:{x:100, y:100}, width:100, height:10, color:'rgba(200,200,0,1)'});
    cpd_designer.add_model({model_name:'WaterTank_atomic', in_port:inport2, out_port:outport2, 
        position:{x:100, y:100}, width:100, height:10, color:'rgba(0,200,0,1)'});
    cpd_designer.add_model({model_name:'Controller_atomic', in_port:inport, out_port:outport, 
        position:{x:100, y:100}, width:100, height:10, color:'rgba(0,0,200,1)'});*/
    add_keyevent_listener();
    animate_loop();

    let model_img_list = document.getElementById('img_list');
    model_img_list.innerHTML = '';
    set_default_btn(model_img_list);

    
}
function get_thumbnail(){
    cpd_designer.adjust_pane(true);
    let new_size = Object.assign({}, cpd_designer.cpd_pane_size);
    new_size.x *= cpd_designer.scale_ratio.x;
    new_size.y *= cpd_designer.scale_ratio.y;

    let nc = document.createElement('canvas');
    let nc_ctx = nc.getContext('2d');
    let xy_r = new_size.y / new_size.x;
    nc.width = 300;
    nc.height = 300 * xy_r;
    console.log(new_size, xy_r, nc.height);
    nc_ctx.drawImage(canvas, 0, 0, new_size.x, new_size.y, 0, 0, nc.width, nc.height);
    let img = nc.toDataURL('image/jpeg');
    return img;
}

function test(){
    
}
function set_default_btn(obj){
    add_model_btn(obj, '외부입력', 'external input', '', 'ein');
    add_model_btn(obj, '외부출력', 'external output', '', 'eout');
    //add_model_btn(obj, 'print', 'print message', '', 'print_msg');
    //add_model_btn(obj, 'graph', '2D graph', '', 'graph_2d');
    //add_model_btn(obj, 'plus', 'in_plus', '', 'plus');
}
function add_model(o, x, y){
    console.log('add_model', o);
    let ret = null;
    if(o == 'ein'){
        ret = cpd_designer.activated_model.add_ein_port({name:'ein_' + cpd_designer.activated_model.in_port.length});
    }
    else if(o == 'eout'){
        ret = cpd_designer.activated_model.add_eout_port({name:'eout_' + cpd_designer.activated_model.out_port.length});
    }
    else if(o == 'print_msg'){
        ret = cpd_designer.activated_model.add_model({model_name:'print', model_type:MODEL_TYPE.EMBEDED_FUNC_PRINT, in_port:[{name:'message'}]});
    }
    else if(o == 'graph_2d'){
        ret = cpd_designer.activated_model.add_model({model_name:'graph 2d', model_type:MODEL_TYPE.EMBEDED_FUNC_GRAPH_2D, in_port:[{name:'y'}]});
    }
    else if(o == 'plus'){
        ret = cpd_designer.activated_model.add_model({model_name:'plus', model_type:MODEL_TYPE.EMBEDED_FUNC_PLUS, in_port:[{name:'in'}], out_port:[{name:'plus_out'}], in_param:[{name:'value', value:0}]});
    }
    else{
        let json = JSON.parse(o);
        
        json.position = new vector2({x:x, y:y});
        json.uid = json.uid || json.id;
        json.iid = get_random_string();
        console.log(json);
        ret = cpd_designer.activated_model.add_model(json);
         
    }
    return ret;
}
function add_param_form(form, obj){
    if(obj.in_param.length == 0)
        return;
    let tr = document.createElement('tr');
    let td = document.createElement('td');
    td.setAttribute('colspan', 2);
    td.setAttribute('bgcolor','#e0e0e0');
    td.setAttribute('align','center');
    td.innerHTML = "<b>" + obj.model_name + "</b>";
    tr.appendChild(td);
    form.appendChild(tr);

    obj.in_param.forEach((v, idx)=>{
        console.log(v);
        let tr = document.createElement('tr');
        let td = document.createElement('td');
        td.setAttribute('style','width:100px;text-align:center;');
        td.setAttribute('bgcolor','white');
        td.setAttribute('title', v.desc);
        td.innerText = v.name;
        tr.appendChild(td);
        td = document.createElement('td');
        td.setAttribute('bgcolor','white');
        let txt = document.createElement('input');
        if(v.d_type == 'int' || v.d_type == 'dbl'){
            txt.setAttribute('type','number');    
        }
        else{
            txt.setAttribute('type','text');
        }
        txt.setAttribute('style','width:99%;');
        txt.setAttribute('name','p[]');
        txt.setAttribute('value',v.value);
        txt.onchange=function(e){
            console.log(e.target.value);
            if(v.d_type == 'int'){
                obj.in_param[idx].value = parseInt(e.target.value);
            }
            else if(v.d_type == 'dbl'){
                obj.in_param[idx].value = parseFloat(e.target.value);
            }
            else{
                obj.in_param[idx].value = e.target.value;
            }
        }
        td.appendChild(txt);
        tr.appendChild(td);
        form.appendChild(tr);
    });
}
function set_param_to_form(obj, clear_form = true){
    let d = document.getElementById('tb_properties');
    if(clear_form)
        d.innerHTML = '';   
    if(obj != null){
        add_param_form(d, obj);
        if(obj.models !== undefined){
            obj.models.forEach(v=>{
                set_param_to_form(v, false);
            });
        }
    }

}

function clear_properties_window(){
    let d = document.getElementById('tb_properties');

}
function add_properties_window(p_name, p_value){

}
function get_model_list(project){
    send_post_ex(api_server_url + '/get_model_list',
        {project_name:project}, (res)=>{
            console.log(res);
        });
}
function stop_animate(){
    animate_state = 0;
}
function save_to_local(filename){
    return cpd_designer.save(filename);
}
function save_to_api_server(projectname, filename, model_name){
    if(projectname == ''){
        alert('프로젝트 명을 넣어주세요');
        return;
    }
    else if(filename == ''){
        alert('저장될 파일명을 넣어주세요');
        return;
    }
    else if(model_name == ''){
        alert('모델명을 넣어주세요');
        return;
    }
    let json = cpd_designer.save();
    json.file_name = filename;
    json.model_name = model_name;
    console.log('save_to_api', json);
    send_post_ex(api_server_url + '/set_coupled_info',
        {id:cpd_designer.id, project_name:projectname, model_name:model_name,
        file_name:filename, json_data:JSON.stringify(json)}, 
        (res)=>{
            console.log(res);
            if(res.result == 1){
                document.getElementById('btn_sim_save').disabled = true;
                alert('저장완료!');
            }
        });
}
function save_to_stack(){
    return;
    let json = cpd_designer.save();
    if(json.models === undefined){
        return;
    }
    
    if(json.models.length > 0){
        json_dict[cpd_designer.file_name] = json;
    }
    
}
function load_from_stack(file_name){
    if(json_dict.hasOwnProperty(file_name)){
        save_to_stack();
        load_from_json(json_dict[file_name], file_name);
    }
}
function add_graph2d_window(parent_iid){
    let d1 = document.createElement('div');
    d1.setAttribute('id', parent_iid + '_div');
    d1.setAttribute('style','display:none;position:absolute;left:100px;top:100px;width:500px;height:300px;resize:both;overflow:auto;');
    
    let d2 = document.createElement('div');
    d2.setAttribute('style','display:table;width:100%;height:100%;');
    d1.appendChild(d2);

    let d3 = document.createElement('div');
    d3.setAttribute('style','display:table-row;height:30px;background-color:#EAEEF1;border:1px solid #1bf;padding:5px;line-height:1rem;border-top-left-radius:0.5rem;border-top-right-radius:0.5rem;');
    d3.onmousedown = function(e){
        _window_z_index++;
        d1.style.zIndex = _window_z_index.toString();
        _selected_window = d1;
    }
    let d4 = document.createElement('div');
    d4.setAttribute('style','position:absolute;left:20px;top:10px;');
    d4.innerHTML = '<font size=4>GRAPH 2D</font>';
    let d5 = document.createElement('button');
    d5.setAttribute('type','button');
    d5.innerHTML = '&times;';
    d5.setAttribute('style','position:absolute;right:10px;top:2px;height:10px;font-size: 1.5rem;border:0px;');
    d5.onclick=function(e){
        d1.style.display='none';
    }
    d3.appendChild(d4);
    d3.appendChild(d5);

    let d_bottom = document.createElement('div');
    d_bottom.setAttribute('style','display:table-row;background-color:black;border:1px solid #1bf;padding:5px;line-height:1rem;border-top:0px;border-bottom-left-radius:0.5rem;border-bottom-right-radius:0.5rem;');
    let d_bottom2 = document.createElement('div');
    d_bottom2.setAttribute('id',parent_iid + '_div_graph');
    d_bottom2.setAttribute('style','height:100%;');
    let d_svg = document.createElementNS('http://www.w3.org/2000/svg','svg');
    d_bottom2.appendChild(d_svg);
    d_bottom.appendChild(d_bottom2);

    d2.appendChild(d3);
    d2.appendChild(d_bottom);
   
    document.body.appendChild(d1); 
    setTimeout(()=>{
        create_graph(parent_iid + '_div_graph');
        add_resize_observer(d1);
    }, 200);
}
function add_print_window(parent_iid){
    let d1 = document.createElement('div');
    d1.setAttribute('id', parent_iid + '_div');
    d1.setAttribute('style','display:none;position:absolute;left:100px;top:100px;width:300px;height:300px;resize:both;overflow:auto;');
    
    let d2 = document.createElement('div');
    d2.setAttribute('style','display:table;width:100%;height:100%;');
    d1.appendChild(d2);

    let d3 = document.createElement('div');
    d3.setAttribute('style','display:table-row;height:30px;background-color:#EAEEF1;border:1px solid #1bf;padding:5px;line-height:1rem;border-top-left-radius:0.5rem;border-top-right-radius:0.5rem;');
    d3.onmousedown = function(e){
        _window_z_index++;
        d1.style.zIndex = _window_z_index.toString();
        _selected_window = d1;
    }
    let d4 = document.createElement('div');
    d4.setAttribute('style','position:absolute;left:20px;top:10px;');
    d4.innerHTML = '<font size=4>PRINT CONSOLE</font>';
    let d5 = document.createElement('button');
    d5.setAttribute('type','button');
    d5.innerHTML = '&times;';
    d5.setAttribute('style','position:absolute;right:10px;top:2px;height:10px;font-size: 1.5rem;border:0px;');
    d5.onclick=function(e){
        d1.style.display='none';
    }
    d3.appendChild(d4);
    d3.appendChild(d5);

    let d_bottom = document.createElement('div');
    d_bottom.setAttribute('style','display:table-row;background-color:black;border:1px solid #1bf;padding:5px;line-height:1rem;border-top:0px;border-bottom-left-radius:0.5rem;border-bottom-right-radius:0.5rem;');
    let d_bottom2 = document.createElement('div');
    d_bottom2.setAttribute('id',parent_iid + '_div_content');
    d_bottom2.setAttribute('style','height:100%;color:green;overflow:auto;');
    d_bottom.appendChild(d_bottom2);

    d2.appendChild(d3);
    d2.appendChild(d_bottom);
   
    document.body.appendChild(d1); 
}
function add_model_btn(parent_div, title, filename, update_time, data){
    let btn = document.createElement('button');
    let st = 'width:100%;height:80px;margin-bottom:5px;margin-top:5px;';
    btn.setAttribute('style', st);
    btn.setAttribute('draggable', true);
    btn.ondragstart = function(e){
        console.log('dragstart');
        e.dataTransfer.setData('data',data);
        e.dataTransfer.dropEffect = 'copy';
        selected_model_data = data;
        this.style.opacity = 0;
    };
    btn.ondragend = function(e){
        console.log('dragendy');
        selected_model_data = null;
        this.style.opacity = 1;
        update_tree();
    };
    btn.ondblclick = function(e){
        let j = JSON.parse(data);
        console.log(j);
        document.all.load_file_name.value = j.file_name;
        save_to_stack();
        load_from_api_server(document.all.module_name.value, document.all.load_file_name.value, j.uid);
        setTimeout(()=>{
            cpd_designer.adjust_pane(true);
        },100);
        
    }
    /*let img = document.createElement('img');
    img.src='http://143.248.187.105:30001/thumbnails/banksim/teller_atomic.py.jpg';
    btn.appendChild(img);*/
    let lb = document.createElement('label');
    lb.innerText = title;
    lb.setAttribute('style','width:100%;font-size:1.8em;');
    btn.appendChild(lb);
    //let br = document.createElement('br');
    //btn.appendChild(br);
    lb = document.createElement('label');
    lb.innerText = filename;
    btn.appendChild(lb);
    //br = document.createElement('br');
    //btn.appendChild(br);
    lb = document.createElement('label');
    lb.innerText = update_time;
    btn.appendChild(lb);
    parent_div.appendChild(btn);
}

function req_list_models(module_name){
    console.log(module_name);
    send_post_ex(api_server_url + '/get_model_list',
        {project_name:module_name}, (res)=>{
            console.log(res);
            if(res.result == 1){
                let model_img_list = document.getElementById('img_list');
                model_img_list.innerHTML = '';
                set_default_btn(model_img_list);
                res.data[0].forEach(element => {
                    let json = JSON.parse(element.json_data);
                    console.log(json);
                    add_model_btn(model_img_list, json.model_name, json.file_name, element.updated, element.json_data);
                });
            }
        });
}
function req_list_projects(element_id) {
    console.log('req_list_projects')
    send_post_ex(
        api_server_url  + '/get_project_list',
        //"http://localhost:8081/get_project_list", // api_server_url + "/get_project_list",
        {_:""}, 
        (res)=>{
            if(res.result == 1){
            let project_list_node = document.getElementById(element_id);
            project_list_node.innerHTML="";
            res.data[0].forEach((element)=>{
                add_project_list_option(project_list_node,element.project_name)
            })
            document.getElementById(element_id).value = element.project_name;
            // req_list_models(project_name);
            }
        }
    );
} 
function update_stack_label(){
    return;
    let lbl = document.getElementById('lbl_stack');
    let str = 'HOME';
    for(let key in json_dict){
        str += " / <a href='#' onclick=\"load_from_stack('" + key + "');\">" + key + "</a>";
    }
    if(!json_dict.hasOwnProperty(cpd_designer.file_name)){
        str += " / <a href='#' onclick=''>" + cpd_designer.file_name + "</a>";
    }
    lbl.innerHTML = str;
}
function load_from_json(j, file_name, id = undefined){
    console.log('load', j);
    cpd_designer.clear_designer();
    cpd_designer.model_name = j.model_name || file_name;
    cpd_designer.file_name = file_name;
    document.all.load_file_name.value = file_name;
    document.all.load_model_name.value = cpd_designer.model_name;
    cpd_designer.iid = j.iid || get_random_string();
    cpd_designer.uid = j.uid || get_random_string();
    cpd_designer.activated_model.load(j);
    _root_model = cpd_designer.activated_model;
    /*
    j.models.forEach((v, idx) =>{
        if(v.model_type === undefined){
            let ext = v.file_name.substring(v.file_name.lastIndexOf('.'), v.file_name.length).toLowerCase();
            if(ext == '.json')
                v.model_type = MODEL_TYPE.COUPLED;
            else
                v.model_type = MODEL_TYPE.DISCRETE;
        }
        cpd_designer.add_model(Object.assign(v));
    });
    j.out_port.forEach((v, idx) =>{
        cpd_designer.add_eout_port(v);
    });
    j.in_port.forEach((v, idx) =>{
        cpd_designer.add_ein_port(v);
    });
    */
    
    /*j.couples.forEach((v) =>{
        cpd_designer.set_coupling(v);
    });*/
    
    setTimeout(cpd_designer.adjust_pane, 100);
    update_stack_label();
    document.getElementById('btn_sim_push').disabled = false;
    document.getElementById('btn_sim_update').disabled = false;
}
function update_tree(){
    let tree_data = [];
    _root_model.get_tree(tree_data);
    $('#tree').jstree("destroy");
    $('#tree').jstree({ 
        'core' : {
            dblclick_toggle : false,
            //themes: { dots: false },
            'data' : tree_data
        }
    });

    $('#tree').bind("dblclick.jstree", function (e){
        var clickId = e.target.id.substring(0, e.target.id.indexOf('_'));
        if(_root_model != null){
            let obj = _root_model.get_model_by_id(clickId, true);
            if(obj != null){
                if(obj.model_type == MODEL_TYPE.COUPLED){
                    cpd_designer.set_model(obj);
                }
            }
        }
    });
    $('#tree').bind("click.jstree", function (e){
        var clickId = e.target.id.substring(0, e.target.id.indexOf('_'));
        if(_root_model != null){
            let obj = _root_model.get_model_by_id(clickId, true);
            if(obj != null){
                if(obj.model_type == MODEL_TYPE.COUPLED){
                    set_param_to_form(obj);
                }
            }
        }
    });
}
function load_from_api_server(project, file_name, id = ''){
    project_name = project;
    document.all.project_name.value = project;
    send_post_ex(api_server_url + '/get_model_info',
        {project_name:project, file_name:file_name, uid:id }, (res)=>{
            console.log(res);
            if(res.result == 1){
                //cpd_designer.file_location = res.data[0][0].location || 'localhost';
                //cpd_designer.model_name = res.data[0][0].model_name;
                let j = JSON.parse(res.data[0][0].json_data);
                load_from_json(j, file_name, id);
                update_tree();
                
            }
        });
}
function load_from_server(project, file_name){
    project_name = project;
    let json = {};
    json['Type'] = 'req_m';
    json['Data'] = {};
    json['Data']['project_name'] = project;
    json['Data']['file_name'] = file_name;
    
    ws.on_data_callback['req_m'] = function(json){
        load_from_json(json.Data.content, file_name);
    }
    ws.send(JSON.stringify(json));

}

function load_from_json_text(json_txt, file_name = 'coupled.json'){
    load_from_json(JSON.parse(json_txt), file_name);    
}
function load_from_uri(json_uri){
    let xhttp = new XMLHttpRequest();
    let _this = this;
    xhttp.onreadystatechange = function () {
        if(xhttp.readyState == 4 && xhttp.status == 200){
            _this.load_from_json_text(this.responseText, json_uri);
        }
    }
    xhttp.open("GET",'cpd/' + json_uri, true);
    xhttp.send();
}
function load_from_local(file){
    if(file === undefined)
        return;
    let file_reader = new FileReader();
    file_reader.readAsText(file);
    file_reader.onload = function(){
        load_from_json_text(file_reader.result, file.name);
    }
}
function unload_model_on_server(){
    let json = {};
    json['Type'] = 'sim_ctl';
    json['Data'] = {};
    json['Data']['project_name'] = 'all';
    //json['Data']['file_name'] = cpd_designer.file_name;
    //json['Data']['uid'] = cpd_designer.uid;
    json['Data']['msg'] = "unload";
    ws.send(JSON.stringify(json));
    document.getElementById('btn_sim_run').disabled = true;
    document.getElementById('btn_sim_pause').disabled = true;
    document.getElementById('btn_sim_stop').disabled = true;
}
function load_model_on_server(project_name, file_name){
    if(project_name == ''){
        alert('프로젝트 명을 넣어주세요');
        return;
    }
    let json_data = cpd_designer.save();
    let json = {};
    json['Type'] = 'sim_ctl';
    json['Data'] = {};
    json['Data']['project_name'] = project_name;
    //json['Data']['file_name'] = cpd_designer.file_name;
    //json['Data']['uid'] = cpd_designer.uid;
    json['Data']['json_data'] = json_data;
    json['Data']['msg'] = "load";

    ws.on_data_callback['res_m'] = function(json){
        console.log(json);
        if(json.Data.load !== undefined && json.Data.load == 'ok'){
            document.getElementById('btn_sim_run').disabled = false;
            document.getElementById('btn_sim_pause').disabled = true;
            document.getElementById('btn_sim_stop').disabled = true;
            document.getElementById('btn_sim_push').disabled = false;
            document.getElementById('btn_sim_update').disabled = false;
        }
        //load_from_json(json.Data.content, file_name);
    }
    ws.on_data_callback['model_init'] = function(json){
        console.log(json);
        let model = _root_model.get_model_by_id(json.Data.iid, true);
        model.on_init_from_sim_server(json.Data);
    }
    ws.send(JSON.stringify(json));
    document.getElementById('btn_sim_push').disabled = true;
    document.getElementById('btn_sim_update').disabled = true;
    document.getElementById('btn_sim_stop').disabled = false;
}

function update_model_on_server(project_name, file_name){
    let json_data = cpd_designer.save();
    let json = {};
    json['Type'] = 'sim_ctl';
    json['Data'] = {};
    json['Data']['project_name'] = project_name;
    //json['Data']['file_name'] = cpd_designer.file_name;
    //json['Data']['uid'] = cpd_designer.uid;
    json['Data']['json_data'] = json_data;
    json['Data']['msg'] = "param";

    ws.send(JSON.stringify(json)); 
}

function run_model_on_server(projectname, sim_start_time, sim_end_time){
    let json = {};
    json['Type'] = 'sim_ctl';
    json['Data'] = {};
    json['Data']['project_name'] = projectname;
    json['Data']['msg'] = "run";
    json['Data']['start_t'] = parseFloat(sim_start_time);
    json['Data']['end_t'] = parseFloat(sim_end_time);

    ws.on_data_callback['res_m'] = function(json){
        console.log(json);
        if(json.Data.stop !== undefined){
            document.getElementById('btn_sim_run').disabled = false;
            document.getElementById('btn_sim_pause').disabled = false;
            document.getElementById('btn_sim_stop').disabled = false;
            document.getElementById('btn_sim_push').disabled = false;
            document.getElementById('btn_sim_update').disabled = false;
        }
        else{
            let model = _root_model.get_model_by_id(json.Data.iid, true);
            if(model !== null || model !== undefined){
                model.on_message_from_sim_server(json.Data);
            }
        }
    }
    ws.send(JSON.stringify(json));
    document.getElementById('btn_sim_run').disabled = true;
    document.getElementById('btn_sim_pause').disabled = false;
    document.getElementById('btn_sim_stop').disabled = false;
    document.getElementById('btn_sim_push').disabled = true;
    document.getElementById('btn_sim_update').disabled = true;
}

function stop_model_on_server(projectname){
    let json = {};
    json['Type'] = 'sim_ctl';
    json['Data'] = {};
    json['Data']['project_name'] = projectname;
    json['Data']['msg'] = "stop";

    ws.on_data_callback['res_m'] = function(json){
 
    }
    ws.send(JSON.stringify(json));
    document.getElementById('btn_sim_run').disabled = false;
    document.getElementById('btn_sim_pause').disabled = true;
    document.getElementById('btn_sim_stop').disabled = true;
    document.getElementById('btn_sim_push').disabled = false;
    document.getElementById('btn_sim_update').disabled = false;
}

function get_model_on_server(atomic_name){

    let json = {};
    json['Type'] = 'getatomicmodel';
    json['Data'] = {};
    json['Data']['atomic_name'] = atomic_name;

    ws2.on_data_callback['atomic_txt'] = function(json){
        openEditorInNewWindow(atomic_name, json.Data.content);
    }
    ws2.send(JSON.stringify(json));
}

function set_model_on_server(atomic_name, param){
    let json = {};
    json['Type'] = 'setatomicmodel';
    json['Data'] = {};
    json['Data']['atomic_name'] =  atomic_name;
    json['Data']['msg'] = param;

    ws2.send(JSON.stringify(json));
}

function on_model_changed(obj){
    document.getElementById('btn_sim_save').disabled = false;
    document.getElementById('btn_sim_push').disabled = false;
    document.getElementById('btn_sim_update').disabled = false;
}

function clear_designer(){
    cpd_designer.clear_designer();
    document.getElementById('btn_sim_save').disabled = true;
    document.getElementById('btn_sim_push').disabled = true;
    document.getElementById('btn_sim_update').disabled = true;
    document.all.load_model_name.value = '';
    document.all.load_file_name.value = '';
}
function add_keyevent_listener() {
    document.onkeydown = function(e){
        cpd_designer.on_keydown(e);
    }

    canvas.onmousewheel = function(e){
        cpd_designer.on_mousewheel(e);
    }

    canvas.onmousedown = function(e) {
        $(".contextmenu").hide();
        cpd_designer.on_mousedown(e);
        //if(e.buttons == 1){
        //    open_model_list_window(true);
        //}
    }

    canvas.onmousemove = function(e) {
        cpd_designer.on_mousemove(e);
    }

    canvas.onmouseup = function(e) {
        cpd_designer.on_mouseup(e);
    }

    canvas.onmouseout = function(e) {
        cpd_designer.on_mouseout(e);
    }

    canvas.ondblclick = function(e){
        cpd_designer.on_mousedbclick(e);
    }

    canvas.ondragenter = function(e){
        e.preventDefault();
        console.log('ondragenter');
        if(selected_model_data != null){
            var x = e.pageX - screenRect.x;
            var y = e.pageY - screenRect.y;
            x -= cpd_designer.canvas_offset.x;
            y -= cpd_designer.canvas_offset.y;
            x /= cpd_designer.scale_ratio.x;
            y /= cpd_designer.scale_ratio.y;


            add_model_tmp = add_model(selected_model_data, x, y);
            if(add_model_tmp != null && add_model_tmp.model_type !== undefined){
                add_model_tmp.selected = true;
            }
        }

    }

    canvas.ondragover = function(e){
        e.preventDefault();
        if(add_model_tmp != null && add_model_tmp.model_type !== undefined){
            cpd_designer.on_mousemove(e);
        }
     //   console.log('ondragover');
    }

    canvas.ondragleave = function(e){
        e.preventDefault();
        console.log('ondragleave');
        if(add_model_tmp != null){
            if(add_model_tmp.model_type !== undefined){
                add_model_tmp.selected = false;
                cpd_designer.delete_model(add_model_tmp);
            }
            else{
                cpd_designer.delete_port(add_model_tmp);
            }
            add_model_tmp = null;
        }
    }

    canvas.ondrop = function(e){
        e.preventDefault();
        console.log('ondragdrop', e);
        if(add_model_tmp != null){
            if(add_model_tmp.model_type !== undefined){
                add_model_tmp.selected = false;
                cpd_designer.adjust_pane();
                

            }
            add_model_tmp = null;
            
        }
        /*var d = e.dataTransfer.getData('data');
        var x = e.pageX - screenRect.x;
        var y = e.pageY - screenRect.y;
        x -= cpd_designer.canvas_offset.x;
        y -= cpd_designer.canvas_offset.y;
        x /= cpd_designer.scale_ratio.x;
        y /= cpd_designer.scale_ratio.y;


        add_model(d, x, y);*/
    }

    canvas.oncontextmenu = function(e){
        e.preventDefault();
        console.log('menu');
        cpd_designer.on_mousedown(e);
        if(_last_selected_obj != null){
            $(".contextmenu").css({
                "left":event.pageX + "px",
                "top":event.pageY + "px"
            }).show();
        }
    }

    window.addEventListener('resize', (e)=>{
        console.log('window resize');
        screenRect = canvas.getBoundingClientRect();
        cpd_designer.adjust_pane(true);
    });
}


function animate_loop() {
    ctx.clearRect(0, 0, canvas.width, canvas.height);
    // ctx.fillStyle = 'rgba(40,40,40,1)';
    // ctx.fillStyle = #7da5d7;
    ctx.rect(0,0,canvas.width, canvas.height);
    // ctx.fill();
    cpd_designer.draw();
    
    requestAnimationFrame(() => {
        if(animate_state == 1)
            animate_loop();
    });

}
function add_project_list_option (parent,projectname) {
    let option = document.createElement("option");
    let project_name_node = document.createTextNode(projectname);
    option.setAttribute("value",projectname);
    option.appendChild(project_name_node);
    parent.appendChild(option);
}

function req_atomic_list(element_id) {
    console.log('req_atomic_list')
        send_post_ex(
            api_server_url  + '/get_atomic_list',
        //"http://localhost:8081/get_atomic_list", // api_server_url + "/get_project_list",
        {_:""}, 
        (res)=>{
            if(res.result == 1){
            let atomic_list_node = document.getElementById(element_id);
            atomic_list_node.innerHTML="";
            res.data[0].forEach((element)=>{
                add_atomic_list_option(atomic_list_node, element.project_name, element.file_name)
            })
            //document.getElementById(element_id).value = atomic_list_node;
            // req_list_models(project_name);
            }
        }
    );
}

function add_atomic_list_option (parent, project_name, file_name) {
    var option = document.createElement("option");
    option.text = project_name + "/" + file_name;
    option.value = project_name + "/" + file_name;
    parent.options.add(option);
}
