package com.khopan.hackontrol.hrsp;

import java.net.Socket;
import java.util.function.Consumer;

import net.dv8tion.jda.api.utils.messages.MessageCreateData;

public class HRSPClient {
	public HRSPClient(String domainName, int port, Consumer<MessageCreateData> consumer) throws Throwable {
		Socket socket = new Socket(domainName, port);
		consumer.accept(MessageCreateData.fromContent("**Connected**"));
		socket.close();
	}
}
