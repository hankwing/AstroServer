// 公共配置
var curCoordType = 1,// 当前选中的坐标系类型，默认是赤道
    STAR_SIZE = 30,// 星星大小
    earthRadius = 200,// 地球半径
    enlargement = 3,// 星星半径放大倍数
    backColor = 0x05143c,// 三维画布背景�?
    starColor = 0x01e7eb,// 星星颜色
    text_options = {// 文字属�?
      size: 12,
      height: 0,
      weight: 0,
      font: 0,
      bevelThickness: 0,
      bevelSize: 0,
      bevelSegments: 0,
      bevelEnabled: 0,
      curveSegments: 1,
      steps: 1
    },
    TEMP_LENG = 20 * 60 * 24,// 前台缓存的点最大数�?
    URL_EXCEPTION = "/anomalystarlist",// 异常星接�?
    URL_CURVE = "/star",// 光变曲线接口
    URL_SEARCH = "/url",// 检索接�?
    DATA_PATH_01 = "../data/home.csv/part-00000",// 前台模拟数据�?
    DATA_PATH_02 = "../data/home.csv/part-00001",
    DATA_PATH_03 = "../data/home.csv/part-00002",
    PATH_FONT = '../fonts/helvetiker_regular.typeface.json',
    IS_SIMULATE = false;// 是否使用前台模拟数据

// 公共变量
var mapDiv = document.querySelector("#map"),// 地图DIV
    width = mapDiv.clientWidth,
    height = mapDiv.clientHeight,
    lightDiv = document.querySelector(".light-curve"),// 光变曲线DIV
    lw = lightDiv.clientWidth,
    lh = lightDiv.clientHeight,
    pointsSystem = [],// 点几何体数组（分批次�?
    earthGroup = new THREE.Object3D(),// 地球几何�?
    points = [],// 加载的点全集
    pointsBatch = [],// 分批的星�?
    curveArr = [],// 光变曲线的点
    batch = 0,//第几�?
    exceptionStarInterval,// 异常星轮�?
    starCurveInterval,// 光变曲线轮询
    projector = new THREE.Projector(),
    raycaster = new THREE.Raycaster(),// 射线
    mouse = new THREE.Vector2(),// 点击的鼠标位�?
    magScale = d3.scaleLinear().domain([8,30]).range([5,8,10]).clamp(true),// 亮度比例�?
    scene = new THREE.Scene(),
    renderer = new THREE.WebGLRenderer({alpha: true}),
    camera = new THREE.PerspectiveCamera(45,width/height,0.1,100000),
    orbit = new THREE.OrbitControls(camera,renderer.domElement),
    utils = new Utils(),
    line = new Line().container(d3.select("#light"))// 光变曲线
      .width(lw-30)
      .height(lh-80)
      .accessorX(function(d){ return +d.timestamp; })
      //.accessorX(function(d, i){ return i; })
      .accessorY(function(d){ return +d.mag; }),
    scatter = new Scatter().container(d3.select("#map"))// 二维散点�?
      .width(width)
      .height(height)
      .accessorX(function(d){ return +d.x_pix; })
      .accessorY(function(d){ return +d.y_pix; })
      .accessorR(function(d){ return magScale(d.mag); })
      .fill(function(d){ return d.new ? "#ffff00" : "#7efaff" }),
    pointMaterial = new THREE.ShaderMaterial({// 自定义点材质
      uniforms: {
        color: { value: new THREE.Color( starColor ) },
        texture: { value: new THREE.TextureLoader().load( "../images/disc.png" ) }
      },
      vertexShader: document.getElementById( 'vertexshader' ).textContent,
      fragmentShader: document.getElementById( 'fragmentshader' ).textContent,
      alphaTest: 0.9
    });

// 初始�?
renderer.setSize(width, height);
camera.position.x = -earthRadius+300;
camera.position.y = 300;
camera.position.z = earthRadius+300;
camera.lookAt(scene.position);
orbit.zoomIn(.7);
