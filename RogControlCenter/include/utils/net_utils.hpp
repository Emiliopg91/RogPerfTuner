#pragma once

class NetUtils {
  public:
	static bool isPortFree(int port);

	static int getRandomFreePort();
};