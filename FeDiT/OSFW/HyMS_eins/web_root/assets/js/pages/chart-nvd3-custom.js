'use strict';
let line_graph = {};
function init_graph(){
	line_graph = {};
}
function create_graph(graph_id){
	nv.addGraph(function() {
		var chart = nv.models.lineChart()
			.useInteractiveGuideline(false)  //We want nice looking tooltips and a guideline!
			//.forceY([0,66])
			
			.showLegend(true)
			.showYAxis(true)        //Show the y-axis
			.showXAxis(true);        //Show the x-axis
		chart.xAxis
			.axisLabel('Time(s)')
			.tickFormat(d3.format(',r'));

		chart.yAxis
			.axisLabel('y')
			.tickFormat(d3.format('.04f'));
			
	    //var myData = [];//sinAndCos(); //You need data...
		var myData = sinAndCos(); //You need data...

		d3.select('#'+graph_id+' svg') //Select the <svg> element you want to render the chart in.
			.datum(myData) //Populate the <svg> element with chart data...
			.call(chart);

		//Update the chart when window resizes.
		nv.utils.windowResize(function() {
			chart.update()
		});
		line_graph[graph_id] = chart;
		
		/*
		line_graph.legend.dispatch.on('legendClick', function(d,i) {
			
			if(i>0)
			   graph_link_ids.splice(i, 1);
			graph_update(animated_time);
			//var dt = d3.select('#nvd3-line-1 svg').datum();   // <-- all data
			//console.log("legendClick", d);
			//console.log("i :", i);
		});
		chart.legend.dispatch.on('legendMouseover', function(e,i) {
			console.log(e, i);
			if(i > 0){
				
				var key = graph_link_ids[i].key;
				console.log(key);
				link_info[key][6].polygon.material = Cesium.Color.MAGENTA ;
				selLinkEntity = link_info[key][6];

			}
		});
		*/
		return chart;
	});
}
function sinAndCos(si = 100) {
    var sin = [],
        sin2 = [],
        cos = [];
    for (var i = 0; i < si; i++) {
		if(i>10){
        sin.push({
            x: i+0.1,
            y: Math.sin(i / 10)
        });
		}
        sin2.push({
            x: i,
            y: Math.sin(i / 10) * 0.25 + 0.5
        });
        cos.push({
            x: i,
            y: .5 * Math.cos(i / 10)
        });
    }
    return [{
            values: sin,
            key: 'Sine Wave',
            color: '#A389D4'
        },
        {
            values: cos,
            key: 'Cosine Wave',
            color: '#04a9f5'
        },
        {
            values: sin2,
            key: 'Another sine wave',
            color: '#1de9b6',
            //area: true
        }
    ];
}
