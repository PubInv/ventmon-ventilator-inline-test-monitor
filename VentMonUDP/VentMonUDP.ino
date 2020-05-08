#include <SPI.h>
#include <WiFi.h>
#include <Ethernet.h>

//#define UDP
#ifdef UDP
#include <EthernetUdp.h>
#include <Dns.h>
#endif

#ifndef htonl
#define htonl(x) ( ((x)<<24 & 0xFF000000UL) | \
                   ((x)<< 8 & 0x00FF0000UL) | \
                   ((x)>> 8 & 0x0000FF00UL) | \
                   ((x)>>24 & 0x000000FFUL) )
#endif

byte mac[6];
char macs[18];

#define PARAMHOST "ventmon.coslabs.com"
#ifdef UDP
#define PARAMPORT 6111
#else
#define PARAMPORT 6110
#endif

char *Loghost = strdup(PARAMHOST);
uint16_t Logport = PARAMPORT;
IPAddress LoghostAddr;

uint32_t LogWait = 1000 * 1;
uint32_t ParamWait = 1000 * 60 * 15;
uint16_t LoopWait = 0;

uint32_t LastLog = 0;
uint32_t LastParam = 0;
uint32_t LastCheck = 0;

uint64_t BoardTime = 0;
uint32_t LastMilli = 0;
uint16_t Offset = 0;

char *Tag;

EthernetClient tcpclient;
#ifdef UDP
EthernetUDP udpclient;
#endif

#define CSIZE 500

struct packet_t {
  char mtype;
  char stype;
  uint8_t loc;
  uint32_t ms;
  int32_t value;
} packets[CSIZE];

uint8_t cbuf_head = 0;
uint8_t cbuf_tail = 0;

void push_data(char mtype, char stype, uint8_t loc, uint32_t ms, int32_t value) {
  packets[cbuf_head].mtype = mtype;
  packets[cbuf_head].stype = stype;
  packets[cbuf_head].loc = loc;
  packets[cbuf_head].ms = ms;
  packets[cbuf_head].value = value;
  cbuf_head++;
  if (cbuf_head > CSIZE) cbuf_head = 0;
}

uint8_t pop_data() {
  while (cbuf_head != cbuf_tail) {
    send_data(packets[cbuf_tail].mtype, packets[cbuf_tail].stype, packets[cbuf_tail].loc,
	      packets[cbuf_tail].ms, packets[cbuf_tail].value);
    cbuf_tail++;
    if (cbuf_tail > CSIZE)
      cbuf_tail = 0;
  }
  return 1;
}

uint32_t
time() {
  uint32_t t = millis();
  if (t < LastMilli)
    BoardTime += 65536;
  LastMilli = t;
  return((BoardTime + LastMilli)/1000);
}

bool
parseparamline(char *line) {
  char *ptr = line;

  if (strncasecmp(line, "TIMESTAMP:", 10) == 0) {
    uint32_t s = atoi(line+10);
    LastMilli = millis();
    BoardTime = s - LastMilli;
    Serial.print(F("Set time to "));
    Serial.println(s);
    return true;
  }
  if (strcasecmp(line, "Connection: Closed") == 0) {
    return true; 
  }
  if (strncasecmp(line, "Content-type: ", 14) == 0) {
    return true;
  }
  if (strncasecmp(line, "Content-length: ", 16) == 0) {
    return true;
  }
  if (strncasecmp(line, "mac:", 4) == 0) {
    return true;
  }
  if (strncasecmp(line, "OFFSET:", 7) == 0) {
    uint32_t s = atoi(line+7);
    Offset = s;
    Serial.print(F("Set Offset to "));
    Serial.println(Offset);
    return true;
  }
  if (strncasecmp(line, "LOG_HOST:", 9) == 0) {
    if (Loghost) free(Loghost);
    char *ptr = line + 9;
    while (*ptr == ' ') ptr++;
    Loghost = (char *)malloc(strlen(ptr)+1);
    strcpy(Loghost, ptr);
    Serial.print(F("Set Loghost to "));
    Serial.println(Loghost);
    return true;
  }
  if (strncasecmp(line, "LOG_PORT:", 9) == 0) {
    uint16_t p = atoi(line+9);
    if (p > 0) Logport = p; 
    Serial.print(F("Set logport to "));
    Serial.println(Logport);
    return true;
  }
  if (strncasecmp(line, "PARAM_WAIT:", 11) == 0) {
    uint32_t s = atoi(line+11);
    s = s * 1000;
  if (s > 0) ParamWait = s; 
    Serial.print(F("Set Paramwait to "));
    Serial.println(ParamWait);
    return true;
  }
  if (strncasecmp(line, "LOOP_WAIT:", 10) == 0) {
    uint32_t s = atoi(line+10);
    if (s > 0) LoopWait = s; 
    Serial.print(F("Set LoopWait to "));
    Serial.println(LoopWait);
    return true;
  }
  if (strncasecmp(line, "LOG_WAIT:", 9) == 0) {
    uint32_t s = atoi(line+9);
    s = s * 1000;
    if (s > 0) LogWait = s; 
    Serial.print(F("Set LogWait to "));
    Serial.println(LogWait);
    return true;
  }
  if (strncasecmp(line, "TAG:", 4) == 0) {
    if (Tag) free(Tag);
    char *ptr = line + 4;
    while (*ptr == ' ') ptr++;
    Tag = (char *)malloc(strlen(ptr)+1);
    strcpy(Tag, ptr);
    Serial.print(F("Set Tag to "));
    Serial.println(Tag);
    return true;
  }
  return false;
}       

