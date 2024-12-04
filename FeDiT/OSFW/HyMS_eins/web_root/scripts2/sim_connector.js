function string_to_utf8_hex_string(text)

{

    var bytes1 = string_to_utf8_bytes (text);

    var hex_str1 = bytes_to_hex_string (bytes1);



    return hex_str1;

}
function utf8_hex_string_to_string(hex_str1)

{ 

    var bytes2 = hex_string_to_bytes (hex_str1);

    var str2 = utf8_bytes_to_string (bytes2);



    return str2;

}
function utf8_bytes_to_string(arr)
{
    if (arr == null)
        return null;
    var result = "";
    var i;
    while (i = arr.shift()) {
        if (i <= 0x7f) {
            result += String.fromCharCode(i);
        } else if (i <= 0xdf) { 
            var c = ((i & 0x1f) << 6);
            c += arr.shift() & 0x3f;
            result += String.fromCharCode(c);
        } else if (i <= 0xe0) { 
            var c = ((arr.shift () & 0x1f) << 6) | 0x0800;
            c += arr.shift() & 0x3f;
            result += String.fromCharCode(c);
        } else {
            var c = ((i & 0x0f) << 12);
            c += (arr.shift() & 0x3f) << 6;
            c += arr.shift() & 0x3f;
            result += String.fromCharCode(c);
        }
    }
    return result;
}
function utf8_uint8array_to_string(arr)
{
    if (arr == null)
        return null;
    var result = "";
    for(var i=0;i<arr.length;i++){

        if (arr[i] <= 0x7f) {
            result += String.fromCharCode(arr[i]);
        } else if (arr[i] <= 0xdf) { 
            var c = ((arr[i] & 0x1f) << 6);
            i++;
            c += arr[i] & 0x3f;
            result += String.fromCharCode(c);
        } else if (arr[i] <= 0xe0) { 
            i++;
            var c = ((arr[i] & 0x1f) << 6) | 0x0800;
            i++;
            c += arr[i] & 0x3f;
            result += String.fromCharCode(c);
        } else {
            var c = ((arr[i] & 0x0f) << 12);
            i++;
            c += (arr[i] & 0x3f) << 6;
            i++;
            c += arr[i] & 0x3f;
            result += String.fromCharCode(c);
        }
    }        
    return result;
}
function string_to_utf8_bytes(text)

{

    var result = [] ;



    if (text == null )

        return result;



    for ( var i = 0; i <text.length; i ++) {

        var c = text.charCodeAt (i);



        if (c <= 0x7f) {

            result.push (c);

        } else if (c <= 0x07ff) {

            result.push (((c >> 6) & 0x1F) | 0xC0);

            result.push ((c & 0x3F) | 0x80);

        } else {

            result.push (((c >> 12) & 0x0F) | 0xE0);

            result.push (((c >> 6) & 0x3F) | 0x80);

            result.push ((c & 0x3F) | 0x80);

        }

    }



    return result;

}
function byte_to_hex(byte_num)

{

    var digits = (byte_num).toString (16);



    if (byte_num <16)

        return '0' + digits;



    return digits;

}
function bytes_to_hex_string(bytes)

{

    var result = "" ;



    for ( var i = 0; i <bytes.length; i ++) { 

        result += byte_to_hex (bytes[i]);

    }



    return result;

}
function hex_to_byte(hex_str)

{

    return parseInt(hex_str, 16);

}
//여기에 시뮬레이션 서버와 통신할 데이터 주고받는 루틴 작성
class sim_connector extends WSClient{
    constructor(url){
        super(url);
        this.on_data_callback = {};
    }
    OnOpenCallback = function(event){//시뮬레이션 서버와 접속 하였을때
        console.log("서버 연결됨~~");
    }
    OnCloseCallback = function(event){//시뮬레이션 서버가 꺼져서 접속 종료되었을때
        console.log("서버 연결 종료됨~~~");
    }
    OnMessageCallback = function(event){//시뮬레이션 서버에서 데이터 수신 할때
        let json = null;
        if(event.data instanceof ArrayBuffer){
            //let d = String.fromCharCode.apply(null, new Uint8Array(event.data));
            let d = utf8_uint8array_to_string(new Uint8Array(event.data));
            json = JSON.parse(d);
        }
        else{
            json = JSON.parse(event.data);
        }
        //console.log(typeof(this.on_data_callback[json.Type]));
        if(typeof(this.on_data_callback[json.Type]) !== undefined){
            this.on_data_callback[json.Type](json);
        }
    }

    RunSimulation() {
        let sendData = {
            Type: 'RunSim',
            Data: {
                //testCase : "200615154227",
                startTime : 3600,
                endTime : 4200,
                ext1 : 'text1',
                ext2 : '한글!@#$',
            }
        };
        console.log(JSON.stringify(sendData));
        super.send(JSON.stringify(sendData));
    }
    StopSimulation(){
        let sendData = {
            Type: 'StopSim'
        };
        super.send(JSON.stringify(sendData));
    }
    PauseSimulation(){
        let sendData = {
            Type: 'PauseSim'
        };
        super.send(JSON.stringify(sendData));
    }
    SetParamPos(){
        let sendData = {
            Type: 'SetParam',
            position : {
                lat : 36.1234,
                lon : 127.1234
            }
        };
        super.send(JSON.stringify(sendData));
    }
}

class model_connector extends WSClient{
    constructor(url){
        super(url);
        this.on_data_callback = {};
    }
    OnOpenCallback = function(event){//시뮬레이션 서버와 접속 하였을때
        console.log("atomic_model_manager connection.");
    }
    OnCloseCallback = function(event){//시뮬레이션 서버가 꺼져서 접속 종료되었을때
        console.log("atomic_model_manager disconnection.");
    }
    OnMessageCallback = function(event){//시뮬레이션 서버에서 데이터 수신 할때
        let json = null;
        if(event.data instanceof ArrayBuffer){
            //let d = String.fromCharCode.apply(null, new Uint8Array(event.data));
            let d = utf8_uint8array_to_string(new Uint8Array(event.data));
            json = JSON.parse(d);
        }
        else{
            json = JSON.parse(event.data);
        }
        //console.log(typeof(this.on_data_callback[json.Type]));
        if(typeof(this.on_data_callback[json.Type]) !== undefined){
            this.on_data_callback[json.Type](json);
        }
    }
}

let ws = new sim_connector('220.124.222.89/sim');
let ws2 = new model_connector('220.124.222.89/model');