/**
* FUNCTION NAME: initMemberListTable
*
* DESCRIPTION: Initialize the membership list
*/
void MP1Node::initMemberListTable(Member *memberNode) {//khoi tao nen table node.
memberNode->memberList.clear();
int id = *(int*)(&memberNode->addr.addr);
int port = *(short*)(&memberNode->addr.addr[4]);
MemberListEntry memberEntry(id, port, 0, par->getcurrtime());//id, port,heatbeat,time.
memberNode->memberList.push_back(memberEntry);
memberNode->myPos = memberNode->memberList.begin();
}
