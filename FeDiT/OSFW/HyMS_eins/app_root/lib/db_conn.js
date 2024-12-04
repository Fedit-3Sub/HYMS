//import React, { Component } from 'react';

var mysql = require('mysql');
//const logger = require('./logger')

const db = mysql.createPool({
    host : "172.28.0.4",
    user : "model_designer", //mysqlÀÇ id
    password : "mdesigner!@#", //mysqlÀÇ password
    port : 3306,
    database : "model_designer", //»ç¿ëÇÒ µ¥ÀÌÅÍº£ÀÌ½º
    connectionLimit : 4,
    multipleStatements : true,
});

module.exports = db;
