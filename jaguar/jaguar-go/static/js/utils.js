function Utils(){

  return {
    /**
     * @Function [球坐标转换笛卡尔坐标]
     * @param    半径,number
     * @param    theta角,number
     * @param    phi角,number
     * @return   THREE.Vector3
     * @DateTime 2017-06-18
     */
    spherical2Cartesian: function(R, theta, phi){
      var x = R*Math.sin(theta)*Math.cos(phi);// x=rsinθcosφ
      var y = R*Math.sin(theta)*Math.sin(phi);// y=rsinθsinφ
      var z = R*Math.cos(theta);// z=rcosθ
      // return { x:x, y:y, z:z }
      return new THREE.Vector3( x, y, z );
    },
    /**
     * @Function [删除几何体]
     * @param    THREE.Object3D对象
     * @param    THREE.Scene
     * @return   null
     * @DateTime 2017-06-18
     */
    delete3DOBJ: function(objName, scene){
      var selectedObject = scene.getObjectByName(objName);
      selectedObject && scene.remove( selectedObject );
    },
    /**
     * @Function [由顶点集合生成曲线]
     * @param    点集合,[THREE.Vector3,...]
     * @param    分段数,number
     * @param    颜色,0xXXXXXX
     * @return   THREE.Line
     * @DateTime 2017-06-18
     */
    curve: function(vertices, segments, color){
      var curve = new THREE.CatmullRomCurve3 (vertices);

      var geometry = new THREE.Geometry();
      geometry.vertices = curve.getPoints(segments);
      var material = new THREE.LineBasicMaterial({ 
        color: color ? color : 0xffffff, 
        opacity: 0.2, 
        fog: true 
      });
      var line = new THREE.Line(geometry, material);
      return line;
    },
    /**
     * @Function [投影转换为平面坐标]
     * @param    经度,number
     * @param    纬度,number
     * @param    半径,number
     * @return   THREE.Vector3
     * @DateTime 2017-06-18
     */
    projection: function(lng, lat, r){
      var theta = (lng+180)*(Math.PI/180),
        phi = (90-lat)*(Math.PI/180),
        radius = r;
      return (new THREE.Vector3()).setFromSpherical(new THREE.Spherical(radius,phi,theta));
    },
    /**
     * @Function [添加地理贴图]
     * @param    地球,THREE.Object3D
     * @param    半径,number
     * @DateTime 2017-06-18
     * @Useage   addEarth(earthGroup,earthRadius,"../images/earth.jpg");
     */
    addEarth: function(gruop,r,path){
      var sphereGeom = new THREE.SphereGeometry(r,40,40,Math.PI/2);
      var loader = new THREE.TextureLoader();
      loader.load(path,function(t){
        var material = new THREE.MeshBasicMaterial({
          map:t
        });
        gruop.add(new THREE.Mesh(sphereGeom,material));
      })
    },
    /**
     * @Function [创建网格（多材质，目前只支持MeshBasicMaterial）]
     * @param    THREE.TextGeometry
     * @param    0xXXXXXX
     * @return   Object3D
     * @DateTime 2017-06-18
     */
    createMesh: function(geom, color) {
      var meshMaterial = new THREE.MeshBasicMaterial({
        color: color ? color : 0xffffff
      });

      var plane = THREE.SceneUtils.createMultiMaterialObject(geom, [meshMaterial]);// 可支持多种材质

      return plane;
    },
    /**
     * @Function [经纬网壳子]
     * @NOTE     复合工具，用到了其他原子方法：this.createMesh，this.curve，this.spherical2Cartesian
     * @param    半径,number
     * @param    行数,number
     * @param    列数,number
     * @return   THREE.Object3D
     * @DateTime 2017-06-18
     */
    getShell: function(R,row,col){
      var group = new THREE.Object3D();
      var points = [];
      var phi=0,theta=0;
      var perPhi = 2*Math.PI/col;
      var perTheta = Math.PI/row;

      // 生成经纬网坐标（首尾多一个坐标，便于连线时j+1）
      for(var i=0; i<=row; i++){
        phi = perPhi * i;
        if(points[i] === undefined) points[i] = [];
        for(var j=0; j<=col; j++){
          theta = perTheta * j;    
          points[i][j] = this.spherical2Cartesian(R, theta, phi);
        }
      }

      // 经线
      for(var i=0; i<row; i+=6){// 经线
        var vertices = [];
        for(var j=0; j<col; j++){// 纬线
          vertices.push(points[i][j],points[i][j+1]);
          
          if(i%6==0 && (j+3)%6==0){
            text = this.createMesh(new THREE.TextGeometry( i/3+"h00", text_options));
            var arr = points[i][j].toArray();
            text.position.set(arr[0], arr[1], arr[2]);
            group.add(text);
          } 
          
        }
        var line = this.curve(vertices, 50);
        group.add(line);
      }

      // 纬线
      for(var i=0; i<col; i+=6){// 纬线
        var vertices = [];
        for(var j=0; j<row; j++){// 经线
          vertices.push(points[j][i],points[j+1][i]);
          
          if(i%6==0 && (j+3)%6==0){
            text = this.createMesh(new THREE.TextGeometry( Math.abs( Math.ceil(180*perTheta * i/Math.PI) - 90 )+"°", text_options));
            var arr = points[j][i].toArray();
            text.position.set(arr[0], arr[1], arr[2]);
            group.add(text);
          } 
        }
        var line = this.curve(vertices, 50);
        group.add(line);
      }

      return group;
    },
    /**
     * @Function [显示经纬线]
     * @NOTE     复合工具，用到了其他原子方法：this.createMesh，this.curve，this.spherical2Cartesian
     * @param    点，obj = {ra, dec}
     * @param    THREE.Scene
     * @return   null
     * @DateTime 2017-06-18
     */
    showQuadrillage: function(d, scene){
      utils.delete3DOBJ('quadrillageGroup', scene);
      var R = earthRadius*3;
      var quadrillageGroup = new THREE.Object3D();
      quadrillageGroup.name = 'quadrillageGroup';
      var col = row = 72;
      var perPhi = 2*Math.PI/col;
      var perTheta = 2*Math.PI/row;
      var R = earthRadius*3;
      var ras = [],decs = [];
      
      // 经线
      phi = d.ra;
      for(var i=0; i<=col; i++){
        theta = perTheta * i;  
        ras[i] = this.spherical2Cartesian(R, theta, phi);
        if((i+3)%6==0){
          text = utils.createMesh(new THREE.TextGeometry( (phi/15).toFixed(1)+"h00", text_options), 0x30e590);
          var arr = this.spherical2Cartesian(R, theta, phi).toArray();
          text.position.set(arr[0], arr[1], arr[2]);
          quadrillageGroup.add(text);
        } 
      }

      var line1 = utils.curve(ras, 50, 0x30e590);

      // 纬线
      theta = d.dec;
      for(var i=0; i<=row; i++){
        phi = perPhi * i;     
        decs[i] = this.spherical2Cartesian(R, theta, phi);
        if((i+3)%6==0){
          text = this.createMesh(new THREE.TextGeometry( theta.toFixed(1)+"°", text_options), 0x30e590);
          var arr = this.spherical2Cartesian(R, theta, phi).toArray();
          text.position.set(arr[0], arr[1], arr[2]);
          quadrillageGroup.add(text);
        } 
      }

      var line2 = utils.curve(decs, 50, 0x30e590);
      quadrillageGroup.add(line1);
      quadrillageGroup.add(line2);
      scene.add(quadrillageGroup);
    },
    /**
     * @Function [初始化监控控件]
     * @return   Stats
     * @DateTime 2017-06-18
     */
    initStats: function(){
      var s = new Stats();
      s.domElement.style.position = "absolute";
      s.domElement.style.left = "10px";
      s.domElement.style.top = "10px";
      return s;
    }
  }
}