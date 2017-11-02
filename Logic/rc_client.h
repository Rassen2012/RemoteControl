#ifndef RC_CLIENT_H
#define RC_CLIENT_H

void RCClient_Start(char *hname);
void RCClient_Stop();
int RCClient_ScaleImage2(int width, int height, int nwidth, int nheight, char *src, char *dst, int n);

#endif // RC_CLIENT_H

