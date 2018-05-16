/* 
NodeMCU ESP32 Chart (Google)

Author : ChungYi Fu (Taiwan)  2018-05-16 22:00

Command Format :  
http://STAIP   (default:LineChart)
http://STAIP/?chartType=LineChart
http://STAIP/?chartType=AreaChart
http://STAIP/?showCount=value
http://STAIP/?timeInterval=value
http://STAIP/?chartWidth=value
http://STAIP/?chartHeight=value
http://STAIP/?xTitle=string
http://STAIP/?yTitle1=string
http://STAIP/?yTitle2=string
(It is necessary to load js from Google webside,so you must let your mobile connect to the internet.)

http://192.168.4.1/?resetwifi=ssid;password
*/

String chartType="LineChart";          //LineChart or AreaChart
String chartData="";                   //Data Format:  hh:mm:ss,data1,data2;
int showCount=10;                      //Data records
int timeInterval=5000;                 //Sensor time interval (ms)
int chartWidth=600;                    //Chart width (px)
int chartHeight=400;                   //Chart height (px)
String xTitle="Time";                  //Title of the X axis 
String yTitle1="Temperature(°F)";      //Title of the Y axis 
String yTitle2="Humidity(%)";          //Title of the Y axis 
unsigned long time1,time2;
int count=0;    
String yTitle="";  

#include <WiFi.h>

const char* ssid     = "";   //your network SSID
const char* password = "";   //your network password

const char* apssid = "ESP32 Chart";
const char* appassword = "12345678";         //AP password require at least 8 characters.

WiFiServer server(80);

String Feedback="", Command="",cmd="",str1="",str2="",str3="",str4="",str5="",str6="",str7="",str8="",str9="";
byte ReceiveState=0,cmdState=1,strState=1,questionstate=0,equalstate=0,semicolonstate=0;

void setup()
{
  Serial.begin(115200);
  delay(10);
  
  WiFi.mode(WIFI_AP_STA);
  
  //WiFi.config(IPAddress(192, 168, 201, 100), IPAddress(192, 168, 201, 2), IPAddress(255, 255, 255, 0));
  WiFi.begin(ssid, password);
  delay(1000);
  Serial.println("");
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  long int StartTime=millis();
  while (WiFi.status() != WL_CONNECTED) 
  {
      delay(500);
      if ((StartTime+5000) < millis()) break;
  } 
  Serial.println("");
  Serial.println("STAIP address: ");
  Serial.println(WiFi.localIP());

  
  if (WiFi.localIP().toString()!="0.0.0.0")
    WiFi.softAP((WiFi.localIP().toString()+"_"+(String)apssid).c_str(), appassword);
  else
    WiFi.softAP(apssid, appassword);
    
  //WiFi.softAPConfig(IPAddress(192, 168, 4, 1), IPAddress(192, 168, 4, 1), IPAddress(255, 255, 255, 0)); 
  Serial.println("");
  Serial.println("APIP address: ");
  Serial.println(WiFi.softAPIP());    
  server.begin(); 
  
  time1 = millis();
}