bool get_params() {
  Serial.println(F("getting parameters"));
  
  if (tcpclient.connect(PARAMHOST, PARAMPORT)) {
    Serial.print(F("connected to "));
    Serial.println(PARAMHOST);
    Serial.print(F("sending param request for mac "));
    Serial.println(macs);
    
    
    char MessageString[45];
    snprintf(MessageString, sizeof MessageString, "GET /%s/params HTTP/1.1\r\n\r\n", macs);
    if (tcpclient.print(MessageString) <= 0)
      return false;

    char line[100];
    uint16_t cused = 0;
    uint16_t cleft = sizeof line;
    while (tcpclient.connected()) {
      while(tcpclient.available()) {
        // if line over sized then ignore xxx not good
        if (cleft == 0) {
          cleft = 100;
          cused = 0;
        }
        line[cused] = tcpclient.read();
        if (line[cused] == '\n' || line[cused] == '\r') {
          if (cused > 0) {
            line[cused] = '\0';
            if (strlen(line)) {
              Serial.println(line);
              if (strcmp(line, "HTTP/1.1 404 OK") == 0) {
                Serial.println(F("No config for this device"));
              } else if (strcmp(line, "HTTP/1.1 200 OK") == 0) {
                Serial.println(F("got 200"));
              } else {
                if (!parseparamline(line)) {
                  Serial.print(F("Bad config line: "));
                  Serial.println(line);
                }
              }
            }
            cused = 0;
            cleft = 100;
          }
        } else {
          cused++;
          cleft--;
        }
      }
    }
    //    tcpclient.stop();  // don't close the connection
    line[cused] = '\0';
    Serial.println(line);
    parseparamline(line);
    return true;
  }
  return false;
}

void ethernet_setup() {
  Ethernet.init(33);  // ESP32 with Adafruit Featherwing Ethernet
  
  WiFi.macAddress(mac); // Get MAC address of wifi chip for ethernet address
  snprintf(macs, sizeof macs, "%02X:%02X:%02X:%02X:%02X:%02X",
    mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  Serial.print(F("Mac: "));
  Serial.print(macs);
  Serial.println();
  
  while (1) {
    // start the Ethernet connection:
    Serial.println(F("Initialize Ethernet with DHCP:"));
    if (Ethernet.begin(mac) == 0) {
      Serial.println(F("Failed to configure Ethernet using DHCP"));
      // Check for Ethernet hardware present
      if (Ethernet.hardwareStatus() == EthernetNoHardware) {
	Serial.println(F("Ethernet shield was not found.  Sorry, can't run without hardware. :("));
	while (1) {
	  delay(1); // do nothing, no point running without Ethernet hardware
	}
      }
      if (Ethernet.linkStatus() == LinkOFF) {
	Serial.println(F("Ethernet cable is not connected."));
	delay(5000);
      }
      delay(5000);
    } else {
      Serial.print(F("  DHCP assigned IP "));
      Serial.println(Ethernet.localIP());
      break;
    }
  }

  // give the Ethernet shield a second to initialize:
  delay(1000);

#ifdef UDP
  udpclient.begin(6111);
#endif
  
  DNSClient dns;
  dns.begin(Ethernet.dnsServerIP());
  if (dns.getHostByName(Loghost, LoghostAddr) == 1) {
    Serial.print("host is ");
    Serial.println(LoghostAddr);
  }

}

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  ethernet_setup();

  get_params();
}

