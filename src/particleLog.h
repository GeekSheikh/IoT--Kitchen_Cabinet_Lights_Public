// get wifi ssid

// get wifi status

String getLocalIP(String args){
    return WiFi.localIP().toString();
}
// send udp (msg, port)
// conver this to logger class
UDP udp;
void sendLog(char output[], IPAddress remoteIP, unsigned int remotePort){
    udp.beginPacket(remoteIP, remotePort);
    udp.write(output);
    udp.endPacket();
}