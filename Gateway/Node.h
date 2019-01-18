
class Node {
  private:
    byte id;
    char lastMsg[BUF_SIZE];
    bool configured; // true if the Node is configured and usable
    long msgTime;
  public:
    Node(byte idIn);
    Node();
    byte getId();
    char* getLastMsg();
    void setMsg(char* inMsg);
    void activateNode(byte inId);
    bool isConfigured();
};

Node::Node(byte idIn) {
  // Constructor for a Node
  if (idIn != 0) {
    this->configured = true;
    this->id = idIn;
  } else {
    this->id = 0;
    this->configured = false;
  }

}
Node::Node() {
  // Constructor for a Node
  
    this->id = 0;
    this->configured = false;
  

}

void Node::activateNode(byte inId) {
  this->id = inId;
  this->configured = true;
}

bool Node::isConfigured() {
  return this->configured;
}
byte Node::getId() {
  return this->id;
}
char* Node::getLastMsg() {
  return this->lastMsg;
}

void Node::setMsg(char* inMsg) {
  //memcpy(prevMsg, lastMsg, BUF_SIZE);
  
  memcpy(lastMsg, inMsg, BUF_SIZE);
  this->msgTime = getTime();
}
