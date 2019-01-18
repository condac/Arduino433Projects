#include "Node.h"

Node nodes[NUM_NODES];

int lastUsedNode = 0;

void setupNodes() {
  for (int i=0;i<NUM_NODES;i++) {
    nodes[i] = Node(0);
  }
}

byte parseId(char* inBuf) {
  return 123;
}

void handleBuffer(char* buf) {
 if (true) { // TODO check what id we are going to insert into
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
}
