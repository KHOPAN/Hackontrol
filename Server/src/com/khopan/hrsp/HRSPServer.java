package com.khopan.hrsp;

import java.net.ServerSocket;
import java.net.Socket;

public class HRSPServer {
	public static void main(String[] args) throws Throwable {
		ServerSocket server = new ServerSocket(42485);
		Socket socket = server.accept();
		server.close();
	}
}
