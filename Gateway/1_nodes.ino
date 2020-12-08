#include "Node.h"

Node nodes[NUM_NODES];

int lastUsedNode = 0;

typedef union {
 float number;
 uint8_t bytes[4];
} FLOATUNION_t;

typedef union {
 int number;
 uint8_t bytes[2];
} INTUNION_t;

typedef union {
 long number;
 uint8_t bytes[4];
} LONGUNION_t;

FLOATUNION_t fdata;
INTUNION_t idata;
LONGUNION_t ldata;


void setupNodes() {
  for (int i=0;i<NUM_NODES;i++) {
    nodes[i] = Node(0);
  }
}

byte parseId(char* inBuf) {
  byte out = inBuf[0];
  inBuf[0] = ' ';
  return out;
}
void fixEnd(char* inBuf) {
  for (int i=0;i<BUF_SIZE;i++) {
    if (inBuf[i] == '\n') {
      inBuf[i+1] = '\0';
    }
  }
  inBuf[BUF_SIZE-1] = '}';
  inBuf[BUF_SIZE] = '\0';
}

void handleBuffer(char* buf) {
  byte nrvar=buf[1];
 if (buf[1] == '{') { // if json format
    //byte foundId = parseId(buf);
    fixEnd(buf);
    addMsg(buf);
  }
  else if (nrvar>2 && nrvar < 6) {
    createJSONfromBinFormat(buf);
  }
}

void createJSONfromBinFormat(char* buf) {
  byte foundId = parseId(buf);
  
  byte nrElements = buf[1];
  String out = " {\"id\":"; // make sure first char is a space that we replace with id later
  out.concat(foundId);
  

  char element = 'a';
  byte x = 2;
  for (int i=0;i<nrElements;i++) {
    out += ",\"";
    out += element;
    out +="\":";
    element++;
    switch (buf[x]) {
      case 'l':    // long 4-bytes
        
        ldata.bytes[0] = buf[x+1];
        ldata.bytes[1] = buf[x+2];
        ldata.bytes[2] = buf[x+3];
        ldata.bytes[3] = buf[x+4];
        out.concat(ldata.number);
        
        x=x+5;
        break;
      case 'b':    // byte 1-byte
        {byte bbdata = (byte)buf[x+1];
        out.concat(bbdata);
        x=x+2;
        break;
        }
      case 'f':    // float 4-bytes
        fdata.bytes[0] = buf[x+1];
        fdata.bytes[1] = buf[x+2];
        fdata.bytes[2] = buf[x+3];
        fdata.bytes[3] = buf[x+4];
        out.concat(fdata.number);
        x=x+5;
        break;
      case 'i':    // int 2-bytes
        idata.bytes[0] = buf[x+1];
        idata.bytes[1] = buf[x+2];
        
        out.concat(idata.number);
        x=x+3;
        break;
    }
    
  }
  out += "}\n\0";
  static char *msg = out.c_str();
  msg[0] = foundId;
  addMsg(msg);
}
void addMsg(char* buf) {
  byte foundId = parseId(buf);
  bool foundNode = false;
  int foundNodeNr = 0;
  
  for (int i=0;i<NUM_NODES;i++) {
    if (nodes[i].getId() == foundId) {
      // Found node in list
      foundNode = true;
      foundNodeNr = i;
      
    }
  }
  if (!foundNode) {
    nodes[lastUsedNode].activateNode(foundId);
    foundNodeNr = lastUsedNode;
    lastUsedNode++;
  }
  nodes[foundNodeNr].setMsg(buf);
}
void debugCreate() {
  
  handleBuffer("a{\"id\":123,\"temp\":23.4,\"hum\":\"80%\"}");
}
void debugPrint() {
  for (int i=0;i<NUM_NODES;i++) {
        if (nodes[i].isConfigured()) {
          //client.println(nodes[i].getId());
          Serial.println(nodes[i].getLastMsg());
          Serial.println(",");
              
      }
    }
}
