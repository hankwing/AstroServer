function Line(){
  // 内部变量
  var container = d3.select('body'),// 画布容器
    points = [],
    width = 500,
    height = 500,
    k = height / width,
    x = d3.scaleLinear(),
    y = d3.scaleLinear(),
    z = d3.schemeCategory10,
    radius = 1,
    padding = 0,
    accessorY = function(d){ return d[1]; },
    accessorX = function(d){ return d[0]; },
    line = d3.line().curve(d3.curveBasis);
    
  function _chart(){

  }

  _chart.draw = function(){
    // 容器
    container.html(null);
    container.style('width',width+"px").style('height',height+"px");
    var svg = container.append("svg").attr('width',width).attr('height',height),
        canvas = container.append("canvas").attr('width',width).attr('id','line').attr('height',height),
        context = canvas.node().getContext("2d");

    // 求数据范围
    x.domain(d3.extent(points, accessorX)).range([0, width]);
    y.domain(d3.extent(points, accessorY)).range([height, 0]);
    k = height / width;
    
    line.x(function(d){ return x(accessorX(d)); }).y(function(d){ return y(accessorY(d)); }).context(context);

    // 绘制坐标轴
    var xAxis = d3.axisTop(x).ticks(5),
        yAxis = d3.axisRight(y).ticks(5);

    var gx = svg.append("g")
        .attr("class", "axis axis--x")
        .attr("transform", "translate(0," + (height-padding -1) + ")")
        .call(xAxis);

    var gy = svg.append("g")
        .attr("class", "axis axis--y")
        .attr("transform", "translate(0,0)")
        .call(yAxis);

    // grid
    // container.selectAll('.axis--x .tick line').attr('y2', -height)
    // container.selectAll('.axis--y .tick line').attr('x2', width)

    context.beginPath();
    line(points);
    context.lineWidth = 1.5;
    // context.strokeStyle = "#05143c";
    context.strokeStyle = "#30e590";
    context.stroke();

    return _chart;
  }

  _chart.container = function(_){
    if(!_) return container;
    // TODO id node selection
    container = _;
    return _chart;
  }

  _chart.width = function(_){
    if(!_) return width;
    width = _;
    return _chart;
  }

  _chart.height = function(_){
    if(!_) return height;
    height = _;
    return _chart;
  }

  _chart.points = function(_){
    if(!_) return points;
    points = _;
    return _chart;
  }

  _chart.accessorX = function(_){
    if(!_) return accessorX;
    accessorX = _;
    return _chart;
  }

  _chart.accessorY = function(_){
    if(!_) return accessorY;
    accessorY = _;
    return _chart;
  }

  return _chart;
}