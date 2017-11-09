// 初始化
scatter.callbackMap({'click': startPollingDetail});

// 初始化加载数据
loadData().then(redrawEquatorial);// 初始渲染的是赤道坐标系

// 持续渲染
render();
// ---------------------------------------------------------

/**
 * @Function [加载数据，本地数据，字体数据]
 * @return   null
 * @DateTime 2017-06-25
 */
function loadData(){
  var p = new Promise(function(resolve, reject){
    // 模拟数据
    if(IS_SIMULATE) d3.queue()
    .defer(d3.csv, DATA_PATH_01, typePoint)
    .defer(d3.csv, DATA_PATH_02, typePoint)
    .defer(d3.csv, DATA_PATH_03, typePoint)
    .await(function ready(){
      if(arguments.length < 2) throw "未加载数据";
      points = [].slice.call(arguments,1).reduce(function(p,c){ return p.concat(c); });
      resolve();
    });

    // 加载字体
    var loader = new THREE.FontLoader();
    loader.load( PATH_FONT, function ( font ) {
      text_options.font = font;
      !IS_SIMULATE && resolve();
    } );

  });
   
  return p;  
}

/**
 * @Function [原始数据转换]
 * @param    obj
 * @return   obj
 * @DateTime 2017-06-25
 */
function typePoint(d){
  return {
    x_pix: +d.XPix,
    y_pix: +d.YPix,
    ra: +d.Ra,
    dec: +d.Dec,
    zone: +d.Zone,
    star_id: d.StarId,
    mag: +d.Mag,
    timestamp: +d.Timestamp,
    ellipticity: +d.Ellipticity,
    ccd_num: +d.CcdNum
  }
}

/**
 * @Function [检索]
 * @param    dom
 * @return   null
 * @DateTime 2017-06-25
 */
function search(self){
  var search_ra = $("#search_ra").val(),
      search_dec = $("#search_dec").val(),
      search_r = $("#search_r").val(),
      search_star_id = $("#search_star_id").val(),
      search_mag = $("#search_mag").val();

  if(!search_ra && !search_dec && !search_r && !search_star_id && !search_mag) return;
  d3.request(URL_SEARCH+
    "?search_ra="+search_ra+
    "&search_dec="+search_dec+
    "&search_r="+search_r+
    "&search_star_id="+search_star_id+
    "&search_mag="+search_mag)
    .mimeType("application/json")
    .response(function(xhr) { 
      return JSON.parse(xhr.responseText); 
    })
    .get(function(arr){
      // 模拟数据，随机生成光变数组
      if(IS_SIMULATE) arr = d3.range( Math.floor(Math.random() * 150) ).map(function(d){
        return points[Math.floor(Math.random() * points.length)];
      });

      // 关闭轮询
      closePollingException();
      closePollingCurve();
      renderExceptionStar(arr, curCoordType == 1 ? '3d' : '2d'); 
      if(arr.length == 1) startPollingDetail(arr[0]);
    });
}

/**
 * @Function [绘制策略]
 * @param    dom
 * @param    num,绘制类型
 * @return   null
 * @DateTime 2017-06-25
 */
function draw(self, type){
  // 当前active
  $(".nav-tabs li").removeClass('active');
  $(self).closest('li').addClass('active');
  closePollingException();
  if(type==1) redrawEquatorial();// 赤道坐标系
  else if(type==2) redrawCartesian2();// 二维笛卡尔坐标系
}

/**
 * @Function [重绘赤道坐标系]
 * @return   null
 * @DateTime 2017-06-25
 */
function redrawEquatorial(){
  curCoordType = 1;
  $(".radio-group").hide();
  d3.select('#map').html('');
  // 清除重绘
  utils.delete3DOBJ('earthGroup', scene);

  scene = new THREE.Scene();
  scene.fog = new THREE.Fog(0x000000,earthRadius*1.5,earthRadius*10);

  earthGroup = new THREE.Object3D();
  earthGroup.name = 'earthGroup';
  // utils.addEarth(earthGroup,earthRadius,"../images/earth.jpg");

  // 点二维数组
  var shell = utils.getShell(earthRadius*3,72, 72);
  earthGroup.add(shell);
  scene.add(earthGroup);

  // 绘制点
  // drawEquatorialPoints(points, earthGroup);
  startPollingException('3d');
  document.querySelector("#map").appendChild(renderer.domElement);
}

/**
 * @Function [二维笛卡尔坐标系]
 * @return   mull
 * @DateTime 2017-06-20
 */
