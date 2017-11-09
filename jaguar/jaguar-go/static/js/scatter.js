function Scatter(){
  // 内部变量
  var container = d3.select('body'),// 画布容器
    points = [],
    width = 500,
    height = 500,
    zoomPan = 'zoom',
    k = height / width,
    x = d3.scaleLinear(),
    y = d3.scaleLinear(),
    z = d3.schemeCategory10,
    radius = 1,
    padding = 20,
    callbackMap = {},
    drawed = false,// 是否绘制过
    zoom = null,
    drag = null, // 缓存上次的定义
    dispatch = d3.dispatch('search'),
    accessorR = function(d){ return radius; },
    accessorY = function(d){ return d[1]; },
    accessorX = function(d){ return d[0]; };

  function _chart(){

  }

  _chart.draw = function(){
    // 容器
    if(container.select('svg').empty() || container.select('canvas').empty()) drawed = false;
    var svg = drawed ? container.select('svg') : container.append("svg").attr('width',width).attr('height',height),
        canvas = drawed ? container.select('canvas') : container.append("canvas").attr('width',width).attr('id','scatter').attr('height',height),
        context = canvas.node().getContext("2d");

    if(drawed) {
      if(zoomPan == 'zoom') canvas.call(zoom).on('.drag',null)
      else if(zoomPan == 'drag') canvas.call(drag).on('.zoom',null); // canvas.on('zoom',null); (not working)
      return;// 已经绘制过就不重绘了
    }

    // 求数据范围
    x.domain(d3.extent(points, accessorX)).range([0, width]);
    y.domain(d3.extent(points, accessorY)).range([height, 0]);
    k = height / width;
    
    // 绘制坐标轴
    var xAxis = d3.axisTop(x).ticks(10),
        yAxis = d3.axisRight(y).ticks(10 * k);

    var gx = svg.append("g")
        .attr("class", "axis axis--x")
        .attr("transform", "translate(0," + (height-padding) + ")")
        .call(xAxis);

    var gy = svg.append("g")
        .attr("class", "axis axis--y")
        .attr("transform", "translate(0,0)")
        .call(yAxis);

    // // grid
    // container.selectAll('.axis--x .tick line').attr('y2', -height)
    // container.selectAll('.axis--y .tick line').attr('x2', width)

    // SVG绑定缩放事件
    zoom = d3.zoom()
      .on("zoom", zoomed);
    drag = d3.drag()
      .subject(dragsubject)
      .on("start", start)
      .on("drag", dragged)
      .on("drag.render end.render", dragged);

    canvas.call(drag);
    // canvas.call(zoom).on('mousedown.zoom',null);

    function dragsubject() {
      var i = 0,
          n = points.length,
          dx,
          dy,
          d2,
          // s2 = radius * radius * 4,
          s2 = 32,
          circle,
          subject;

      for (i = 0; i < n; ++i) {
        circle = points[i];
        dx = d3.event.x - x(accessorX(circle));
        dy = d3.event.y - y(accessorY(circle));
        d2 = dx * dx + dy * dy;
        if (d2 < s2) subject = circle, s2 = d2;
      }

      return subject;
    }

    function dragged(){
      console.log(d3.event.subject);
      if(callbackMap.click) {
        var node = d3.event.subject;
        handleClick(node);
      }
    }

    // 处理点击事件
    var handleClick = function(node){
        callbackMap.click(node);
        var nx = x(accessorX(node));
        var ny = y(accessorY(node));
        // 绘制一个方框和一文字
        context.strokeStyle= "#30e590";
        context.strokeRect(nx-1, ny-1, 4, 4);
        context.font = '16px';
        context.textAlign = 'left';
        context.textBaseline = 'top';
        context.fillStyle = '#30e590';
        context.fillText('['+accessorX(node)+' , '+accessorY(node)+']', nx+6, ny-6);
    }

    document.addEventListener('search',function(d){ 
      var searched = points.filter(function(f){ return f.star_id == d.star_id; });
      if(searched.length > 0) handleClick(searched[0]);
    });

    function start(){
      d3.event.sourceEvent.stopPropagation();
      console.log('start');
    }

    svg.selectAll(".domain")
      .style("display", "none");

    drawPoints(x,y);
    function zoomed() {
      // 重置比例尺
      var transform = d3.event.transform,
          zx = transform.rescaleX(x),
          zy = transform.rescaleY(y);

      // 更新轴的比例尺
      gx.call(xAxis.scale(zx));
      gy.call(yAxis.scale(zy));

      drawPoints(zx,zy);
      
    }

    // 绘制节点
    function drawPoints(x,y){
      // 重新绘制点
      context.clearRect(0, 0, width, height);
      
      for (var i = 0, n = points.length, p, px, py; i < n; ++i) {// 其中一个数据集
        p = points[i], px = x(accessorX(p)), py = y(accessorY(p));
        context.beginPath();
        context.fillStyle = "#7efaff";
        context.moveTo(px + 2.5, py);
        context.arc(px, py, accessorR(p), 0, 2 * Math.PI);
        context.fill();
      }
    }

    drawed = true;// 绘制过
    return _chart;
  }

  _chart.switchZommPan = function(_){
    if(!_) return zoomPan;
    zoomPan = _;
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

  _chart.accessorR = function(_){
    if(!_) return accessorR;
    accessorR = _;
    return _chart;
  }

  _chart.callbackMap = function(_){
    if(!_) return callbackMap;
    for(var key in _) callbackMap[key] = _[key];
    return _chart;
  }

  _chart.searchById = function(_){
    var evt = document.createEvent("HTMLEvents");
    evt.initEvent("search", false, false);
    evt.star_id = _;
    document.dispatchEvent(evt);
  }

  return _chart;
}