angular
.module('ui_2.graphs', ['ui_2.deps'])
.directive('graphs', ['$', function($) {
	var ratio = 0.6
	
	return function (scope, elem, attrs) {
		var canDraw = false;
		
		elem.height (elem.width() * ratio)
		
		var graphSize = 100,
			inval = 0;
			conf = {
			label: attrs.label,
			data: [],
			color: "#00ff00",
			lines: {show: true}
		}
		
		var graph = $.plot(elem, [conf], {
			legend: {show: true},
			xaxis: {
				show: true
			},
			yaxis: {
				show: true,
				min: 0
			}
		});
			
		var draw = function () {
		
			if (elem.width() == 0 || elem.height() == 0)
				return;
				
			graph.setData([conf]);
			graph.setupGrid();
			graph.draw();
		};
		
		function update () {
			var current = new Date ();
			current = Date.UTC (
				current.getFullYear(),
				current.getMonth(),
				current.getDate(),
				current.getHours(),
				current.getMinutes(),
				current.getSeconds());
				
				if (conf.data.length === graphSize)
					conf.data.shift();
				conf.data.push([current, inval]);
				
				if (canDraw) 
					draw();
		}
		
		scope.$watch(attrs.inval, function(val) {
			if (val === undefined)
				return;
				
			inval = parseFloat(val) || 0;
		});

		scope.$watch(attrs.draw, function(val) {
			canDraw = val;
		});
		
		scope.$watch(attrs.label, function(val) {
			if (val === undefined)
				return;
				
			conf.label = val;
		});
		
		var interval = setInterval(update, 1000);
		
		angular.element(window).bind('resize', draw);
		elem.bin('$destroy', function() {
			clearInterval(interval);
		});
		
	};
}]);
