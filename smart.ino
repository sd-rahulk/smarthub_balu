
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DHT.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define DHTPIN D4
#define DHTTYPE DHT11
#define GASPIN A0

DHT dht(DHTPIN, DHTTYPE);
LiquidCrystal_I2C lcd(0x27,16,2);

ESP8266WebServer server(80);

const char* ssid="SmartHub";
const char* password="12345678";

float temperature;
float humidity;
int gasValue;
String statusText;

const char webpage[] PROGMEM = R"=====(

<!DOCTYPE html>
<html>
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>Smart Hub</title>

<script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
<script src="https://cdnjs.cloudflare.com/ajax/libs/three.js/r134/three.min.js"></script>

<style>
body{margin:0;background:#050507;color:white;font-family:Segoe UI}
#canvas-container{position:fixed;top:0;left:0;z-index:-1}
#ui-wrapper{padding:20px;max-width:1000px;margin:auto}
.stats-grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(200px,1fr));gap:20px}
.card{background:rgba(255,255,255,0.05);padding:20px;border-radius:15px}
.value{font-size:30px;color:#00f2ff}
#status-banner{margin:20px 0;font-weight:bold}
</style>

</head>
<body>

<div id="canvas-container"></div>

<div id="ui-wrapper">
<h2>SYSTEM MONITOR</h2>

<div id="status-banner">INITIALIZING...</div>

<div class="stats-grid">
<div class="card">Temperature<div id="temp" class="value">--</div></div>
<div class="card">Humidity<div id="hum" class="value">--</div></div>
<div class="card">Gas<div id="gas" class="value">--</div></div>
</div>

<canvas id="chart"></canvas>

</div>

<script>

const scene=new THREE.Scene();
const camera=new THREE.PerspectiveCamera(75,window.innerWidth/window.innerHeight,0.1,1000);
const renderer=new THREE.WebGLRenderer({alpha:true});
renderer.setSize(window.innerWidth,window.innerHeight);
document.getElementById("canvas-container").appendChild(renderer.domElement);

const geometry=new THREE.IcosahedronGeometry(7,2);
const material=new THREE.MeshBasicMaterial({color:0x00f2ff,wireframe:true,opacity:0.2,transparent:true});
const mesh=new THREE.Mesh(geometry,material);
scene.add(mesh);

camera.position.z=15;

function animate(){
requestAnimationFrame(animate);
mesh.rotation.y+=0.002;
mesh.rotation.x+=0.001;
renderer.render(scene,camera);
}
animate();

const ctx=document.getElementById("chart").getContext("2d");

const chart=new Chart(ctx,{
type:"line",
data:{labels:[],datasets:[
{label:"Temp",borderColor:"#ff4d4d",data:[],tension:0.4},
{label:"Hum",borderColor:"#00f2ff",data:[],tension:0.4},
{label:"Gas",borderColor:"#ffaa33",data:[],tension:0.4}
]},
options:{responsive:true}
});

async function updateData(){

const res=await fetch("/data");
const data=await res.json();

document.getElementById("temp").innerText=data.temp+"°C";
document.getElementById("hum").innerText=data.hum+"%";
document.getElementById("gas").innerText=data.gas;

document.getElementById("status-banner").innerText="ENVIRONMENT STATUS: "+data.status;

const time=new Date().toLocaleTimeString();

chart.data.labels.push(time);

chart.data.datasets[0].data.push(data.temp);
chart.data.datasets[1].data.push(data.hum);
chart.data.datasets[2].data.push(data.gas);

if(chart.data.labels.length>20){
chart.data.labels.shift();
chart.data.datasets.forEach(ds=>ds.data.shift());
}

chart.update();

}

setInterval(updateData,2000);

</script>

</body>
</html>

)=====";


void handleRoot(){
server.send_P(200,"text/html",webpage);
}


void handleData(){

temperature=dht.readTemperature();
humidity=dht.readHumidity();
gasValue=analogRead(GASPIN);

if(gasValue < 200)
statusText="AWESOME";

else if(gasValue < 400)
statusText="NORMAL";

else if(gasValue < 700)
statusText="WARNING";

else
statusText="DANGER";

String json="{";
json+="\"temp\":"+String(temperature)+",";
json+="\"hum\":"+String(humidity)+",";
json+="\"gas\":"+String(gasValue)+",";
json+="\"status\":\""+statusText+"\"";
json+="}";

server.send(200,"application/json",json);

}


void setup(){

Serial.begin(115200);

dht.begin();

lcd.init();
lcd.backlight();

WiFi.softAP(ssid,password);

Serial.println("Access Point Started");
Serial.println(WiFi.softAPIP());

server.on("/",handleRoot);
server.on("/data",handleData);

server.begin();

}


void loop(){

temperature=dht.readTemperature();
humidity=dht.readHumidity();
gasValue=analogRead(GASPIN);

lcd.clear();

lcd.setCursor(0,0);
lcd.print("T:");
lcd.print(temperature);

lcd.setCursor(8,0);
lcd.print("H:");
lcd.print(humidity);

lcd.setCursor(0,1);
lcd.print("Gas:");
lcd.print(gasValue);

server.handleClient();

delay(2000);

}}
