function send_post(url, params){
    var html = "<html><head></head><body>Hidden iframe</body></html>";
    var iframe = document.createElement("iframe");
    iframe.setAttribute("name", "_hidden_frame");
    iframe.setAttribute("id", "_hidden_frame");
    iframe.setAttribute("frameborder", "0");
    iframe.setAttribute("width", "0");
    iframe.setAttribute("height", "0");
    iframe.src = "data:text/html;charset=utf-8," + encodeURI(html);
    document.body.appendChild(iframe);

    let form = document.createElement('form');
    form.setAttribute('method','post');
    form.setAttribute('target','_hidden_frame');
    form.setAttribute('action', url);
    //document.charset = 'utf-8';
    for(let key in params){
        let fd = document.createElement('input');
        fd.setAttribute('type','hidden');
        fd.setAttribute('name',key);
        fd.setAttribute('value',params[key]);
        form.appendChild(fd);
        console.log(key, params[key]);
    }
    document.body.appendChild(form);
    form.submit();
    
    setTimeout(5000, ()=>{
        document.body.removeChild(form);
        document.body.removeChild(iframe);
    });
    
}
function send_get_ex(url, success_cb, error_cb, complete_cb){
    $.ajax({
        // [요청 시작 부분]
        url: url, //주소
        type: "GET", //전송 타입
        async: true, //비동기 여부
        timeout: 5000, //타임 아웃 설정
        dataType: "JSONP", //응답받을 데이터 타입 (XML,JSON,TEXT,HTML,JSONP)    			
        //contentType: "application/json; charset=utf-8", //헤더의 Content-Type을 설정
                        
        // [응답 확인 부분 - json 데이터를 받습니다]
        success: success_cb || function(response) {
            console.log("");
            console.log("[requestPostBodyJson] : [response] : " + JSON.stringify(response));    				
            console.log("");    				
        },
                        
        // [에러 확인 부분]
        error: error_cb || function(xhr) {
            console.log("");
            console.log("[requestPostBodyJson] : [error] : " + JSON.stringify(xhr));
            console.log("");    				
        },
                        
        // [완료 확인 부분]
        complete:complete_cb || function(data,textStatus) {
            console.log("");
            console.log("[requestPostBodyJson] : [complete] : " + textStatus);
            console.log("");    				
        }
    });	
}
function send_post_ex(url, jsondata, success_cb, error_cb, complete_cb){
    console.log(url);
    $.ajax({
        // [요청 시작 부분]
        url: url, //주소
        data: JSON.stringify(jsondata), //전송 데이터
        type: "POST", //전송 타입
        async: true, //비동기 여부
        timeout: 5000, //타임 아웃 설정
        dataType: "JSON", //응답받을 데이터 타입 (XML,JSON,TEXT,HTML,JSONP)    			
        contentType: "application/json; charset=utf-8", //헤더의 Content-Type을 설정
                        
        // [응답 확인 부분 - json 데이터를 받습니다]
        success: success_cb || function(response) {
            console.log("");
            console.log("[requestPostBodyJson] : [response] : " + JSON.stringify(response));    				
            console.log("");    				
        },
                        
        // [에러 확인 부분]
        error: error_cb || function(xhr) {
            console.log("");
            console.log("[requestPostBodyJson] : [error] : " + JSON.stringify(xhr));
            console.log("");    				
        },
                        
        // [완료 확인 부분]
        complete:complete_cb || function(data,textStatus) {
            console.log("");
            console.log("[requestPostBodyJson] : [complete] : " + textStatus);
            console.log("");    				
        }
    });	
}

function send_image(url, canvas, pane_pos, pane_size){
    console.log('send_image', pane_pos, pane_size);
    let nc = document.createElement('canvas');
    let nc_ctx = nc.getContext('2d');
    let xy_r = pane_size.y / pane_size.x;
    nc.width = 300;
    nc.height = 300 * xy_r;
    console.log(xy_r, nc.width, nc.height);
    nc_ctx.drawImage(canvas, pane_pos.x, pane_pos.y, pane_size.x,pane_size.y, 0, 0, nc.width, nc.height);
    let img = nc.toDataURL('image/jpeg');
    $.ajax({
        method: 'POST',
        url: url,
        data: {
          image: img
        }
      });
}