function redrawCartesian2(){
  curCoordType = 2;
  $(".radio-group").show();
  d3.select("#map").html(null);
  startPollingException('2d');
  // 切换选择方式
  $(".radio").click(function(){
    scatter.switchZommPan($('input:radio:checked').val()).draw();
  });
}

/**
 * @Function [点击星空]
 * @param    event
 * @return   null
 * @DateTime 2017-06-20
 */
function clickMap(event){
  if(curCoordType != 1 && curCoordType != 3) return;
  event.preventDefault();
  mouse.x = ( event.offsetX / width ) * 2 - 1,
  mouse.y = - ( event.offsetY / height ) * 2 + 1;
  raycaster.setFromCamera( mouse, camera );
  var result = getIntersectPoint();
  if(result.index >= 0){
    var attributes = pointsSystem[+result.intersect.object.name].geometry.attributes;
    var INTERSECTED = result.intersect.index;
    attributes.size.array[ INTERSECTED ] = attributes.size.array[ INTERSECTED ] == 10 ? 1 : 10;
    var color = new THREE.Color();
    color.setHSL( 0.1*2, 1, .5);
    color.toArray( attributes.customColor.array, INTERSECTED * 3 );

    attributes.size.needsUpdate = true;
    attributes.customColor.needsUpdate = true;
    startPollingDetail(pointsBatch[result.index][INTERSECTED]);
    utils.showQuadrillage(pointsBatch[result.index][INTERSECTED], scene);
  } 
}

/**
 * @Function [从批次几何体中获取点击的星星]
 * @return   { index: number, intersect: THREE.BufferGeometry };
 * @DateTime 2017-06-20
 */
function getIntersectPoint(){
  for (var i = pointsSystem.length - 1; i >= 0; i--) {
    var intersects = raycaster.intersectObject(pointsSystem[i]);
    if(intersects.length > 0) return { index: i, intersect: intersects[0] };
  }
  return { index: -1, intersect: null };
}

/**
 * @Function [显示星星详情和光变曲线]
 * @NOTE     函数中用到了所有信息数组points，连线组件line
 * @param    选中的星星，包含属性有：['x_pix','y_pix','ra','dec','zone','ccd_num','star_id','mag','timestamp','ellipticity','mag']
 * @return   null
 * @DateTime 2017-06-18
 */
function showStarDetail(stars){
  var star = stars[stars.length-1];
  // 显示星星信息
  ['x_pix','y_pix','ra','dec','zone','ccd_num','star_id',/*'mag','timestamp',*/'ellipticity','mag'].forEach(function(key){
    $('#'+key).text(star[key]);
  });

  // 显示光变曲线
  stars.forEach(function(star){
    if(curveArr.length > TEMP_LENG) {
      curveArr.shift();
    }
    curveArr.push(star);
  });
  var extent = d3.extent(curveArr, function(d){ return +d.mag; });
  var padding = 2;
  line.yDomain([extent[0]-padding, extent[1]+padding])
    .points(curveArr.sort(function(a,b){ return a.timestamp - b.timestamp; }))
    .draw();
}

/**
 * @Function [循环渲染]
 * @return   null
 * @DateTime 2017-06-18
 */
function render(){
  orbit.update();
  // earthGroup.rotation.y += 0.003;
  renderer.render(scene,camera);
  requestAnimationFrame(render);
}

/**
 * @Function [轮询异常星星]
 * @return   null
 * @DateTime 2017-06-20
 */
function startPollingException(type){
  exceptionStarInterval = setInterval(function(){
    d3.request(URL_EXCEPTION)
      .mimeType("application/json")
      .response(function(xhr) { 
        return JSON.parse(xhr.responseText); 
      })
      .get(function(arr){
        if(!arr) arr = [];// Fix 空数据后台返回null
        // 模拟数据，随机生成光变数组
        if(IS_SIMULATE) arr = d3.range( Math.floor(Math.random() * 150) ).map(function(d){
          return points[Math.floor(Math.random() * points.length)];
        });
        renderExceptionStar(arr.map(function(d){ return typePoint(d); }), type);
      });
  }, 3000);
}

/**
 * @Function [轮询光变曲线]
 * @return   null
 * @DateTime 2017-06-25
 */
