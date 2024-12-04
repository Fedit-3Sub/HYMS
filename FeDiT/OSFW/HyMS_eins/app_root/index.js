const express = require('express')
const req_ip = require('request-ip')
const app = express()
const port = 8081
const db = require('./lib/db_conn');
const cors = require("cors");
const fs = require('fs')
const request = require('request')

function mkdir( dirPath ) {
    const isExists = fs.existsSync( dirPath );
    if( !isExists ) {
        fs.mkdirSync( dirPath, { recursive: true } );
    }
}
 
app.use(express.json({
    limit : "50mb"
}));
app.use(express.urlencoded({
    limit:"50mb",
    extended: false
}));
 
const bodyParser = require("body-parser");
app.use(bodyParser.urlencoded({ extended: false }));
app.use(bodyParser.json());
//app.use(cors());
app.use(cors({
    origin: '*',
	methods: ['GET','POST','DELETE','UPDATE','PUT','PATCH']
}));

function leftPad(v){
 	if(v >= 10){
		return v;
	}
	else{
		return '0' + v;
	}
}
app.post('/get_model_info', (req, res) => {
	let q = Object.keys(req.query).length === 0 ? req.body:req.query;
	if(q.project_name === undefined || q.project_name == ''
		|| q.file_name === undefined || q.file_name == ''
	){
		res.send({result : "error param"});
		console.log("error param");
		return;
	}
	let ext = q.file_name.substring(q.file_name.lastIndexOf('.'), q.file_name.length).toLowerCase();
	if(ext == '.json'){
		sql = "call get_coupled('" + q.project_name + "','" + q.file_name + "','" + q.uid + "')";
	}
	else if(ext == '.py' || ext == '.dll' || ext == '.m'){
		sql = "call get_atomic('" + q.project_name + "','" + q.file_name + "','" + q.uid + "')";
	}
	else{
		res.send({result : "error filename"});
		console.log("error filename");
		return;
	}
	console.log(sql);
	db.query(sql, (err, data) => {
		if(!err) res.send({ result : 1, data:data });
		else res.send({result:0, data:err});
	})
});

app.post('/get_project_list', (req, res) => {
	sql = "call get_project_list()";
	console.log(sql);
	db.query(sql, (err, data) => {
		if(!err) {
			res.send({ result : 1, data:data });
		}
		else {
			res.send({result:0, data:err});
		}
	})
});

app.post('/get_model_list', (req, res) => {
	let q = Object.keys(req.query).length === 0 ? req.body:req.query;
	console.log(q);
	if(q.project_name === undefined || q.project_name == ''
	){
		res.send({result : "error param"});
		return;
	}
	sql = "call get_model_list('"+q.project_name+"')";
	console.log(sql);
	db.query(sql, (err, data) => {
		if(!err) res.send({ result : 1, data:data });
		else res.send({result:0, data:err});
	})
});

app.post('/get_coupled_info', (req, res) => {
	let q = Object.keys(req.query).length === 0 ? req.body:req.query;
	if(q.uid === undefined || q.uid == ''
	){
		res.send({result : "error"});
		console.log("error");
		return;
	}
	sql = "call get_coupled('','','" + q.uid + "')";
	db.query(sql, (err, data) => {
		if(!err) res.send({ result : 1, data:data });
		else res.send({result:0, data:err});
	})
});

app.post('/set_coupled_info',(req, res) => {
	let q = Object.keys(req.query).length === 0 ? req.body:req.query;
	if(q.project_name === undefined || q.project_name == ''
		|| q.model_name === undefined || q.model_name == ''
		|| q.file_name === undefined || q.file_name == ''
		|| q.json_data === undefined || q.json_data == ''
	){
		res.send({result : "error"});
		console.log(q);
		return;
	}

	if(q.image !== undefined){
		let p1 = q.image.indexOf(',');
		let img = q.image.substring(p1+1);
		console.log(img);
		let decode = Buffer.from(img, 'base64');
		mkdir('./thumbnails/' + q.project_name);
		let f = fs.writeFileSync('./thumbnails/' + q.project_name + '/' + q.file_name + '.jpg', decode);
	}

	let in_json = JSON.parse(q.json_data);

	let sql = "call update_coupled('"
					+ in_json.uid + "','" 
					+ q.project_name + "','" 
					+ q.model_name + "','" 
					+ q.file_name + "','" 
					+ q.json_data + "')";

	console.log(sql);
	db.query(sql, (err, data) => {
		console.log(data);
		if(!err) res.send({ result : 1 });
		else{
			console.log(err);
			res.send({result:0, data:err});
		} 
	})
});

app.post('/get_atomic_info', (req, res) => {
	let q = Object.keys(req.query).length === 0 ? req.body:req.query;
	if(q.project_name === undefined || q.project_name == ''
		|| q.file_name === undefined || q.file_name == ''
	){
		res.send({result : "error"});
		console.log("error");
		return;
	}
	sql = "call get_atomic('" + q.project_name + "','" + q.file_name + "')";
	db.query(sql, (err, data) => {
		if(!err) res.send({ result : 1, data:data });
		else res.send({result:0, data:err});
	})
});

app.post('/set_atomic_info',(req, res) => {
	let q = Object.keys(req.query).length === 0 ? req.body:req.query;
	console.log(q);
	if(q.project_name === undefined || q.project_name == ''){
		res.send({result : "error : project_name is null"});
		return;
	}
	else if(q.model_name === undefined || q.model_name == ''){
		res.send({result : "error : model_name is null"});
		return;
	}
	else if(q.model_type === undefined){
		res.send({result : "error : model_type is null"});
		return;
	}
	else if(q.file_name === undefined || q.file_name == ''){
		res.send({result : "error : file_name is null"});
		return;
	}
	else if(q.file_path === undefined || q.file_path == ''){
		res.send({result : "error : file_path is null"});
		return;
	}
	else if(q.ip === undefined || q.ip == ''){
		res.send({result : "error : local_ip is null"});
		return;
	}
	else if(q.iid === undefined || q.iid == ''){
		res.send({result : "error : iid is null"});
		return;
	}
	let location = q.ip + ':' + q.port;
	let sql = "call update_atomic('"
					+ q.iid + "','" 
					+ q.project_name + "'," 
					+ q.model_type + ",'"
					+ q.model_name + "','" 
					+ q.file_name + "','" 
					+ location + "','" 
					+ JSON.stringify(q) + "')";
					 
	console.log(sql);
	db.query(sql, (err, data) => {
		console.log(data);
		if(!err) res.send({ result : 1 });
		else{
			console.log(err);
			res.send({result:0, data:err});
		} 
	});
});

app.listen(port, '0.0.0.0', () => {
  console.log(`listening on port ${port}`)
})


app.post('/get_atomic_list', (req, res) => {
	sql = "call get_atomic_list()";
	console.log(sql);
	db.query(sql, (err, data) => {
		if(!err) {
			res.send({ result : 1, data:data });
		}
		else {
			res.send({result:0, data:err});
		}
	})
});