#ifndef UDP
bool send_data(char mtype, char stype, uint8_t loc, uint32_t ms, int32_t value) {
  if (!tcpclient.connected() || !tcpclient.available()) {
    Serial.print(F("Connecting to "));
    Serial.print(Loghost);
    Serial.print(F("..."));
    
    if (!tcpclient.connect(Loghost, Logport)) {
      // if you didn't get a connection to the server:
      Serial.println(F("failed"));
      return false;
    }
  }
  
  Serial.print(F(" TCP Send to "));
  Serial.println(tcpclient.remoteIP());

  union {
    struct {
      char first;
      char mtype;
      char stype;
      uint8_t loc;
      uint32_t ms;
      int32_t value;
      char nl;
    } a;
    byte m[12];
  } message;
  uint8_t *mm = message.m;
  message.a.first = 'M';
  message.a.mtype = mtype;
  message.a.stype = stype;
  message.a.loc = 0 + loc;
  message.a.ms = htonl(ms);
  message.a.value = htonl(value);
  message.a.nl = '\n';
  tcpclient.write(message.m, 13);
  
  char line[10];
  uint16_t cused = 0;
  uint16_t cleft = sizeof line;
  while (tcpclient.connected()) {
    while(tcpclient.available()) {
      // if line over sized then ignore xxx not good
      if (cleft == 0) {
	cleft = 100;
	cused = 0;
      }
      line[cused] = tcpclient.read();
      if (line[cused] == '\n' || line[cused] == '\r') {
	if (cused > 0) {
	  line[cused] = '\0';
	  if (strncmp(line, "OK", 2) == 0) {
	    return true;
	  } else {
	    return false;
	  }
	}
	cused = 0;
	cleft = 100;
      } else {
	cused++;
	cleft--;
      }
    }
  }
  return false;
}
#else
bool send_data(char mtype, char stype, uint8_t loc, uint32_t ms, int32_t value) {
  union {
    struct {
      char first;
      char mtype;
      char stype;
      uint8_t loc;
      uint32_t ms;
      int32_t value;
      char nl;
    } a;
    byte m[13];
  } message;
  uint8_t *mm = message.m;
  message.a.first = 'M';
  message.a.mtype = mtype;
  message.a.stype = stype;
  message.a.loc = 0 + loc;
  message.a.ms = htonl(ms);
  message.a.value = htonl(value);
  message.a.nl = '\n';

  Serial.print(F(" UDP send to "));
  Serial.println(LoghostAddr);

  if (udpclient.beginPacket(LoghostAddr, Logport) != 1)
    return false;
  udpclient.write(message.m, 13);
  if (udpclient.endPacket() != 1)
    return false;

  return true;
}
#endif
    
bool Firstrun = true;
uint32_t LastDHCP = 0;
#define DHCPWAIT 5000
uint32_t lastcap = 0;
uint8_t cnt = 0;
uint8_t d = 2;

void loop() {
  if (millis() - lastcap > 25) {
    push_data('A', 'B', 0, d*25, millis() - lastcap);
    lastcap = millis();
  }

  if (d < 101) {
    if (cnt < 3) {
      if (Firstrun || millis() - LastLog >= d*25) {
	cnt++;
	Serial.print(F("send data "));
	Serial.println(cnt);
	pop_data();
	LastLog = millis();
	
      }
    } else {
      d++;
      cnt=0;
    }
  }

  if (millis() - LastDHCP >= DHCPWAIT) {
    Ethernet.maintain();
    LastDHCP = millis();
  }
  
  Firstrun = 0;
  delay(LoopWait);
}