void loop()
{
  time2 = millis();
  if (time2>=(time1+timeInterval))
  {
    //Sensor Data
    int Temperature = rand()%300-100;    
    int Humidity = rand()%100; 
    yTitle=yTitle1+"="+String(Temperature)+"    "+yTitle2+"="+String(Humidity); 
    
    int t=time2/1000;
    t%=86400;
    chartData=chartData+String(t/3600)+":"+String((t%3600)/60)+":"+String(t%60)+","+Temperature+","+Humidity+";";
    time1 = time2;
    if (count==showCount)
      chartData=chartData.substring(chartData.indexOf(";")+1);
    else
      count++;
    //Serial.println(chartData);
  }
    
  Command="";cmd="";str1="";str2="";str3="";str4="";str5="";str6="";str7="";str8="";str9="";
  ReceiveState=0,cmdState=1,strState=1,questionstate=0,equalstate=0,semicolonstate=0;

  WiFiClient client = server.available();

  if (client) 
  { 
    String currentLine = "";

    while (client.connected()) 
    {
      if (client.available()) 
      {
        char c = client.read();             
        
        getCommand(c);
                
        if (c == '\n') 
        {
          if (currentLine.length() == 0) 
          {
            client.println("HTTP/1.1 200 OK");
            client.println("Access-Control-Allow-Headers: Origin, X-Requested-With, Content-Type, Accept");
            client.println("Access-Control-Allow-Methods: GET,POST,PUT,DELETE,OPTIONS");
            client.println("Content-Type: text/html; charset=utf-8");
            client.println("Access-Control-Allow-Origin: *");
            //client.println("Connection: close");
            client.println();
            client.println("<!DOCTYPE HTML>");
            client.println("<html><head>");
            client.println("<meta charset=\"UTF-8\">");
            client.println("<meta http-equiv=\"Access-Control-Allow-Origin\" content=\"*\">");
            client.println("<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">");
            client.println("<script type=\"text/javascript\" src=\"http://www.google.com/jsapi\"></script>");
            client.println("</head>");
            client.println("<script>");
            client.println("    google.load(\"visualization\", \"1\", { packages: [\"corechart\"] });");
            client.println("    google.setOnLoadCallback(drawChart);");
            client.println("    function drawChart() ");
            client.println("  {");
            client.println("    var data = new google.visualization.DataTable();");
            client.println("    data.addColumn('string', '"+xTitle+"');");
            client.println("    data.addColumn('number', '"+yTitle1+"');");
            client.println("    data.addColumn('number', '"+yTitle2+"');");
            client.println("    var getPara, ParaVal, ParaVal0, ParaVal1, ParaVal2;");
            client.println("    var getData = '"+chartData+"';");
            client.println("    if (getData.length>0)");
            client.println("    {");
            client.println("      var getPara = getData.split(\";\");");
            client.println("      data.addRows(getPara.length);");
            client.println("      for (i = 0; i < (getPara.length-1); i++) {");
            client.println("        ParaVal = getPara[i].split(\",\");");
            client.println("        data.setValue(i, 0, ParaVal[0]);");
            client.println("        data.setValue(i, 1, ParaVal[1]);");
            client.println("        data.setValue(i, 2, ParaVal[2]);");       
            client.println("      }");
            if (chartType=="AreaChart")
            {
              client.println("      //Stepped Area Chart");
              client.println("      var chart = new google.visualization.SteppedAreaChart(document.getElementById('myChart'));");
            }
            else
            {
              client.println("      //Line Chart");
              client.println("      var chart = new google.visualization.LineChart(document.getElementById('myChart'));");
            }
            client.println("      var options = {");
            client.println("            title: '"+yTitle+"',");
            client.println("            titleTextStyle:{");
            client.println("            color: 'red', ");
            client.println("            fontName: 'Times New Roman', ");
            client.println("            fontSize: 20, ");
            client.println("            bold: true,  ");
            client.println("            italic: false ");
            client.println("          },");
            client.println("        hAxis : { ");
            client.println("            textStyle : {fontSize: 14}");
            client.println("          },");
            client.println("        vAxis : { ");
            client.println("            textStyle : {fontSize: 18}");
            client.println("          },");
            client.println("            allowHtml: true,");
            client.println("            showRowNumber: true,");
            client.println("        width:'"+String(chartWidth)+"',");
            client.println("        height:'"+String(chartHeight)+"',");
            client.println("        legend: { position: 'bottom' }");
            client.println("           };");
            client.println("    chart.draw(data, options);");
            client.println("    }");
            client.println("  } ");
            client.println("</script>");
            client.println("<div id=\"myChart\"/>");
            client.println("<body onload=\"drawChart();setTimeout('location.reload();',"+String(timeInterval)+");\">");
            client.println("</body></html>");
            client.println();
                        
            Feedback="";
            break;
          } else {
            currentLine = "";
          }
        } 
        else if (c != '\r') 
        {
          currentLine += c;
        }

        if ((currentLine.indexOf("/?")!=-1)&&(currentLine.indexOf(" HTTP")!=-1))
        {
          currentLine="";
          Feedback="";
          ExecuteCommand();
        }
      }
    }
    delay(1);
    client.stop();
  } 
}

void ExecuteCommand()
{
  Serial.println("");
  //Serial.println("Command: "+Command);
  Serial.println("cmd= "+cmd+" ,str1= "+str1+" ,str2= "+str2+" ,str3= "+str3+" ,str4= "+str4+" ,str5= "+str5+" ,str6= "+str6+" ,str7= "+str7+" ,str8= "+str8+" ,str9= "+str9);
  Serial.println("");

  if (cmd=="chartType")
  {
    chartType=str1;
  }
  else if (cmd=="showCount")
  {
    if (str1.toInt()<showCount)
    {
      chartData="";
      count=0;
    }
    showCount=str1.toInt();
  }   
  else if (cmd=="timeInterval")
  {
    timeInterval=str1.toInt();
  }    
  else if (cmd=="chartWidth")
  {
    chartWidth=str1.toInt();
  }  
  else if (cmd=="chartHeight")
  {
    chartHeight=str1.toInt();
  }       
  else if (cmd=="xTitle")
  {
    xTitle=str1;
  }
  else if (cmd=="yTitle1")
  {
    yTitle1=str1;
  }    
  else if (cmd=="yTitle2")
  {
    yTitle2=str1;
  }   
  else if (cmd=="settings")
  {
    chartType=str1;
    chartData=str2;
    showCount=str3.toInt();
    timeInterval=str4.toInt();
    chartWidth=str5.toInt();
    chartHeight=str6.toInt();
    xTitle=str7; 
    yTitle1=str8; 
    yTitle2=str9;
  }
  else if (cmd=="resetwifi")
  {
    WiFi.begin(str1.c_str(), str2.c_str());
    Serial.print("Connecting to ");
    Serial.println(str1);
    long int StartTime=millis();
    while (WiFi.status() != WL_CONNECTED) 
    {
        delay(500);
        if ((StartTime+5000) < millis()) break;
    } 
    Serial.println("");
    Serial.println("STAIP: "+WiFi.localIP().toString());
    Feedback="STAIP: "+WiFi.localIP().toString();
  }   
  else 
  {
    Feedback="Command is not defined";
  }
}

void getCommand(char c)
{
  if (c=='?') ReceiveState=1;
  if ((c==' ')||(c=='\r')||(c=='\n')) ReceiveState=0;
  
  if (ReceiveState==1)
  {
    Command=Command+String(c);
    
    if (c=='=') cmdState=0;
    if (c==';') strState++;
  
    if ((cmdState==1)&&((c!='?')||(questionstate==1))) cmd=cmd+String(c);
    if ((cmdState==0)&&(strState==1)&&((c!='=')||(equalstate==1))) str1=str1+String(c);
    if ((cmdState==0)&&(strState==2)&&(c!=';')) str2=str2+String(c);
    if ((cmdState==0)&&(strState==3)&&(c!=';')) str3=str3+String(c);
    if ((cmdState==0)&&(strState==4)&&(c!=';')) str4=str4+String(c);
    if ((cmdState==0)&&(strState==5)&&(c!=';')) str5=str5+String(c);
    if ((cmdState==0)&&(strState==6)&&(c!=';')) str6=str6+String(c);
    if ((cmdState==0)&&(strState==7)&&(c!=';')) str7=str7+String(c);
    if ((cmdState==0)&&(strState==8)&&(c!=';')) str8=str8+String(c);
    if ((cmdState==0)&&(strState>=9)&&((c!=';')||(semicolonstate==1))) str9=str9+String(c);
    
    if (c=='?') questionstate=1;
    if (c=='=') equalstate=1;
    if ((strState>=9)&&(c==';')) semicolonstate=1;
  }
}