function startPollingDetail(star){
  // 每次新开启就关闭之前的轮询
  clearInterval(starCurveInterval);
  starCurveInterval = setInterval(function(){
    d3.request(URL_CURVE+"?id="+star.star_id)
      .mimeType("application/json")
      .response(function(xhr) { 
        return JSON.parse(xhr.responseText); 
      })
      .get(function(arr){
        // 模拟数据，随机生成光变数组
        if(IS_SIMULATE) arr = d3.range( Math.floor(Math.random() * TEMP_LENG) ).map(function(d){
          return points[Math.floor(Math.random() * points.length)];
        }).sort(function(a,b){
          return a.timestamp - b.timestamp;
        });
        showStarDetail(arr.map(function(d){ return typePoint(d);}));
      });
  }, 3000);
}

/**
 * @Function [关闭异常星轮询]
 * @return   null
 * @DateTime 2017-06-25
 */
function closePollingException(){
  clearInterval(exceptionStarInterval);
}

/**
 * @Function [关闭光变曲线轮询]
 * @return   null
 * @DateTime 2017-06-26
 */
function closePollingCurve(){
  clearInterval(starCurveInterval);
}

/**
 * @Function [显示新的异常星]
 * @param    星星数组
 * @param    2d/3d
 * @return   null
 * @DateTime 2017-06-25
 */
function renderExceptionStar(stars, type){
  // 绘制点
  type == '3d' ? drawEquatorialPoints(stars, earthGroup) : drawScatterPoints(stars);
}

/**
 * @Function [绘制散点图]
 * @param    点数组
 * @return   null
 * @DateTime 2017-06-25
 */
function drawScatterPoints(points){
  // 如果超过阈值则删除
  if(pointsBatch.length > TEMP_LENG ){
    pointsBatch.shift();
  }
  // 上一批次白兰
  if(pointsBatch.length > 0){
    var batch = pointsBatch[pointsBatch.length-1];
    for(var i = 0; i < batch.length; i++){
      batch[i].new = false;
    }
  } 

  points.forEach(function(d){ d.new = true; });

  pointsBatch.push(points);

  scatter.points(pointsBatch.reduce(function(a,b){
    return a.concat(b)
  })).reDraw();
}

/**
 * @Function [动态绘制点，删除过期数据，标颜色等]
 * @param    点数据
 * @param    天球几何体分组
 * @return   null
 * @DateTime 2017-06-20
 */
function drawEquatorialPoints(points, earthGroup){
  var positions = new Float32Array(points.length*3);
  var colors = new Float32Array(points.length*3);
  var sizes = new Float32Array(points.length);
  
  // 如果超过阈值则删除
  if(pointsBatch.length > TEMP_LENG ){
    var toDel = pointsSystem.shift();
    pointsBatch.shift();
    deletePoints(toDel.name, earthGroup);
  }
  // 上一批次白兰
  if(pointsBatch.length > 0){
    var color = new THREE.Color();
    color.setRGB( 0, 255, 255);// 白兰
    var attributes = pointsSystem[pointsSystem.length-1].geometry.attributes;
    for(var i = 0; i < attributes.customColor.array.length; i+=3){
      attributes.customColor.array[i] = 0;
      attributes.customColor.array[i+1] = 255;
      attributes.customColor.array[i+2] = 255;
      attributes.customColor.needsUpdate = true;
    }
  } 

  // 当前批次黄色
  points.forEach(function(d,i){
    var color = new THREE.Color();
    color.setRGB( 255, 255, 0);// 黄色
    var lng = parseFloat(d.ra),// 赤经 
        lat = parseFloat(d.dec);// 赤纬
    var vertex = utils.spherical2Cartesian(earthRadius*3, lat, lng);
    sizes[ i ] =  0.1*d.mag*d.mag;
    color.toArray( colors, i * 3 );
    vertex.toArray( positions, i * 3 );
  });

  var pointGeom = new THREE.BufferGeometry();
  pointGeom.addAttribute( 'customColor', new THREE.BufferAttribute( colors, 3 ) );
  pointGeom.addAttribute( 'position', new THREE.BufferAttribute( positions, 3 ) );
  pointGeom.addAttribute( 'size', new THREE.BufferAttribute( sizes, 1 ) );

  var pointsGeom = new THREE.Points(pointGeom,pointMaterial);
  pointsGeom.name = batch++;
  earthGroup.add(pointsGeom);
  pointsSystem.push(pointsGeom);
  pointsBatch.push(points);
}

/**
 * @Function [从分组中删除几何体]
 * @param    几何体名称
 * @param    几何体分组
 * @return   null
 * @DateTime 2017-06-20
 */
function deletePoints(name, group){
  for(var i = 0; i < group.children.length; i++){
    if(group.children[i].name === name) {
      group.children.splice(i,1);
    }
  }
}
