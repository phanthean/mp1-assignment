/**
* FUNCTION NAME: nodeLoopOps
*
* DESCRIPTION: Check if any node hasn't responded within a timeout period and then delete
* the nodes
* Propagate your membership list
*/
void MP1Node::nodeLoopOps() {
if (par->getcurrtime() > 3 && memberNode->memberList.size() > 1) {
memberNode->memberList.begin()->heartbeat++;
memberNode->memberList.begin()->timestamp = par->getcurrtime();
int pos = rand() % (memberNode->memberList.size() - 1) + 1;
MemberListEntry& member = memberNode->memberList[pos];
if (par->getcurrtime() - member.timestamp > TFAIL) {
// FIXME escolher outro
return;
}
Address memberAddr;
memcpy(&memberAddr.addr[0], &member.id, sizeof(int));
memcpy(&memberAddr.addr[4], &member.port, sizeof(short));
sendMemberList("HEARTBEATREQ", HEARTBEATREQ, &memberAddr);
}
}
