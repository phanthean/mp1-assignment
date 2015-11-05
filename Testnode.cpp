 **********************************/
#include "MP1Node.h"
+#include <sstream>
/*
* Note: You can change/add any functions in MP1Node.{h,cpp}
@@ -107,7 +108,7 @@ int MP1Node::initThisNode(Address *joinaddr) {
memberNode->heartbeat = 0;
memberNode->pingCounter = TFAIL;
memberNode->timeOutCounter = -1;
- initMemberListTable(memberNode);
+ initMemberListTable(memberNode, id, port);
return 0;
}
@@ -218,8 +219,91 @@ bool MP1Node::recvCallBack(void *env, char *data, int size ) {
/*
* Your code goes here
*/
+ assert(size >= sizeof(MessageHdr));
+
+ MessageHdr* msg = (MessageHdr*) data;
+ Address *src_addr = (Address*)(msg+1);
+
+ size -= sizeof(MessageHdr) + sizeof(Address) + 1;
+ data += sizeof(MessageHdr) + sizeof(Address) + 1;
+
+ switch (msg->msgType) {
+ case JOINREQ:
+ onJoin(src_addr, data, size);
+ onHeartbeat(src_addr, data, size);
+ break;
+ case PING:
+ onHeartbeat(src_addr, data, size);
+ break;
+ case JOINREP:
+ {
+ memberNode->inGroup = 1;
+ stringstream msg;
+ msg << "JOINREP from " << src_addr->getAddress();
+ msg << " data " << *(long*)(data );
+ log->LOG(&memberNode->addr, msg.str().c_str());
+ onHeartbeat(src_addr, data, size);
+ break;
+ }
+ default:
+ log->LOG(&memberNode->addr, "Received other msg");
+ break;
+ }
}
+Address AddressFromMLE(MemberListEntry* mle){
+ Address a;
+ memcpy(a.addr, &mle->id, sizeof(int));
+ memcpy(&a.addr[4], &mle->port, sizeof(short));
+ return a;
+}
+
+void MP1Node::onJoin(Address* addr, void* data, size_t size){
+ MessageHdr* msg;
+ size_t msgsize = sizeof(MessageHdr) + sizeof(memberNode->addr) + sizeof(long) + 1 ;
+ msg = (MessageHdr *) malloc(msgsize * sizeof(char));
+ msg->msgType = JOINREP;
+ memcpy((char *)(msg+1), &memberNode->addr, sizeof(memberNode->addr));
+ memcpy((char *)(msg+1) + sizeof(memberNode->addr) +1, &memberNode->heartbeat, sizeof(long));
+ stringstream ss;
+ ss<< "dang gui JOINREP den " << addr->getAddress() <<" heartbeat "<<memberNode->heartbeat;
+ log->LOG(&memberNode->addr, ss.str().c_str());
+ emulNet->ENsend(&memberNode->addr, addr, (char *)msg, msgsize);
+ free(msg);
+}
+
+void MP1Node::onHeartbeat(Address* addr, void* data, size_t size){
+ std::stringstream msg;
+ assert(size >= sizeof(long));
+ long *heartbeat = (long*)data;
+ bool newData = UpdateMemberList(addr, *heartbeat);
+ if(newData){
+ LogMemberList();
+ SendHBSomewhere(addr, *heartbeat);
+ }
+ else{
+
+ }
+}
+
+bool MP1Node::UpdateMemberList(Address *addr, long heartbeat){
+ vector<MemberListEntry>::iterator it;
+ for(it = memberNode->memberList.begin();it != memberNode->memberList.end();it++){
+ if((AddressFromMLE(&(*it)) == *addr) ==0){
+ if(heartbeat > it->getheartbeat()){
+ it->setheartbeat(heartbeat);
+ it->settimestamp(par->getcurrtime());
+ return true;
+ }else{
+ return false;
+ }
+ }
+ }
+ MemberListEntry mle(*((int*)addr->addr), *((short*)&(addr->addr[4])), heartbeat, par->getcurrtime());
+ memberNode->memberList.push_back(mle);
+ log->logNodeAdd(&memberNode->addr, addr);
+ return true;
+}
/**
* FUNCTION NAME: nodeLoopOps
*
@@ -232,8 +316,30 @@ void MP1Node::nodeLoopOps() {
/*
* Your code goes here
*/
+ int timeout = 5;
+ stringstream ss;
+ for(vector<MemberListEntry>::iterator it = memberNode->memberList.begin(); it != memberNode->memberList.end(); it++) {
+ if(par->getcurrtime() - it->timestamp > timeout){
+ Address addr = AddressFromMLE(&(*it));
+ ss << "Timing out" << addr.getAddress();
+ log->LOG(&memberNode->addr, ss.str().c_str());
+ ss.str("");
+
+ vector<MemberListEntry>::iterator next_it = it;
+ vector<MemberListEntry>::iterator next_next_it = it+1;
+ for(next_it = it; next_next_it != memberNode->memberList.end(); next_it++, next_next_it++){
+ *next_it = *next_next_it;
+ }
+ memberNode->memberList.resize(memberNode->memberList.size()-1);
+ it -=1;
+ LogMemberList();
+ log->logNodeRemove(&memberNode->addr, &addr);
+ }
+ }
+ UpdateMemberList(&memberNode->addr, ++memberNode->heartbeat);
+ SendHBSomewhere(&memberNode->addr, memberNode->heartbeat);
- return;
+ return;
}
/**
@@ -265,8 +371,48 @@ Address MP1Node::getJoinAddress() {
*
* DESCRIPTION: Initialize the membership list
*/
-void MP1Node::initMemberListTable(Member *memberNode) {
+void MP1Node::initMemberListTable(Member *memberNode, int id, short port) {
memberNode->memberList.clear();
+ MemberListEntry mle = MemberListEntry(id, port);
+ mle.settimestamp(par->getcurrtime());
+ mle.setheartbeat(memberNode->heartbeat);
+ memberNode->memberList.push_back(mle);
+}
+
+void MP1Node::LogMemberList(){
+ stringstream msg;
+ msg << "[";
+ for(vector<MemberListEntry>::iterator it = memberNode->memberList.begin(); it != memberNode->memberList.end(); it++){
+ msg << it->getid() << ": " << it->getheartbeat() << "(" << it->gettimestamp() << "), ";
+ }
+ msg << "]";
+}
+
+void MP1Node::SendHBSomewhere(Address *src_addr, long heartbeat){
+ int k=30;
+ double prob = k / (double)memberNode->memberList.size();
+
+ MessageHdr *msg;
+
+ size_t msgsize = sizeof(MessageHdr) + sizeof(src_addr->addr) + sizeof(long) + 1;
+ msg = (MessageHdr *) malloc(msgsize * sizeof(char));
+
+ msg->msgType = PING;
+ memcpy((char *)(msg+1), src_addr->addr, sizeof(src_addr->addr));
+ memcpy((char *)(msg+1) + sizeof(src_addr->addr) + 1, &heartbeat, sizeof(long));
+
+ for(vector<MemberListEntry>::iterator it = memberNode->memberList.begin(); it != memberNode->memberList.end(); it++) {
+ Address dst_addr = AddressFromMLE(&(*it));
+ if ((dst_addr == memberNode->addr) == 0 || ((dst_addr == *src_addr) ==0 )) {
+ continue;
+ }
+ if ((((double)(rand() % 100))/100) < prob) {
+ emulNet->ENsend(&memberNode->addr, &dst_addr, (char *)msg, msgsize);
+ }else{
+
+ }
+ }
+ free(msg);
}
/**
