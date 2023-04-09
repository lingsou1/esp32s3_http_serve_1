/*
接线说明:该程序应该烧录进源地的板子中去

程序说明:通过建立一个webServe服务器检查BOOT按键状态,根据按键状态向客户端请求发送信息,使得客户端的LED灯能够根据
         服务器端的按键状态来改变
         – 实时获取服务器端开发板上按钮引脚状态
         – 当有客户端向ESP32S3服务器的/update发送请求时，将服务器的按键引脚状态通过响应信息回复给客户端


注意事项:当该程序只运行一点时,有可能不是他自己的问题,可能是客户端出现问题了,导致服务器无数据可处理,不运行

函数示例:无

作者:灵首

时间:2023_3_26

*/
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <WebServer.h>

WebServer esp32s3_webServe(80);//实例化一个网页服务的对象
WiFiMulti wifi_multi;  //建立WiFiMulti 的对象,对象名称是 wifi_multi

//通过 ping ipconfig 方法实现找到以下设置参数
IPAddress local_IP(192, 168, 0, 123); // 设置ESP32s3-NodeMCU联网后的IP
IPAddress gateway(192, 168, 0, 1);    // 设置网关IP（通常网关IP是WiFI路由IP）
IPAddress subnet(255, 255, 255, 0);   // 设置子网掩码
IPAddress dns(192,168,0,1);           // 设置局域网DNS的IP（通常局域网DNS的IP是WiFI路由IP）
 
#define BOOT 0
static int bootValue;


/*
# brief 连接WiFi的函数
# param 无
# retval 无
*/
void wifi_multi_con(void){
  int i=0;
  while(wifi_multi.run() != WL_CONNECTED){
    delay(1000);
    i++;
    Serial.print(i);
  }

}


/*
# brief 写入自己要连接的WiFi名称及密码,之后会自动连接信号最强的WiFi
# param 无
# retval  无
*/
void wifi_multi_init(void){
  wifi_multi.addAP("LINGSOU123","12345678");
  wifi_multi.addAP("LINGSOU12","12345678");
  wifi_multi.addAP("LINGSOU1","12345678");
  wifi_multi.addAP("LINGSOU234","12345678");   //通过 wifi_multi.addAP() 添加了多个WiFi的信息,当连接时会在这些WiFi中自动搜索最强信号的WiFi连接
}


/*
# brief   处理服务器端的"/update"页面的请求操作,根据BOOT键的状态返回给客户端不同的HTML页面
# param    无
# retval  无
*/
void handleUpdate(){
  String bootPinState;
  
  if (bootValue ==  0){
    bootPinState = "0";
  }
  else{
    bootPinState = "1";
  }
  Serial.print("bootValue : ");
  Serial.print(bootValue);
  Serial.print("\n");
  //注意"buttonstate: "中有一个空格
  esp32s3_webServe.send(200,"text/html","buttonState: " + bootPinState);
}

/*
# brief  网络服务器的初始化,建立一个 "/update" 的页面并给出相应的处理函数
# param 无
# retval 无
*/
void esp32s3_webServe_init(){
  esp32s3_webServe.on("/update",handleUpdate);
  esp32s3_webServe.begin();
  Serial.print("HTTP Serve begin successfully!!!\n");
}


void setup() {
  // 启动串口通讯
  Serial.begin(9600);          
  Serial.println("");
  
  //设置引脚模式
  pinMode(BOOT,INPUT_PULLUP);


  //设置开发板的网络环境
  if(!WiFi.config(local_IP,gateway,subnet)){
    Serial.print("Failed to config the esp32s3 ip!!!");
  }

  //WiFi连接设置
  wifi_multi_init();//储存多个WiFi
  wifi_multi_con();//自动连接WiFi

  //输出连接信息(连接的WIFI名称及开发板的IP地址)
  Serial.print("\nconnect wifi:");
  Serial.print(WiFi.SSID());
  Serial.print("\n");
  Serial.print("\nIP address:");
  Serial.print(WiFi.localIP());
  Serial.print("\n");

  //处理服务器更新函数
  esp32s3_webServe_init();
}

void loop() {
  bootValue = digitalRead(BOOT);    //检查BOOT按键的状态,并及时处理
  esp32s3_webServe.handleClient();    //保证服务器一直在工作能接收到来自客户端的信息
}