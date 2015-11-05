/**
* FUNCTION NAME: getJoinAddress
*
* DESCRIPTION: Returns the Address of the coordinator
*/
Address MP1Node::getJoinAddress() {//tra ve 1 dia chi
Address joinaddr;
memset(&joinaddr, 0, sizeof(memberNode->addr.addr));
*(int *)(&joinaddr.addr) = 1;//gan ip 1.0.0.0
*(short *)(&joinaddr.addr[4]) = 0;// gan port 0
return joinaddr;
}
